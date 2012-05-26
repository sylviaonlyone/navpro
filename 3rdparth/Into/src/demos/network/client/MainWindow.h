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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <DemoMainWindow.h>
#include "ui_mainwindow.h"

#include <PiiEngine.h>
#include <PiiOperation.h>
#include <PiiProbeInput.h>

class MainWindow : public DemoMainWindow, private Ui::MainWindow
{
  Q_OBJECT
  
public:
  MainWindow(QWidget *parent = 0);

protected:
  QStringList requiredPlugins() const { return QStringList() << "piibase" << "piiimage" << "piinetwork"; }
  PiiEngine* createEngine();

signals:
  void selectImage(int);
  
private slots:
  void selectImages();
  void nextButtonClicked();
  void prevButtonClicked();
  
private:
  void init();

  PiiProbeInput *_pSourceProbeInput, *_pResultProbeInput;
  PiiOperation *_pImageFileReader;
};



#endif //_MAINWINDOW_H
