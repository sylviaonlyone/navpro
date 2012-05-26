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

#include "PiiSocketItem.h"
#include "PiiOperationItem.h"
#include "PiiGraphicsScene.h"

#include <QPainter>

PiiSocketItem::PiiSocketItem(PiiAbstractSocket *socket, PiiOperationItem *parent) : PiiItem(parent),
                                                                                    _pSocket(socket),
                                                                                    _iSocketIndex(0)
{
  setAcceptHoverEvents(true);
  setBrush(QBrush(Qt::NoBrush));
}

PiiSocketItem::~PiiSocketItem()
{
}

void PiiSocketItem::setSize(int size)
{
  prepareGeometryChange();
  _boundingRect = QRectF(-size/2, -size/2, size, size);
}

void PiiSocketItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(pen());
  painter->setBrush(brush());
  painter->drawEllipse(boundingRect().center(), 3,3);
}

void PiiSocketItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  QGraphicsItem::hoverEnterEvent(event);
  emit underMouse(true);  
}

void PiiSocketItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  QGraphicsItem::hoverLeaveEvent(event);
  emit underMouse(false);
  setPen(QPen(Qt::black));
}

QVariant PiiSocketItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemSceneChange && value.isValid())
    static_cast<PiiGraphicsScene*>(value.value<QGraphicsScene*>())->addPointer(_pSocket, this);

  return PiiItem::itemChange(change, value);
}

void PiiSocketItem::setSocketIndex(int index)
{
  _iSocketIndex = index;
}

int PiiSocketItem::socketIndex() const
{
  return _iSocketIndex;
}
