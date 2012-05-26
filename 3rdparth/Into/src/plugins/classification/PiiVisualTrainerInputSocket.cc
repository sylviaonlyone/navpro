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

#include "PiiVisualTrainerInputSocket.h"
#include <QtAlgorithms>

PiiVisualTrainerInputSocket::PiiVisualTrainerInputSocket(const QString& name, LayerType layerType) :
  PiiInputSocket(name),
  _layerType(layerType), _gridSize(10,10), _iValueCnt(0), _iCellCount(100)
{
  resetValues();
}

void PiiVisualTrainerInputSocket::storeValue(int index, double value)
{
  if (index < 0 || index >= _iCellCount)
    return;
  
  _iValueCnt++;

  switch (_layerType)
    {
    case ClassLayer:
      // Build a histogram of different values
      {
        int label = int(value);
        while (label >= _lstLabels[index].size())
          _lstLabels[index] << 0;
        _lstLabels[index][label]++;
        //qDebug("_lstLabels[%i][%i] = %i", index, label, _lstLabels[index][label]);
      }
      break;
    case FloatLayer:
      // Calculate average value for the label iteratively
      {
        double mu = 1.0 / _iValueCnt;
        _lstAverages[index] = _lstAverages[index] * (1.0 - mu) + value * mu;
      }
      break;
    }
}

void PiiVisualTrainerInputSocket::resetValues()
{
  _iCellCount = _gridSize.width() * _gridSize.height();
  _lstLabels.clear();
  while (_lstLabels.size() < _iCellCount)
    _lstLabels << QList<int>();
  _lstAverages.resize(_iCellCount);
  qFill(_lstAverages.begin(), _lstAverages.end(), 0.0);
  _iValueCnt = 0;
}

void PiiVisualTrainerInputSocket::setLayerType(LayerType layerType)
{
  _layerType = layerType;
  resetValues();
}

void PiiVisualTrainerInputSocket::setGridSize(QSize gridSize)
{
  _gridSize = gridSize;
  resetValues();
}

double PiiVisualTrainerInputSocket::label(int index)
{
  switch (_layerType)
    {
    case ClassLayer:
      {
        // Find the label with the maximum number of votes.
        int maximum = 0, maxIndex = 0;
        QList<int>& labels = _lstLabels[index];
        for (int i=labels.size(); i--; )
          {
            if (labels[i] > maximum)
              {
                maximum = labels[i];
                maxIndex = i;
              }
          }
        if (maximum == 0)
          return -1;
        return maxIndex;
      }
    case FloatLayer:
      // Return average label
      return _lstAverages[index];
    }
  return 0;
}
