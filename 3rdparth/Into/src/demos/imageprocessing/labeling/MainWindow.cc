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

#include "MainWindow.h"
#include <PiiImageViewport.h>

#include <QPen>

MainWindow::MainWindow(QWidget *parent) :
  DemoMainWindow(parent),
  _pSourceProbeInput(0),
  _pResultProbeInput(0),
  _pImageFileReader(0)
{
  // Initialize engine
  initEngine();
  
  // Initialize ui
  init();

  // Start processing
  startProcessing();
  emit selectImage(1);
}

void MainWindow::init()
{
  setupUi(this);
  setWindowTitle(demoName());

  _pSourceImageDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pResultImageDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  
  // Make ui-connections
  connect(_pBrowseImageButton, SIGNAL(clicked()), this, SLOT(selectImage()));

  connect(_pSourceProbeInput, SIGNAL(objectReceived(PiiVariant)), _pSourceImageDisplay, SLOT(setImage(PiiVariant)));
  connect(_pResultProbeInput, SIGNAL(objectReceived(PiiVariant)), _pResultImageDisplay, SLOT(setImage(PiiVariant)));

  // Update filename
  QStringList lstFileNames = _pImageFileReader->property("fileNames").toStringList();
  _pSourceImageName->setText(lstFileNames.size() > 0 ? lstFileNames[0] : "");

  // Configure image displays
  _pSourceImageDisplay->setProperty("displayType", "AutoScale");
  _pResultImageDisplay->setProperty("displayType", "AutoScale");
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine *pEngine = new PiiEngine;

  // Create trigger source
  PiiOperation *pTriggerSource = pEngine->createOperation("PiiTriggerSource");

  // Create probe input for source, thresholded and result image display
  _pSourceProbeInput = new PiiProbeInput;
  _pResultProbeInput = new PiiProbeInput;
  
  // Create image source
  _pImageFileReader = pEngine->createOperation("PiiImageFileReader");
  _pImageFileReader->setProperty("imageType", "Color");
  QString strImageName = QString("%1/intopii.jpg").arg(defaultImageDirPath());
  _pImageFileReader->setProperty("fileNames", strImageName);
  
  // Create thresholding, labeling and objectPropertyExtractor operations
  PiiOperation *pThresholdingOperation = pEngine->createOperation("PiiThresholdingOperation");
  pThresholdingOperation->setProperty("thresholdType", "OtsuThreshold");
  pThresholdingOperation->setProperty("inverse", true);
  
  PiiOperation *pLabelingOperation = pEngine->createOperation("PiiLabelingOperation");
  PiiOperation *pObjectPropertyExtractor = pEngine->createOperation("PiiObjectPropertyExtractor");

  PiiOperation *pImageAnnotator = pEngine->createOperation("PiiImageAnnotator");
  pImageAnnotator->setProperty("annotationType", "Rectangle");
  pImageAnnotator->setProperty("pen", QPen(Qt::red));
  
  // Make operation connections
  pTriggerSource->connectOutput("trigger", _pImageFileReader, "trigger");
  _pImageFileReader->connectOutput("image", pThresholdingOperation, "image");
  pThresholdingOperation->connectOutput("image", pLabelingOperation, "image");
  pLabelingOperation->connectOutput("image", pObjectPropertyExtractor, "image");
  pLabelingOperation->connectOutput("labels", pObjectPropertyExtractor, "labels");

  _pImageFileReader->connectOutput("image", pImageAnnotator, "image");
  pObjectPropertyExtractor->connectOutput("boundingboxes", pImageAnnotator, "annotation");
  
  _pSourceProbeInput->connectOutput(_pImageFileReader->output("image"));
  _pResultProbeInput->connectOutput(pImageAnnotator->output("image"));
  
  connect(this, SIGNAL(selectImage(int)), pTriggerSource, SLOT(trigger(int)));

  return pEngine;
}

void MainWindow::selectImage()
{
  QStringList lstFileNames = getImageFiles(QFileDialog::ExistingFile);

  if (lstFileNames.size() > 0)
    {
      // Pause processing when setting the new image
      pauseProcessing();
      
      // Set new image
      _pImageFileReader->setProperty("fileNames", lstFileNames);
      _pSourceImageName->setText(lstFileNames[0]);
      
      // Restart engine
      startProcessing();

      // Trig the image
      emit selectImage(1);
    }
}
