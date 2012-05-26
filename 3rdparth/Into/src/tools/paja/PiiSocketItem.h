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

#ifndef _PIISOCKETITEM_H
#define _PIISOCKETITEM_H

#include <PiiItem.h>

#include <PiiAbstractSocket.h>

class PiiOperationItem;

class PiiSocketItem : public PiiItem
{
  Q_OBJECT
  
public:
  PiiSocketItem(PiiAbstractSocket *socket, PiiOperationItem *parent);
  ~PiiSocketItem();
  
  /**
   * Set size of the socket.
   */
  void setSize(int size);

  QRectF boundingRect() const { return _boundingRect; }

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  /**
   * Set and get the index of the socket.
   */
  void setSocketIndex(int index);
  int socketIndex() const;

  PiiAbstractSocket* socket() const { return _pSocket; }
  
signals:
  void underMouse(bool);
  
protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
  QVariant itemChange(GraphicsItemChange change, const QVariant& value);

private:
  PiiAbstractSocket *_pSocket;
  QRectF _boundingRect;
  int _iSocketIndex;
 
};

#endif //_PIISOCKETITEM_H
