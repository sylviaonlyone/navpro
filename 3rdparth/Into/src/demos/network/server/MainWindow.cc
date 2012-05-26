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
  _pProbeInput(0)
{
  // Initialize engine
  initEngine();

  // Initialize ui
  init();

  // Start processing
  startProcessing();
}

void MainWindow::init()
{
  setupUi(this);
  setWindowTitle(demoName());

  _pImageDisplay->imageViewport()->setProperty("fitMode", "FitToView");

  _pStopButton->setIcon(QIcon(QPixmap(":stop")));
  _pStartButton->setIcon(QIcon(QPixmap(":play")));

  // Make ui-connections
  connect(_pProbeInput, SIGNAL(objectReceived(PiiVariant)), _pImageDisplay, SLOT(setImage(PiiVariant)));
  connect(_pStopButton, SIGNAL(clicked()), this, SLOT(stopProcessing()));
  connect(_pStartButton, SIGNAL(clicked()), this, SLOT(startProcessing()));
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine* pEngine = new PiiEngine;
  
  // Create probe input for image display
  _pProbeInput = new PiiProbeInput;

  // Create network input operation
  PiiOperation *pReceiver = pEngine->createOperation("PiiNetworkInputOperation", "receiver");
  pReceiver->setProperty("outputNames", QStringList() << "image");
  pReceiver->setProperty("inputNames", QStringList() << "result");
  pReceiver->setProperty("httpServer", "tcp://0.0.0.0:8080");

  // Create edge detector
  PiiOperation *pDetector = pEngine->createOperation("PiiEdgeDetector");
  
  // Connect operations
  pReceiver->connectOutput("image", pDetector, "image");
  pDetector->connectOutput("edges", pReceiver, "result");

  _pProbeInput->connectOutput(pReceiver->output("image"));

  connect(pEngine, SIGNAL(stateChanged(int)), this, SLOT(updateButtonStates(int)));

  return pEngine;
}


void MainWindow::updateButtonStates(int state)
{
  _pStartButton->setEnabled(state != PiiOperation::Running);
  _pStopButton->setEnabled(state != PiiOperation::Stopped);
}
