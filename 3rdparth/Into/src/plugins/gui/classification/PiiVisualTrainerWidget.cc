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

#include "PiiVisualTrainerWidget.h"
#include "PiiImageOverlay.h"
#include "PiiGridLayer.h"
#include "PiiGridSelector.h"
#include "PiiVisualTrainerLabelingCommand.h"
#include "PiiVisualTrainerLayersCommand.h"
#include "PiiLayerEditorDialog.h"
#include "PiiFloatLayerInfo.h"
#include "PiiClassNameLayerInfo.h"
#include "PiiBufferingPropertiesDialog.h"

#include <ui_quality_map_info.h>

#include <PiiImageDisplay.h>
#include <PiiImageViewport.h>

#include <QDialog>

PiiVisualTrainerWidget::PiiVisualTrainerWidget(QWidget *parent) :
  PiiConfigurationWidget(parent),
  _pImageViewport(0),
  _pLayersCombo(0),
  _pLayerTool(0),
  _pOperation(0),
  _bDisplayAllImages(false),
  _bAcceptImmediately(false),
  _displayMode(Normal)
{
  setupUi(this);

  // Configure grid selector
  _pSelector = new PiiGridSelector(4,5);
  _pSelectorScrollArea->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
  _pSelectorScrollArea->setAspectRatioMode(Qt::KeepAspectRatio);
  _pSelectorScrollArea->setWidget(_pSelector);
  _pSelector->setScrollArea(_pSelectorScrollArea);

  // Configure image display
  _pImageViewport = _pImageDisplay->imageViewport();
  if (_pImageViewport != 0)
    {
      _pImageViewport->setShowOverlayColoring(true);
      _pImageViewport->setFitMode(PiiImageViewport::FitToView);
      QImage* pImage = _pImageViewport->image();
      if (pImage != 0)
        _strMainImageName = pImage->text("pii_id");
      else
        _strMainImageName = QString("");
    }
  
  // Configure info button
  _pInfoButton->setIcon(QIcon(QString::fromUtf8(":/help.png")));

  // Connect ui-signals
  connect(_pSelector, SIGNAL(acceptChanges()), this, SLOT(acceptChanges()));
  connect(_pSelector, SIGNAL(bufferingActionTriggered()), this, SLOT(openBufferingPropertiesDialog()));
  connect(_pSelector, SIGNAL(stateChanged(bool)), this, SLOT(stateChanged(bool)));
  connect(_pSelector, SIGNAL(allImagesUnselected()),this, SLOT(removeOverlays()));
  connect(_pSelector,SIGNAL(layerEditorToolChanged(QWidget*)),this,SLOT(setLayerTool(QWidget*)));
  connect(_pSelector,SIGNAL(layerAdded()), this,SLOT(updateLayersCombo()));
  connect(_pSelector, SIGNAL(imageSelectionChanged(QList<QImage*>,QList<double>,bool)),
          this, SLOT(imagesSelected(QList<QImage*>,QList<double>,bool)));
  connect(_pSelector, SIGNAL(imagesSelected(QList<QImage*>,QList<double>)),
          this, SLOT(imagesSelected(QList<QImage*>,QList<double>)));

  connect(_pShowLayerEditorButton, SIGNAL(clicked()), this, SLOT(openLayerEditor()));

  connect(_pZoomIn, SIGNAL(clicked()), _pSelector, SLOT(zoomIn()));
  connect(_pZoomOut, SIGNAL(clicked()), _pSelector, SLOT(zoomOut()));
  connect(_pFitToView, SIGNAL(clicked()), _pSelector, SLOT(setFitToView()));

  connect(_pInfoButton, SIGNAL(clicked()), SLOT(openInfoDialog()));

  connect(_pImageViewport, SIGNAL(clicked(QPoint,int)), _pSelector, SLOT(selectImagesContaining(QPoint,int)));
  connect(_pImageViewport, SIGNAL(areaSelected(QRect,int)), _pSelector, SLOT(selectImagesIntersecting(QRect,int)));

  // Create default combo with no content
  updateLayersCombo();

  setDisplayMode(Normal);
}

