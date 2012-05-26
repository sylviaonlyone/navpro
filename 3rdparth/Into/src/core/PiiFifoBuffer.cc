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

#include "PiiFifoBuffer.h"
#include <QMutexLocker>
#include <cstdlib>

PiiFifoBuffer::Data::Data(qint64 size) :
  iSize(size),
  iReadStart(0),
  iWriteStart(0),
  iFreeSpace(size),
  iReadTotal(0),
  pBuffer((char*)std::malloc(size)),
  ulWaitTime(100),
  bWriteFinished(false)
{
}

PiiFifoBuffer::PiiFifoBuffer(qint64 size) :
  d(new Data(size))
{
  open(QIODevice::ReadWrite);
}

PiiFifoBuffer::~PiiFifoBuffer()
{
  ::free(d->pBuffer);
  delete d;
}

bool PiiFifoBuffer::atEnd() const
{
  return (d->iSize - d->iFreeSpace) == 0 && d->bWriteFinished;
}

qint64 PiiFifoBuffer::pos() const
{
  //qDebug("Pos: %d", (int)(d->iReadTotal - QIODevice::bytesAvailable()));
  return d->iReadTotal - QIODevice::bytesAvailable();
}

bool PiiFifoBuffer::seek(qint64 position)
{
  d->bufferLock.lock();
  //qDebug("Seek to: %d, current: %d", (int)position, (int)pos());
  // Cannot seek back beyond the unget buffer
  if (position < pos())
    {
      d->bufferLock.unlock();
      return false;
    }

  QIODevice::seek(position);
  //qDebug("Pos after QIODevice::seek: %d", (int)pos());
  qint64 diff = position - d->iReadTotal;
  if (diff > 0)
    {
      //qDebug("Discarding %d bytes", (int)diff);
      return readBytes(0, diff) == diff;
    }
  d->bufferLock.unlock();
  return true;
}

qint64 PiiFifoBuffer::bytesAvailable () const
{
  /*qDebug("Bytes available: %d (unget buffer: %d)",
         (int)(d->iSize-d->iFreeSpace+QIODevice::bytesAvailable()),
         (int)QIODevice::bytesAvailable());
  */
  // My free space + the size of the unget buffer
  return d->iSize - d->iFreeSpace + QIODevice::bytesAvailable();
}

void PiiFifoBuffer::setWaitTime(unsigned long readWaitTime) { d->ulWaitTime = readWaitTime; }
unsigned long PiiFifoBuffer::waitTime() const { return d->ulWaitTime; }

bool PiiFifoBuffer::isSequential() const { return true; }

bool PiiFifoBuffer::reset()
{
  QIODevice::reset();
  QMutexLocker lock(&d->bufferLock);
  d->iReadStart = d->iWriteStart = d->iReadTotal = 0;
  d->iFreeSpace = d->iSize;
  d->bWriteFinished = false;
  return true;
}

void PiiFifoBuffer::finishWriting()
{
  QMutexLocker lock(&d->bufferLock);
  d->bWriteFinished = true;
  // Wake a waiting reader
  d->dataWritten.wakeOne();
}

qint64 PiiFifoBuffer::readData(char * data, qint64 maxSize)
{
  //qDebug("PiiFifoBuffer::readData(0x%x, %d)", (int)data, (int)maxSize);
  if (maxSize == 0)
    return 0;
  
  d->bufferLock.lock();
  return readBytes(data, maxSize);
}

