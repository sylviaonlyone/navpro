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

#include "PiiFloatLayerInfo.h"
#include "PiiEditorWidgetDoubleSpin.h"
#include "PiiDoubleSpinBox.h"
#include <PiiFloatLayerInfoEditor.h>

PiiFloatLayerInfo::PiiFloatLayerInfo(QColor color1)
{
  _color1 = color1;
  _color1.setAlphaF(0.01f);
  _color2 = _color1.toRgb();
  _color2.setAlphaF(1.0f);
}

PiiFloatLayerInfo::PiiFloatLayerInfo(QColor color1, QColor color2)
{
  _color1 = color1;
  _color2 = color2;
}

QWidget* PiiFloatLayerInfo::editorWidget(int pointIndex, double current) const
{
  QDoubleSpinBox* spinner = new QDoubleSpinBox();
  spinner->setRange(minimum(), maximum());
  spinner->setSingleStep(resolution());
  spinner->setAlignment(Qt::AlignRight);
  
  QString dec = QString("%1").arg(resolution());
  int i = dec.indexOf(".");
  int deci = 0;
  if(i!=-1)
    deci = dec.size()-i-1;
  
  spinner->setDecimals(deci);
  spinner->setValue(current);

  PiiEditorWidgetDoubleSpin* editorWidget = new PiiEditorWidgetDoubleSpin(spinner, _iLayerIndex, pointIndex);
  QObject::connect(editorWidget, SIGNAL(valueChanged(double,int,int)), this, SIGNAL(valueChanged(double,int,int)));
  
  return editorWidget;
}

QWidget* PiiFloatLayerInfo::editor() const
{
  PiiFloatLayerInfoEditor *pEditor = new PiiFloatLayerInfoEditor;
  pEditor->setRange(minimum(), maximum(), resolution());
  
  QObject::connect(pEditor,SIGNAL(valueChanged(double)), SLOT(assignEditorValue(double)));
  QObject::connect(this, SIGNAL(changeEditorValue(int)), pEditor, SLOT(setValue(int)));
  return pEditor;
}

QMenu* PiiFloatLayerInfo::menu() const
{
  QMenu* menu = new QMenu(_name);
  
  //Set data a main index. When action is received, editor for given layer is 
  //Created
  QAction *action = new QAction("Set Value...",menu);
  menu->addAction(action);
  connect(action, SIGNAL(triggered()), SLOT(menuActionPerformed()));
  
  return menu;
}

QIcon PiiFloatLayerInfo::icon() const
{
  QImage im(30,30,QImage::Format_RGB32);
  for(int i=0;i<30;i++)
    {
      QColor c = calculateGradientColor( (double)i/(double)15);
      for(int j=0;j<30;j++)
        {
          im.setPixel(i,j,qRgb(c.red(),c.green(),c.blue()));
        }
    }
  QPixmap map = QPixmap::fromImage(im);
  return QIcon(map);
}

QBrush PiiFloatLayerInfo::brush(int value) const
{
  return brush((double)value);
}

QBrush PiiFloatLayerInfo::brush(double value) const
{
  return QBrush(calculateGradientColor(value));
}

QPen PiiFloatLayerInfo::pen(double value) const
{
  QColor tmp_color = calculateGradientColor(value);
  tmp_color.setAlphaF(1.0f);
  return QPen(tmp_color, 2);
}


QColor PiiFloatLayerInfo::calculateGradientColor(double value) const
{
  QColor tmp_color;
  if(value<_minimum)
    value = _minimum;
  if(value>_maximum)
    value = _maximum;
  
  // Relative value (between 0 and 1)
  double rval = (value-_minimum) / (_maximum - _minimum);
  
  // NOTE: use here integer values to rgb, using double fails!
  tmp_color.setRgb( (int)( (1.0f-rval)*_color1.red()  +rval*_color2.red()),
                    (int)( (1.0f-rval)*_color1.green()+rval*_color2.green()),
                    (int)( (1.0f-rval)*_color1.blue() +rval*_color2.blue()));
  
  tmp_color.setAlphaF(_transparency * ((1.0f-rval)*_color1.alphaF()+rval*_color2.alphaF()));    
  return tmp_color;
}

void PiiFloatLayerInfo::setEditorValue(double value)
{
  if(value < minimum())
    value = minimum();
  
  if(value > maximum())
    value = maximum();

  setCurrentValue(value);
  
  double resol = resolution(); 
  
  if(resol > 0.0001f)
    resol = 0.0001f;
  
  double val =  (value - minimum()) / resolution();
  int ret = (int)( (val/resol+0.5f)*resol);

  emit changeEditorValue(ret);
}


void PiiFloatLayerInfo::assignEditorValue(double value)
{
  emit valueChanged(value, _iLayerIndex);
}

/**
 * method for overcome lacks on QInputDialog.
 * - uses setSingleStep instead of giving decimals like in original file.
 * - has a parameter for a coordinate where to put dialog.
 **/
double doubleDialog(	const QString &title, const QString &label,
                        double value, double minValue, double maxValue,
                        double step, bool *okpressed)
{
  QDialog* q = new QDialog();
  QVBoxLayout *vbox = new QVBoxLayout(q);
  
  QLabel* l = new QLabel(label, q);
  vbox->addWidget(l);
  vbox->addStretch(1);
  PiiDoubleSpinBox *input = new PiiDoubleSpinBox(q);
  input->setAlignment(Qt::AlignRight);
  
  vbox->addWidget(input);
  vbox->addStretch(1);
  
  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout(hbox, Qt::AlignRight);
  
  QPushButton *ok;
  ok = new QPushButton(QInputDialog::tr("OK"), q);
  ok->setDefault(true);
  QPushButton *cancel = new QPushButton(QInputDialog::tr("Cancel"), q);
  
  QSize bs = ok->sizeHint().expandedTo(cancel->sizeHint());
  ok->setFixedSize(bs);
  cancel->setFixedSize(bs);
  
  hbox->addStretch();
  
  // Order of buttons in "MAC" style.
  hbox->addWidget(cancel);
  hbox->addWidget(ok);
  
  QObject::connect(ok, SIGNAL(clicked()), q, SLOT(accept()));
  QObject::connect(cancel, SIGNAL(clicked()), q, SLOT(reject()));
  
  q->resize(q->sizeHint());
  
  input->setRange(minValue, maxValue);
  
  input->setSingleStep(step);
  
  QString dec = QString("%1").arg(step);
  int i = dec.indexOf(".");
  int deci = 0;
  if(i!=-1) deci = dec.size()-i-1;
  
  input->setDecimals(deci);
  
  q->setWindowTitle(title);

  input->setValue(value);
  
  bool accepted = (q->exec() == QDialog::Accepted);
  if (okpressed)
    *okpressed = accepted;
  return input->value();
}

void PiiFloatLayerInfo::menuActionPerformed()
{
  showMenuWidget(_currentValue);
}

void PiiFloatLayerInfo::showMenuWidget(double value)
{
  bool ok;
  QString rangestring = QString("Range is (%1 - %2)").arg(minimum()).arg(maximum());
  
  double d = doubleDialog(_name,
                             QString("Enter value for current cell. ") + rangestring,
                             value,
                             minimum(),
                             maximum(),
                             resolution(),
                             &ok);
  
  if(ok)
    emit valueChanged((double)d, _iLayerIndex);
}

void PiiFloatLayerInfo::unselectEditor()
{
}