PiiVisualTrainerWidget::~PiiVisualTrainerWidget()
{
  removeOverlays();
  // as most pointers on this class are Children, they do not need to
  // delete here.
}


void PiiVisualTrainerWidget::checkOperationStatus(int status)
{
  if (_pOperation)
    {
      // Large image is displayed only if the location input is
      // connected
      PiiAbstractInputSocket *pInput = _pOperation->input("location");
      if (pInput)
        setDisplayMode(pInput->connectedOutput() == 0 ? Normal : Splitted);
      else
        setDisplayMode(Normal);
    }
  else
    removeAllSubImages();
  
  if (status < 0 || !hasChanged())
    {
      QSize gridSize(5,4);
      QVariantList layers, labels;
      
      if (_pOperation != 0)
        {
          gridSize = _pOperation->property("gridSize").toSize();
          layers = _pOperation->property("layers").toList();
          labels = _pOperation->property("labels").toList();
        }
      
      _pSelector->setGridSize(gridSize);
      setLayers(layers);
      setLabels(labels);
    }

}


void PiiVisualTrainerWidget::setOperation(PiiOperation *op)
{
  _pOperation = op;
  checkOperationStatus();
}

void PiiVisualTrainerWidget::setDisplayMode(const DisplayMode& mode)
{
  _displayMode = mode;
  _pImageDisplay->setVisible(mode == Splitted);
}


Qt::Orientation PiiVisualTrainerWidget::splitterOrientation() const
{
  return _pMainSplitter->orientation();
}

void PiiVisualTrainerWidget::setSplitterOrientation(Qt::Orientation val)
{
  _pMainSplitter->setOrientation(val);
  update();
}

void PiiVisualTrainerWidget::setLayerCombo(QComboBox* layersCombo)
{
  // Clear layout
  while (_pToolButtonGroupLayout->itemAt(0) != 0) 
    _pToolButtonGroupLayout->removeItem(_pToolButtonGroupLayout->itemAt(0));
  
  if (_pLayerTool != 0)
    {
      _pToolButtonGroupLayout->removeWidget(_pLayerTool);
      delete _pLayerTool;
      _pLayerTool = 0;
    }
  if (_pLayersCombo != 0)
    {
      _pToolButtonGroupLayout->removeWidget(_pLayersCombo);
      delete _pLayersCombo;
    }

  if (layersCombo->count() > 1)
    {
      _pToolButtonGroupLayout->insertWidget(0,layersCombo);
      _pToolButtonGroupLayout->setAlignment(layersCombo,Qt::AlignLeft);
    }
  
  _pLayersCombo = layersCombo;
}

void PiiVisualTrainerWidget::setLayerTool(QWidget* layertool)
{
  if (_pLayerTool!=0)
    {
      _pToolButtonGroupLayout->removeWidget(_pLayerTool);
      delete _pLayerTool;
    }
  _pToolButtonGroupLayout->insertWidget(1,layertool);
  _pToolButtonGroupLayout->setAlignment(layertool,Qt::AlignLeft);
  
  _pLayerTool = layertool;
  
  if (_pToolButtonGroupLayout->count()<=2)
    _pToolButtonGroupLayout->addStretch();
  
  _pSelector->reselect();
}

void PiiVisualTrainerWidget::keyPressEvent ( QKeyEvent * e )
{
  switch (e->key())
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      _pSelector->assignValueToSelected(QString(e->key()).toInt());
      break;
    case 'n':
      _pSelector->nextImageOnSelected(); break;
    case 'p':
      _pSelector->prevImageOnSelected(); break;
    }
  if (e->key() ==Qt::Key_PageUp)
    _pSelector->nextImageOnSelected(); 
  else if (e->key() ==Qt::Key_PageDown)
    _pSelector->prevImageOnSelected();  
}

