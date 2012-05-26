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

#include "PiiThumbnailLoader.h"
#include <PiiQImage.h>

PiiThumbnailLoader::PiiThumbnailLoader(QObject *parent) : QThread(parent), _bRunning(false)
{
}

void PiiThumbnailLoader::run()
{
  while (_bRunning)
    {
      _loadingMutex.lock();
      if (_lstFileNames.isEmpty())
        {
          _bRunning = false;
          _loadingMutex.unlock();
          break;
        }
      QString strFileName = _lstFileNames.takeFirst();
      _loadingMutex.unlock();
      
      QImage image(strFileName);
      if (image.format() == QImage::Format_ARGB32)
        Pii::setQImageFormat(&image, QImage::Format_RGB32);
      emit thumbnailReady(strFileName, image.scaled(70,90,Qt::KeepAspectRatio));

      _loadingMutex.lock();
      if (_lstFileNames.isEmpty())
        _bRunning = false;
      _loadingMutex.unlock();
    }
}

void PiiThumbnailLoader::setFileNames(const QStringList& fileNames)
{
  _loadingMutex.lock();
  _lstFileNames = fileNames;
  _loadingMutex.unlock();

  if (!_bRunning)
    startLoading();
}

void PiiThumbnailLoader::addFileName(const QString& fileName)
{
  _loadingMutex.lock();
  _lstFileNames << fileName;
  _loadingMutex.unlock();

  if (!_bRunning)
    startLoading();
}

void PiiThumbnailLoader::startLoading()
{
  _bRunning = true;
  start();
}

void PiiThumbnailLoader::stopLoading()
{
  _bRunning = false;
}

