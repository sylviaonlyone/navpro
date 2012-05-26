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

#include "PiiLineItem.h"
#include <PiiSocketItem.h>
#include <PiiOperationItem.h>

#include <QPainter>
#include <QtDebug>
#include <QGraphicsScene>

PiiLineItem::PiiLineItem(const QPointF& position, QGraphicsItem *parent) : PiiItem(parent),
                                                                           _pOutputItem(0),
                                                                           _pInputItem(0),
                                                                           _pOutputParentItem(0),
                                                                           _pInputParentItem(0)
{
  setPos(position);
  setZValue(-1);
}


QRectF PiiLineItem::boundingRect() const
{
  return _boundingRect;
}

void PiiLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  painter->setPen(pen());
  painter->setBrush(brush());
  painter->setRenderHint(QPainter::Antialiasing, true);

  if (_lstPoints.size() > 0)
    {
      for (int i=0; i<_lstPoints.size()-1; i++)
        painter->drawLine(_lstPoints[i], _lstPoints[i+1]);
    }
}

void PiiLineItem::highlight(bool para)
{
  setPen(para ? QPen(Qt::green) : QPen(Qt::black));
}

bool PiiLineItem::intersects(const QLineF& line, const QRectF& rect)
{
  if (rect.top()-10 < line.y1() &&
      rect.bottom()+10 > line.y1() &&
      rect.left()-10 > line.x1() &&
      rect.right()+10 < line.x2())
    return true;

  return false;
}

double PiiLineItem::findBestVerticalValue(double ox, double oy, double ix, double iy, const QList<QRectF>& boundingRects)
{
  /**
   * First we make vertical lines.
   */
  QList<QLineF> lstLines;
  for (int i=0; i<boundingRects.size(); i++)
    {
      QRectF rect = boundingRects[i];
      if (rect.right()+10 > ox &&
          rect.left()-10 < ix)
        lstLines << QLineF(0,boundingRects[i].top(), 0, boundingRects[i].bottom());
    }

  double middleY = (oy+iy)/2;

  // If there is no any operation items in the way, we can return
  // middle point
  if (lstLines.isEmpty())
    return middleY;
  
  // Find min and max and test if middle point is free
  double minY = lstLines.first().y1();
  double maxY = lstLines.first().y2();
  bool bMiddleFree = true;
  
  for (int i=0; i<lstLines.size(); i++)
    {
      if (lstLines[i].y1() < minY)
        minY = lstLines[i].y1();
      if (lstLines[i].y2() > maxY)
        maxY = lstLines[i].y2();

      if (lstLines[i].y1() < middleY && middleY < lstLines[i].y2())
        bMiddleFree = false;
    }

  if (bMiddleFree)
    return middleY;
  
  // Combine lines
  QList<QLineF> lstCombinedLines;
  while (!lstLines.isEmpty())
    {
      QLineF currentLine = lstLines.takeLast();
      bool bContinue = true;
      while(bContinue)
        {
          bContinue = false;
          for (int i=lstLines.size(); i--;)
            {
              if ((lstLines[i].y1() > currentLine.y1() && lstLines[i].y1() < currentLine.y2()) ||
                  (currentLine.y1() > lstLines[i].y1() && currentLine.y1() < lstLines[i].y2()))
                {
                  currentLine.setP1(QPointF(0,qMin(lstLines[i].y1(), currentLine.y1())));
                  currentLine.setP2(QPointF(0,qMax(lstLines[i].y2(), currentLine.y2())));
                  lstLines.takeAt(i);
                  bContinue = true;
                }
            }
        }

      lstCombinedLines << currentLine;
    }

  // Order and collect possible y-values
  QList<QLineF> lstOrderedCombinedLines;
  while (!lstCombinedLines.isEmpty())
    {
      double min = lstCombinedLines.first().y1();
      int index = 0;
      for (int i=0; i<lstCombinedLines.size(); i++)
        {
          if (lstCombinedLines[i].y1() < min)
            {
              index = i;
              min = lstCombinedLines[i].y1();
            }
        }
      lstOrderedCombinedLines << lstCombinedLines.takeAt(index);
    }
  
  QList<double> lstPossibleYValues;
  lstPossibleYValues << minY - 10;
  for (int i=1; i<lstOrderedCombinedLines.size(); i++)
    lstPossibleYValues << (lstOrderedCombinedLines[i-1].y2() + lstOrderedCombinedLines[i].y1()) / 2;
  lstPossibleYValues << maxY + 10;

  // Calculate the shortest way
  double dTemp = (oy+iy)/2;
  double dDistance = qAbs(lstPossibleYValues[0] - dTemp);
  int index = 0;
  
  for (int i=0; i<lstPossibleYValues.size(); i++)
    {
      double dist = qAbs(lstPossibleYValues[i] - dTemp);
      if (dist < dDistance)
        {
          dDistance = dist;
          index = i;
        }
    }

  return lstPossibleYValues[index];
}