void PiiVisualTrainerWidget::setMainImage(QImage* im)
{
  if (_pImageViewport!=0)  
    _pImageViewport->setImage(im);
  
  if (im!=0)
    _strMainImageName = im->text("pii_id");
  else
    _strMainImageName = QString("");

  removeOverlays();
  _pSelector->unselectAll();
}

void PiiVisualTrainerWidget::removeSubImages(double x,double y)
{
  //PENDING remove overlays
  _pSelector->removeImages(x,y, !_bDisplayAllImages);
}

void PiiVisualTrainerWidget::addSubImage(QImage* im, double x, double y)
{
  _pSelector->addImage(im, x,y,!_bDisplayAllImages);
}

void PiiVisualTrainerWidget::removeSubImage(QImage* im)
{
  _pSelector->removeImage(im, !_bDisplayAllImages);

    // Prevent drawing of deleted overlays
  _pImageViewport->setUpdatesEnabled(false);

  // Delete the corresponding overlay, if it exists
  removeOverlay(im);

  // It is now safe to allow drawing again
  _pImageViewport->setUpdatesEnabled(true);
}

void PiiVisualTrainerWidget::removeOverlay(QImage* image)
{
  for (int i=_lstSelectionPairs.size(); i--; )
    {
      if (_lstSelectionPairs[i].first == image)
        {
          // We don't have selection pairs if we don't have display.
          // No check for _pImageViewport == 0 needed here.
          _pImageViewport->removeOverlay(_lstSelectionPairs[i].second);
          // We allocated the overlay. Delete.
          delete _lstSelectionPairs[i].second;
          _lstSelectionPairs.removeAt(i);
          // NOTE this should be safe since each image should have
          // only one overlay.
          break;
        }
    }
}


void PiiVisualTrainerWidget::removeAllSubImages()
{
  removeOverlays();
  _pSelector->removeAllImages();
  _lstSubImages.clear();
}

double PiiVisualTrainerWidget::valueAt(double x, double y) const
{
  return _pSelector->currentLayer()->valueAt((int)x,(int)y);
}

double PiiVisualTrainerWidget::valueAt(PiiGridLayer* layer, double x, double y) const
{
  return layer->valueAt((int)x,(int)y);
}

QVector<double> PiiVisualTrainerWidget::allValuesAt(PiiSelectorLayer* layer) const
{
  return layer->values();
}

QVector<double> PiiVisualTrainerWidget::allValuesAt(int layer_index) const
{
  return _pSelector->allValues(layer_index);
}   

/**************************************************
 * SLOTS:
 **************************************************/
void PiiVisualTrainerWidget::imagesSelected(const QList<QImage*>& images, const QList<double>& values, bool selection)
{
  if (_pImageViewport == 0)
    return;

  // We may delete overlays the display still holds. This disables
  // drawing them.
  _pImageViewport->setUpdatesEnabled(false);
  if (selection == false)
    {
      // remove all overlays bound to the deselected images
      for (int i = images.size(); i--; )
        removeOverlay(images[i]);
    }
  else
    {
      for (int j = images.size(); j--; )
        {
          bool found = false;

          // Look up the selected image in our map of overlays.
          for (int i=_lstSelectionPairs.size(); i--; )
            {
              if (_lstSelectionPairs.at(i).first == images.at(j))
                {
                  // We found it. Update the overlay.
                  if (_pSelector->currentLayer() != 0 &&
                     _pSelector->currentLayer()->info() != 0)
                    {
                      QBrush brush = _pSelector->currentLayer()->info()->brush(values.at(j));
                      ((PiiRectangleOverlay*)_lstSelectionPairs.at(i).second)->setBrush(brush);
                      
                      QPen pen = _pSelector->currentLayer()->info()->pen(values.at(j));
                      ((PiiRectangleOverlay*)_lstSelectionPairs.at(i).second)->setPen(pen);
                    }
                  found = true;
                  break;
                }
            }
          // Not found. Must add a new ovelay if the image is part of
          // our main image.
          if (!found && images[j]->text("pii_part_of") == _strMainImageName)
            {
              QPoint offset = images.at(j)->offset();
              QRect rec(offset.x(),offset.y(),images.at(j)->width(),images.at(j)->height());
              PiiRectangleOverlay* overlay = new PiiRectangleOverlay(rec);
              if (_pSelector->currentLayer() != 0 &&
                  _pSelector->currentLayer()->info() != 0)
                {
                  QBrush brush = _pSelector->currentLayer()->info()->brush(values.at(j));
                  overlay->setBrush(brush);
                  
                  QPen pen = _pSelector->currentLayer()->info()->pen(values.at(j));
                  overlay->setPen(pen);
                }
              QPair<QImage*, PiiImageOverlay*> temp(images.at(j),overlay);
              _lstSelectionPairs.append(temp);
              _pImageViewport->addOverlay(overlay);
            }
        }
    }
  
  _pImageViewport->setUpdatesEnabled(true);
  _pImageViewport->update();
}

