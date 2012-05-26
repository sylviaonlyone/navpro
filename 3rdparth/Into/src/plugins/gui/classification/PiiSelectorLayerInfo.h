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

#ifndef PIISELECTORLAYERINFO_H
#define PIISELECTORLAYERINFO_H

#include <QtGui>

/**
 * Base class for layer info.
 * Info class contains information related to visualizing layer values
 * and limits for layer values.
 */
class PiiSelectorLayerInfo : public QObject
{
  Q_OBJECT
public:
  PiiSelectorLayerInfo();  

  virtual QWidget* editorWidget(int pointIndex, double current) const = 0;
  virtual QWidget* editor() const = 0;
  virtual QMenu* menu() const = 0;
  virtual QIcon icon() const = 0;
  virtual QBrush brush(int value) const = 0;
  virtual QBrush brush(double value) const = 0;
  virtual QPen pen(double value) const = 0;
  virtual void setEditorValue(double value) = 0;
  virtual void showMenuWidget(double value) = 0;
  
  virtual QMenu* selectionMenu() const { return 0; }
  
  void setTransparency(double val);
  double transparency() const { return _transparency; }

  void setMinimum(double min) { _minimum = min; }
  double minimum() const { return _minimum; }
  
  void  setMaximum(double max) { _maximum = max; }
  double maximum() const { return _maximum; }
  
  void  setResolution(double res) { _resolution = res; }
  double resolution() const { return _resolution; }
  
  void  setLayerIndex(int value) { _iLayerIndex = value; }
  int layerIndex() const { return _iLayerIndex; }

  void setName(QString name) { _name = name; }
  QString name() { return _name; }

  virtual void setCurrentValue(double value) { _currentValue = value; }
  double currentValue() { return _currentValue; }
  
signals:
  void valueChanged(double value, int layer);
  void valueChanged(double value, int layer, int pointIndex);
  void changeEditorValue(int value);
  void selectionChanged(double value, int layer);
  
public slots:
  virtual void unselectEditor() = 0;

protected:
  
  double _resolution;
  double _minimum;
  double _maximum;
  double _transparency;
  double _currentValue;
  
  int _iLayerIndex;
  QString _name;
};
#endif
