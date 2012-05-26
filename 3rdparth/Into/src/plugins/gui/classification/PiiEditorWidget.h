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

#ifndef _PIIEDITORWIDGET_H
#define _PIIEDITORWIDGET_H

#include <QtGui>

class PiiEditorWidget : public QWidget
{
  Q_OBJECT

public:
  PiiEditorWidget(int layer = -1, int pointIndex = 0, QWidget *parent = 0);
  void setPointIndex(int pointIndex) { _iPointIndex = pointIndex; }
  void setLayer(int layer) { _iLayer = layer; }
  
signals:
  void valueChanged(double value, int layer, int pointIndex);

protected:
  int _iLayer, _iPointIndex;
};

#endif //_PIIEDITORWIDGET_H
