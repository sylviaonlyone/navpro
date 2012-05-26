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

#include "PiiVisualSelector.h"
PiiVisualSelector::PiiVisualSelector() : _pCurrentLayer(0), _pScrollarea(0)
{ }


void PiiVisualSelector::addLayer(PiiSelectorLayer* layer)
{
  _layers.append(layer);
  //selectLayer(_layers.size()-1);
  emit layerAdded();
}

void PiiVisualSelector::removeLayer(PiiSelectorLayer* layer)
{
  if(layer == 0 || layer==_pCurrentLayer) // remove current layer
    {
      if(_pCurrentLayer != 0)
        {
          _layers.removeAll(_pCurrentLayer);
          delete _pCurrentLayer;
          _pCurrentLayer = 0;
        }
    }
  else
    {
      _layers.removeAll(layer);
      delete layer;
      layer = 0;
    }
}

void PiiVisualSelector::removeLayers()
{
  qDeleteAll(_layers);
  _layers.clear();
}

QComboBox* PiiVisualSelector::createLayersCombo()
{
  QComboBox* lc = new QComboBox();
  _lstTempIndexes.clear();
  for(int i=0;i<_layers.size();i++)
    {
      if ( _layers[i]->isEnabled() )
        {
          lc->insertItem(i,_layers[i]->info()->icon(),_layers[i]->info()->name());
          _lstTempIndexes.append(i);
        }
    }
  QObject::connect(lc,SIGNAL(activated(int)),this,SLOT(selectEnabledLayer(int)));
  
  //setLayerTool(QWidget* _layertool)     
  return lc;
}

void PiiVisualSelector::selectEnabledLayer(int i)
{
  if (i >= 0 && i<_lstTempIndexes.size())
    selectLayer(_lstTempIndexes[i]);
}

void PiiVisualSelector::selectLayer(int i)
{ 
  if(i >= 0 && i<_layers.size())
    {
      _pCurrentLayer = _layers.at(i);
      setCurrentLayer(_pCurrentLayer);
      update();
    }
}


/**
 * Change the current layer
 **/
void PiiVisualSelector::setCurrentLayer(PiiSelectorLayer* layer){
  _pCurrentLayer = layer;
  update();
  reselect();
  if(_pCurrentLayer != 0)
    emit layerEditorToolChanged(_pCurrentLayer->info()->editor());
}


QVector<double> PiiVisualSelector::values() const {
  if(_pCurrentLayer != 0)
    return _pCurrentLayer->values();
  return QVector<double>();
}

QVector<double> PiiVisualSelector::allValues(int layer_index) const {
  if(layer_index>=0 && layer_index<_layers.size())
    return _layers.at(layer_index)->values();
  return QVector<double>();
}

QVector< QVector<double> > PiiVisualSelector::allValues() const {
  QVector< QVector<double> > allvalues;
  PiiSelectorLayer* layer;
  foreach(layer, _layers)
    {
      allvalues.append(layer->values());
    }
  return allvalues;
}


