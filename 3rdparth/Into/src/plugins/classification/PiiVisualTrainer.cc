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

#include "PiiVisualTrainer.h"
#include <PiiYdinTypes.h>
#include <PiiQImageUtil.h>
#include <PiiUtil.h>
#include <PiiQImageUtil.h>
#include <PiiColor.h>
#include <PiiDefaultFlowController.h>
#include <PiiYdinUtil.h>

#include <QFileInfo>
#include <QtDebug>

using namespace std;
using namespace PiiYdin;
using namespace Pii;

// The number of non-movable inputs
#define STATIC_INPUT_COUNT 5

PiiVisualTrainer::Data::Data() :
  bufferMode(PiiVisualTrainer::BufferTotal), iBufferSize(0),
  dBufferFrequency(0), iBufferUpdateInterval(0),
  iMainImageIndex(0), iEnabledLayerSum(0),
  gridSize(10,10), iCellCount(100),
  bDisplayEnabled(true), bMainImageConnected(false),
  bLocationConnected(false), bSubImageConnected(false),
  bResetImageBuffers(false)
{
}

PiiVisualTrainer::PiiVisualTrainer() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  d->pMainImageInput = new PiiInputSocket("image");
  d->pSubImageInput = new PiiInputSocket("subimage");
  d->pLocationInput = new PiiInputSocket("location");
  d->pIndexInput = new PiiInputSocket("class index");
  d->pFileNameInput = new PiiInputSocket("filename");

  // sub-image and location must always work in sync
  d->pSubImageInput->setGroupId(1);
  d->pLocationInput->setGroupId(1);

  d->pMainImageInput->setOptional(true);
  d->pSubImageInput->setOptional(true);
  d->pLocationInput->setOptional(true);
  d->pFileNameInput->setOptional(true);

  addSocket(d->pMainImageInput);
  addSocket(d->pSubImageInput);
  addSocket(d->pLocationInput);
  addSocket(d->pIndexInput);
  addSocket(d->pFileNameInput);

  setSizeOfBuffList();
}

QVariant PiiVisualTrainer::socketProperty(PiiAbstractSocket* socket, const char* name) const
{
  const PII_D;

  if (!PiiYdin::isNameProperty(name))
    {
      // Searh layers. We must notice enabled/disabled layers
      int iOutputIndex = 0;
      for (int i=0; i<d->lstLayers.size(); i++)
        {
          QMap<QString, QVariant> map = d->lstLayers[i].toMap();
          
          // Check layer-map if it is not disabled
          if (!map.value("disabled", false).toBool())
            {
              if (socket == outputAt(iOutputIndex))
                {
                  return map.value(QString(name) == "displayName" ? "name" : name, QVariant());
                  iOutputIndex++;
                }
            }
        }
    }
  
  return PiiDefaultOperation::socketProperty(socket, name);
}


void PiiVisualTrainer::setBufferMode(PiiVisualTrainer::BufferMode mode)
{
  PII_D;
  // Clear the buffer first.
  removeFromBuffer(0);
  d->bufferMode = mode;
  setSizeOfBuffList();
}

void PiiVisualTrainer::setBufferSize(int size)
{
  PII_D;
  // If the new size is smaller than our current buffer size, we need
  // to remove some images.
  removeFromBuffer(size);
  d->iBufferSize = size;
}


void PiiVisualTrainer::setBufferFrequency(double frequency)
{
  PII_D;
  d->dBufferFrequency = frequency;
  if (d->dBufferFrequency == 0)
    d->iBufferUpdateInterval = 0;
  else
    d->iBufferUpdateInterval = (int)(1.0/frequency*1000.0);
}

