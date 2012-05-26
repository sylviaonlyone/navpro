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

#include "PiiSequenceFile.h"

#include <QFile>
#include <QString>


PiiSequenceFile::Data::Data() :
  iBytesPerFrame(0)
{
}

PiiSequenceFile::Data::~Data()
{
}

PiiSequenceFile::PiiSequenceFile(Data *data, const QString& fileName) :
  QFile(fileName),
  d(data)
{
}

PiiSequenceFile::PiiSequenceFile(const QString& fileName) :
  QFile(fileName),
  d(new Data)
{
}


PiiSequenceFile::~PiiSequenceFile()
{
  delete d;
}

void PiiSequenceFile::setFrameSize(QSize frameSize)
{
  d->header.width = frameSize.width();
  d->header.height = frameSize.height();
  calculateFrameSize();
}

QSize PiiSequenceFile::frameSize() const
{
  return QSize(d->header.width, d->header.height);
}

void PiiSequenceFile::setBitsPerPixel(unsigned int bitsPerPixel)
{
  d->header.bitsPerPixel = bitsPerPixel;
  calculateFrameSize();
}

void PiiSequenceFile::calculateFrameSize()
{
  d->iBytesPerFrame = d->header.height * d->header.width * (d->header.bitsPerPixel >> 3);
}

bool PiiSequenceFile::writeFrame(const void* frameBuffer)
{
  if (!isOpen())
    return false;

  return write(static_cast<const char*>(frameBuffer), bytesPerFrame()) < bytesPerFrame();
}

bool PiiSequenceFile::readFrame(void* frameBuffer)
{
  if (!isOpen() || atEnd())
    return false;

  // Check that we really get a full frame from file.
  if (read(static_cast<char*>(frameBuffer), bytesPerFrame()) < bytesPerFrame())
    return false;
 
  return true;
}


bool PiiSequenceFile::seekFrame(int frameIndex)
{
  if (!isOpen())
    return false;
  
  if (frameIndex < 0)
    {
      // Negative index and total number unknown
      if (frameCount() < 0)
        return false;
      // Index from the end
      frameIndex += frameCount();
      if (frameIndex < 0)
        return false;
    }

  return seek(sizeof(Header) + bytesPerFrame() * frameIndex);
}


bool PiiSequenceFile::open(QIODevice::OpenMode mode)
{
  if (!QFile::open(mode))
    return false;

  bool result = true;
  
  // Opened for writing -> write initial header
  if (mode & WriteOnly && pos() == 0)
    result = write(reinterpret_cast<const char*>(&d->header), sizeof(Header)) == sizeof(Header);  
  // Opened for reading -> check header
  else if (mode & ReadOnly)
    {
      if (read(reinterpret_cast<char*>(&d->header), sizeof(Header)) != sizeof(Header))
        result = false;
      else if (d->header.magic != 0x31415927)
        return false;
    }

  if (!result)
    QFile::close();

  return result;
}

void PiiSequenceFile::close()
{
   //If the file was opened with write mode, save the current header.
  if (openMode() & QIODevice::WriteOnly)
    {
      seek(0);
      write(reinterpret_cast<const char*>(&d->header), sizeof(Header));
    }

  QFile::close();
}

PiiSequenceFile::Header PiiSequenceFile::header() const
{
  return d->header;
}


unsigned int PiiSequenceFile::bitsPerPixel() const { return d->header.bitsPerPixel; }
void PiiSequenceFile::setImageFormat(PiiCamera::ImageFormat format) { d->header.imageFormat = format; }
PiiCamera::ImageFormat PiiSequenceFile::imageFormat() const { return d->header.imageFormat; }
void PiiSequenceFile::setFrameCount(int frameCount) { d->header.frameCount = frameCount; }
int PiiSequenceFile::frameCount() const { return d->header.frameCount; }
int PiiSequenceFile::bytesPerFrame() const { return d->iBytesPerFrame; }
