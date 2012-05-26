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

#ifndef _PIIOPERATIONITEM_H
#define _PIIOPERATIONITEM_H

#include <PiiItem.h>
#include <QRectF>

#include <PiiOperation.h>

class PiiInputSocketItem;
class PiiOutputSocketItem;

class PiiOperationItem : public PiiItem
{
  Q_OBJECT
  
public:
  PiiOperationItem(PiiOperation *operation, QGraphicsItem *parent = 0);
  ~PiiOperationItem();

  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  /**
   * Returns the type of the item.
   */
  int type() const { return OperationItem; }
  
  /**
   * Set an operation.
   */
  void setOperation(PiiOperation *operation);

  /**
   * Return input count.
   */
  int inputCount() const;

  /**
   * Return output count.
   */
  int outputCount() const;
  
signals:
  void positionChanged(const QRectF&);
  
protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value);
  void mousePressEvent(QGraphicsSceneMouseEvent* event);
  
private:
  void addInputs(const QStringList& inputNames);
  void addOutputs(const QStringList& outputNames);

  int _iNameHeight;
  PiiOperation *_pOperation;
  QRectF _boundingRect;
  QString _strClassName;
  QStringList _lstInputNames, _lstOutputNames;
  QList<PiiInputSocketItem*> _lstInputs;
  QList<PiiOutputSocketItem*> _lstOutputs;
};

#endif //_PIIOPERATIONITEM_H