void PiiVisualTrainer::setGridSize(const QSize& size)
{
  PII_D;
  if ( size != d->gridSize )
    {
      d->gridSize = size;
      d->iCellCount = size.width() * size.height();
      // Clear the buffer.
      removeFromBuffer(0);
      // Resize the list of buffers.
      setSizeOfBuffList();

      for (int i=STATIC_INPUT_COUNT; i<inputCount(); ++i)
        static_cast<PiiVisualTrainerInputSocket*>(inputAt(i))->setGridSize(size);

      int labelsSize = d->lstLabels.size();
      
      //clear labels
      d->lstLabels.clear();

      //initialize labels to zero
      for ( int i=0; i<labelsSize; i++)
        d->lstLabels << QVector<double>(d->gridSize.width() * d->gridSize.height(), 0.0);

      emit gridSizeChanged(size);
      emit labelsChanged(labels());
    }
}

QSize PiiVisualTrainer::gridSize() const
{
  const PII_D;
  return d->gridSize;
}


void PiiVisualTrainer::check(bool reset)
{
  PII_D;


  /* PENDING
   * Propertyt pitäisi asettaa voimaan ja tarkistaa vasta täällä!
   *
   * bDisplayEnabled -flagi (tms.) pitää asettaa falseksi, jos
   * operaatioon ei ole kytketty UI:ta. Koska operaatio ei enää itse
   * luo UI:ta, pitää tutkia, onko signaaleihin kytketty slotteja.
   * Aiemmin käytettiin iOpenConfigurators -muuttujaa.
   */
  
  if (reset)
    d->bResetImageBuffers = true;
  
  if (d->pSubImageInput->isConnected() && !d->pLocationInput->isConnected())
    PII_THROW(PiiExecutionException, tr("If %1 is connected also %2 must be connected.")
              .arg("subimage").arg("location"));
  
  if (!d->pMainImageInput->isConnected() && d->pLocationInput->isConnected())
    PII_THROW(PiiExecutionException, tr("If %1 is connected, %2 must also be connected.")
              .arg("location").arg("image"));
  if (d->lstLabels.size() != d->lstLayers.size() )
    {
      PII_THROW(PiiExecutionException, tr("The size of labels (%1) must be same as size of layers (%2)")
                .arg(d->lstLabels.size()).arg(d->lstLayers.size()));
    }

  for ( int i=0; i<d->lstLabels.size(); i++)
    {
      int cells = d->gridSize.width() * d->gridSize.height();
      if ( d->lstLabels[i].size() != cells )
        {
          PII_THROW(PiiExecutionException, tr("The size of labels[%1] (%2) must be same as size of the cells (%3).")
                    .arg(i).arg(d->lstLabels[i].size()).arg(cells));
        }
    }
  
  // If sub-images are input, each must be accompanied with a class
  // index, and all outputs respond to incoming sub-images.
  // If not, each "large image" must have a class index, and the
  // outputs respond to them.
  int groupId = d->pLocationInput->isConnected() ? 1 : 0;

  d->pIndexInput->setGroupId(groupId);
  for (int i=outputCount(); i--; )
    outputAt(i)->setGroupId(groupId);

  // The label inputs work in sync with the index input
  for (int i=STATIC_INPUT_COUNT; i<inputCount(); ++i)
    inputAt(i)->setGroupId(groupId);

  // Clear collected values from input sockets
  if (reset)
    {
      for (int i=STATIC_INPUT_COUNT; i<inputCount(); ++i)
        static_cast<PiiVisualTrainerInputSocket*>(inputAt(i))->resetValues();
      d->varMainImage = PiiVariant();
      d->lstBufferedLocations.clear();
    }

  d->bMainImageConnected = d->pMainImageInput->isConnected();
  d->bLocationConnected = d->pLocationInput->isConnected();
  d->bSubImageConnected = d->pSubImageInput->isConnected();
  d->bFileNameConnected = d->pFileNameInput->isConnected();
  
  PiiDefaultOperation::check(reset);
}

