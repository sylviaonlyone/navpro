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
  _pApproximationProbeInput(0),
  _pHorizontalProbeInput(0),
  _pVerticalProbeInput(0),
  _pDiagonalProbeInput(0),
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

  _pNextImageButton->setIconMode(PushButton::IconRight);

  _pSourceImageDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pApproximationDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pHorizontalDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pVerticalDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pDiagonalDisplay->imageViewport()->setProperty("fitMode", "FitToView");

  // Make ui-connections
  connect(_pPreviousImageButton, SIGNAL(clicked()), this, SLOT(prevButtonClicked()));
  connect(_pNextImageButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
  connect(_pActionSelectImages, SIGNAL(triggered()), this, SLOT(selectImages()));

  connect(_pSourceProbeInput, SIGNAL(objectReceived(PiiVariant)), _pSourceImageDisplay, SLOT(setImage(PiiVariant)));
  connect(_pApproximationProbeInput, SIGNAL(objectReceived(PiiVariant)),_pApproximationDisplay, SLOT(setImage(PiiVariant)));
  connect(_pHorizontalProbeInput, SIGNAL(objectReceived(PiiVariant)), _pHorizontalDisplay, SLOT(setImage(PiiVariant)));
  connect(_pVerticalProbeInput, SIGNAL(objectReceived(PiiVariant)), _pVerticalDisplay, SLOT(setImage(PiiVariant)));
  connect(_pDiagonalProbeInput, SIGNAL(objectReceived(PiiVariant)), _pDiagonalDisplay, SLOT(setImage(PiiVariant)));

  _pApproximationDisplay->setProperty("displayType", "AutoScale");
  _pHorizontalDisplay->setProperty("displayType", "AutoScale");
  _pVerticalDisplay->setProperty("displayType", "AutoScale");
  _pDiagonalDisplay->setProperty("displayType", "AutoScale");
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine *pEngine = new PiiEngine;

  // Create trigger source
  PiiOperation *pTriggerSource = pEngine->createOperation("PiiTriggerSource");

  // Create probe inputs for source and result image display
  _pSourceProbeInput = new PiiProbeInput;
  _pApproximationProbeInput = new PiiProbeInput;
  _pHorizontalProbeInput = new PiiProbeInput;
  _pVerticalProbeInput = new PiiProbeInput;
  _pDiagonalProbeInput = new PiiProbeInput;
  
  // Create image source
  _pImageFileReader = pEngine->createOperation("PiiImageFileReader");
  _pImageFileReader->setProperty("imageType", "GrayScale");
  _pImageFileReader->setProperty("fileNamePattern", QString("%1/*.jpg").arg(defaultImageDirPath()));

  // Create wavelet operation
  PiiOperation *pWaveletTransform = pEngine->createOperation("PiiWaveletTransform");
  
  // Make operation connections
  connect(this, SIGNAL(selectImage(int)), pTriggerSource, SLOT(trigger(int)));

  pTriggerSource->connectOutput("trigger", _pImageFileReader, "trigger");
  _pImageFileReader->connectOutput("image", pWaveletTransform, "input");
  
  _pSourceProbeInput->connectOutput(_pImageFileReader->output("image"));
  _pApproximationProbeInput->connectOutput(pWaveletTransform->output("approximation"));
  _pHorizontalProbeInput->connectOutput(pWaveletTransform->output("horizontal"));
  _pVerticalProbeInput->connectOutput(pWaveletTransform->output("vertical"));
  _pDiagonalProbeInput->connectOutput(pWaveletTransform->output("diagonal"));

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
