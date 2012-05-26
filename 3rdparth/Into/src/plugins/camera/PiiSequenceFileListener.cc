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

#include "PiiSequenceFileListener.h"

PiiSequenceFileListener::Data::Data(PiiCameraDriver *driver) :
  pDriver(driver)
{
}

PiiSequenceFileListener::PiiSequenceFileListener(const QString& fileName, PiiCameraDriver* driver) :
  PiiSequenceFile(new Data(driver), fileName)
{
  _d()->pDriver->setListener(this);
}

bool PiiSequenceFileListener::open(QIODevice::OpenMode mode)
{
  PII_D;
  if ((mode & WriteOnly) && (d->pDriver == 0 || !d->pDriver->isOpen()))
    return false;

  setFrameSize(d->pDriver->frameSize());
  setImageFormat((PiiCamera::ImageFormat)d->pDriver->imageFormat());
  setBitsPerPixel(d->pDriver->bitsPerPixel());

  return PiiSequenceFile::open(mode);
}

void PiiSequenceFileListener::frameCaptured(int frameIndex)
{
  if (!isOpen())
    return;
  
  void* buffer = _d()->pDriver->frameBuffer(frameIndex);
  if (buffer != 0)
    writeFrame(buffer);
}
