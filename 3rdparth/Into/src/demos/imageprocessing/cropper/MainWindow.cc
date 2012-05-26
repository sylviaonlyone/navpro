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
  _pImageCropper(0)
{
  // Intialize engine
  initEngine();
  
  // Initialize ui
  init();

  // Start processing
  startProcessing();
  emit selectImage(0);
}

void MainWindow::init()
{
  setupUi(this);
  setWindowTitle(demoName());

  // Make ui-connections
  connect(_pSourceImageDisplay, SIGNAL(areaSelected(const QRect&, int)),
          this, SLOT(setCroppedArea(const QRect&, int)));
  connect(_pSourceProbeInput, SIGNAL(objectReceived(PiiVariant)), _pSourceImageDisplay, SLOT(setImage(PiiVariant)));
  connect(_pResultProbeInput, SIGNAL(objectReceived(PiiVariant)), _pResultImageDisplay, SLOT(setImage(PiiVariant)));
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine *pEngine = new PiiEngine;

  // Create trigger source
  PiiOperation *pTriggerSource = pEngine->createOperation("PiiTriggerSource");

  // Create probe input for source and result image display
  _pSourceProbeInput = new PiiProbeInput;
  _pResultProbeInput = new PiiProbeInput;
  
  // Create image source
  PiiOperation *pImageFileReader = pEngine->createOperation("PiiImageFileReader");
  pImageFileReader->setProperty("imageType", "Original"); //GrayScale");
  pImageFileReader->setProperty("fileNames", QString("%1/olga.jpg").arg(defaultImageDirPath()));

  // Create image croppper
  _pImageCropper = pEngine->createOperation("PiiImageCropper");
  _pImageCropper->setProperty("area", QRect(0,0,100,100));
  
  // Make operation connections
  pTriggerSource->connectOutput("trigger", pImageFileReader, "trigger");
  pImageFileReader->connectOutput("image", _pImageCropper, "image");
  
  _pSourceProbeInput->connectOutput(pImageFileReader->output("image"));
  _pResultProbeInput->connectOutput(_pImageCropper->output("image"));
  
  connect(this, SIGNAL(selectImage(int)), pTriggerSource, SLOT(trigger(int)));

  return pEngine;
}

void MainWindow::setCroppedArea(const QRect& area, int modifiers)
{
  // Fix area x and y coordinates if necessary
  QRect fixedArea = area;
  fixedArea.setX(qMax(fixedArea.x(), 0));
  fixedArea.setY(qMax(fixedArea.y(), 0));
  
  // Change cropping area
  _pImageCropper->setProperty("area", fixedArea);
  
  // Trig the image again
  emit selectImage(0);
}
