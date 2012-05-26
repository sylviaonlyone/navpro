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
#include <PiiOperationItem.h>

#include <QFileDialog>
#include <QSettings>
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          _pScene(0)
{
  init();
}

void MainWindow::init()
{
  try
    {
      PiiEngine::loadPlugin("piibase");
      PiiEngine::loadPlugin("piiimage");
      PiiEngine::loadPlugin("piidsp");
      PiiEngine::loadPlugin("piicamera");
      PiiEngine::loadPlugin("piiclassification");
      PiiEngine::loadPlugin("piicolors");
      PiiEngine::loadPlugin("piidatabase");
      PiiEngine::loadPlugin("piifeatures");
      PiiEngine::loadPlugin("piiflowcontrol");
      PiiEngine::loadPlugin("piigeometry");
      PiiEngine::loadPlugin("piiio");
      //PiiEngine::loadPlugin("piioptimization");
      PiiEngine::loadPlugin("piistatistics");
      PiiEngine::loadPlugin("piitexture");
      PiiEngine::loadPlugin("piitracking");
      PiiEngine::loadPlugin("piitransforms");
      PiiEngine::loadPlugin("piivideo");
      PiiEngine::loadPlugin("kidebase");
    }
  catch (PiiLoadException& ex)
    {
      QMessageBox::critical(0, tr("Application error"), ex.message() );
      exit(1);
    }

  setupUi(this);
  connect(_pActionExit, SIGNAL(triggered()), this, SLOT(close()));
  connect(_pActionSaveConfiguration, SIGNAL(triggered()), this, SLOT(saveConfiguration()));
  connect(_pActionLoadConfiguration, SIGNAL(triggered()), this, SLOT(loadConfiguration()));
  
  _pScene = new PiiGraphicsScene(this);
  _pGraphicsView->setScene(_pScene);
  _pScene->setEngine(new PiiEngine);
}

void MainWindow::loadConfiguration()
{
  //open an existing configuration
  QSettings settings("Intopii", "Paja");
  QFileDialog dialog;
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setFilter(tr("Configuration files (*.cft)"));
  dialog.setWindowTitle(tr("Select an configuration file"));
  dialog.setDirectory(settings.value("configurationDirectory").toString());

  QList<QUrl> lstUrls = dialog.sidebarUrls();
  lstUrls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
  dialog.setSidebarUrls(lstUrls);
  
  if (dialog.exec())
    loadConfiguration(dialog.selectedFiles()[0]);
}

void MainWindow::loadConfiguration(const QString& fileName)
{
  try
    {
      QFile f(fileName);
      if (!f.open(QIODevice::ReadOnly))
        {
          showError(tr("Cannot open the configuration file '%1'").arg(fileName));
          return;
        }
      else
        {
          _pScene->setEngine(PiiEngine::load(fileName));
        }
    }
  catch(PiiSerializationException& ex)
    {
      showError(tr("Error in loading configuration.\n"
                   "Message: %1\n"
                   "Info: %2").arg(ex.message()).arg(ex.info()));
    }
}

void MainWindow::saveConfiguration()
{
  QSettings settings("Intopii", "Paja");

  QFileDialog dialog;
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setFilter(tr("Configuration files (*.cft)"));
  dialog.setWindowTitle(tr("Save the configuration"));
  dialog.setDirectory(settings.value("configurationDirectory").toString());
  
  QList<QUrl> lstUrls = dialog.sidebarUrls();
  lstUrls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
  dialog.setSidebarUrls(lstUrls);
  
  if (dialog.exec())
    {
      QFileInfo fileInfo(dialog.selectedFiles()[0]);
      settings.setValue("configurationDirectory", fileInfo.absolutePath());
      saveConfiguration(QString("%1/%2.cft").arg(fileInfo.path()).arg(fileInfo.baseName()));
    }
  else
    return;
}

void MainWindow::saveConfiguration(const QString& fileName)
{
  try
    {
      QFile f(fileName);
      
      if (f.open(QIODevice::WriteOnly))
        _pScene->engine()->save(fileName);
      else
        showError(tr("Cannot save the configuration file '%1'").arg(fileName));
    }
  catch (PiiSerializationException& ex)
    {
      showError(QString("Error in saving configuration\n"
                        "Message: %1\n"
                        "Info: %2").arg(ex.message()).arg(ex.info()));
    }
}

void MainWindow::showError(const QString& message)
{
  QMessageBox::critical(0, tr("Error"), message);
  qCritical("%s", message.toUtf8().constData());
}

