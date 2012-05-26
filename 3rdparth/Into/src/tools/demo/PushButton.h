/* This file is part of Into demo.
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
#include <QPushButton>
#include <QPixmap>
#include <QPaintEvent>

class PushButton : public QPushButton
{
  Q_OBJECT
  
public:
  PushButton(QWidget *parent = 0);
  
  /**
   * Set a pixmap.
   */
  void setPixmap(const QPixmap& pixmap);

  /**
   * Set a name of the button.
   */
  void setName(const QString& name);
  
  /**
   * Set and get a directory name.
   */
  void setDirname(const QString& dirname);
  QString dirname() const;
  
protected:
  void paintEvent(QPaintEvent *event);

private:
  QPixmap _pixmap;
  QString _strName, _strDirname;
};


#endif //_PUSHBUTTON_H
