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
#include <PiiMatrix.h>
#include <PiiVariant.h>
#include <PiiYdinTypes.h>
#include <PiiMath.h>


MainWindow::MainWindow(QWidget *parent) :
  DemoMainWindow(parent)
{
  // Initialize ui
  init();
}

void MainWindow::updateValues()
{
  PiiMatrix<unsigned char> fractalMatrix;

  _fractalGenerator.setMinimum(_pMinimum->value());
  _fractalGenerator.setMaximum(_pMaximum->value());
  _fractalGenerator.setRoughnessScale(_pRoughnessScale->value());
  
  fractalMatrix = _fractalGenerator.generateSquareFractal<unsigned char>(Pii::pow(2,_pFractalSize->currentIndex()+1),
                                                                         _pLeftTop->value(),
                                                                         _pRightTop->value(),
                                                                         _pLeftBottom->value(),
                                                                         _pRightBottom->value(),
                                                                         _pRoughness->value());

  emit imageChanged(PiiVariant(fractalMatrix));
}

void MainWindow::init()
{
  setupUi(this);
  setWindowTitle(demoName());

  // Make ui-connections
  connect(_pFractalSize, SIGNAL(currentIndexChanged(int)), this, SLOT(updateValues()));

  connect(_pMinimum, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));
  connect(_pMaximum, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));
  connect(_pRoughnessScale, SIGNAL(valueChanged(double)), this, SLOT(updateValues()));
  connect(_pRoughness, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));
  
  connect(_pLeftTop, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));
  connect(_pRightTop, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));
  connect(_pLeftBottom, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));
  connect(_pRightBottom, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));

  connect(this, SIGNAL(imageChanged(PiiVariant)), _pImageDisplay, SLOT(setImage(PiiVariant)));
  
  // Init button states
  updateValues();
}