PiiFlowController* PiiVisualTrainer::createFlowController()
{
  PII_D;
  // If main image is connected, we need to assign a strict
  // relationsip between groups 0 and 1.
  if (d->bMainImageConnected)
    {
      PiiDefaultFlowController::RelationList lstRelations;
      lstRelations << PiiDefaultFlowController::strictRelation(0, 1);
      return new PiiDefaultFlowController(inputSockets(), outputSockets(), lstRelations);
    }
  // Otherwise, the default stuff is ok.
  else
    return PiiDefaultOperation::createFlowController();
}

void PiiVisualTrainer::process()
{
  PII_D;
  int classIndex = -1;
  if (activeInputGroup() == d->pIndexInput->groupId())
    {
      classIndex = PiiYdin::convertPrimitiveTo<int>(d->pIndexInput);
      if (classIndex > d->iCellCount)
        PII_THROW(PiiExecutionException, tr("Class index (%1) out of range (%2-%3).").arg(classIndex).arg(0).arg(d->iCellCount-1));
      // Emit labels for each layer
      for (int i=0; i<outputCount(); ++i)
        {
          int layerIndex = d->lstEnabledLayerIndex[i];
          if (layerIndex < d->lstLabels.size() && classIndex >= 0 && classIndex < d->lstLabels[layerIndex].size())
            {
              outputAt(i)->emitObject(d->lstLabels[layerIndex][classIndex]);
            }
          else
            {
              outputAt(i)->emitObject(0.0);
            }

          // Check for training values
          if (!inputAt(i+STATIC_INPUT_COUNT)->isEmpty())
            {
              PiiVariant labelObj = inputAt(i+STATIC_INPUT_COUNT)->firstObject();
              double label;
              switch (labelObj.type())
                {
                  PII_PRIMITIVE_CASES(label = (double)PiiYdin::primitiveAs, labelObj);
                default:
                  PII_THROW_UNKNOWN_TYPE(inputAt(i+STATIC_INPUT_COUNT));
                }
              static_cast<PiiVisualTrainerInputSocket*>(inputAt(i+STATIC_INPUT_COUNT))->storeValue(classIndex, label);
            }
        }
    }
  
  // We only handle images if display is enabled and at least one
  // configuration dialog is open.
  if (d->bDisplayEnabled && d->bMainImageConnected)
    {
      if (activeInputGroup() == 0)
        {
          PiiVariant mainImage = d->pMainImageInput->firstObject();
          QImagePtr pImage(0);
          // A new main image has arrived. Create a QImage out of it.
          switch (mainImage.type())
            {
              PII_ALL_IMAGE_CASES(pImage = PiiYdin::createQImage, mainImage);
            default:
              PII_THROW_UNKNOWN_TYPE(d->pMainImageInput);
            }

          // If we were given a file name, annotate the image with it.
          if (d->bFileNameConnected)
            {
              PiiVariant fileName = d->pFileNameInput->firstObject();
              if (fileName.type() == PiiYdin::QStringType)
                pImage->setText("pii_filename", 0, fileName.valueAs<QString>());
            }

          // If we are given both large images and pieces, this
          // changes the main (large) image.
          if (d->bLocationConnected)
            {
              // Store main image in an internal variable and set a
              // unique ID for the visual selector.
              d->varMainImage = mainImage;
              d->iMainImageIndex++;
              pImage->setText("pii_id", 0, QString::number(d->iMainImageIndex, 16));

              // If buffer size is non-positive, all images belonging to a
              // single main image are buffered. A new main image causes
              // the removal of all old sub-images.
              if (d->iBufferSize <= 0)
                {
                  emit allSubImagesRemoved();
                  clearBuffers();
                }

              // Inform the visualizer
              emit mainImageChanged(pImage);

              //send all buffered sub-image locations
              while (d->lstBufferedLocations.size() > 0)
                {
                  QPair<PiiMatrix<int>, int> pair = d->lstBufferedLocations.takeFirst();
                  storeSubImage(mainImage, pair.first, pair.second);
                }
            }
          // If no sub-images are given, we put the image right into
          // the selector's grid.
          else
            {
              //if we must reset the display, all old images will be removed
              if (d->bResetImageBuffers)
                {
                  emit allSubImagesRemoved();
                  clearBuffers();
                  d->bResetImageBuffers = false;
                }

              int maxImageSize = 128;
              // Check if we must scale this image
              if (qMax(pImage->width(), pImage->height()) > maxImageSize)
                {
                  // Scale image and copy all text keys from an old image
                  QImagePtr pNewQImage(new QImage(pImage->scaled(maxImageSize, maxImageSize, Qt::KeepAspectRatio)));
                  QStringList keys = pImage->textKeys();
                  for (int i=0; i<keys.size(); i++)
                    pNewQImage->setText(keys[i], pImage->text(keys[i]));

                  // Add new key, which tell that this image has scaled
                  pNewQImage->setText("pii_scaled", "yes");
                  pNewQImage->setText("pii_imagesize", QString("%1x%2").arg(pImage->width()).arg(pImage->height()));
                  
                  // Store new image
                  storeSubImage(pNewQImage, classIndex);
                }
              else // Only store the image
                storeSubImage(pImage, classIndex);
            }
        }
      else
        {
          PiiVariant location = d->pLocationInput->firstObject();
          // Check the validity of the location input
          if (location.type() != PiiYdin::IntMatrixType)
            PII_THROW_UNKNOWN_TYPE(d->pLocationInput);
          
          const PiiMatrix<int> locationMatrix = location.valueAs<PiiMatrix<int> >();
          if (locationMatrix.rows() != 1 ||
              locationMatrix.columns() != 4)
            PII_THROW_WRONG_SIZE(d->pLocationInput, locationMatrix, 1, 4);

          if (d->bSubImageConnected)
            {
              PiiVariant subImage = d->pSubImageInput->firstObject();
              
              QImagePtr pImage(0);
              // Create a sub-image
              switch (subImage.type())
                {
                  PII_ALL_IMAGE_CASES(pImage = PiiYdin::createQImage, subImage);
                default:
                  PII_THROW_UNKNOWN_TYPE(d->pSubImageInput);
                }
              updateInformation(pImage, locationMatrix);
              
              // Store the sub-image
              storeSubImage(pImage, classIndex);
            }
          else
            {
              //save the sub-image location or send it
              if (!d->varMainImage.isValid())
                d->lstBufferedLocations << qMakePair(locationMatrix, classIndex);
              else
                storeSubImage(d->varMainImage, locationMatrix, classIndex);
            }
        }
    }
}

