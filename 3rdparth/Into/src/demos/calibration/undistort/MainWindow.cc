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

MainWindow::MainWindow(QWidget *parent) :
  DemoMainWindow(parent),
  _pProbeInput1(0),
  _pProbeInput2(0)
{
  // Initialize engine
  initEngine();

  // Initialize ui
  init();

  updateButtonStates(PiiOperation::Stopped);
}

void MainWindow::updateButtonStates(int state)
{
  _pStopButton->setEnabled(state != PiiOperation::Stopped);
  _pStartButton->setEnabled(state == PiiOperation::Stopped);
  
  _pRadioImageFile->setEnabled(state == PiiOperation::Stopped);
  _pImageFile->setEnabled(state == PiiOperation::Stopped);
  _pRadioImageUrl->setEnabled(state == PiiOperation::Stopped);
  _pImageUrlCombo->setEnabled(state == PiiOperation::Stopped);
  
  _pDistortionSlider->setEnabled(state != PiiOperation::Stopped);
  _pspnFocalLength->setEnabled(state != PiiOperation::Stopped);
}

void MainWindow::init()
{
  setupUi(this);
  setWindowTitle(demoName());

  // Make ui-connections
  connect(_pStartButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
  connect(_pStopButton, SIGNAL(clicked()), this, SLOT(stopProcessing()));
  connect(_pImageFile, SIGNAL(clicked()), this, SLOT(selectImageFile()));

  connect(_pProbeInput1, SIGNAL(objectReceived(PiiVariant)), _pImageDisplay1, SLOT(setImage(PiiVariant)));
  connect(_pProbeInput2, SIGNAL(objectReceived(PiiVariant)), _pImageDisplay2, SLOT(setImage(PiiVariant)));

  
  // Init button states
  updateButtonStates(engine()->state());

  _pDistortionSlider->setValue(int(_pUndistortOperation->property("radial2nd").toDouble()*100));
  connect(_pDistortionSlider, SIGNAL(valueChanged(int)), this, SLOT(changeDistortion(int)));

  connect(_pspnFocalLength, SIGNAL(valueChanged(int)), this, SLOT(changeFocalLength(int)));
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine* pEngine = new PiiEngine;
  connect(pEngine, SIGNAL(stateChanged(int)), this, SLOT(updateButtonStates(int)));
  
  // Create probe inputs for image display
  _pProbeInput1 = new PiiProbeInput;
  _pProbeInput2 = new PiiProbeInput;
  
  _pUndistortOperation = pEngine->createOperation("PiiUndistortOperation", "undistortOperation");
  _pUndistortOperation->setProperty("radial2nd", -0.3);
  _pUndistortOperation->setProperty("interpolation", "Pii::NearestNeighborInterpolation");

  static_cast<PiiInputSocket*>(_pUndistortOperation->input("image"))->setQueueCapacity(2);
  _pProbeInput2->connectOutput(_pUndistortOperation->output("image"));

  createImageReaderOperations(pEngine);
                                  
  return pEngine;
}

void MainWindow::createImageReaderOperations(PiiEngine *engine)
{
  // Create image reader operations
  PiiOperation *pImageTrigger = engine->createOperation("PiiTriggerSource", "imageTrigger");
  PiiOperation *pImageFileReader = engine->createOperation("PiiImageFileReader", "imageFileReader");
  PiiOperation *pSwitchTrigger = engine->createOperation("PiiTriggerSource", "switchTrigger");
  PiiOperation *pSwitch = engine->createOperation("PiiSwitch", "switch");
  pSwitch->setProperty("dynamicInputCount", 1);
  pSwitch->setProperty("operationMode", "AsynchronousMode");
  pSwitch->setProperty("passThrough", true);

  pImageTrigger->connectOutput("trigger", pImageFileReader, "trigger");
  pImageFileReader->connectOutput("image", pSwitch, "input0");
  pSwitchTrigger->connectOutput("trigger", pSwitch, "trigger");
  pSwitch->connectOutput("output0", _pUndistortOperation, "image");
  _pProbeInput1->connectOutput(pImageFileReader->output("image"));

  connect(this, SIGNAL(readImage(int)), pImageTrigger, SLOT(trigger(int)));
  connect(this, SIGNAL(updateImage(int)), pSwitchTrigger, SLOT(trigger(int)));
}

void MainWindow::removeImageReaderOperations(PiiEngine *engine)
{
  PiiOperation *pImageTrigger = engine->findChild<PiiOperation*>("imageTrigger");
  PiiOperation *pImageFileReader = engine->findChild<PiiOperation*>("imageFileReader");
  PiiOperation *pSwitchTrigger = engine->findChild<PiiOperation*>("switchTrigger");
  PiiOperation *pSwitch = engine->findChild<PiiOperation*>("switch");
  
  engine->removeOperation(pImageTrigger);
  engine->removeOperation(pImageFileReader);
  engine->removeOperation(pSwitchTrigger);
  engine->removeOperation(pSwitch);

  delete pImageTrigger;
  delete pImageFileReader;
  delete pSwitchTrigger;
  delete pSwitch;
}

void MainWindow::updateImage()
{
  // We must pause the engine before update the next image,
  // because undistortOperation calculate transformations in
  // check-function
  pauseProcessing();
  
  startProcessing();
  
  if (_pRadioImageFile->isChecked())
    emit updateImage(0);
}

void MainWindow::changeFocalLength(int value)
{
  _pUndistortOperation->setProperty("focalX", value);
  _pUndistortOperation->setProperty("focalY", value);

  updateImage();
}

void MainWindow::changeDistortion(int value)
{
  _pUndistortOperation->setProperty("radial2nd", double(value)/100);

  updateImage();
  
  showSliderValue(_pDistortionSlider, QString::number(double(value)/100, 'f', 2));
}

void MainWindow::startButtonClicked()
{
  PiiEngine *pEngine = engine();

  PiiOperation *pImageFileReader = pEngine->findChild<PiiOperation*>("imageFileReader");
  PiiOperation *pNetworkCameraOperation = pEngine->findChild<PiiOperation*>("networkCameraOperation");

  bool bReadImage = false;
  
  if (_pRadioImageFile->isChecked())
    {
      bReadImage = true;
      if (pNetworkCameraOperation != 0)
        {
          pEngine->removeOperation(pNetworkCameraOperation);
          delete pNetworkCameraOperation;
        }

      // Create image file reader if necessary
      if (pImageFileReader == 0)
        {
          createImageReaderOperations(pEngine);
          pImageFileReader = pEngine->findChild<PiiOperation*>("imageFileReader");
        }
      
      // update file name
      pImageFileReader->setProperty("fileNames", QStringList() << _pImageFile->text());
    }
  else
    {
      if (pImageFileReader != 0)
        removeImageReaderOperations(pEngine);

      if (pNetworkCameraOperation == 0)
        {
          pNetworkCameraOperation = pEngine->createOperation("PiiNetworkCameraOperation", "networkCameraOperation");
          pNetworkCameraOperation->setProperty("imageType", "Color");
          
          pNetworkCameraOperation->connectOutput("image", _pUndistortOperation, "image");
          _pProbeInput1->connectOutput(pNetworkCameraOperation->output("image"));
        }

      //update image url
      pNetworkCameraOperation->setProperty("imageUrl", _pImageUrlCombo->currentText());
    }
  

  // Start processing
  startProcessing();

  if (bReadImage)
    {
      emit readImage(0);
      emit updateImage(0);
    }
}

void MainWindow::selectImageFile()
{
  QStringList lstFileNames = getImageFiles(QFileDialog::ExistingFile);

  if (lstFileNames.size() > 0)
    _pImageFile->setText(lstFileNames[0]);
}

