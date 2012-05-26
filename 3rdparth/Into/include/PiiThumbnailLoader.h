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

#ifndef _PIITHUMBNAILLOADER_H
#define _PIITHUMBNAILLOADER_H

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QStringList>

#include "PiiGui.h"

class PII_GUI_EXPORT PiiThumbnailLoader : public QThread
{
  Q_OBJECT
  
public:
  PiiThumbnailLoader(QObject *parent = 0);

  void run();

  /**
   * Start the loading thread.
   */
  void startLoading();

  /**
   * Stop the loading thread.
   */
  void stopLoading();

  /**
   * Get the list of the file names which are waiting list.
   */
  QStringList fileNames() const;
  
  /**
   * Set the given file names for the loading. If the thread is not
   * running, we will start it automatically.
   */
  void setFileNames(const QStringList& fileNames);

  /**
   * Add the given file name to the loading list. If the thread is not
   * running, we will start it automatically.
   */
  void addFileName(const QString& fileName);

signals:
  /**
   * This signal was emitted just after we have created the thumbnail
   * by the file name.
   */
  void thumbnailReady(const QString& fileName, const QImage& image);
  
private:
  bool _bRunning;
  QMutex _loadingMutex;
  QStringList _lstFileNames;
};

#endif //_PIITHUMBNAILLOADER_H