void PiiVisualTrainer::syncEvent(SyncEvent* event)
{
  PII_D;
  if (//d->iOpenConfigurators > 0 &&
      d->bMainImageConnected &&
      d->bLocationConnected &&
      event->type() == SyncEvent::EndInput &&
      event->groupId() == d->pMainImageInput->groupId())
    {
      emit allSubImagesArrived();
      d->varMainImage = PiiVariant();
      d->lstBufferedLocations.clear();
    }
}

void PiiVisualTrainer::updateInformation(QImagePtr image, const PiiMatrix<int>& location)
{
  PII_D;
  // Tell the visualizer that this image belongs to the
  // current main image.
  image->setText("pii_part_of", 0, QString::number(d->iMainImageIndex, 16));
  image->setOffset(QPoint(location(0,0), location(0,1)));
}

template <class T> QImage* PiiVisualTrainer::subMatrix(const PiiVariant& bigImage, const PiiMatrix<int>& location)
{
  const PiiMatrix<T> matImage = bigImage.valueAs<PiiMatrix<T> >();

  // Check the limits
  if ((location(0,0) < 0 || location(0,1) < 0) ||
      (location(0,0) + location(0,2) > matImage.columns()) ||
      (location(0,1) + location(0,3) > matImage.rows()))
    PII_THROW(PiiExecutionException, tr("Location of the sub-image is the outside of the main image area."));
  
  return Pii::createQImage<T>(matImage(location(0,1), location(0,0), location(0,3), location(0,2)));
}


