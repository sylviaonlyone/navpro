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

#include "PiiCameraLinkSerialDevice.h"
#include <PiiDelay.h>
#include "PiiCameraLinkSerial.h"

#include <string.h> // for memcpy
#include <cstdlib> // for free and malloc

#include <QtDebug>

PiiCameraLinkSerialDevice::Data::Data() :
  pSerialRef(0),
  iPortIndex(0),
  iBaudRate(9600)
{
}

PiiCameraLinkSerialDevice::Data::~Data()
{
}

PiiCameraLinkSerialDevice::PiiCameraLinkSerialDevice() : d(new Data)
{
}

PiiCameraLinkSerialDevice::~PiiCameraLinkSerialDevice()
{
  delete d;
}

void PiiCameraLinkSerialDevice::close()
{
  if (isOpen())
    {
      clSerialClose(d->pSerialRef);
    }
}

qint64 PiiCameraLinkSerialDevice::bytesAvailable() const
{
  //qDebug("PiiCameraLinkSerialDevice::bytesAvailable() BEGIN");
  int status;
  unsigned int numBytes;
  status = clGetNumBytesAvail(d->pSerialRef, &numBytes);
  if (status != CL_ERR_NO_ERR)
    {
      //qDebug("PiiCameraLinkSerialDevice::bytesAvailable() status = %d", status);
      //qDebug("PiiCameraLinkSerialDevice::bytesAvailable() RETURN 0");
      return 0;
    }
  //qDebug("PiiCameraLinkSerialDevice::bytesAvailable() RETURN NON ZERO, numBytes = %u", numBytes);
  return (qint64)numBytes + QIODevice::bytesAvailable();
}

bool PiiCameraLinkSerialDevice::open(OpenMode mode)
{
  return connectToPort(mode, d->iPortIndex, d->iBaudRate);
}

bool PiiCameraLinkSerialDevice::waitForReadyRead(int msecs)
{
  if (bytesAvailable() > 0)
    return true;

  int elapsed = 0;
  
  while (elapsed < msecs)
    {
      PiiDelay::msleep(10);
      if (bytesAvailable()>0)
        {
          return true;
        }
      elapsed += 10;
    }
  return false;
}


// TODO: handle the situation, where the port is allready open!!
bool PiiCameraLinkSerialDevice::connectToPort(OpenMode mode, int portIndex, int baudRate)
{
  int status = 0;
  bool success = false;
  
  // If the serial link connection is allready open, close it before
  // opening another connection.
  if (isOpen())
    close();
  
  // Initialize Camera Link Serial Connection
  status = clSerialInit ((unsigned int)portIndex, &d->pSerialRef);
  if (status != CL_ERR_NO_ERR)
    {
      // Error in initializing the serial device.
      d->pSerialRef = 0;
      return false;
    }
  QIODevice::open(mode);
  
  // Inform the base class about changed open mode.
  success = initBaudRate(baudRate);
  if (success == false)
    {
      // Error in setting the baud rate.
      d->pSerialRef = 0;
      setOpenMode(NotOpen);
      return false;
    }

  return true;
}


int PiiCameraLinkSerialDevice::portCount() const
{
  unsigned int numPorts;
  int status;
  status = clGetNumSerialPorts (&numPorts);
  if (status != CL_ERR_NO_ERR)
    {
      return -1;
    }
  return int(numPorts);
}

QList<unsigned long> PiiCameraLinkSerialDevice::supportedBaudRates() const
{
  QList<unsigned long> retList;
  unsigned int baudRates;
  if (isOpen())
    {
      int status = clGetSupportedBaudRates (d->pSerialRef, &baudRates);
      if (status != CL_ERR_NO_ERR)
        {
          if (baudRates&CL_BAUDRATE_9600)
            retList << 9600;
          if (baudRates&CL_BAUDRATE_19200)
            retList << 19200;
          if (baudRates&CL_BAUDRATE_38400)
            retList << 38400;
          if (baudRates&CL_BAUDRATE_57600)
            retList << 57600;
          if (baudRates&CL_BAUDRATE_115200)
            retList << 115200;
          if (baudRates&CL_BAUDRATE_230400)
            retList << 230400;
          if (baudRates&CL_BAUDRATE_460800)
            retList << 460800;
          if (baudRates&CL_BAUDRATE_921600)
            retList << 921600;
        }
    }
  return retList; 
}

bool PiiCameraLinkSerialDevice::initBaudRate(unsigned int rate)
{
  if (isOpen())
    {
      unsigned int baudRateId = 0;
      if (supportedBaudRates().contains(rate))
        {
          if (rate == 9600)
            baudRateId = CL_BAUDRATE_9600;
          else if (rate == 19200)
            baudRateId = CL_BAUDRATE_19200;
          else if (rate == 38400)
            baudRateId = CL_BAUDRATE_38400;
          else if (rate == 57600)
            baudRateId = CL_BAUDRATE_57600;
          else if (rate == 115200)
            baudRateId = CL_BAUDRATE_115200;
          else if (rate == 230400)
            baudRateId = CL_BAUDRATE_230400;
          else if (rate == 460800)
            baudRateId = CL_BAUDRATE_460800;
          else if (rate == 921600)
            baudRateId = CL_BAUDRATE_921600;
          else
            return false; // Unsupported baud rate
        }
      int status = clSetBaudRate (d->pSerialRef, baudRateId);
      if (status == CL_ERR_NO_ERR)
        return true;
    }
  return false;
}
    
qint64 PiiCameraLinkSerialDevice::readData(char* data, qint64 maxSize)
{
  //qDebug() << "PiiCameraLinkSerialDevice::readData() BEGIN, maxSize =" << maxSize;
  
  if (isReadable())
    {
      unsigned long size = (unsigned long)maxSize;
      int status = clSerialRead (d->pSerialRef, data, &size, 1000);
      if (status != CL_ERR_NO_ERR)
        {
          // Error in reading the data from the device.
          //qDebug("PiiCameraLinkSerialDevice::readData() RETURN -1");
          return -1;
        }
      //qDebug("PiiCameraLinkSerialDevice::readData() RETURN %lu", size);
      return size;
    }
  //qDebug("PiiCameraLinkSerialDevice::readData() RETURN -1");
  return -1; //Error, not readable.
}

qint64 PiiCameraLinkSerialDevice::writeData(const char* data, qint64 maxSize)
{
  //qDebug("This: %p, serialRef: %p", this, d->pSerialRef);
  if (isWritable())
    {
  
      char *copyData;
      long unsigned int size = (long unsigned int)maxSize;
      copyData = (char*)malloc(sizeof(char)*maxSize);
      memcpy(copyData, data, maxSize);
  
      int status = clSerialWrite (d->pSerialRef, copyData, &size, 1000);
      free(copyData);
      if (status != CL_ERR_NO_ERR)
        {
          // Error in writing the data
          return -1;
        }
      return (qint64)size;
    }
  return -1; // Error, device not writable
}  


bool PiiCameraLinkSerialDevice::isSequential() const { return true; }
void PiiCameraLinkSerialDevice::setPortIndex(int index) { d->iPortIndex = index; }
void PiiCameraLinkSerialDevice::setBaudRate(int baudRate) { d->iBaudRate = baudRate; }
