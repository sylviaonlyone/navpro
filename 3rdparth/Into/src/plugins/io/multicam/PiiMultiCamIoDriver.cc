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

#include "PiiMultiCamIoDriver.h"
#include <QtDebug>

PiiMultiCamIoDriver::Data::Data() :
  iBoard(0),
  bOpen(false)
{}

PiiMultiCamIoDriver::PiiMultiCamIoDriver() :
  PiiDefaultIoDriver(new Data)
{
}

PiiMultiCamIoDriver::~PiiMultiCamIoDriver()
{
  PiiMultiCamIoDriver::close();
}

bool PiiMultiCamIoDriver::initialize()
{
  if (d->bOpen)
    {
      qWarning(tr("MultiCam io-driver is already open. Close the driver first.").toAscii());
      return false;
    }

  // Open driver
  qDebug("io:McOpenDriver");
  MCSTATUS status = McOpenDriver(0);
  if (status != MC_OK)
    {
      qWarning(tr("Cannot open multicam io-driver.").toAscii());
      return false;
    }
  
  d->bOpen = true;

  return PiiDefaultIoDriver::initialize();
}

bool PiiMultiCamIoDriver::selectUnit(const QString& unit)
{
  PII_D;
  d->iBoard = unit.toInt();

  return true;
}

bool PiiMultiCamIoDriver::isOpen()
{
  return _d()->bOpen;
}

bool PiiMultiCamIoDriver::close()
{
  if (_d()->bOpen)
    {
      MCSTATUS status = McCloseDriver();
      if (status != MC_OK)
        return false;
    }

  return true;
}

int PiiMultiCamIoDriver::channelCount() const
{
  const PII_D;
  
  int count=0;
  int value;
  MCSTATUS status;
  do
    {
      status = McGetParamInt(MC_BOARD + d->iBoard, MC_InputFunction + count, &value);
      if (status != MC_OK)
        break;
      ++count;
    }
  while(value != MC_InputFunction_NONE);

  return count;
}

PiiIoChannel* PiiMultiCamIoDriver::createChannel(int channel)
{
  return new PiiMultiCamIoChannel(this, channel, _d()->iBoard);
}
