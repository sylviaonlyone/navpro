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

#ifndef PIIVISUALSELECTOR_H
#define PIIVISUALSELECTOR_H

#include <QtGui>
#include <QImage>
#include <QColor>
#include <QString>

#include "PiiSelectorLayer.h"
#include "PiiScrollArea.h"
#include "PiiComboBox.h"

/**
 * Base class for different type of visual selectors.
 * VisualSelector consists from two main parts:
 * - Layers (see PiiSelectorLayer.h) that contains information assosiated to visual data
 *   that is: labels, relative values, fuzzy value of certain features
 * - Visual information. That is, images that are assosiated to certain coordinates.
 * Note! most of the methods in this class has no effect.
 */
class PiiVisualSelector : public QWidget
{
  Q_OBJECT
public:
  PiiVisualSelector();
   
  /**
   * Returns a value assosiated with given coordinate
   */
  virtual double value(double x,double y) const = 0;

  /**
   * Returns a value assosiated to given image on given layer.
   * if no layer is specified, _pCurrentLayer is used.
   */
  virtual double value(QImage* image, PiiSelectorLayer* layer=0) const = 0;
   
  /**
   * Adds new layer to list of layers in this selector
   */
  virtual void addLayer(PiiSelectorLayer* layer);


  virtual QList<PiiSelectorLayer*> layers() const { return _layers; }

  /**
   * removes given selector from the layer.
   * if removed layers is _pCurrentLayer, _pCurrentLayer becomes 0
   * NOTE: This may cause unwanted behaviour.
   */
  virtual void removeLayer(PiiSelectorLayer* layer);

  /**
   * Removes all layers from selector.
   */
  virtual void removeLayers();
   
  /**
   * Do reselction to selected units. This is used to notify selection 
   * in many cases, for example when layer changes.
   */
  virtual void reselect() = 0;
   
  /**
   * Creates a selection combo of layers currently on selector
   * NOTE! if set of layers chages, this method must be called again
   */
  QComboBox* createLayersCombo();
   
  PiiSelectorLayer* currentLayer() const { return _pCurrentLayer; }
  
  QVector<double> values() const;
   
  QVector<double> allValues(int layer_index) const;
   
  QVector< QVector<double> > allValues() const;
  
  virtual void setScrollArea(PiiScrollArea* scrollarea) { _pScrollarea = scrollarea; }

protected:
  QList<PiiSelectorLayer*> _layers;
  QList<int> _lstTempIndexes;
  PiiSelectorLayer* _pCurrentLayer;
   
  PiiScrollArea* _pScrollarea;
   
signals:

  void acceptChanges();
  
  /**
   * Indicates that only the images listed in @p images are now
   * selected.
   *
   * @param images selected images
   *
   * @param values the values of the selected images (on current layer)
   */
  void imagesSelected(const QList<QImage*>& images, const QList<double>& values);
  
  /**
   * Indicates that the selection state of images listed in @p images
   * has changed.
   *
   * @param images images whose selection changed
   *
   * @param values the values of the selected images (current layer)
   *
   * @param the current selection state (@p true means selected, @p
   * false unselected)
   */
  void imageSelectionChanged(const QList<QImage*>& images, const QList<double>& values, bool selected);
  

  /**
   * this signal is sent when all selections been removed
   */
  void allImagesUnselected();
  
  /**
   * emits a signal of new editor for selected layer
   */
  void layerEditorToolChanged(QWidget* widget);

  void layerAdded();
  
  /**
   * This is sent when value for selected is changed
   */
  void selectionStateChanged(double value);
 
  /**
   * This is sent when value for selected is changed
   * Q = sends quantizated value, from min (0) to max(number of steps)
   * the resolution of layer info defines step size
   */
  void selectionQStateChanged(int value);
  
  /**
   * This is sent when severaal valus has been selected at the same time.
   */
  void selectionStateMultivalue();
  
public slots:
  virtual void unselectAll() = 0;
  virtual void addImage(QImage*, double x, double y, bool silent=false) = 0;
  virtual void removeImage(QImage*, bool silent=false) = 0;
  virtual void removeImages(double x, double y, bool silent=false) = 0;
  virtual void removeAllImages() = 0;
   
  virtual void setCellSize(int size) = 0;
   
  /**
   * Assing given value to cell
   * if layer not given (=-1) adds to current layer
   */
  //virtual void assignValueToSelected(int value, int layer=-1) = 0;
   
  /**
   * Sets given value to all selected components on the selector in _pCurrentLayer.
   * NOTE: if no layer is used, this method has no effect.
   */
  virtual void assignValueToSelected(double value, int layer=-1) = 0;
  
  /**
   * Sets given value to node x,y.
   */
  virtual void assignValueToPoint(double value, int layer, int pointIndex) = 0;
  
  virtual void decreaseValueToSelected() = 0;
  virtual void increaseValueToSelected() = 0;
  
  virtual void nextImageOnSelected() = 0; 
  virtual void prevImageOnSelected() = 0;
   
  virtual void selectEnabledLayer(int i);
  virtual void selectLayer(int i);
  virtual void setCurrentLayer(PiiSelectorLayer* layer);

  /**
   * Select all images that contain @p p.
   */
  virtual void selectImagesContaining(QPoint p, int modifiers) = 0;
  /**
   * Select all images that intersect @p r.
   */
  virtual void selectImagesIntersecting(QRect r, int modifiers) = 0;
  
};
#endif
