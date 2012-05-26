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

#include "PiiSocketDevice.h"

#include "PiiProgressController.h"

#include <QAbstractSocket>
#include <QLocalSocket>

PiiSocketDevice::Data::Data() :
  pDevice(0),
  type(IODevice)
{}

PiiSocketDevice::Data::Data(QIODevice* device)
{
  if (qobject_cast<QAbstractSocket*>(device) != 0)
    type = AbstractSocket;
  else if (qobject_cast<QLocalSocket*>(device) != 0)
    type = LocalSocket;
  else
    type = IODevice;
  pDevice = device;
}

PiiSocketDevice::Data::Data(QIODevice* device, Type t) :
  pDevice(device),
  type(t)
{
}

PiiSocketDevice::PiiSocketDevice() :
  d(new Data)
{
}

PiiSocketDevice::PiiSocketDevice(QIODevice* device) :
  d(new Data(device))
{
}
PiiSocketDevice::PiiSocketDevice(QAbstractSocket* device) :
  d(new Data(device, AbstractSocket))
{
}

PiiSocketDevice::PiiSocketDevice(QLocalSocket* device) :
  d(new Data(device, LocalSocket))
{
}

PiiSocketDevice::PiiSocketDevice(const PiiSocketDevice& other) :
  d(other.d)
{
  d->reserve();
}

PiiSocketDevice::~PiiSocketDevice()
{
  d->release();
}

PiiSocketDevice& PiiSocketDevice::operator= (const PiiSocketDevice& other)
{
  other.d->assignTo(d);
  return *this;
}

bool PiiSocketDevice::isReadable() const
{
  switch (d->type)
    {
    case AbstractSocket:
      return static_cast<QAbstractSocket*>(d->pDevice)->state() == QAbstractSocket::ConnectedState ||
        d->pDevice->bytesAvailable() > 0;
    case LocalSocket:
      return static_cast<QLocalSocket*>(d->pDevice)->state() == QLocalSocket::ConnectedState ||
        d->pDevice->bytesAvailable() > 0;
    case IODevice:
    default:
      return d->pDevice->openMode() & QIODevice::ReadOnly;
    }
}

bool PiiSocketDevice::isWritable() const
{
  switch (d->type)
    {
    case AbstractSocket:
      return static_cast<QAbstractSocket*>(d->pDevice)->state() == QAbstractSocket::ConnectedState;
    case LocalSocket:
      return static_cast<QLocalSocket*>(d->pDevice)->state() == QLocalSocket::ConnectedState;
    case IODevice:
    default:
      return d->pDevice->openMode() & QIODevice::WriteOnly;
    }
}

bool PiiSocketDevice::waitForDataAvailable(int waitTime, PiiProgressController* controller)
{
  int iTimeout = qMin(waitTime, 100);
  PiiTimer timer;
  do
    {
      if (!isReadable() || (controller != 0 && !controller->canContinue()))
        return false;
      else if (d->pDevice->waitForReadyRead(iTimeout))
        return true;
    }
  while (timer.milliseconds() < waitTime);
  return false;
}
  
qint64 PiiSocketDevice::readWaited(char* data, qint64 maxSize, int waitTime, PiiProgressController* controller)
{
  // Count the total number of bytes read during this call
  qint64 iBytesRead = 0;
  do
    {
      // Read what we can
      qint64 iBytesInPiece = d->pDevice->read(data, maxSize);
      if (iBytesInPiece > 0)
        {
          iBytesRead += iBytesInPiece;
          data += iBytesInPiece;
          maxSize -= iBytesInPiece;
        }
      else if (iBytesInPiece < 0) // error
        return iBytesRead > 0 ? iBytesRead : -1;
      else if (!waitForDataAvailable(waitTime, controller))
        return iBytesRead;
    }
  while (maxSize > 0);
  
  return iBytesRead;
}

bool PiiSocketDevice::waitForDataWritten(int waitTime, PiiProgressController* controller)
{
  int iTimeout = qMin(waitTime, 100);
  PiiTimer timer;
  do
    {
      if (!isWritable() || (controller != 0 && !controller->canContinue()))
        return false;
      else if (d->pDevice->waitForBytesWritten(iTimeout))
        {
          if (d->pDevice->bytesToWrite() <= 0)
            return true;
        }
      else
        return false;
    }
  while (timer.milliseconds() < waitTime);
  return true;
}
  
qint64 PiiSocketDevice::writeWaited(const char* data, qint64 maxSize, int waitTime, PiiProgressController* controller)
{
  qint64 iBytesWritten = 0;
  do
    {       
      qint64 iBytesInPiece = d->pDevice->write(data, maxSize);
      if (iBytesInPiece > 0)
        {
          data += iBytesInPiece;
          iBytesWritten += iBytesInPiece;
          maxSize -= iBytesInPiece;
        }
      else if (iBytesInPiece < 0)
        return iBytesWritten != 0 ? iBytesWritten : -1;
      else if (!waitForDataWritten(waitTime, controller))
        return iBytesWritten;
    }
  while (maxSize > 0);
  
  return iBytesWritten;
}

void PiiSocketDevice::disconnect()
{
  switch (d->type)
    {
    case AbstractSocket:
      static_cast<QAbstractSocket*>(d->pDevice)->disconnectFromHost();
      break;
    case LocalSocket:
      static_cast<QLocalSocket*>(d->pDevice)->disconnectFromServer();
      break;
    }
}

QIODevice* PiiSocketDevice::device() const
{
  return d->pDevice;
}

PiiSocketDevice::operator QIODevice* () const
{
  return d->pDevice;
}
  
QIODevice* PiiSocketDevice::operator-> () const
{
  return d->pDevice;
}
