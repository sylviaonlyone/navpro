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
  _pResultProbeInput(0),
  _pFileSystemWatcher(0)
{
  // Intialize engine.
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
  
  // Update info text
  QStringList lstFileNames = _pFileSystemWatcher->property("directories").toStringList();
  QString strPath = lstFileNames.size() > 0 ? lstFileNames[0] : "";
  _pInfoLabel->setText(tr("When you copy the image file (jpg or png) to the directory (%1) we will show it for you.").arg(strPath));

  // Make ui-connections
  connect(_pResultProbeInput, SIGNAL(objectReceived(PiiVariant)), _pResultImageDisplay, SLOT(setImage(PiiVariant)));
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine *pEngine = new PiiEngine;

  // Create probe input for result image display
  _pResultProbeInput = new PiiProbeInput;

  // Create file system watcher
  _pFileSystemWatcher = pEngine->createOperation("PiiFileSystemWatcher");
  _pFileSystemWatcher->setProperty("nameFilters", QStringList() << "*.jpg" << "*.png");
  _pFileSystemWatcher->setProperty("directories", QStringList() << QString("%1").arg(applicationDirPath()));
  _pFileSystemWatcher->setProperty("watchDelay", 1);

  // Create image file reader
  PiiOperation *pImageFileReader = pEngine->createOperation("PiiImageFileReader");
  pImageFileReader->setProperty("imageType", "Color");

  // Make operation connections
  _pFileSystemWatcher->connectOutput("filename", pImageFileReader, "filename");
  _pResultProbeInput->connectOutput(pImageFileReader->output("image"));

  return pEngine;
}
