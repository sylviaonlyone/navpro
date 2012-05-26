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
  _pProbeInput(0)
{
  // Initialize engine
  initEngine();

  // Initialize ui
  init();
}

void MainWindow::updateButtonStates(int state)
{
  _pStopButton->setEnabled(state == PiiOperation::Running);
  _pStartButton->setEnabled(state != PiiOperation::Running);
}

void MainWindow::init()
{
  setupUi(this);
  setWindowTitle(demoName());

  // Make ui-connections
  connect(_pImageUrlCombo, SIGNAL(activated(const QString&)), this, SLOT(urlActivated(const QString&)));
  connect(_pStartButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
  connect(_pStopButton, SIGNAL(clicked()), this, SLOT(stopProcessing()));

  connect(_pProbeInput, SIGNAL(objectReceived(PiiVariant)), _pImageDisplay, SLOT(setImage(PiiVariant)));

  // Init button states
  updateButtonStates(engine()->state());
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine* pEngine = new PiiEngine;
  connect(pEngine, SIGNAL(stateChanged(int)), this, SLOT(updateButtonStates(int)));
  
  // Create probe input for image display
  _pProbeInput = new PiiProbeInput;
  
  // Create network camera operation
  _pNetworkCameraOperation = pEngine->createOperation("PiiNetworkCameraOperation");
  _pNetworkCameraOperation->setProperty("imageType", "Color");
  
  // Connect operations
  _pProbeInput->connectOutput(_pNetworkCameraOperation->output("image"));
  
  return pEngine;
}

void MainWindow::startButtonClicked()
{
  // Update image url
  _pNetworkCameraOperation->setProperty("imageUrl", _pImageUrlCombo->currentText());

  // Start processing
  startProcessing();
}

void MainWindow::urlActivated(const QString& imageUrl)
{
  // Pause processing
  pauseProcessing();

  // Set image url
  _pNetworkCameraOperation->setProperty("imageUrl", imageUrl);
  
  // Restart processing
  startProcessing();
}
