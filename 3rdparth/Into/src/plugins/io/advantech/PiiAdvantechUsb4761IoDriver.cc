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

#include "PiiAdvantechUsb4761IoDriver.h"

PiiAdvantechUsb4761IoDriver::Data::Data() :
  iBoard(0), driverHandle(0)
{
}

PiiAdvantechUsb4761IoDriver::PiiAdvantechUsb4761IoDriver() :
  PiiDefaultIoDriver(new Data)
{
}

PiiAdvantechUsb4761IoDriver::~PiiAdvantechUsb4761IoDriver()
{
  PiiAdvantechUsb4761IoDriver::close();
}

bool PiiAdvantechUsb4761IoDriver::initialize()
{
  PII_D;
  // Open driver
  LRESULT ErrCode = DRV_DeviceOpen(d->iBoard, &d->driverHandle);
  if (ErrCode != SUCCESS)
    return false;

  return PiiDefaultIoDriver::initialize();
}

bool PiiAdvantechUsb4761IoDriver::selectUnit(const QString& unit)
{
  PII_D;
  d->iBoard = unit.toInt();

  return true;
}

bool PiiAdvantechUsb4761IoDriver::close()
{
  PII_D;
  //close driver
  LRESULT ErrCode = DRV_DeviceClose(&d->driverHandle);
  if (ErrCode != SUCCESS)
    return false;
  
  return true;
}

int PiiAdvantechUsb4761IoDriver::channelCount() const
{
  return 16;
}

PiiIoChannel* PiiAdvantechUsb4761IoDriver::createChannel(int channel)
{
  PII_D;
  return new PiiAdvantechUsb4761IoChannel(this,channel,d->driverHandle);
}
