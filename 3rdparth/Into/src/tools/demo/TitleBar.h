/* This file is part of Into demo.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * Created: Mon Mar 15 2010 by Lasse Raiha
 *
 * $Revision:$
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

#ifndef _TITLEBAR_H
#define _TITLEBAR_H

#include <QApplication>
#include <QtGui>
#include <QLayout>
#include <QStyle>
#include <ContentWidget.h>
#include <ui_titlebar.h>

class TitleBar : public QWidget, private Ui_TitleBar
{
  Q_OBJECT
  
public:
  TitleBar(QWidget *parent);

public slots:
  void showSmall();
  void showMaxRestore();
  
protected:
  void mousePressEvent(QMouseEvent *me);
  void mouseMoveEvent(QMouseEvent *me);
  
private:
  QPixmap _restorePixmap, _maxPixmap;
  bool _bMaxNormal;
  QPoint _startPos;
  QPoint _clickPos;
};

#endif //_TITLEBAR_H
