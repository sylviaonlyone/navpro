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

#include "DemoMainWindow.h"

#include <QSettings>
#include <QImageReader>
#include <QMessageBox>
#include <QToolTip>

#include <PiiLoadException.h>

#include <QtDebug>

#include <signal.h>
#include <PiiYdinUtil.h>

static PiiEngine *dumpingEngine;

extern "C" void dumpEngine(int /*sig*/)
{
  qDebug("KideApplicationDataImpl : dumpEngine");
  qDebug("************************************");
  if (dumpingEngine)
    PiiYdin::dumpOperation(dumpingEngine,0,PiiYdin::ShowInputQueues | PiiYdin::ShowOutputStates);
  else
    qDebug(" dumpingEngine == 0");
  qDebug("************************************");
}

DemoMainWindow::DemoMainWindow(QWidget *parent) :
  QMainWindow(parent),
  _pEngine(0)
{
#ifdef __linux
  signal(SIGUSR1, dumpEngine);
#endif

  // Initialize application path
  QDir dir(QCoreApplication::applicationDirPath());
  if (dir.dirName() == "release" || dir.dirName() == "debug")
    dir.cdUp();

  _strApplicationPath = dir.path();

  QSettings settings("Intopii", "Demos");
  _strDefaultImageDirPath = settings.value("defaultImageDirectory", ".").toString();
  _strDefaultVideoDirPath = settings.value("defaultVideoDirectory", ".").toString();

  // Set window title
  _strDemoName = QString("Into demo");
  
  QFile file(QString("%1/README").arg(_strApplicationPath));
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream stream(&file);
      _strDemoName = stream.readLine();
    }
  file.close();
}

DemoMainWindow::~DemoMainWindow()
{
  if (_pEngine)
    {
      stopProcessing();
      delete _pEngine;
    }
}

void DemoMainWindow::initEngine()
{
  // Load all necessary plugins
  loadPlugins();
  
  // Create an engine.
  _pEngine = createEngine();
  dumpingEngine = _pEngine;

  if (_pEngine)
    connect(_pEngine, SIGNAL(errorOccured(PiiOperation*,const QString&)),
            this, SLOT(handleError(PiiOperation*,const QString&)));
}

void DemoMainWindow::handleError(PiiOperation*, const QString& text)
{
  QMessageBox::critical(0, "Application error", text);
}

void DemoMainWindow::loadPlugins()
{
  QStringList lstPlugins = requiredPlugins();

  // Load necessary plugins
  try
    {
      for (int i=0; i<lstPlugins.size(); i++)
        PiiEngine::loadPlugin(lstPlugins[i]);
    }
  catch (PiiLoadException& ex)
    {
      QMessageBox::critical(0, "Application error", ex.message() );
      exit(1);
    }
}

QStringList DemoMainWindow::getImageFiles(QFileDialog::FileMode mode)
{
  // Get existing image directory
  QSettings settings("Intopii", "Demos");
  QString strImageDirectory = settings.value("imageDirectory", ".").toString();

  // Create and configure fileDialog
  QFileDialog dialog;
  dialog.setFileMode(mode);
  dialog.setDirectory(strImageDirectory);
  dialog.setWindowTitle(tr("Select image files"));

  // Get supported image formats
  QList<QByteArray> formats = QImageReader::supportedImageFormats();
  QString filterStr("Image files (");
  for (int i=0; i<formats.size(); i++)
    {
      if (i>0)
        filterStr += ' ';
      filterStr += QString("*.") + formats[i];
    }
  filterStr += ')';
  dialog.setFilter(filterStr);

  QStringList lstFileNames;
  
  // If all is okay, set file names
  if (dialog.exec())
    {
      // Get select file names
      lstFileNames = dialog.selectedFiles();

      // Store current directory to the settings
      settings.setValue("imageDirectory", QFileInfo(lstFileNames[0]).absolutePath());
    }

  return lstFileNames;
}

void DemoMainWindow::pauseProcessing()
{
  if (_pEngine->state() == PiiOperation::Running)
    {
      _pEngine->pause();
      _pEngine->wait(PiiOperation::Paused);
    }
}

void DemoMainWindow::stopProcessing()
{
  if (_pEngine->state() != PiiOperation::Stopped)
    {
      _pEngine->interrupt();
      _pEngine->wait(PiiOperation::Stopped);
    }
}

void DemoMainWindow::startProcessing()
{
  if (_pEngine->state() != PiiOperation::Running)
    {
       try
        {
          _pEngine->execute();
          /*
          if (!_pEngine->wait(PiiOperation::Running, 1000))
            PiiYdin::dumpOperation(_pEngine, 0,
            PiiYdin::ShowInputQueues | PiiYdin::ShowState);
          */
        }
      catch (PiiExecutionException& ex)
        {
          QMessageBox::critical(0, "Application error", ex.message() );
        }
    }
}


void DemoMainWindow::showSliderValue(QSlider *pSlider, const QString& text)
{
  // Show tooltip
  pSlider->setToolTip(text);
  int x = pSlider->width() * (pSlider->value() - pSlider->minimum())
    / (pSlider->maximum() - pSlider->minimum());
  QToolTip::showText(pSlider->mapToGlobal(QPoint(x,5)), text, this);
}

