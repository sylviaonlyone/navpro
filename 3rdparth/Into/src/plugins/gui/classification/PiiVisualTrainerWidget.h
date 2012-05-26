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

#ifndef _PIIVISUALTRAINERWIDGET_H
#define _PIIVISUALTRAINERWIDGET_H

#include "PiiSelectorLayer.h"

#include <PiiConfigurationWidget.h>
#include <PiiUndoCommandProducer.h>
#include <PiiOperation.h>
#include <PiiQImage.h>
#include <ui_visualtrainerwidget.h>

#include <QLinkedList>
#include <QPoint>
#include <QSplitter>
#include <QListView>
#include <QTextEdit>

class PiiImageOverlay;
class PiiImageViewport;
class PiiGridLayer;
class PiiGridSelector;

/**
 * TODO DESCRIPTION
 **/
class PiiVisualTrainerWidget :  public PiiConfigurationWidget, public PiiUndoCommandProducer, private Ui_VisualTrainerWidget
{
  Q_OBJECT
  Q_INTERFACES(PiiUndoCommandProducer)

  /**
   * If this property is true, we will update the grid selector each
   * time, when the new image was received. Otherwise the grid
   * selector will be updated when the allSubImageArrived() -signal
   * received. The default value is false.
   */
  Q_PROPERTY(bool displayAllImages READ displayAllImages WRITE setDisplayAllImages);

  /**
   * If this property is true, we will show the buffering properties
   * selection in the popup menu. The default value is false.
   */
  Q_PROPERTY(bool showBufferingProperties READ showBufferingProperties WRITE setShowBufferingProperties STORED false);

  /**
   * If this property is true, we will accept all changes immediately.
   * The default value is false.
   */
  Q_PROPERTY(bool acceptImmediately READ acceptImmediately WRITE setAcceptImmediately);

  /**
   * gridSize description
   */
  Q_PROPERTY(QSize gridSize READ gridSize WRITE setGridSize STORED false);
  
  /**
   * displayMode description
   */
  Q_PROPERTY(DisplayMode displayMode READ displayMode WRITE setDisplayMode);
  Q_ENUMS(DisplayMode);
  
public:

  enum DisplayMode { Normal, Splitted };
  
  PiiVisualTrainerWidget(QWidget *parent = 0);
  ~PiiVisualTrainerWidget();

  void setDisplayAllImages(bool displayAllImages) { _bDisplayAllImages = displayAllImages; }
  bool displayAllImages() const { return _bDisplayAllImages; }

  void setShowBufferingProperties(bool showBufferingProperties);
  bool showBufferingProperties() const;
  
  void setAcceptImmediately(bool acceptImmediately) { _bAcceptImmediately = acceptImmediately; }
  bool acceptImmediately() const { return _bAcceptImmediately; }

  void setDisplayMode(const DisplayMode& displayMode);
  DisplayMode displayMode() const { return _displayMode; }

  /**
   * Changes the orientation of splitter (the one that splits
   * selector widget and image display.
   **/
  void setSplitterOrientation(Qt::Orientation val);
  Qt::Orientation splitterOrientation() const;
   
  /**
   * Returns a value on selector in given coordinate on current_layer.
   **/
  double valueAt(double x, double y) const;
   
  /**
   * Returns a value on selector in given coordinate on given layer.
   */
  double valueAt(PiiGridLayer* layer, double x, double y) const;
   
  /**
   * Returns a value on given coordinate on given layer
   */
  double valueAt(int layer_index, double x, double y) const;
      
  /**
   * Returns a value on given coordinate on given layer
   */
  QVector<double> allValuesAt(PiiSelectorLayer* layer) const;
   
  /**
   * Returns a value on given coordinate on given layer
   */
  QVector<double> allValuesAt(int layer_index) const;   

  void setOperation(PiiOperation *op);

  QSize gridSize() const;
  
protected:
  void keyPressEvent(QKeyEvent * e);

public slots:
  void acceptChanges();
  void reset();
  void updateView();
  void checkOperationStatus(int status = -1);
  
  void addSubImage(QImagePtr image, double x, double y);
  void removeSubImage(QImagePtr image);
  void setMainImage(QImagePtr image);
  void setGridSize(const QSize& gridSize);
  void setLabels(const QVariantList& labels, bool checkChanges = false);
  void setLayers(const QVariantList& layers, bool checkChanges = false);
  
  /**
   * Changes the widget on toolbar
   */
  void setLayerCombo(QComboBox* layersCombo);
 
  /**
   * Method for changing layer editor tool.
   */
  void setLayerTool(QWidget* layertool);
   

  /**
   * slot to notify what images are selected and what it their selection.
   * This is automatically connected to selector's selection signal
   * and it sets overlays to display (if it exists).
   */
  void imagesSelected(const QList<QImage*>& images, const QList<double>& values, bool selection);
  void imagesSelected(const QList<QImage*>& images, const QList<double>& values);
   
  /**
   * Chages the main image (on image display)
   */
  void setMainImage(QImage* image);
 
  /**
   * Adds sub image to selector.
   */
  void addSubImage(QImage* image, double x, double y);

  /**
   * Searches and removes all matchin subimages from selector.
   */
  void removeSubImage(QImage* image);
 
  /**
   * Removes ALL images from given selector position
   */
  void removeSubImages(double x,double y);
 
  /**
   * Clears (Removes ALL images) the selector
   */
  void removeAllSubImages();
 
  /**
   * Removes all overlays
   */
  void removeOverlays();
  void updateLayersCombo();
  void removeOverlay(QImage* im);

private slots:
  void openInfoDialog();
  void openBufferingPropertiesDialog();
  void openLayerEditor();
  void stateChanged(bool para);
  
private:
  void deleteOverlays();
  void checkState();
  void checkAcception();
  QVariantList labels() const;
  QVariantList layers() const;
  bool isLabelsChanged() const;
  bool isLayersChanged() const;
  
  PiiImageViewport* _pImageViewport;
  PiiGridSelector* _pSelector;
  QComboBox* _pLayersCombo;
  QString _strMainImageName;
   
  /**
   * Current widget for adjusting layer value
   */
  QWidget* _pLayerTool;

  /**
   * Table of images currently selected from grid and their 
   * matching overlays.
   * This is used when changing values (and brushes) of active 
   * overlays on the display.
   */
  QList<QPair<QImage*, PiiImageOverlay*> > _lstSelectionPairs;

  PiiOperation* _pOperation;
  QImagePtr _mainImage;
  QLinkedList<QImagePtr> _lstSubImages;
  bool _bDisplayAllImages;
  bool _bAcceptImmediately;
  DisplayMode _displayMode;

  QVariantList _lstPreviousLabels, _lstPreviousLayers;
};
#endif