void PiiVisualTrainer::storeSubImage(const PiiVariant& bigImage, const PiiMatrix<int>& location, int classIndex)
{
  PII_D;
  QImagePtr pImage;
  switch (bigImage.type())
    {
      PII_ALL_IMAGE_CASES_M(pImage = subMatrix, (bigImage, location));
    default:
      throw PiiExecutionException(tr("An object of an unknown type (0x%1) was received in \"%2\" input.")
                                  .arg(bigImage.type(), 0, 16)
                                  .arg(d->pMainImageInput->objectName()));
        //PII_THROW_UNKNOWN_TYPE(d->pMainImageInput);
    }
  updateInformation(pImage, location);

  // Store the sub-image
  storeSubImage(pImage, classIndex);
}

void PiiVisualTrainer::storeSubImage(QImagePtr image, int classIndex)
{
  PII_D;
  // Store the sub-image to the internal buffer. The way how the
  // buffer works, depends on the current buffer mode. If the buffer
  // mode is BufferTotal, all the images are stored in index 0 of d->lstBuffer.
  // If the buffer mode is BufferPerCell, there is own buffer for each
  // classIndex, and the image is stored in index [classIndex] in the
  // same list. This function also takes into account the bufferFrequency
  // property. Only at most d->dBufferFrequency images are added in a buffer
  // per second.

  // -1 denotes invalid class. Such images have no place on the grid.
  if (classIndex < 0 || classIndex >= d->iCellCount)
    return;

  int buffIndex;
  if (d->bufferMode == PiiVisualTrainer::BufferTotal)
    buffIndex = 0;
  else
    buffIndex = classIndex;

  // Check, if it has been enough time elapsed that adding an image to
  // the buffer is possible.
  // If the d->iBufferUpdateInterval is 0 or negative, the elapsed time
  // of the QTime objects are not checked, nor they are restarted.
  bool bAddImage = true;
  if (d->iBufferUpdateInterval > 0)
    {
      int elapsed = d->lstTimes[buffIndex].elapsed();
      if (d->iBufferUpdateInterval > elapsed)
        bAddImage = false;
      else
        d->lstTimes[buffIndex].restart();
    }
  
  //If we find this image from the buffer, we must remove it first
  for (int i=0; i<d->lstBuffers.size(); i++)
    {
      if (d->lstBuffers[i].contains(image))
        d->lstBuffers[i].removeAll(image);
    }
  
  if (bAddImage)
    {
      d->lstBuffers[buffIndex] << image;

      // Did we exceed its capacity?
      if (d->iBufferSize > 0 && d->lstBuffers[buffIndex].size() > d->iBufferSize)
        {
          // YES -> remove from buffer, inform the selector and
          // delete the image (auto-delete).
          emit subImageRemoved(d->lstBuffers[buffIndex].takeFirst());
        }

      emit subImageAdded(image, classIndex % d->gridSize.width(), classIndex / d->gridSize.width());
    }
}

/* This private function removes images from the buffer, starting
   from the index startFrom. It also emits a singal about each removed image. */
void PiiVisualTrainer::removeFromBuffer(int startFrom)
{
  PII_D;
  // If the buffer mode == BufferTotal, the size of the d->lstBuffers is
  // assumed to be one. Else (buffer mode == BufferPerCell), the size of
  // d->lstBuffer should equal to height*width (== maximum classIndex) of the som.
  
  for (int i = 0; i<d->lstBuffers.size(); i++)
    while (d->lstBuffers[i].size() > startFrom)
      emit subImageRemoved(d->lstBuffers[i].takeFirst());
}

/* This private function clears the image buffer, without emitting
   signals about removed images. */
