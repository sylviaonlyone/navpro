/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "PiiGraphicsScene.h"
#include <QtDebug>
#include <QGraphicsItem>
#include <QMenu>

#include <PiiItem.h>
#include <PiiLineItem.h>
#include <PiiSocketItem.h>
#include <PiiDottedLineItem.h>
#include <PiiOperationItem.h>
#include <PiiInputSocketItem.h>
#include <PiiOutputSocketItem.h>
#include <PiiPajaUtil.h>
#include <PiiResourceDatabase.h>

PiiGraphicsScene::PiiGraphicsScene(QObject *parent) :
  QGraphicsScene(parent),
  _pConnectingLineItem(0),
  _bConnecting(false),
  _pEngine(0)
{
  addItem(_pDottedLineItem = new PiiDottedLineItem);
  PiiResourceDatabase *db = PiiYdin::resourceDatabase();
  QStringList lstPluginResourceNames = PiiEngine::pluginResourceNames();
  lstPluginResourceNames.sort();
  
  for (int i=0; i<lstPluginResourceNames.size(); i++)
    {
      QStringList lstOperationNames = db->select(Pii::subject, Pii::attribute("pii:class") == "PiiOperation" || Pii::attribute("pii:class") == "PiiOperationCompound") &&
        db->select(Pii::subject, Pii::attribute("pii:parent") == lstPluginResourceNames[i]);
      lstOperationNames.sort();

      OperationNames names;
      names.pluginName = lstPluginResourceNames[i].remove("Pii").remove("Plugin");
      names.operationNames = lstOperationNames;
      _lstAvailableOperationNames << names;
    }
}

PiiGraphicsScene::~PiiGraphicsScene()
{
  clear();
}

void PiiGraphicsScene::setEngine(PiiEngine *engine)
{
  clear();
  addItem(_pDottedLineItem = new PiiDottedLineItem);
  
  _pEngine = engine;

  QList<QList<PiiOperation*> > lstOperations = PiiPajaUtil::checkOrder(engine->childOperations());
  QList<QList<QPointF> > lstPositions = PiiPajaUtil::checkPositions(lstOperations);
  
  for (int i=0; i<lstOperations.size(); i++)
    {
      QList<PiiOperation*> lstOps = lstOperations[i];
      for (int k=0; k<lstOps.size(); k++)
        {
          PiiOperationItem *pItem = new PiiOperationItem(lstOps[k]);
          pItem->setPos(QPointF(lstPositions[i][k].x() * 300, lstPositions[i][k].y() * 150));
          addItem(pItem);
        }
    }
      
  updateOperationConnections();
  operationItemLocationChanged(sceneRect());
}

void PiiGraphicsScene::updateOperationConnections()
{
  for (int i=0; i<_lstItemPointers.size(); i++)
    {
      if (_lstItemPointers[i]->type() == PiiItem::InputSocketItem)
        {
          PiiAbstractInputSocket *pInputSocket = static_cast<PiiAbstractInputSocket*>(_lstPointers[i]);
          PiiAbstractOutputSocket *pOutputSocket = pInputSocket->connectedOutput();
          if (pOutputSocket != 0)
            {
              int index = _lstPointers.indexOf(static_cast<void*>(pOutputSocket));
              if (index >= 0)
                {
                  PiiLineItem *pItem = new PiiLineItem;
                  pItem->setOutputItem(static_cast<PiiOutputSocketItem*>(_lstItemPointers[index]));
                  pItem->setInputItem(static_cast<PiiInputSocketItem*>(_lstItemPointers[i]));
                  addItem(pItem);
                }
            }
        }
    }
}

void PiiGraphicsScene::addPointer(void *key, PiiItem *value)
{
  int index = _lstPointers.indexOf(key);
  if (index >= 0)
    {
      _lstPointers[index] = key;
      _lstItemPointers[index] = value;
    }
  else
    {
      _lstPointers << key;
      _lstItemPointers << value;
    }
  
}

void PiiGraphicsScene::removePointer(void *key)
{
  int index = _lstPointers.indexOf(key);
  if (index >= 0)
    {
      _lstPointers.removeAt(index);
      QGraphicsItem *pItem = _lstItemPointers.takeAt(index);

      // If removed item type is output or input, we will delete all line
      // items where depends on this item
      if (pItem->type() == PiiItem::OutputSocketItem ||
          pItem->type() == PiiItem::InputSocketItem)
        {
          removeLineItem(pItem);
        }
      else if (pItem->type() == PiiItem::OperationItem)
        {
          _pEngine->removeOperation(static_cast<PiiOperation*>(key));
        }
      
    }
}

