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

#ifndef _PIIFLOATLAYERINFOEDITOR_H
#define _PIIFLOATLAYERINFOEDITOR_H

#include <QWidget>
#include <QSlider>
#include <QLabel>

class PiiFloatLayerInfoEditor : public QWidget
{
  Q_OBJECT
  
public:
  PiiFloatLayerInfoEditor(QWidget *parent = 0);

  void setRange(double min, double max, double res);
  
public slots:
  void setValue(int value) { _pSlider->setValue(value); }

private slots:
  void assignEditorValue(int value);

signals:
  void valueChanged(double value);

private:
  QLabel *_pLabelMin, *_pLabelMax;
  QSlider *_pSlider;
  int _iDecimals;
  double _dMin, _dMax, _dResolution;
};

#endif //_PIIFLOATLAYERINFOEDITOR_H