void PiiVisualTrainer::clearBuffers()
{
  PII_D;
  
  // If the buffer mode == BufferTotal, the size of the d->lstBuffers is
  // assumed to be one. Else (buffer mode == PerCell), the size of
  // d->lstBuffer should equal to height*width (== maximum classIndex)
  // of the som.
  
  for (int i = 0; i<d->lstBuffers.size(); i++)
    d->lstBuffers[i].clear();
}

/* Resize the list of buffers */
void PiiVisualTrainer::setSizeOfBuffList()
{
  PII_D;

  if (d->bufferMode == PiiVisualTrainer::BufferTotal)
    {
      d->lstBuffers.resize(1);
      d->lstTimes.resize(1);
    }
  else
    {
      d->lstBuffers.resize(d->gridSize.width() * d->gridSize.height());
      d->lstTimes.resize(d->gridSize.width() * d->gridSize.height());
    }

  QTime time;
  time.start();
  d->lstTimes.fill(time);
}

void PiiVisualTrainer::setLayers(const QList<QVariant>& layers)
{
  PII_D;
  QList<int> oldLayerIds = layerIds();
  QStringList oldLayerNames = layerNames();
  QVector<QVector<double> > oldLabels = d->lstLabels;
  QList<QVariant> oldLayers = d->lstLayers;

  QVector<QVector<double> > lstNewLabels;
  d->lstLayers.clear();
  d->lstEnabledLayerIndex.clear();
  d->iEnabledLayerSum = 0;

  //init max id
  int maxId = -1;
  for (int i=0; i<layers.size(); i++)
    {
      int id = layers[i].toMap().value("id",-1).toInt();
      if (id > maxId)
        maxId = id;
    }
  
  //checking contents of the all layers
  for (int i=0; i<layers.size(); i++)
    {
      QMap<QString,QVariant> map = layers[i].toMap();

      if (map.isEmpty())
        map["type"] = "float";
      
      if (!map.contains("id"))
        {
          maxId++;
          map["id"] = maxId;
        }
      
      if (!map.contains("name"))
        map["name"] = tr("Anonymous");

      if (!map.contains("disabled"))
        map["disabled"] = false;

      if (!map.contains("color0") )
        map["color0"] = QColor(QColor(170,255,255));
      
      if (!map.contains("color1") )
        map["color1"] = QColor(Qt::red);

      if (!map.contains("classNames"))
        {
          map["classNames"] = QStringList() << "Unassigned" << "Good" << "Poor";
          map["classColors"] = QList<QVariant>() << QColor(Qt::gray) << QColor(Qt::green) << QColor(Qt::red);
          map["classIndices"] = QList<QVariant>() << 0 << 1 << 2;
        }
      else if (!map.contains("classIndices"))
        {
          QList<QVariant> classIndices;
          QStringList names = map["classNames"].toStringList();
          for (int i=0; i<names.size(); i++)
            classIndices << i;

          map["classIndices"] = classIndices;
        }

      if (!map.contains("min"))
        map["min"] = 0.0;
      if (!map.contains("max"))
        map["max"] = 1.0;
      if (!map.contains("resolution"))
        map["resolution"] = 0.1;
      if (!map.contains("startColor"))
        map["startColor"] = QColor(QColor(170,255,255));
      if (!map.contains("endColor"))
        map["endColor"] = QColor(Qt::red);

      PiiVisualTrainerInputSocket::LayerType layerType = PiiVisualTrainerInputSocket::FloatLayer;
      
      if (map["type"] == "class")
        {
          map["min"] = 0;
          map["max"] = map["classNames"].toStringList().size() - 1;
          map["resolution"] = 1;
          layerType = PiiVisualTrainerInputSocket::ClassLayer;
        }
      else if (map["type"] == "physical")
        {
          map["min"] = 0;
          map["max"] = 1;
          map["resolution"] = 1;
          layerType = PiiVisualTrainerInputSocket::ClassLayer;
        }
      else
        map["type"] = "float";

      if (!map["disabled"].toBool())
        {
          createInput(layerType);
          createOutput(map);
          d->iEnabledLayerSum++;
          d->lstEnabledLayerIndex.append(i);
        }

      int layerIndex = -1;
      if (map.contains("id"))
        layerIndex = oldLayerIds.indexOf(map["id"].toInt());

      if (layerIndex == -1)
        layerIndex = oldLayerNames.indexOf(map["name"].toString());
      
      QVector<double> labelValues;

      if (layerIndex > -1)
        {
          QVector<double> oldLabelValues = oldLabels[layerIndex];
          labelValues = oldLabelValues;
          
          /*
           * Update labels depends on global class indices or class
           * names when the layer type is "class". If the map doesn't
           * contains classIndices or classNames and there is changes
           * in classes, we must guess the new label values.
           */
          QList<int> lstSwaps;
          if (map["type"] == "class")
            {
              if (oldLayers[layerIndex].toMap().contains("classIndices") && map.contains("classIndices") &&
                  (map["classIndices"].toList().size() == map.value("classNames", QStringList()).toStringList().size()))
                lstSwaps = swapList(oldLayers[layerIndex].toMap()["classIndices"].toList(), map["classIndices"].toList());
              else if (oldLayers[layerIndex].toMap().contains("classNames") && map.contains("classNames"))
                lstSwaps = swapList(oldLayers[layerIndex].toMap()["classNames"].toList(), map["classNames"].toList());
            }

          if (lstSwaps.size() > 0)
            {
              //update label values depends on swapList
              for ( int i=0; i<oldLabelValues.size(); i++)
                {
                  int newValue = lstSwaps[static_cast<int>(oldLabelValues[i])];
                  labelValues[i] = static_cast<double>(newValue > -1 ? newValue : 0);
                }
            }
          else
            {
              /*
               * The map doesn't contains classIndices or classNames.
               * If there is changes in "class"-type layers, the
               * labels will go to the wrong values. Sorry, no bonus.
               */
              for ( int g=0; g<labelValues.size(); g++ )
                {
                  if ( labelValues[g] < map["min"].toDouble() )
                    labelValues[g] = map["min"].toDouble();
                  else if ( labelValues[g] > map["max"].toDouble() )
                    labelValues[g] = map["max"].toDouble();
                }
            }
        }
      else
        labelValues = QVector<double>(d->gridSize.width() * d->gridSize.height(),map["min"].toDouble());
      
      lstNewLabels << labelValues;      
      d->lstLayers << map;
            
    }

  
  // Delete extra outputs
  while (outputCount() > d->iEnabledLayerSum)
    delete d->lstOutputs.takeLast();

  emit layersChanged(layers);
  setLabels(lstNewLabels);
}

