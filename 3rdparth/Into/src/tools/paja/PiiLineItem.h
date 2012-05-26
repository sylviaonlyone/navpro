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

#ifndef _PIILINEITEM_H
#define _PIILINEITEM_H

#include <PiiItem.h>

class PiiSocketItem;
class PiiOperationItem;

class PiiLineItem : public PiiItem
{
  Q_OBJECT
  
public:
  PiiLineItem(const QPointF& position = QPointF(0,0), QGraphicsItem *parent = 0);

  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  /**
   * Returns the type of the item.
   */
  int type() const { return LineItem; }
  
  /**
   * Set an output item.
   */
  void setOutputItem(PiiSocketItem *input);
  PiiSocketItem* outputItem() const;
  
  /**
   * Set an input item.
   */
  void setInputItem(PiiSocketItem *input);
  PiiSocketItem* inputItem() const;
  
  void disconnectSockets();
  void connectSockets();
  
public slots:
  void updateBoundingRect();
  void highlight(bool para);

private:
  double findBestVerticalValue(double ox, double oy, double ix, double iy, const QList<QRectF>& boundingRects);
  bool intersects(const QLineF& line, const QRectF& rect);

  QRectF _boundingRect;
  QList<QPointF> _lstPoints;
  PiiSocketItem *_pOutputItem, *_pInputItem;
  PiiOperationItem *_pOutputParentItem, *_pInputParentItem;
};

#endif //_PIILINEITEM_H
