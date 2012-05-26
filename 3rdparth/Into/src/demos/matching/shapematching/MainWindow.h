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

#include <PiiQImage.h>
#include <PiiImageOverlay.h>

#include <QImage>
#include <QList>
#include <QButtonGroup>

class MainWindow : public DemoMainWindow, private Ui::MainWindow
{
  Q_OBJECT
  
public:
  MainWindow(QWidget *parent = 0);

  struct Model
  {
    QString strFileName;
    QColor color;
    QList<PiiMatrix<int> > lstBoundaries;
  };
  
protected:
  QStringList requiredPlugins() const { return QStringList() << "piiimage" << "piibase" << "piimatching"; }
  PiiEngine* createEngine();

  
signals:
  void trigImage(int);
  
private slots:
  void nextButtonClicked();
  void prevButtonClicked();
  void updateModels(const PiiVariant& name, const QVariantList& information);
  void updateImage(const PiiVariant& image, const QVariantList& information);
  
private:
  void init();
  
  QList<Model> _lstModels;
  QList<PiiImageOverlay*> _lstOverlays;
  QList<QColor> _lstColors;
};



#endif //_MAINWINDOW_H