QList<int> PiiVisualTrainer::swapList(const QVariantList& oldList, const QVariantList& newList)
{
  QList<int> swapList;
  for ( int i=0; i<oldList.size(); i++)
    swapList << newList.indexOf(oldList[i]);
  
  return swapList;
}

QList<QVariant> PiiVisualTrainer::layers() const
{
  return _d()->lstLayers;
}

QList<int> PiiVisualTrainer::layerIds() const
{
  const PII_D;
  QList<int> result;
  for (int i=0; i<d->lstLayers.size(); i++)
    {
      if (d->lstLayers[i].toMap().contains("id"))
        result << d->lstLayers[i].toMap()["id"].toInt();
    }
  return result;
}

QStringList PiiVisualTrainer::layerNames() const
{
  const PII_D;
  QStringList result;
  for (int i=0; i<d->lstLayers.size(); i++)
    result << d->lstLayers[i].toMap()["name"].toString();
  return result;
}

QStringList PiiVisualTrainer::enabledLayerNames() const
{
  const PII_D;
  QStringList result;
  for (int i=0; i<d->lstLayers.size(); i++)
    if ( !d->lstLayers[i].toMap()["disabled"].toBool() )
      result << d->lstLayers[i].toMap()["name"].toString();
  return result;
}

void PiiVisualTrainer::setLabels(const QVector<QVector<double> >& newLabels)
{
  PII_D;
  d->lstLabels.clear();
  d->lstLabels = newLabels;

  emit labelsChanged(labels());
}

