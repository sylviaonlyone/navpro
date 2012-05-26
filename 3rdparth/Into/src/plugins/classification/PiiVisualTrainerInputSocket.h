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

#ifndef _PIIVISUALTRAINERINPUTSOCKET_H
#define _PIIVISUALTRAINERINPUTSOCKET_H

#include <PiiInputSocket.h>
#include <QVector>
#include <QList>
#include <QSize>

/**
 * A special input socket that stores incoming labels. This class also
 * gives a suggestion for a class label if one is requested.
 *
 * @ingroup PiiClassification
 */
class PiiVisualTrainerInputSocket : public PiiInputSocket
{
  Q_OBJECT

public:
  enum LayerType { ClassLayer, FloatLayer };
  PiiVisualTrainerInputSocket(const QString& name, LayerType type = ClassLayer);

  void storeValue(int index, double value);
  void resetValues();
  double label(int index);

  void setLayerType(LayerType layerType);
  LayerType layerType() const { return _layerType; }
  void setGridSize(QSize gridSize);
  QSize gridSize() const { return _gridSize; }
  
private:
  LayerType _layerType;
  QList<QList<int> > _lstLabels;
  QVector<double> _lstAverages;
  QSize _gridSize;
  int _iValueCnt;
  int _iCellCount;
};

#endif //_PIIVISUALTRAINERINPUTSOCKET_H
