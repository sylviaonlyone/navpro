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

#ifndef PIIGRIDLAYER_H
#define PIIGRIDLAYER_H

#include "PiiSelectorLayer.h"

class PiiGridLayer : public PiiSelectorLayer
{
   Q_OBJECT

public:
  
  PiiGridLayer(int width, int height);
   
  /**
   * Override this method on different kind of layers
   **/
  virtual double valueAt(int x, int y) const ;

  /**
   * returns a 2D vector (vector of vectors of float) containing all values.
   **/
  virtual QVector<QVector<double> > valueArray() const;
  virtual QVector<double> values() const ;

  int rows() const  { return _gridSize.height(); }
  int cols() const { return _gridSize.width(); }

protected:
 QSize                     _gridSize;
 QVector< QVector<double> > _values;
  
public slots:
     
  virtual void setLimits(QSize val);
  virtual void setValueAt(int x, int y, double value);
  

};
#endif
