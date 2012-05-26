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

void MainWindow::init()
{
  setupUi(this);
  setWindowTitle(demoName());

  _pStopButton->setIcon(QIcon(QPixmap(":stop")));
  _pPauseButton->setIcon(QIcon(QPixmap(":pause")));
  _pPlayButton->setIcon(QIcon(QPixmap(":play")));

  // Make ui-connections
  connect(_pProbeInput, SIGNAL(objectReceived(PiiVariant)), _pVideoDisplay, SLOT(setImage(PiiVariant)));
  connect(_pStopButton, SIGNAL(clicked()), this, SLOT(stopProcessing()));
  connect(_pPauseButton, SIGNAL(clicked()), this, SLOT(pauseProcessing()));
  connect(_pPlayButton, SIGNAL(clicked()), this, SLOT(startProcessing()));
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine *pEngine = new PiiEngine;

  // Create trigger timer
  PiiOperation *pClock = pEngine->createOperation("PiiClock");
  pClock->setProperty("interval",200); //200 msec -> 5 fps
  
  // Create probe input video display
  _pProbeInput = new PiiProbeInput;
  
  // Create video reader
  PiiOperation *pVideoFileReader = pEngine->createOperation("PiiVideoFileReader");
  pVideoFileReader->setProperty("imageType", "Color");
  pVideoFileReader->setProperty("fileName", QString("%1/source.avi").arg(defaultVideoDirPath()));
  
  // Make operation connections
  pClock->connectOutput("time", pVideoFileReader, "trigger");
  
  _pProbeInput->connectOutput(pVideoFileReader->output("image"));

  connect(pEngine, SIGNAL(stateChanged(int)), this, SLOT(updateButtonStates(int)));
  
  return pEngine;
}

void MainWindow::updateButtonStates(int state)
{
  _pPlayButton->setEnabled(state != PiiOperation::Running);
  _pStopButton->setEnabled(state != PiiOperation::Stopped);
  _pPauseButton->setEnabled(state == PiiOperation::Running);
}
