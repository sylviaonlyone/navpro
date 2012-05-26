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
#include <PiiImage.h>
#include <PiiMatrixUtil.h>
#include <PiiImageViewport.h>
#include <PiiYdinUtil.h>

#include <QToolButton>
#include <QMessageBox>
#include <QtDebug>

#include <PiiYdinTypes.h>

MainWindow::MainWindow(QWidget *parent) :
  DemoMainWindow(parent)
{
  _lstColors << QColor(Qt::red) << QColor(Qt::blue) << QColor(Qt::green)
             << QColor(Qt::yellow) << QColor(Qt::cyan) << QColor(Qt::magenta);
  
  // Initialize engine
  initEngine();

  // Initialize ui
  init();

  startProcessing();
  nextButtonClicked();
}

void MainWindow::init()
{
  setupUi(this);
  setWindowTitle(demoName());

  _pResultImageDisplay->setProperty("displayType", "AutoScale");
  _pResultImageDisplay->imageViewport()->setProperty("fitMode", "FitToView");
  _pResultImageDisplay->imageViewport()->setShowOverlayColoring(false);
  
  _pNextImageButton->setIconMode(PushButton::IconRight);

  // Get model images and make tool buttons
  for (int i=0; i<_lstModels.size(); i++)
    {
      // Create toolbutton
      QToolButton *pToolButton = new QToolButton;
      pToolButton->setFixedSize(QSize(60,60));
      pToolButton->setIconSize(QSize(50,50));
      pToolButton->setIcon(QIcon(QPixmap(_lstModels[i].strFileName)));
      QPalette pal = pToolButton->palette();
      pal.setColor(QPalette::Button, _lstColors[i]);
      pToolButton->setPalette(pal);
      
      _pModelLayout->addWidget(pToolButton);
    }
  _pModelLayout->addStretch(1);

  // Make ui-connections
  connect(_pPreviousImageButton, SIGNAL(clicked()), this, SLOT(prevButtonClicked()));
  connect(_pNextImageButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
}

PiiEngine* MainWindow::createEngine()
{
  // Create engine
  PiiEngine *pEngine = new PiiEngine;

  QString strPath = QString("%1/shapematching").arg(defaultImageDirPath());
  
  // Create operations
  QStringList lstModelNames;
  for (int i=0; i<5; ++i)
    lstModelNames << QString("%1/model%2.png").arg(strPath).arg(i);

  PiiOperation *pImageSource = pEngine->createOperation("PiiImageFileReader", "imageSource");
  pImageSource->setProperty("fileNames",lstModelNames);
  pImageSource->setProperty("repeatCount", 1);

  PiiOperation *pThresholding = pEngine->createOperation("PiiThresholdingOperation", "thresholding");
  pThresholding->setProperty("absoluteThreshold", 180);
  pThresholding->setProperty("thresholdType", "StaticThreshold");
  pThresholding->setProperty("inverse", true);

  PiiOperation *pBoundaryFinder = pEngine->createOperation("PiiBoundaryFinderOperation", "boundaryFinder");
  pBoundaryFinder->setProperty("minLength", 100);

  PiiOperation *pObjectCapturer = pEngine->createOperation("PiiObjectCapturer", "objectCapturer");
  pObjectCapturer->setProperty("dynamicInputCount", 1);

  pImageSource->connectOutput("filename", pObjectCapturer, "sync");
  pBoundaryFinder->connectOutput("boundary", pObjectCapturer, "input0");
  
  connect(pObjectCapturer, SIGNAL(objectsCaptured(PiiVariant, QVariantList)),
          this, SLOT(updateModels(PiiVariant, QVariantList)));

  PiiOperation *pShapeContextOperation = pEngine->createOperation("PiiShapeContextOperation", "shapeContextOperation");
  pShapeContextOperation->setProperty("shapeJoiningMode", "JoinNestedShapes");
  
  PiiOperation *pRigidPlaneMatcher = pEngine->createOperation("PiiRigidPlaneMatcher", "rigidPlaneMatcher");
  pRigidPlaneMatcher->setProperty("learningBatchSize", -1);
  // Minimum number of matched points for an accepted detection
  pRigidPlaneMatcher->setProperty("minInliers", 20);
  // Loose merging
  pRigidPlaneMatcher->setProperty("angleToleranceDeg", 15);
  pRigidPlaneMatcher->setProperty("translationTolerance", 60);
  pRigidPlaneMatcher->setProperty("scaleTolerance", 0.2);
  
  // Connect operations
  pImageSource->connectOutput("image", pThresholding, "image");
  pThresholding->connectOutput("image", pBoundaryFinder, "image");

  pBoundaryFinder->connectOutput("boundaries", pShapeContextOperation, "boundaries");
  pBoundaryFinder->connectOutput("limits", pShapeContextOperation, "limits");

  pShapeContextOperation->connectOutput("points", pRigidPlaneMatcher, "points");
  pShapeContextOperation->connectOutput("features", pRigidPlaneMatcher, "features");
    
  // Train classifier
  try
    {
      pEngine->execute();
      pEngine->wait(PiiOperation::Stopped);
    }
  catch (PiiExecutionException& ex)
    {
      QMessageBox::critical(0, "Application error", ex.message() );
      exit(1);
    }

  QMetaObject::invokeMethod(pRigidPlaneMatcher, "startLearningThread", Qt::DirectConnection);
  while (pRigidPlaneMatcher->property("learningThreadRunning").toBool())
    QCoreApplication::processEvents();
  pRigidPlaneMatcher->setProperty("learningBatchSize", 0);

  PiiOperation* pSwitch = pEngine->createOperation("PiiSwitch", "switch");
  pSwitch->setProperty("operationMode", "AsynchronousMode");
  
  PiiOperation *pSequenceGenerator = pEngine->createOperation("PiiSequenceGenerator", "sequenceGenerator");
  pSequenceGenerator->setProperty("sequenceStart", 0);
  pSequenceGenerator->setProperty("sequenceEnd", 2*M_PI - M_PI/12);
  pSequenceGenerator->setProperty("step", M_PI / 12);
    
  PiiOperation *pRotation = pEngine->createOperation("PiiImageRotationOperation", "rotation");
  pRotation->setProperty("transformedSize", "PiiImage::RetainOriginalSize");
  pRotation->setProperty("backgroundColor", QColor(Qt::white));

  pImageSource->connectOutput("image", pSwitch, "input");
  pSwitch->connectOutput("output", pRotation, "image");
  pSequenceGenerator->connectOutput("output", pRotation, "angle");
  pSequenceGenerator->connectOutput("output", pSwitch, "trigger");
  pRotation->connectOutput("image", pThresholding, "image");

  PiiOperation *pTriggerSource = pEngine->createOperation("PiiTriggerSource", "triggerSource");
  connect(this, SIGNAL(trigImage(int)), pTriggerSource, SLOT(trigger(int)));
  pTriggerSource->connectOutput("trigger", pImageSource, "trigger");
  
  // init image file names
  QStringList lstImageNames;
  for (int i=0; i<3; ++i)
    lstImageNames << QString("%1/test%2.png").arg(strPath).arg(i);
  pImageSource->setProperty("fileNames", lstImageNames);
  
  pObjectCapturer->setProperty("dynamicInputCount", 2);
  pRotation->connectOutput("image", pObjectCapturer, "sync");
  
  pRigidPlaneMatcher->connectOutput("model index", pObjectCapturer, "input0");
  pRigidPlaneMatcher->connectOutput("transform", pObjectCapturer, "input1");
  
  disconnect(pObjectCapturer, SIGNAL(objectsCaptured(PiiVariant, QVariantList)),
             this, SLOT(updateModels(PiiVariant, QVariantList)));
  connect(pObjectCapturer, SIGNAL(objectsCaptured(PiiVariant, QVariantList)),
          this, SLOT(updateImage(PiiVariant, QVariantList)));
  
  return pEngine;
}

void MainWindow::prevButtonClicked()
{
  emit trigImage(-1);
}

void MainWindow::nextButtonClicked()
{
  emit trigImage(1);
}

void MainWindow::updateModels(const PiiVariant& name, const QVariantList& information)
{
  if (information.size() == 1)
    {
      Model model;
      model.strFileName = name.valueAs<QString>();
      model.color = _lstColors[_lstModels.size() % _lstColors.size()];
      
      QVariantList lstModelBoundaries = information[0].toList();
      QList<PiiMatrix<int> > lstBoundaries;

      for (int i=0; i<lstModelBoundaries.size(); i++)
        lstBoundaries << lstModelBoundaries[i].value<PiiVariant>().valueAs<PiiMatrix<int> >();
      model.lstBoundaries = lstBoundaries;

      _lstModels << model;
    }
}

void MainWindow::updateImage(const PiiVariant& image, const QVariantList& information)
{
  // Clear overlays
  _pResultImageDisplay->imageViewport()->removeOverlay(0);
  while (!_lstOverlays.isEmpty())
    delete _lstOverlays.takeFirst();

  if (information.size() == 2)
    {
      QVariantList lstModelIndices = information[0].toList();
      QVariantList lstTransforms = information[1].toList();
      
      if (lstModelIndices.size() == lstTransforms.size())
        {
          for (int i=0; i<lstModelIndices.size(); i++)
            {
              int iModelIndex = lstModelIndices[i].value<PiiVariant>().valueAs<int>();
              if (iModelIndex >= 0 && iModelIndex < _lstModels.size())
                {
                  Model model = _lstModels[iModelIndex];
                  QPen pen(model.color);

                  for (int k=0; k<model.lstBoundaries.size(); k++)
                    {
                      PiiMatrix<double> matTransform = lstTransforms[i].value<PiiVariant>().valueAs<PiiMatrix<double> >();
                      PiiMatrix<int> matTransformedPoints = PiiImage::transformHomogeneousPoints<double>(matTransform,
                                                                                                         model.lstBoundaries[k]);
                      if (matTransformedPoints.rows() > 1)
                        {
                          QPainterPath path(QPointF(matTransformedPoints(0,0), matTransformedPoints(0,1)));
                          for (int j=1; j<matTransformedPoints.rows(); j++)
                            path.lineTo(matTransformedPoints(j,0), matTransformedPoints(j,1));
                          
                          PiiPolygonOverlay *pOverlay = new PiiPolygonOverlay(path);
                          pOverlay->setPen(pen);
                          _lstOverlays << pOverlay;
                        }
                    }
                }
            }
        }
    }
  _pResultImageDisplay->imageViewport()->setOverlays(_lstOverlays);
  _pResultImageDisplay->setImage(image);
}
