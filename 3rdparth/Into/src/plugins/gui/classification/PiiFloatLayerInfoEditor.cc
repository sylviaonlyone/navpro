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

#include "PiiFloatLayerInfoEditor.h"
#include <QHBoxLayout>
#include <QToolTip>

PiiFloatLayerInfoEditor::PiiFloatLayerInfoEditor(QWidget *parent) : QWidget(parent),
                                                                    _pLabelMin(0),
                                                                    _pLabelMax(0),
                                                                    _pSlider(0),
                                                                    _iDecimals(0),
                                                                    _dMin(0.0),
                                                                    _dMax(1.0),
                                                                    _dResolution(0.1)
{
  _pLabelMin = new QLabel;
  _pLabelMax = new QLabel;
  _pSlider = new QSlider(Qt::Horizontal);

  connect(_pSlider, SIGNAL(valueChanged(int)), this, SLOT(assignEditorValue(int)));
  
  QHBoxLayout *pLayout = new QHBoxLayout;
  pLayout->addWidget(_pLabelMin);
  pLayout->addWidget(_pSlider);
  pLayout->addWidget(_pLabelMax);

  setLayout(pLayout);
}

void PiiFloatLayerInfoEditor::setRange(double min, double max, double res)
{
  _dMin = min;
  _dMax = max;
  _dResolution = res;
  
  _iDecimals = 0;
  double dStartValue = 1.0;
  for (int i=1; i<4; i++)
    {
      if (_dResolution < dStartValue)
        {
          _iDecimals = i;
          dStartValue /= 10;
        }
      else
        break;
    }

  _pLabelMin->setText(QString::number(_dMin,'f',_iDecimals));
  _pLabelMax->setText(QString::number(_dMax,'f',_iDecimals));
  
  _pSlider->setRange(0, (int)((_dMax-_dMin)/_dResolution + 0.5));
  _pSlider->setSliderPosition(0);
}

void PiiFloatLayerInfoEditor::assignEditorValue(int value)
{
  //update tooltip
  double val = _dMin + (double)value * _dResolution;
  
  QString toolTipText = QString::number(val,'f',_iDecimals);
  _pSlider->setToolTip(toolTipText);
  int x = _pSlider->width() * _pSlider->value() / _pSlider->maximum();
  QToolTip::showText(_pSlider->mapToGlobal(QPoint(x,-30)), toolTipText, this);
  
  emit valueChanged(val);
}