void PiiGraphicsScene::removeLineItem(QGraphicsItem *socketItem)
{
  QList<QGraphicsItem*> lstItems = items();
  for (int i=lstItems.size(); i--;)
    {
      if (lstItems[i]->type() == PiiItem::LineItem)
        {
          PiiLineItem *pLineItem = static_cast<PiiLineItem*>(lstItems[i]);
          if (pLineItem->outputItem() == socketItem ||
              pLineItem->inputItem() == socketItem)
            {
              pLineItem->disconnectSockets();
              delete pLineItem;
            }
        }
    }
}


void PiiGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsScene::mousePressEvent(event);

  QGraphicsItem *pItem = itemAt(event->scenePos());
  
  if (event->button() == Qt::LeftButton)
    {
      if (pItem != 0 && !_bConnecting && pItem->type() == PiiItem::OutputSocketItem)
        {
          // Create a new PiiLineItem
          _pConnectingLineItem = new PiiLineItem(event->scenePos());
          _pConnectingLineItem->setOutputItem(static_cast<PiiSocketItem*>(pItem));
          addItem(_pConnectingLineItem);
          _bConnecting = true;
          
          // Update dotted line
          _pDottedLineItem->setPoint1(event->scenePos());
          _pDottedLineItem->setPoint2(event->scenePos());
          _pDottedLineItem->setVisible(true);
        }
      else if (pItem != 0 && _bConnecting && pItem->type() == PiiItem::InputSocketItem)
        {
          removeLineItem(pItem);
          
          // Finish line
          _pConnectingLineItem->setInputItem(static_cast<PiiSocketItem*>(pItem));
          _bConnecting = false;
          
          // Hide dotted line
          _pDottedLineItem->setVisible(false);
          _pConnectingLineItem = 0;
        }
      else if (_bConnecting)
        {
          // Don't make connection
          delete _pConnectingLineItem;
          _pConnectingLineItem = 0;
          _bConnecting = false;
          
          // Hide dotted line
          _pDottedLineItem->setVisible(false);
        }
    }
  else if (pItem == 0 && event->button() == Qt::RightButton)
    {
      _clickedMenuPos = event->scenePos();
      QMenu menu;
      menu.setSeparatorsCollapsible(false);

      QAction *pSeparator = menu.addSeparator();
      pSeparator->setText(tr("Add operation"));
      
      for (int i=0; i<_lstAvailableOperationNames.size(); i++)
        {
          QMenu *pMenu = menu.addMenu(_lstAvailableOperationNames[i].pluginName);
          QStringList lstOperationNames = _lstAvailableOperationNames[i].operationNames;
          if (lstOperationNames.isEmpty())
            {
              QAction *pEmptyAction = pMenu->addAction(tr("Empty"));
              pEmptyAction->setEnabled(false);
            }
          
          for (int k=0; k<lstOperationNames.size(); k++)
            {
              QAction *pAction = pMenu->addAction(lstOperationNames[k], this, SLOT(addOperation()));
              pAction->setData(lstOperationNames[k]);
            }
        }
      menu.exec(event->screenPos());
    }
}

void PiiGraphicsScene::addOperation()
{
  QAction *pAction = qobject_cast<QAction*>(sender());
  if (pAction != 0 && _pEngine != 0)
    {
      PiiOperationItem *pItem = new PiiOperationItem(_pEngine->createOperation(pAction->_d().toString()));
      pItem->setPos(_clickedMenuPos);
      addItem(pItem);
    }
}

void PiiGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsScene::mouseMoveEvent(event);

  // If we are in connecting state, we will show a dotted line and
  // change its state depends on mouse location
  if (_bConnecting)
    {
      _pDottedLineItem->setPoint2(event->scenePos());

      QGraphicsItem *pItem = itemAt(event->scenePos());
      if (pItem != 0)
        {
          if (pItem->type() == PiiItem::OperationItem ||
              pItem->type() == PiiItem::OutputSocketItem)
            _pDottedLineItem->setState(PiiDottedLineItem::CannotConnect);
          else if (pItem->type() == PiiItem::InputSocketItem)
            _pDottedLineItem->setState(PiiDottedLineItem::CanConnect);
          else
            _pDottedLineItem->setState(PiiDottedLineItem::Normal);
        }
      else
        _pDottedLineItem->setState(PiiDottedLineItem::Normal);
    }
}

void PiiGraphicsScene::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape && _bConnecting)
    {
      // Don't make connection
      delete _pConnectingLineItem;
      _pConnectingLineItem = 0;
      _bConnecting = false;
      
      // Hide dotted line
      _pDottedLineItem->setVisible(false);
    }
}

void PiiGraphicsScene::operationItemLocationChanged(const QRectF& rect)
{
  QList<QGraphicsItem*> lstItems = items();
  //QRectF adjustedRect = rect.adjusted(-10,-10,10,10);
  
  for (int i=0; i<lstItems.size(); i++)
    {
      if (lstItems[i]->type() == PiiItem::LineItem) // && adjustedRect.intersects(lstItems[i]->sceneBoundingRect()))
        static_cast<PiiLineItem*>(lstItems[i])->updateBoundingRect();
    }
}
