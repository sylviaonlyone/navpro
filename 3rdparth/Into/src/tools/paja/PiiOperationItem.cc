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

#include "PiiOperationItem.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QMenu>

#include <PiiGraphicsScene.h>
#include <PiiInputSocketItem.h>
#include <PiiOutputSocketItem.h>

PiiOperationItem::PiiOperationItem(PiiOperation *operation, QGraphicsItem *parent) : PiiItem(parent),
                                                                                     _pOperation(0)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  
  setOperation(operation);
}

PiiOperationItem::~PiiOperationItem()
{
  if (scene())
    static_cast<PiiGraphicsScene*>(scene())->removePointer(_pOperation);
}

QRectF PiiOperationItem::boundingRect() const
{
  return _boundingRect;
}

void PiiOperationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  painter->setFont(font());
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->drawRoundedRect(_boundingRect,6,6);
  
  // Draw classname
  painter->drawLine(_boundingRect.left(), _boundingRect.y()+_iNameHeight, _boundingRect.right(), _boundingRect.y()+_iNameHeight);
  painter->drawText(QRectF(_boundingRect.x(), _boundingRect.y(), _boundingRect.width(), _iNameHeight),
                    Qt::AlignHCenter | Qt::AlignVCenter, _strClassName);

  int iTextCap = 5;
  double dStartY = _boundingRect.y() + _iNameHeight;

  // Draw input names
  for (int i=0; i<_lstInputNames.size(); i++)
    {
      painter->drawText(QRectF(_boundingRect.x()+iTextCap, dStartY+i*_iNameHeight,
                               _boundingRect.width()-2*iTextCap, _iNameHeight),
                        Qt::AlignLeft | Qt::AlignVCenter, _lstInputNames[i]);
    }

  // Draw output names
  for (int i=0; i<_lstOutputNames.size(); i++)
    {
      painter->drawText(QRectF(_boundingRect.x()+iTextCap, dStartY+i*_iNameHeight,
                               _boundingRect.width()-2*iTextCap, _iNameHeight),
                        Qt::AlignRight | Qt::AlignVCenter, _lstOutputNames[i]);
    }

}

void PiiOperationItem::setOperation(PiiOperation *operation)
{
  if (scene())
    static_cast<PiiGraphicsScene*>(scene())->removePointer(_pOperation);

  _pOperation = operation;

  int iHeight = 50;
  int iWidth = 100;
  int iTextCap = 5;
  int iTextVerticalCap = 2;
  
  QFontMetrics fm(font());
  
  if (operation)
    {
      if (scene())
        static_cast<PiiGraphicsScene*>(scene())->addPointer(operation, this);
  
      _strClassName = _pOperation->metaObject()->className();
      _lstInputNames = _pOperation->inputNames();
      _lstOutputNames = _pOperation->outputNames();

      int iClassNameWidth = fm.width(_strClassName);

      int iMaxInputWidth = 0;
      for (int i=0; i<_lstInputNames.size(); i++)
        {
          int width = fm.width(_lstInputNames[i]);
          if (width > iMaxInputWidth)
            iMaxInputWidth = width;
        }

      int iMaxOutputWidth = 0;
      for (int i=0; i<_lstOutputNames.size(); i++)
        {
          int width = fm.width(_lstOutputNames[i]);
          if (width > iMaxInputWidth)
            iMaxOutputWidth = width;
        }

      iWidth = qMax(iClassNameWidth+2*iTextCap, iMaxInputWidth+iMaxOutputWidth+3*iTextCap);
    }
  else
    {
      _strClassName = "Unknown";
      iWidth = fm.width(_strClassName);
      
      _lstInputNames = QStringList();
      _lstOutputNames = QStringList();
    }
  
  _iNameHeight = fm.height() + 2*iTextVerticalCap;

  iHeight = (qMax(qMax(_lstInputNames.size(),_lstOutputNames.size()),1) + 1) * _iNameHeight;
  prepareGeometryChange();
  _boundingRect = QRectF(-iWidth/2, -iHeight/2, iWidth, iHeight);

  addInputs(_lstInputNames);
  addOutputs(_lstOutputNames);
}

void PiiOperationItem::addInputs(const QStringList& inputNames)
{
  double x = _boundingRect.left();
  double y = _boundingRect.y() + 1.5*_iNameHeight;
  
  for (int i=0; i<inputNames.size(); i++, y+=_iNameHeight)
    {
      PiiInputSocketItem *pItem = new PiiInputSocketItem(_pOperation->input(inputNames[i]), this);
      pItem->setSize(_iNameHeight);
      pItem->setPos(QPointF(x,y));
      pItem->setSocketIndex(i);
      _lstInputs << pItem;
    }
}

void PiiOperationItem::addOutputs(const QStringList& outputNames)
{
  double x = _boundingRect.right();
  double y = _boundingRect.y() + 1.5*_iNameHeight;

  for (int i=0; i<outputNames.size(); i++, y+=_iNameHeight)
    {
      PiiOutputSocketItem *pItem = new PiiOutputSocketItem(_pOperation->output(outputNames[i]), this);
      pItem->setSize(_iNameHeight);
      pItem->setPos(QPointF(x,y));
      pItem->setSocketIndex(i);
      _lstOutputs << pItem;
    }
}

QVariant PiiOperationItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemSceneChange && value.isValid())
    {
      PiiGraphicsScene *pScene = static_cast<PiiGraphicsScene*>(value.value<QGraphicsScene*>());
      pScene->addPointer(_pOperation, this);
      connect(this, SIGNAL(positionChanged(const QRectF&)), pScene, SLOT(operationItemLocationChanged(const QRectF&)));
    }
  else if (change == ItemPositionHasChanged && scene())
    {
      emit positionChanged(sceneBoundingRect());
    }

  return PiiItem::itemChange(change, value);
}

int PiiOperationItem::inputCount() const
{
  return _lstInputs.size();
}

int PiiOperationItem::outputCount() const
{
  return _lstOutputs.size();
}

void PiiOperationItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  if (event->button() == Qt::RightButton)
    {
      QMenu menu;
      menu.addAction(tr("Remove '%1'").arg(_strClassName), this, SLOT(deleteLater()));
      menu.exec(event->screenPos());
    }
}

