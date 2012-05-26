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

MainWindow::MainWindow(QWidget *parent) :
  DemoMainWindow(parent),
  _pSourceProbeInput(0),
  _pThresholdedProbeInput(0),
  _pResultProbeInput(0),
  _pImageFileReader(0),
  _pThresholdingOperation(0),
  _pMorphologyOperation(0)
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
  _pThresholdedImageDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pResultImageDisplay->imageViewport()->setProperty("fitMode", "FitToView");

  // Make ui-connections
  connect(_pBrowseImageButton, SIGNAL(clicked()), this, SLOT(selectImage()));
  connect(_pThreshold, SIGNAL(valueChanged(int)), this, SLOT(setThreshold(int)));
  connect(_pInverseThreshold, SIGNAL(toggled(bool)), this, SLOT(setInverseThreshold(bool)));
  connect(_pMorphologyTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setMorphologyType(int)));
  connect(_pMorphologySize, SIGNAL(valueChanged(int)), this, SLOT(setMorphologySize(int)));

  connect(_pSourceProbeInput, SIGNAL(objectReceived(PiiVariant)), _pSourceImageDisplay, SLOT(setImage(PiiVariant)));
  connect(_pThresholdedProbeInput, SIGNAL(objectReceived(PiiVariant)), _pThresholdedImageDisplay, SLOT(setImage(PiiVariant)));
  connect(_pResultProbeInput, SIGNAL(objectReceived(PiiVariant)), _pResultImageDisplay, SLOT(setImage(PiiVariant)));

  // Update filename
  QStringList lstFileNames = _pImageFileReader->property("fileNames").toStringList();
  _pSourceImageName->setText(lstFileNames.size() > 0 ? lstFileNames[0] : "");

  // Configure image displays
  _pThresholdedImageDisplay->setProperty("displayType", "AutoScale");
  _pResultImageDisplay->setProperty("displayType", "AutoScale");

  // Init threshold
  _pThresholdingOperation->setProperty("absoluteThreshold", _pThreshold->value());

}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine *pEngine = new PiiEngine;

  // Create trigger sources
  PiiOperation *pReadImageTriggerSource = pEngine->createOperation("PiiTriggerSource", "readImageTriggerSource");
  connect(this, SIGNAL(selectImage(int)), pReadImageTriggerSource, SLOT(trigger(int)));

  PiiOperation *pUpdateImageTriggerSource = pEngine->createOperation("PiiTriggerSource", "updateImageTriggerSource");
  connect(this, SIGNAL(updateImage(int)), pUpdateImageTriggerSource, SLOT(trigger(int)));

  PiiOperation *pSwitch = pEngine->createOperation("PiiSwitch", "switch");
  pSwitch->setProperty("operationMode", "AsynchronousMode");
  pSwitch->setProperty("passThrough", true);

  // Create probe input for source, thresholded and result image display
  _pSourceProbeInput = new PiiProbeInput;
  _pThresholdedProbeInput = new PiiProbeInput;
  _pResultProbeInput = new PiiProbeInput;
  
  // Create image source
  _pImageFileReader = pEngine->createOperation("PiiImageFileReader");
  _pImageFileReader->setProperty("imageType", "Color");
  QString strImageName = QString("%1/olga.jpg").arg(defaultImageDirPath());
  _pImageFileReader->setProperty("fileNames", strImageName);
  
  // Create thresholding and morphology operations
  _pThresholdingOperation = pEngine->createOperation("PiiThresholdingOperation");
  _pThresholdingOperation->setProperty("thresholdType", "StaticThreshold");
  _pThresholdingOperation->setProperty("inverse", true);
  
  _pMorphologyOperation = pEngine->createOperation("PiiMorphologyOperation");
  _pMorphologyOperation->setProperty("maskType", "Rectangular");
  _pMorphologyOperation->setProperty("type", "Erode");
  _pMorphologyOperation->setProperty("maskSize", QSize(3,3));

  // Make operation connections
  pReadImageTriggerSource->connectOutput("trigger", _pImageFileReader, "trigger");
  _pImageFileReader->connectOutput("image", pSwitch, "input");
  pSwitch->connectOutput("output", _pThresholdingOperation, "image");
  pUpdateImageTriggerSource->connectOutput("trigger", pSwitch, "trigger");
  _pThresholdingOperation->connectOutput("image", _pMorphologyOperation, "image");
  
  _pSourceProbeInput->connectOutput(_pImageFileReader->output("image"));
  _pThresholdedProbeInput->connectOutput(_pThresholdingOperation->output("image"));
  _pResultProbeInput->connectOutput(_pMorphologyOperation->output("image"));
  
  return pEngine;
}

void MainWindow::setThreshold(int th)
{
  // Change absolute threshold
  _pThresholdingOperation->setProperty("absoluteThreshold", th);
  
  // Trig the same image again
  emit updateImage(0);
}

void MainWindow::setInverseThreshold(bool inverse)
{
  // Change the inverse-property
  _pThresholdingOperation->setProperty("inverse", inverse);
  
  // Trig the same image again
  emit updateImage(0);
}

void MainWindow::setMorphologyType(int type)
{
  // Change type of the morphology
  _pMorphologyOperation->setProperty("operation", type);

  // Trig the same image again
  emit updateImage(0);
}

void MainWindow::setMorphologySize(int size)
{
  // Change size of the moprhology
  _pMorphologyOperation->setProperty("maskSize", QSize(size,size));

  // Trig the same image again
  emit updateImage(0);
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
