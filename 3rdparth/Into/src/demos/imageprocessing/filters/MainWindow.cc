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
  _pResultProbeInput(0),
  _pImageFileReader(0),
  _pImageFilter(0)
{
  // Intialize engine.
  initEngine();
  
  // Initialize ui
  init();
   
  startProcessing();

  // Select default filter
  selectFilter(0);

  // Select first image
  emit selectImage(0);
}

QStringList MainWindow::requiredPlugins() const
{
  return QStringList() << "piiimage" << "piibase" << "piiflowcontrol";
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine *pEngine = new PiiEngine;

  // Create trigger source
  PiiOperation *pTriggerSource = pEngine->createOperation("PiiTriggerSource");

  // Create probe input for source and result image
  _pSourceProbeInput = new PiiProbeInput;
  _pResultProbeInput = new PiiProbeInput;
  
  // Create image source
  _pImageFileReader = pEngine->createOperation("PiiImageFileReader", "ImageSource");
  _pImageFileReader->setProperty("fileNamePattern", QString("%1/*.jpg").arg(defaultImageDirPath()));

  // Create switch and trigger
  PiiOperation *pSwitchTrigger = pEngine->createOperation("PiiTriggerSource", "switchTrigger");
  PiiOperation *pSwitch = pEngine->createOperation("PiiSwitch", "switch");
  pSwitch->setProperty("dynamicInputCount", 1);
  pSwitch->setProperty("operationMode", "AsynchronousMode");
  pSwitch->setProperty("passThrough", true);

  // Create filter operation
  _pImageFilter = pEngine->createOperation("PiiImageFilterOperation", "Filter");
  
  // Make operation connections
  pTriggerSource->connectOutput("trigger", _pImageFileReader, "trigger");
  pSwitchTrigger->connectOutput("trigger", pSwitch, "trigger");
  _pImageFileReader->connectOutput("image", pSwitch, "input0");
  pSwitch->connectOutput("output0", _pImageFilter, "image");
  
  _pSourceProbeInput->connectOutput(_pImageFileReader->output("image"));
  _pResultProbeInput->connectOutput(_pImageFilter->output("image"));
  
  connect(this, SIGNAL(selectImage(int)), pTriggerSource, SLOT(trigger(int)));
  connect(this, SIGNAL(updateImage(int)), pSwitchTrigger, SLOT(trigger(int)));
  
  return pEngine;
}

void MainWindow::init()
{
  // Initialize ui made by designer
  setupUi(this);
  setWindowTitle(demoName());
  
  _pNextImageButton->setIconMode(PushButton::IconRight);

  _pSourceImageDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pResultImageDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  
  // Make ui-connections
  connect(_pPreviousImageButton, SIGNAL(clicked()), this, SLOT(prevButtonClicked()));
  connect(_pNextImageButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
  connect(_pActionSelectImages, SIGNAL(triggered()), this, SLOT(selectImages()));
  connect(_pFilterSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(selectFilterSize(int)));
  connect(_pFilterNameCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(selectFilter(int)));
  connect(_pBorderHandlingCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(selectBorderHandling(int)));

  connect(_pSourceProbeInput, SIGNAL(objectReceived(PiiVariant)), _pSourceImageDisplay, SLOT(setImage(PiiVariant)));
  connect(_pResultProbeInput, SIGNAL(objectReceived(PiiVariant)), _pResultImageDisplay, SLOT(setImage(PiiVariant)));

  _updateTimer.setSingleShot(true);
  _updateTimer.setInterval(30);
  connect(&_updateTimer, SIGNAL(timeout()), this, SLOT(updateFilterSize()));

}

void MainWindow::selectFilter(int index)
{
  // Change filter name
  QString strFilterName;
  switch (index)
    {
    case 0: strFilterName = "uniform"; break;
    case 1: strFilterName = "gaussian"; break;
    case 2: strFilterName = "log"; break;
    case 3: strFilterName = "median"; break;
    default: strFilterName = "gaussian"; break;
    }
  _pImageFilter->setProperty("filterName", strFilterName);
  
  // Trig the same image again
  emit updateImage(0);
}

void MainWindow::selectBorderHandling(int index)
{
  // Change filter name
  _pImageFilter->setProperty("borderHandling", index);
  
  // Trig the same image again
  emit updateImage(0);
}

void MainWindow::prevButtonClicked()
{
  emit selectImage(-1);
  updateFilterSize();
}

void MainWindow::nextButtonClicked()
{
  emit selectImage(1);
  updateFilterSize();
}

void MainWindow::selectFilterSize(int value)
{
  _updateTimer.start();

  showSliderValue(_pFilterSizeSlider, QString::number(value));
}

void MainWindow::updateFilterSize()
{
  int value = _pFilterSizeSlider->value();
  
  // Pause engine
  pauseProcessing();
  
  // Change filter size
  _pImageFilter->setProperty("filterSize", value);

  // Restart engine
  startProcessing();
  
  // Select the same image again
  emit updateImage(0);
}

void MainWindow::selectImages()
{
  QStringList lstFileNames = getImageFiles();

  if (lstFileNames.size() > 0)
    {
      // Pause processing when setting new images
      pauseProcessing();

      // Set new images
      _pImageFileReader->setProperty("fileNames", lstFileNames);

      // Restart engine
      startProcessing();

      // Trig the image
      emit selectImage(0);
    }
}

