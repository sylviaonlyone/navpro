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

#ifndef _FRAME_H
#define _FRAME_H

#include <QApplication>
#include <QtGui>
#include <QLayout>
#include <QStyle>
#include <TitleBar.h>
#include <ContentWidget.h>
#include <ui_frame.h>

class Frame : public QFrame, private Ui_Frame
{
public:
    
  Frame();

  // Allows you to access the content area of the frame
  // where widgets and layouts can be added
  QWidget *contentWidget() const;
  void mousePressEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void leaveEvent(QEvent *e);
  
private:
  QPoint _oldPos;
  bool _bMouseDown;
  bool _bLeft, _bRight, _bBottom;
};


#endif //_FRAME_H
