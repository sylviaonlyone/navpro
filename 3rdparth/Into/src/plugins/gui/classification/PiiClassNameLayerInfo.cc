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

#include "PiiClassNameLayerInfo.h"
#include "PiiEditorWidgetCombo.h"

PiiClassNameLayerInfo::PiiClassNameLayerInfo(const QStringList& labels) :
  _labels(labels)
{
  setMinimum(0);
  setMaximum(_labels.size()-1);
  setResolution(1);
  fillDefaultColors();
}

PiiClassNameLayerInfo::PiiClassNameLayerInfo(const QList<QColor>& colors, const QStringList& labels) :
  _colors(colors), _labels(labels)
{
  setMinimum(0);
  setMaximum(_labels.size()-1);
  setResolution(1);
  fillDefaultColors();
}

void PiiClassNameLayerInfo::fillDefaultColors()
{
  int ci =_labels.size()-_colors.size();
  int i = 0;
  while(_colors.size() < _labels.size())
    {
      QColor c;
      c.setHsv((int)(255.0 *(double)i/(double)ci), 255, 255 );
      _colors.append(c);
      i++;
    }
}

QWidget* PiiClassNameLayerInfo::editorWidget(int pointIndex, double current) const
{
  PiiComboBox* cb = createEditor(current);
  PiiEditorWidgetCombo* editorWidget = new PiiEditorWidgetCombo(cb,_iLayerIndex, pointIndex);
  QObject::connect(editorWidget, SIGNAL(valueChanged(double,int,int)), this, SIGNAL(valueChanged(double,int,int)));
  
  return editorWidget;
}

QWidget* PiiClassNameLayerInfo::editor() const
{
  PiiComboBox* lc = createEditor(_currentValue);

  QObject::connect(lc, SIGNAL(currentIndexChanged(int)), SLOT(assignEditorValue(int)));
  QObject::connect(this, SIGNAL(changeEditorValue(int)), lc, SLOT(setCurrentValue(int)));
  QObject::connect(this, SIGNAL(unselect()), lc, SLOT(unselected()));
  
  return lc;
}

PiiComboBox* PiiClassNameLayerInfo::createEditor(double current) const
{
  PiiComboBox* cb = new PiiComboBox();
  for(int i=0;i< _labels.size();i++)
    {
      QPixmap map(20,20);
      map.fill( _colors[i]);
      cb->insertItem(i,QIcon(map), _labels[i]);
    }
  cb->setCurrentIndex((int)(current + 0.5));
  
  return cb;
}

QMenu* PiiClassNameLayerInfo::menu() const
{
  QMenu* menu = new QMenu(_name);
  for(int i=0;i<_labels.size();i++)
    {
      QPixmap map(20,20);
      map.fill( _colors[i]);
      QAction *action = new QAction(QIcon(map),_labels[i],menu);
      action->setData(QVariant(i));
      connect(action, SIGNAL(triggered()), SLOT(menuActionPerformed()));
      menu->addAction(action);
      
      //menu->addAction(QIcon(map), _labels[i])->setData(QVariant(i));
      // Set the data to match index of layer
      // TRICK no5. add menu item a data (QSize), 
      // that actually contains layer index and value index
    }

  //connect(menu, SIGNAL(triggered(QAction*)), SLOT(menuActionPerformed(QAction*)));
  return menu;
}

QMenu* PiiClassNameLayerInfo::selectionMenu() const
{
  QMenu* menu = new QMenu(_name);
  for(int i=0;i<_labels.size();i++)
    {
      QPixmap map(20,20);
      map.fill( _colors[i]);
      QAction *action = new QAction(QIcon(map),_labels[i],menu);
      action->setData(QVariant(i));
      connect(action, SIGNAL(triggered()), SLOT(selectionActionPerformed()));
      menu->addAction(action);
    }

  return menu;
}

QIcon PiiClassNameLayerInfo::icon() const
{
  QImage im(30,30,QImage::Format_RGB32);
  int step = 30;
  if( _labels.size() != 0)
    step = 30 / (_labels.size());

  if (step < 1)
    step = 1;
  
  int ci = -1;
  for(int i=0;i<30;i++)
    {
      if(i%step == 0)
        ci++;
      if(ci>=_labels.size())
        ci = _labels.size()-1;

      QColor c = QColor(Qt::red);
      if ( ci >= 0 && ci < _colors.size() )
        c = _colors[ci];
      
      for(int j=0;j<30;j++)
        im.setPixel(i,j,qRgb(c.red(),c.green(),c.blue()));
    }
  QPixmap map = QPixmap::fromImage(im);
  return QIcon(map);
}

QBrush PiiClassNameLayerInfo::brush(int value) const
{
  if(value>= _colors.size())
    value = _colors.size()-1;
  if(value<0)
    value = 0;
  
  QColor tmp_color = _colors[value];
  tmp_color.setAlphaF( _transparency * tmp_color.alphaF());
  return  QBrush (tmp_color);
}

QBrush PiiClassNameLayerInfo::brush(double val) const
{
  int value = (int)val;
  if(value>= _colors.size())
    value = _colors.size()-1;
  if(value<0)
    value = 0;
  QColor tmpc = _colors[value];
  tmpc.setAlphaF(_transparency * tmpc.alphaF());

  return  QBrush (tmpc);
}

QPen PiiClassNameLayerInfo::pen(double val) const
{
  int value = (int)val;
  if(value>= _colors.size())
    value = _colors.size()-1;
  if(value<0)
    value = 0;
  QColor tmpc = _colors[value];
  return  QPen(tmpc,2);
}

//public slot
void PiiClassNameLayerInfo::setEditorValue(double value)
{
  //qDebug("PiiClassNameLayerInfo::setEditorValue(%f)", value);
  setCurrentValue(value);
  emit changeEditorValue((int)(value+0.5f));
}

void PiiClassNameLayerInfo::unselectEditor()
{
  //emit unselect();
}


//private slot
void PiiClassNameLayerInfo::assignEditorValue(int value)
{
  //qDebug("PiiClassNameLayerInfo::assignEditorValue(%i)", value);
  emit valueChanged((double)value, _iLayerIndex);
}


void PiiClassNameLayerInfo::menuActionPerformed()
{
  //qDebug("PiiClassNameLayerInfo::menuActionPerformed");
  emit valueChanged((double)(((QAction*)sender())->data().toInt()), _iLayerIndex);
}

void PiiClassNameLayerInfo::selectionActionPerformed()
{
  //qDebug("PiiClassNameLayerInfo::menuActionPerformed");
  emit selectionChanged((double)(((QAction*)sender())->data().toInt()), _iLayerIndex);
}

void PiiClassNameLayerInfo::showMenuWidget(double value)
{
  //int res = (int)(value + 0.5);
  emit valueChanged(value, _iLayerIndex);
}