void PiiVisualTrainerWidget::imagesSelected(const QList<QImage*>& images, const QList<double>& values)
{
  if (_pImageViewport == 0)
    return;

  _pImageViewport->setUpdatesEnabled(false);
  deleteOverlays();
  imagesSelected(images, values, true);
}

void PiiVisualTrainerWidget::removeOverlays()
{
  if (!_pImageViewport)
    return;

  // Prevent drawing of deleted overlays
  _pImageViewport->setUpdatesEnabled(false);

  deleteOverlays();

  // It is now safe to allow drawing again
  _pImageViewport->setUpdatesEnabled(true);
  _pImageViewport->update();
}

void PiiVisualTrainerWidget::updateLayersCombo()
{
  setLayerCombo(_pSelector->createLayersCombo());
}

void PiiVisualTrainerWidget::deleteOverlays()
{
  // Delete overlays
  _pImageViewport->removeOverlay();
  for (int i=_lstSelectionPairs.size(); i--; )
    delete _lstSelectionPairs[i].second;
  _lstSelectionPairs.clear();
}

void PiiVisualTrainerWidget::openInfoDialog()
{
  QDialog window(this);
  Ui::QualityMap ui;
  ui.setupUi(&window);
  window.setModal(true);
  window.setWindowFlags(window.windowFlags() & ~Qt::WindowContextHelpButtonHint);
  //window.setWindowTitle(tr("Quality Map"));
  window.exec();
}


void PiiVisualTrainerWidget::stateChanged(bool /*para*/)
{
  QVariantList newState = labels();

  undoCommandPerformed(new PiiVisualTrainerLabelingCommand(this,
                                                           _lstPreviousLabels,
                                                           newState,
                                                           tr("Labeling")));
  _lstPreviousLabels = newState;
  checkAcception();
}

void PiiVisualTrainerWidget::addSubImage(QImagePtr image, double x, double y)
{
  //If we find this image from the buffer, we must remove it first
  removeSubImage(image);
  
  addSubImage((QImage*)image, x, y);
  _lstSubImages << image;
}

void PiiVisualTrainerWidget::updateView()
{
  update();
  _pSelector->updateView();
}

void PiiVisualTrainerWidget::removeSubImage(QImagePtr image)
{
  removeSubImage((QImage*)image);
  _lstSubImages.removeAll(image);
}

void PiiVisualTrainerWidget::setMainImage(QImagePtr image)
{
  setMainImage((QImage*)image);
  _mainImage = image;
}

void PiiVisualTrainerWidget::setGridSize(const QSize& gridSize)
{
  _pSelector->setGridSize(gridSize);
}

QSize PiiVisualTrainerWidget::gridSize() const
{
  return _pSelector->gridSize();
}

