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

#ifndef _DEMOMAINWINDOW_H
#define _DEMOMAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QSlider>

#include <PiiEngine.h>

#include "DemoUtils.h"

class PII_DEMOUTILS_EXPORT DemoMainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  DemoMainWindow(QWidget *parent = 0);
  virtual ~DemoMainWindow();

protected:
  /**
   * Virtual function which tell what plugins we must load. The
   * default implementation returns an empty list.
   */
  virtual QStringList requiredPlugins() const { return QStringList(); }

  /**
   * Pure virtual function for creating the engine. DemoMainWindow
   * takes the ownership of the PiiEngine pointer. The default
   * implementation returns 0.
   */
  virtual PiiEngine *createEngine() { return 0; }

  /**
   * This function load all necessary plugins and initialize an
   * engine.
   */
  void initEngine();
   
  /**
   * Returns the pointer of the engine.
   */
  PiiEngine *engine() { return _pEngine; }

  /**
   * Returns the current application path.
   */
  QString applicationDirPath() const { return _strApplicationPath; }

  /**
   * Returns the default images path.
   */
  QString defaultImageDirPath() const { return _strDefaultImageDirPath; }
  
  /**
   * Returns the default images path.
   */
  QString defaultVideoDirPath() const { return _strDefaultVideoDirPath; }

  /**
   * Returns the name of the demo.
   */
  QString demoName() const { return _strDemoName; }
  
  /**
   * Open the file dialog where the user can select the image files.
   * These files are returned as QStringList. You can also change the
   * fileMode. See QFileDialog::FileMode for more information.
   */
  QStringList getImageFiles(QFileDialog::FileMode mode = QFileDialog::ExistingFiles);
  
  /**
   * Show the value of the slider.
   */
  void showSliderValue(QSlider *pSlider, const QString& text);

protected slots:
  /**
   * Start the processing if necessary.
   */
  void startProcessing();

  /**
   * Pause the processing if necessary.
   */
  void pauseProcessing();

  /**
   * Stop the processing if necessary.
   */
  void stopProcessing();

private slots:
  /**
   * This function show the error message if the is error occured in engine.
   */
  void handleError(PiiOperation*, const QString& text);
  
private:
  /**
   * Load all necessary plugins. This function try to load all plugins
   * which are listed in requiredPlugins() function.
   */
  void loadPlugins();

  PiiEngine *_pEngine;
  QString _strApplicationPath, _strDefaultImageDirPath, _strDefaultVideoDirPath, _strDemoName;
  
};



#endif //_DEMOMAINWINDOW_H