qint64 PiiFifoBuffer::readBytes(char * data, qint64 maxSize)
{
  qint64 bytesRemaining = maxSize;
  
  // Read until everything was received
  while (bytesRemaining > 0)
    {
      // If no data is available, we must wait
      if (d->iSize - d->iFreeSpace == 0)
        {
          // If writing has not been finished, and wait time is set, we need to wait a bit
          if (!d->bWriteFinished && d->ulWaitTime > 0)
            d->dataWritten.wait(&d->bufferLock, d->ulWaitTime);
          // No more data is available -> return
          // This happens if writing is finished or new data comes too late
          if (d->iSize - d->iFreeSpace == 0)
            {
              /*qDebug("Read wait exited with no signal! "
                     "remaining: %d, maxSize: %d, bytes available: %d",
                     (int)bytesRemaining, (int)maxSize, (int)(d->iSize - d->iFreeSpace));
              */
              d->bufferLock.unlock();
              return maxSize - bytesRemaining;
            }
        }
      // Read as much as possible
      // Len must be larger than zero because new data has arrived
      qint64 len = bytesRemaining < (d->iSize - d->iFreeSpace) ? bytesRemaining : (d->iSize - d->iFreeSpace);
      
      //qDebug("Reading %d bytes", (int)len);
      qint64 end = d->iReadStart + len;
      //printf("len = %d, freeSpace = %d, start = %d, end = %d\n", len, d->iFreeSpace, d->iReadStart, end);
      // The buffer is addressed circularly.
      if (end <= d->iSize)  // We can read the whole part at once
        {
          if (data)
            ::memcpy(data, d->pBuffer + d->iReadStart, len);
          d->iReadStart += len;
          if (d->iReadStart >= d->iSize)
            d->iReadStart -= d->iSize;
        }
      else // We must read both at the end and at the beginning
        {
          qint64 firstPieceLen = d->iSize - d->iReadStart;
          if (data)
            {
              ::memcpy(data, d->pBuffer + d->iReadStart, firstPieceLen);
              ::memcpy(data + firstPieceLen, d->pBuffer, len-firstPieceLen);
            }
          d->iReadStart = len-firstPieceLen;
        }
      d->iFreeSpace += len;
      data += len;
      bytesRemaining -= len;
      d->iReadTotal += len;

      // Wake up any pending write operation
      d->dataRead.wakeOne();
    }

  d->bufferLock.unlock();
  return maxSize;
}

qint64 PiiFifoBuffer::writeData(const char * data, qint64 maxSize)
{
  //qDebug("PiiFifoBuffer::writeData(0x%x, %d)", (int)data, (int)maxSize);
  if (maxSize == 0)
    return 0;
  
  d->bufferLock.lock();

  // Analogous to readData
  qint64 bytesRemaining = maxSize;

  // Write in pieces until everything is completed
  while (bytesRemaining > 0)
    {
      // Write as much as possible
      // Can't write yet. Wait for a while
      if (d->iFreeSpace == 0)
        {
          //qDebug("Cannot write yet. Waiting for %ld msec.", d->ulWaitTime);
          // This is signalled whenever new data arrives
          bool dataRead = d->ulWaitTime > 0 && d->dataRead.wait(&d->bufferLock, d->ulWaitTime);
          if (!dataRead)
            {
              /*qDebug("Write wait exited with no signal! "
                     "remaining: %d, maxSize: %d, free space: %d",
                     (int)bytesRemaining, (int)maxSize, (int)d->iFreeSpace);
              */
              d->bufferLock.unlock();
              return maxSize - bytesRemaining;
            }
        }
      qint64 len = bytesRemaining < d->iFreeSpace ? bytesRemaining : d->iFreeSpace;

      //qDebug("Writing %d bytes", (int)len);
      qint64 end = d->iWriteStart + len;
      //printf("len = %d, freeSpace = %d, start = %d, end = %d\n", len, d->iFreeSpace, d->iWriteStart, end);
      if (end <= d->iSize)
        {
          ::memcpy(d->pBuffer + d->iWriteStart, data, len);
          d->iWriteStart += len;
          if (d->iWriteStart >= d->iSize)
            d->iWriteStart -= d->iSize;
        }
      else
        {
          qint64 firstPieceLen = d->iSize - d->iWriteStart;
          ::memcpy(d->pBuffer + d->iWriteStart, data, firstPieceLen);
          ::memcpy(d->pBuffer, data + firstPieceLen, len-firstPieceLen);
          d->iWriteStart = len-firstPieceLen;
        }
      d->iFreeSpace -= len;
      data += len;
      bytesRemaining -= len;

      // Wake up any pending read operation
      d->dataWritten.wakeOne();
    }

  d->bufferLock.unlock();
  return maxSize;
}
