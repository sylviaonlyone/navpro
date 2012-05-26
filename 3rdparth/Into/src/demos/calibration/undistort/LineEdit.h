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

#ifndef _KIDELINEEDIT_H
#define _KIDELINEEDIT_H

#include <QLineEdit>
#include <QMouseEvent>

class LineEdit : public QLineEdit
{
  Q_OBJECT
  
public:
  LineEdit(QWidget *parent = 0);

signals:
  void clicked();

protected:
  void mousePressEvent(QMouseEvent *e);
};
  

#endif //_KIDELINEEDIT_H