void PiiVisualTrainerWidget::openLayerEditor()
{
  if (_pOperation)
    {
      PiiLayerEditorDialog layerEditorDialog(_pOperation->property("layers").toList(), this);
      if (layerEditorDialog.exec() == QDialog::Accepted)
        {
          QVariantList newState = layerEditorDialog.layerList();

          if (newState != layers())
            {
              PiiVisualTrainerLayersCommand *pLayersCommand = new PiiVisualTrainerLayersCommand(this,
                                                                                                _lstPreviousLayers,
                                                                                                newState,
                                                                                                tr("Layers"));
              pLayersCommand->setLabels(_lstPreviousLabels);
              undoCommandPerformed(pLayersCommand);
              
              setLayers(newState, true);
            }
        }
    }
}

void PiiVisualTrainerWidget::setLayers(const QVariantList& layers, bool checkChanges)
{
  QSize gridSize = _pOperation != 0 ? _pOperation->property("gridSize").toSize() : _pSelector->gridSize();
  int rows = gridSize.height();
  int cols = gridSize.width();
  QList<PiiSelectorLayer*> gridList = _pSelector->layers();
  QList<PiiSelectorLayer*> newLayers;
  
  for (int i=0; i<layers.size(); i++)
    {
      QMap<QString, QVariant> map = layers[i].toMap();
      QString name = map["name"].toString();
      PiiGridLayer* layer = new PiiGridLayer(cols, rows);
      if (map["type"] == "float")
        {
          PiiFloatLayerInfo *info = new PiiFloatLayerInfo(map["startColor"].value<QColor>(), map["endColor"].value<QColor>());
          info->setResolution(map["resolution"].toDouble());
          info->setMinimum(map["min"].toDouble());
          info->setMaximum(map["max"].toDouble());
          info->setLayerIndex(i);
          info->setName(name);
          layer->setInfo(info);
          QObject::connect(_pSelector, SIGNAL(selectionStateMultivalue()),   info, SLOT(unselectEditor()));
          QObject::connect(info,   SIGNAL(valueChanged(double, int)),    _pSelector,  SLOT(assignValueToSelected(double, int)));
          QObject::connect(info,   SIGNAL(valueChanged(double,int,int)),    _pSelector,  SLOT(assignValueToPoint(double,int,int)));
          QObject::connect(info, SIGNAL(selectionChanged(double, int)), _pSelector,  SLOT(selectByClass(double, int)));
              
        }
      else
        {
          PiiClassNameLayerInfo *info;
          if (map.contains("classColors"))
            info = new PiiClassNameLayerInfo(Pii::variantsToList<QColor>(map["classColors"].toList()), map["classNames"].toStringList());
          else
            info = new PiiClassNameLayerInfo(map["classNames"].toStringList());
          info->setLayerIndex(i);
          info->setName(name);
          layer->setInfo(info);
          QObject::connect(_pSelector, SIGNAL(selectionStateMultivalue()),   info, SLOT(unselectEditor()));
          QObject::connect(info,   SIGNAL(valueChanged(double, int)),    _pSelector,  SLOT(assignValueToSelected(double, int)));
          QObject::connect(info,   SIGNAL(valueChanged(double,int,int)),    _pSelector,  SLOT(assignValueToPoint(double,int,int)));
          QObject::connect(info, SIGNAL(selectionChanged(double, int)), _pSelector,  SLOT(selectByClass(double, int)));
        }

      if ( !map["disabled"].toBool() )
        layer->setEnabled(true);
      else
        layer->setEnabled(false);
          
          
      newLayers << layer;
    }
      
  _pSelector->removeLayers(); //remove old layers
  //add new layers
  for ( int i=0; i<newLayers.size(); i++ )
    _pSelector->addLayer(newLayers[i]);
      
  _pSelector->selectEnabledLayer(0);
      
  _lstPreviousLayers = layers;

  /**
   * We must check changes, if checkChanges == true. For example if
   * this function has been called from the
   * PiiVisualTrainerLayersCommand.
   */
  if (checkChanges)
    checkAcception();
  else
    checkState();

  updateView();
}

