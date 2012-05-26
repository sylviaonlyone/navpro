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

#ifndef _PUSHBUTTON_H
#define _PUSHBUTTON_H

#include <QWidget>
#include <QWidget>
#include <QPushButton>
#include <QPixmap>
#include <QPaintEvent>

#include "DemoUtils.h"

class PII_DEMOUTILS_EXPORT PushButton : public QPushButton
{
  Q_OBJECT
  
public:
  enum IconMode { IconLeft, IconRight };
  
  PushButton(QWidget *parent = 0);
  
  /**
   * Set the place of the icon.
   */
  void setIconMode(IconMode mode);
  
protected:
  void paintEvent(QPaintEvent *event);

private:
  QPixmap _pixmapLeft, _pixmapRight;
  QString _strName;
  IconMode _mode;
};


#endif //_PUSHBUTTON_H
