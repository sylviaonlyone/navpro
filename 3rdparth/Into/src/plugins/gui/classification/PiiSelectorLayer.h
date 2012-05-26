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

#ifndef PIISELECTORLAYER_H
#define PIISELECTORLAYER_H

#include "PiiSelectorLayerInfo.h"

/**
 * Class for layer in selector.
 **/
class PiiSelectorLayer : public QObject
{
     Q_OBJECT

public:
  PiiSelectorLayer();
  ~PiiSelectorLayer();
  
  /**
   * Override this method on different kind of layers
   **/
  virtual double valueAt(int x, int y) const = 0;
  virtual QVector<double> values() const = 0;
  
  void setInfo(PiiSelectorLayerInfo* _info);
  PiiSelectorLayerInfo* info() const { return _pInfo;}

  void setEnabled(bool enabled) { _bEnabled = enabled; }
  bool isEnabled() { return _bEnabled; }
  
protected: 
  PiiSelectorLayerInfo* _pInfo;
  bool _bEnabled;
  
public slots:

  virtual void setLimits(QSize val) = 0;
  virtual void setValueAt(int x, int y, double value) =0;
   
  void increaseValueAt(int x, int y);
  void decreaseValueAt(int x, int y);
};

#endif

