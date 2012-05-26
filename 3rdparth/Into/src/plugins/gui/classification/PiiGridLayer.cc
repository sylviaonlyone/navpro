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

#include "PiiGridLayer.h"

PiiGridLayer::PiiGridLayer(int width, int height)
{
  _gridSize.setHeight(0);
  _gridSize.setWidth(0);
  setLimits( QSize(width, height));
}

double PiiGridLayer::valueAt(int x, int y) const
{
  if(x<0 || x > _gridSize.width() || y < 0 ||y> _gridSize.height()) return 0;
  else return _values[y][x];
}

QVector<QVector<double> > PiiGridLayer::valueArray() const
{
  return _values;
}

QVector<double> PiiGridLayer::values() const
{
  QVector<double> val;
  
  for(int i=0;i<_values.size();i++){
    val << ( _values[i] );
  }
  return val;
}


void PiiGridLayer::setLimits(QSize val)
{
  if(val.height()>_gridSize.height())
    {
      int n_rows = val.height()-_gridSize.height(); // number of new rows
      for(int i=0;i<n_rows;i++)
        {
          QVector<double> row;
          for(int x=0;x<_gridSize.width();x++)
            {
              row.append(0);
            }
          _values.append(row);
        }
    }
  else if(val.height() < _gridSize.height())
    {
      int n_rows = _gridSize.height()-val.height();   
      for(int i=0;i<n_rows;i++)
        {
          _values.pop_back();// LIST:takeLast(); // Take out last row
        }
    }
  _gridSize.setHeight(val.height());
    
  if(val.width()>_gridSize.width())
    {
      for(int r=0;r< _gridSize.height();r++)
        {
          int n_cols = val.width()-_gridSize.width();
          for(int i=0; i< n_cols ;i++)
            {
              _values[r].append(0);
            }
        }
    }
  else if(val.width()<_gridSize.width())
    {
      for(int r=0;r< _gridSize.height();r++)
        {
          int n_cols = _gridSize.width()-val.width();
          for(int i=0; i< n_cols ;i++)
            {
              _values[r].pop_back();// Take out last from each row
            }
        }
    }
  _gridSize.setWidth(val.width());
}

void PiiGridLayer::setValueAt(int x, int y, double value)
{
  if(x<0 || x >= _gridSize.width() || y < 0 ||y>= _gridSize.height()){
    return;
  }
  // Limit value between min and max
  if(value<_pInfo->minimum())
    value = _pInfo->minimum();
  if(value>_pInfo->maximum())
    value = _pInfo->maximum();

  //qDebug("value == %f", value);
  //qDebug("resolution == %f", _pInfo->resolution());
  //float eka = value/_pInfo->resolution();
  //qDebug("value/_pInfo->resolution() == %f", eka);
  //float toka = eka + 0.5f;
  //qDebug("value/_pInfo->resolution() + 0.5f == %f", toka);
  //int kolmas = (int)toka;
  //qDebug("(int)(value/_pInfo->resolution()+0.5f) == %i", kolmas);
  //float neljas = kolmas * _pInfo->resolution();
  //qDebug("lopuksi == %f", neljas);
  
  // ROUNDING:
  if(value>= 0.0f)
    _values[y][x] = (int)(value/_pInfo->resolution()+0.5f)*_pInfo->resolution();
  else
    _values[y][x] = (int)(value/_pInfo->resolution()-0.5f)*_pInfo->resolution();

 //qDebug("_values[y][x] == %f", _values[y][x]);
  
}