void PiiLineItem::updateBoundingRect()
{
  _lstPoints.clear();
  
  if (_pOutputItem != 0 && _pInputItem != 0)
    {
      QPointF outputPos = mapFromScene(_pOutputItem->scenePos());
      QPointF inputPos = mapFromScene(_pInputItem->scenePos());

      QPointF outputParentPos = mapFromScene(_pOutputParentItem->scenePos());
      QPointF inputParentPos = mapFromScene(_pInputParentItem->scenePos());
      
      double dBaseCap = 20;
      double dOutputCap = _pOutputItem->socketIndex() * 5;
      double dInputCap = _pInputItem->socketIndex() * 5;
      if (outputPos.x()+dBaseCap*2 < inputPos.x())
        {
          if (outputPos.y() < inputPos.y())
            dOutputCap = (_pOutputParentItem->outputCount() - _pOutputItem->socketIndex() + 1) * 5;

          // Find the best y-value
          QList<QRectF> lstItemBoundingRects;
          if (scene())
            {
              QList<QGraphicsItem*> lstItems = scene()->items();
              for (int i=lstItems.size(); i--;)
                if (lstItems[i]->type() == PiiItem::OperationItem)
                  lstItemBoundingRects << mapRectFromScene(lstItems[i]->sceneBoundingRect());
            }
          
          double outputX = outputPos.x() + dBaseCap + dOutputCap;
          double outputY = outputPos.y();
          double inputX = inputPos.x() - dBaseCap - dInputCap;
          double inputY = inputPos.y();

          double bestY = findBestVerticalValue(outputX, outputY, inputX, inputY, lstItemBoundingRects);

          _lstPoints << outputPos;
          _lstPoints << QPointF(outputX, outputY);
          _lstPoints << QPointF(outputX, bestY);
          _lstPoints << QPointF(inputX, bestY);
          _lstPoints << QPointF(inputX, inputY);
          _lstPoints << inputPos;
        }
      else
        {
          double y = (outputParentPos.y() + inputParentPos.y()) / 2;
          
          if (outputPos.y() < inputPos.y())
            {
              dOutputCap = (_pOutputParentItem->outputCount() - _pOutputItem->socketIndex() + 1) * 5;
              y += dOutputCap;
            }
          else
            {
              dInputCap = (_pInputParentItem->inputCount() - _pInputItem->socketIndex() + 1) * 5;
              y -= dOutputCap;
            }
          
          if (_pOutputParentItem == _pInputParentItem)
            {
              QRectF outputParentRect = mapRectFromScene(_pOutputParentItem->sceneBoundingRect());
              double height = outputParentRect.height() / 2 + dBaseCap + dOutputCap;
              if (outputPos.y() < outputParentPos.y())
                y = outputParentPos.y() - height;
              else
                y = outputParentPos.y() + height;
            }
          _lstPoints << outputPos;
          _lstPoints << QPointF(outputPos.x() + dBaseCap + dOutputCap, outputPos.y());
          _lstPoints << QPointF(outputPos.x() + dBaseCap + dOutputCap, y);
          _lstPoints << QPointF(inputPos.x()-(dBaseCap+dInputCap), y);
          _lstPoints << QPointF(inputPos.x()-(dBaseCap+dInputCap), inputPos.y());
          _lstPoints << inputPos;
        }
    }

  double dMinX, dMaxX, dMinY, dMaxY;
  dMinX = dMaxX = _lstPoints.size() > 0 ? _lstPoints[0].x() : 0;
  dMinY = dMaxY = _lstPoints.size() > 0 ? _lstPoints[0].y() : 0;
  
  for (int i=0; i<_lstPoints.size(); i++)
    {
      double x = _lstPoints[i].x();
      double y = _lstPoints[i].y();
      if (x < dMinX) dMinX = x;
      if (x > dMaxX) dMaxX = x;
      if (y < dMinY) dMinY = y;
      if (y > dMaxY) dMaxY = y;
    }
  
  prepareGeometryChange();
  _boundingRect = QRectF(QPointF(dMinX, dMinY), QPointF(dMaxX, dMaxY));
}

void PiiLineItem::setOutputItem(PiiSocketItem *output)
{
  disconnectSockets();
  
  _pOutputItem = output;
  connectSockets();
  
  _pOutputParentItem = static_cast<PiiOperationItem*>(_pOutputItem->parentItem());
  connect(_pOutputItem, SIGNAL(underMouse(bool)), this, SLOT(highlight(bool)));
  
  updateBoundingRect();
}

PiiSocketItem* PiiLineItem::outputItem() const
{
  return _pOutputItem;
}

void PiiLineItem::disconnectSockets()
{
  if (_pInputItem)
    {
      PiiAbstractInputSocket *pInput = _pInputItem->socket()->socket()->asInput();
      pInput->disconnectOutput();
    }
}

void PiiLineItem::connectSockets()
{
  PiiAbstractOutputSocket *pOutput = _pOutputItem == 0 ? 0 : _pOutputItem->socket()->socket()->asOutput();
  PiiAbstractInputSocket *pInput = _pInputItem == 0 ? 0 : _pInputItem->socket()->socket()->asInput();
  
  if (pInput != 0 && pOutput != 0)
    pInput->connectOutput(pOutput);
}

void PiiLineItem::setInputItem(PiiSocketItem *input)
{
  disconnectSockets();
  
  _pInputItem = input;
  connectSockets();

  _pInputParentItem = static_cast<PiiOperationItem*>(_pInputItem->parentItem());
  
  connect(_pInputItem, SIGNAL(underMouse(bool)), this, SLOT(highlight(bool)));
  
  updateBoundingRect();
}

PiiSocketItem* PiiLineItem::inputItem() const
{
  return _pInputItem;
}
