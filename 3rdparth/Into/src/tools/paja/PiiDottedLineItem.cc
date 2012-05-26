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

#include "PiiDottedLineItem.h"

PiiDottedLineItem::PiiDottedLineItem(QGraphicsItem *parent) : QGraphicsLineItem(parent)
{
  setLine(0,0,0,0);
  _greenPen = QPen(Qt::DotLine);
  _greenPen.setColor(Qt::green);

  _normalPen = QPen(Qt::DotLine);
  
  _redPen = QPen(Qt::DotLine);
  _redPen.setColor(Qt::red);

  setPen(_normalPen);
  setVisible(false);
  setZValue(-1);
}

PiiDottedLineItem::~PiiDottedLineItem()
{
}


void PiiDottedLineItem::setState(PiiDottedLineItem::DottedLineState state)
{
  switch(state)
    {
    case Normal: setPen(_normalPen); break;
    case CannotConnect: setPen(_redPen); break;
    case CanConnect: setPen(_greenPen); break;
    }
}

void PiiDottedLineItem::setPoint1(const QPointF& point1)
{
  _line.setP1(point1);
  setLine(_line);
}

void PiiDottedLineItem::setPoint2(const QPointF& point2)
{
  _line.setP2(point2);
  setLine(_line);
}

