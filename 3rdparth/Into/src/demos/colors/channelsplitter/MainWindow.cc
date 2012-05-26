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
  _pRedProbeInput(0),
  _pGreenProbeInput(0),
  _pBlueProbeInput(0),
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
  _pRedChannelDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pGreenChannelDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pBlueChannelDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  
  _pNextImageButton->setIconMode(PushButton::IconRight);
  
  // Make ui-connections
  connect(_pPreviousImageButton, SIGNAL(clicked()), this, SLOT(prevButtonClicked()));
  connect(_pNextImageButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
  connect(_pActionSelectImages, SIGNAL(triggered()), this, SLOT(selectImages()));

  connect(_pSourceProbeInput, SIGNAL(objectReceived(PiiVariant)), _pSourceImageDisplay, SLOT(setImage(PiiVariant)));
  connect(_pRedProbeInput, SIGNAL(objectReceived(PiiVariant)), _pRedChannelDisplay, SLOT(setImage(PiiVariant)));
  connect(_pGreenProbeInput, SIGNAL(objectReceived(PiiVariant)), _pGreenChannelDisplay, SLOT(setImage(PiiVariant)));
  connect(_pBlueProbeInput, SIGNAL(objectReceived(PiiVariant)), _pBlueChannelDisplay, SLOT(setImage(PiiVariant)));
  
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine* pEngine = new PiiEngine;

  // Create trigger source
  PiiOperation *pTriggerSource = pEngine->createOperation("PiiTriggerSource");

  // Create probe input for source and all channels image
  _pSourceProbeInput = new PiiProbeInput;
  _pRedProbeInput = new PiiProbeInput;
  _pGreenProbeInput = new PiiProbeInput;
  _pBlueProbeInput = new PiiProbeInput;
  
  // Create image source
  _pImageFileReader = pEngine->createOperation("PiiImageFileReader");
  _pImageFileReader->setProperty("imageType", "Color");
  _pImageFileReader->setProperty("fileNamePattern", QString("%1/*.jpg").arg(defaultImageDirPath()));
  
  // Create color channel splitter
  PiiOperation *pColorChannelSplitter = pEngine->createOperation("PiiColorChannelSplitter");
  
  // Make operation connections
  connect(this, SIGNAL(selectImage(int)), pTriggerSource, SLOT(trigger(int)));
  
  pTriggerSource->connectOutput("trigger", _pImageFileReader, "trigger");
  _pImageFileReader->connectOutput("image", pColorChannelSplitter, "image");
  
  _pSourceProbeInput->connectOutput(_pImageFileReader->output("image"));
  _pRedProbeInput->connectOutput(pColorChannelSplitter->output("channel0"));
  _pGreenProbeInput->connectOutput(pColorChannelSplitter->output("channel1"));
  _pBlueProbeInput->connectOutput(pColorChannelSplitter->output("channel2"));

  return pEngine;
}

void MainWindow::prevButtonClicked()
{
  emit selectImage(-1);
}

void MainWindow::nextButtonClicked()
{
  emit selectImage(1);
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
      emit selectImage(1);
    }
}