void PiiVisualTrainerWidget::setLabels(const QVariantList& labels, bool checkChanges)
{
  QVector<QVector<double> > lstLabels;
  for (int i=0; i<labels.size(); ++i)
    lstLabels << Pii::variantsToVector<double>(labels[i].toList());

  QList<PiiSelectorLayer*> layers = _pSelector->layers();
  QSize gridSize = _pOperation != 0 ? _pOperation->property("gridSize").toSize() : _pSelector->gridSize();
  
  int rows = gridSize.height();
  int cols = gridSize.width();

  for (int i=0; i<layers.size(); i++)
    {
      // Initialize labels
      if (lstLabels.size() > i)
        {
          PiiSelectorLayer* layer = layers[i];
          for (int y=0, index=0; y<rows; y++)
            {
              for (int x=0; x<cols; x++, index++)
                {
                  if (lstLabels[i].size() > index)
                    layer->setValueAt(x, y, lstLabels[i][index]);
                }
            }
        }
    }

  _lstPreviousLabels = labels;

  /**
   * We must check changes, if checkChanges == true. For example if
   * this function has been called from the
   * PiiVisualTrainerLabelingCommand.
   */
  if (checkChanges)
    checkAcception();
  else
    checkState();
  
  updateView();
}

QVariantList PiiVisualTrainerWidget::labels() const
{
  QVector<QVector<double> > lstLabels = _pSelector->allValues();
  QVariantList labels;
  for (int i=0; i<lstLabels.size(); ++i)
    labels << QVariant(Pii::vectorToVariants(lstLabels[i]));
  return labels;
}

QVariantList PiiVisualTrainerWidget::layers() const
{
  return _lstPreviousLayers;
}

void PiiVisualTrainerWidget::checkAcception()
{
  if ( _bAcceptImmediately )
    acceptChanges();
  else
    checkState();
}

bool PiiVisualTrainerWidget::isLabelsChanged() const
{
  if (_pOperation && _pOperation->property("labels").toList() != labels())
    return true;

  return false;
}

bool PiiVisualTrainerWidget::isLayersChanged() const
{
  if (_pOperation && _pOperation->property("layers").toList() != layers())
    return true;

  return false;
}

void PiiVisualTrainerWidget::acceptChanges()
{
  if (_pOperation)
    {
      if (isLayersChanged())
        _pOperation->setProperty("layers", layers());

      if (isLabelsChanged())
        _pOperation->setProperty("labels", labels());
    }
  
  checkState();
}

void PiiVisualTrainerWidget::checkState()
{
  setChanged(isLabelsChanged() || isLayersChanged());
}


void PiiVisualTrainerWidget::reset()
{
  setLayers(_pOperation != 0 ? _pOperation->property("layers").toList() : _lstPreviousLayers);
  setLabels(_pOperation != 0 ? _pOperation->property("labels").toList() : _lstPreviousLabels);

  _pSelector->updateView();
  checkState();
}

void PiiVisualTrainerWidget::setShowBufferingProperties(bool showBufferingProperties)
{
  if ( _pSelector )
    _pSelector->setShowBufferingProperties(showBufferingProperties);
}

bool PiiVisualTrainerWidget::showBufferingProperties() const
{
  if ( _pSelector )
    return _pSelector->showBufferingProperties();

  return false;
}

void PiiVisualTrainerWidget::openBufferingPropertiesDialog()
{
  if (_pOperation)
    {
      int oldBufferMode = _pOperation->property("bufferMode").toInt();
      int oldBufferSize = _pOperation->property("bufferSize").toInt();
      
      PiiBufferingPropertiesDialog dialog;
      dialog.setBufferMode(oldBufferMode);
      dialog.setBufferSize(oldBufferSize);
      
      if ( dialog.exec() == QDialog::Accepted )
        {
          int newBufferMode = dialog.bufferMode();
          int newBufferSize = dialog.bufferSize();
          
          if ( oldBufferMode != newBufferMode )
            _pOperation->setProperty("bufferMode", newBufferMode);
          
          if ( oldBufferSize != newBufferSize )
            _pOperation->setProperty("bufferSize", newBufferSize);
        }
    }
}
