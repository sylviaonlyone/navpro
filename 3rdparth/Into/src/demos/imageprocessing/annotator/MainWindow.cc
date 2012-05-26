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
  _pImageAnnotator(0),
  _pResultProbeInput(0),
  _pButtonGroup(0)
{
  // Initialize engine
  initEngine();
  
  // Initialize ui
  init();

  // Start processing
  startProcessing();
  emit selectImage(0);
  emit updateImage(0);
}

void MainWindow::init()
{
  setupUi(this);
  setWindowTitle(demoName());

  // Init button group
  _pButtonGroup = new QButtonGroup(this);
  _pButtonGroup->addButton(_pText,0);
  _pButtonGroup->addButton(_pPoint,1);
  _pButtonGroup->addButton(_pLine,2);
  _pButtonGroup->addButton(_pRectangle,3);
  _pButtonGroup->addButton(_pEllipse,4);
  _pButtonGroup->addButton(_pCircle,5);

  connect(_pResultProbeInput, SIGNAL(objectReceived(PiiVariant)), _pResultImageDisplay, SLOT(setImage(PiiVariant)));

  // Make ui-connections
  connect(_pResultImageDisplay, SIGNAL(areaSelected(const QRect&, int)),
          this, SLOT(updateAnnotations(const QRect&, int)));
  connect(_pResultImageDisplay, SIGNAL(clicked(const QPoint&, int)),
          this, SLOT(mouseClicked(const QPoint&, int)));
  connect(_pClearButton, SIGNAL(clicked()), this, SLOT(clearAnnotations()));
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
  
  // Create probe input for result image display
  _pResultProbeInput = new PiiProbeInput;
  
  // Create image source
  PiiOperation *pImageFileReader = pEngine->createOperation("PiiImageFileReader");
  pImageFileReader->setProperty("imageType", "Color");
  pImageFileReader->setProperty("fileNames", QString("%1/olga.jpg").arg(defaultImageDirPath()));

  // Create image annotator
  _pImageAnnotator = pEngine->createOperation("PiiImageAnnotator");

  // Make operation connections
  pReadImageTriggerSource->connectOutput("trigger", pImageFileReader, "trigger");
  pImageFileReader->connectOutput("image", pSwitch, "input");
  pSwitch->connectOutput("output", _pImageAnnotator, "image");
  pUpdateImageTriggerSource->connectOutput("trigger", pSwitch, "trigger");
  
  _pResultProbeInput->connectOutput(_pImageAnnotator->output("image"));
  
  return pEngine;
}

void MainWindow::clearAnnotations()
{
  _lstAnnotations.clear();

  // Change the annotations
  _pImageAnnotator->setProperty("annotations", _lstAnnotations);
  
  // Trig the same image again
  emit updateImage(0);
}

void MainWindow::mouseClicked(const QPoint& point, int modifiers)
{
  updateAnnotations(QRect(point.x(), point.y(), 10, 10), modifiers);
}

void MainWindow::updateAnnotations(const QRect& area, int modifiers)
{
  QVariantMap annotation;
  annotation["annotationType"] = _pButtonGroup->checkedId();
  annotation["x"] = area.x();
  annotation["y"] = area.y();

  switch(_pButtonGroup->checkedId())
    {
    case 0: //Text
      annotation["text"] = "Intopii";
      break;
    case 1: //Point
      break;
    case 2: //Line
     annotation["x2"] = area.right();
      annotation["y2"] = area.bottom();
      break;
    case 3: //Rectangle
    case 4: //Ellipse
      annotation["width"] = area.width();
      annotation["height"] = area.height();
      break;
    case 5: //Circle
      annotation["x"] = area.x() + area.width() / 2;
      annotation["y"] = area.y() + area.height() / 2;
      annotation["radius"] = qMin(area.width()/2, area.height()/2);
      break;
    }

  // Append annotation
  _lstAnnotations << annotation;
  
  // Change the annotations
  _pImageAnnotator->setProperty("annotations", _lstAnnotations);
  
  // Trig the same image again
  emit updateImage(0);
}



