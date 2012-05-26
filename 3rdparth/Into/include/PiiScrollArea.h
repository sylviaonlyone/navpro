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

#ifndef _PIISCROLLAREA_H
#define _PIISCROLLAREA_H

#include <QtGui>

/**
 * Scroll area with aspect ratio
 **/
class PiiScrollArea : public QScrollArea
{
  Q_OBJECT
  
public:  
  PiiScrollArea ( QWidget * parent = 0, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio );
  
  void setAspectRatioMode(Qt::AspectRatioMode aspectMode);
  
  Qt::AspectRatioMode aspectRatioMode() const;

  virtual void resizeEvent ( QResizeEvent * event );
  
  
protected:
  void keyPressEvent(QKeyEvent *e);
  Qt::AspectRatioMode _aspectMode;
  bool _fitToView;

signals:
 void pageUpPressed();
 void pageDownPressed();

private:
  void resizeWidget(QSize size);
};

#endif // _PIISCROLLAREA