void PiiVisualTrainer::setLabels(const QVariantList& labels)
{
  QVector<QVector<double> > lstLabels;
  for (int i=0; i<labels.size(); ++i)
    lstLabels << Pii::variantsToVector<double>(labels[i].toList());

  setLabels(lstLabels);
}

QVariantList PiiVisualTrainer::labels() const
{
  const PII_D;
  QVariantList labels;
  for (int i=0; i<d->lstLabels.size(); ++i)
    labels << QVariant(Pii::vectorToVariants(d->lstLabels[i]));
  return labels;
}

QVariantList PiiVisualTrainer::suggestedLabels() const
{
  const PII_D;
  QVariantList result;
  // Get the suggested labels from each label input
  for (int i=STATIC_INPUT_COUNT; i<inputCount(); i++)
    {
      QVariantList layerLabels;
      for (int c=0; c<d->iCellCount; ++c)
        layerLabels << static_cast<PiiVisualTrainerInputSocket*>(inputAt(i))->label(c);
      result << QVariant(layerLabels);
    }
  return result;
}

void PiiVisualTrainer::createInput(PiiVisualTrainerInputSocket::LayerType type)
{
  PII_D;
  PiiVisualTrainerInputSocket* socket;
  if (inputCount() - STATIC_INPUT_COUNT <= d->iEnabledLayerSum)
    {
      socket = new PiiVisualTrainerInputSocket(QString("label%1").arg(inputCount() - STATIC_INPUT_COUNT));
      socket->setOptional(true);
      socket->setGridSize(d->gridSize);
      addSocket(socket);
    }
  else
    socket = static_cast<PiiVisualTrainerInputSocket*>(inputAt(d->iEnabledLayerSum + STATIC_INPUT_COUNT));

  socket->setLayerType(type);
}

void PiiVisualTrainer::createOutput(const QVariantMap& properties)
{
  PII_D;
  PiiOutputSocket* socket;
  if (outputCount() <= d->iEnabledLayerSum)
    {
      socket = new PiiOutputSocket(QString("label%1").arg(outputCount()));
      //socket->setDisplayName(properties["name"].toString());
      socket->setProperty("min", properties["min"]);
      socket->setProperty("max", properties["max"]);
      socket->setProperty("resolution", properties["resolution"]);
      addSocket(socket);
      socket->synchronizeTo(d->pIndexInput);
    }
  else
    socket = outputAt(d->iEnabledLayerSum);
}

PiiVisualTrainer* PiiVisualTrainer::clone() const
{
  const PII_D;
  
  // First create the default clone
  PiiVisualTrainer *pResult = static_cast<PiiVisualTrainer*>(PiiOperation::clone());

  if (pResult != 0)
    {
      //fix the labels
      pResult->_d()->lstLabels = d->lstLabels;
    }
  
  return pResult;
}

PiiVisualTrainer::BufferMode PiiVisualTrainer::bufferMode() const { return _d()->bufferMode; }
int PiiVisualTrainer::bufferSize() const { return _d()->iBufferSize; }
double PiiVisualTrainer::bufferFrequency() const { return _d()->dBufferFrequency; }
bool PiiVisualTrainer::displayEnabled() const { return _d()->bDisplayEnabled; }
void PiiVisualTrainer::setDisplayEnabled(bool enabled) { _d()->bDisplayEnabled = enabled; }
void PiiVisualTrainer::setLabelTable(const QVector<QVector<double> >& labels) { _d()->lstLabels = labels; }
QVector<QVector<double> > PiiVisualTrainer::labelTable() const { return _d()->lstLabels; }
int PiiVisualTrainer::layerCount() const { return _d()->lstLayers.size(); }
