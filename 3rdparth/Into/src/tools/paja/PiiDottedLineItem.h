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

#ifndef _PIIDOTTEDLINEITEM_H
#define _PIIDOTTEDLINEITEM_H

#include <QGraphicsLineItem>
#include <QPen>
#include <QLineF>

class PiiDottedLineItem : public QGraphicsLineItem
{
public:
  PiiDottedLineItem(QGraphicsItem *parent = 0);
  ~PiiDottedLineItem();
  
  enum DottedLineState { Normal, CannotConnect, CanConnect };

  void setState(DottedLineState state);

  void setPoint1(const QPointF& point1);
  void setPoint2(const QPointF& point2);
  
private:
  QLineF _line;
  QPen _normalPen, _redPen, _greenPen;
};

#endif //_PIIDOTTEDLINEITEM_H
