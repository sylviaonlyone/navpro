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

#ifndef _PIIGRAPHICSSCENE_H
#define _PIIGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QList>

#include <PiiEngine.h>

class PiiItem;
class PiiLineItem;
class PiiDottedLineItem;

class PiiGraphicsScene : public QGraphicsScene
{
  Q_OBJECT

public:
  /**
   * Constructor.
   */
  PiiGraphicsScene(QObject *parent = 0);
  ~PiiGraphicsScene();

  bool isConnecting() const { return _bConnecting; }

  void addPointer(void *key, PiiItem *value);
  void removePointer(void *key);

  void setEngine(PiiEngine *engine);
  PiiEngine* engine() { return _pEngine; }
  
  struct OperationNames
  {
    QString pluginName;
    QStringList operationNames;
  };
  
public slots:
  void operationItemLocationChanged(const QRectF&);
  
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

private slots:
  void addOperation();
  
private:
  void updateOperationConnections();
  /**
   * Remove line item where input or output socket is same as the
   * given socketItem.
   */
  void removeLineItem(QGraphicsItem *socketItem);

  PiiLineItem *_pConnectingLineItem;
  PiiDottedLineItem *_pDottedLineItem;
  bool _bConnecting;

  QList<void*> _lstPointers;
  QList<PiiItem*> _lstItemPointers;

  PiiEngine *_pEngine;
  QPointF _clickedMenuPos;

  QList<OperationNames> _lstAvailableOperationNames;

};

#endif //_PIIGRAPHICSSCENE_H
