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

#include "PiiCurreraIoDriver.h"

PiiCurreraIoDriver::Data::Data() :
  iDevice(0), driverHandle(0)
{
}

PiiCurreraIoDriver::PiiCurreraIoDriver() :
  PiiDefaultIoDriver(new Data)
{
}

PiiCurreraIoDriver::~PiiCurreraIoDriver()
{
  PiiCurreraIoDriver::close();
}

bool PiiCurreraIoDriver::initialize()
{
  PII_D;
  // Open driver
  XI_RETURN ret = XiApiExt::xiOpenDevice(d->iDevice, &d->driverHandle);
  if (ret != XI_OK)
    {
      qWarning(tr("Couldn't open currera-device (%1)").arg(d->iDevice).toAscii().constData());
      return false;
    }
  
  return PiiDefaultIoDriver::initialize();
}

bool PiiCurreraIoDriver::selectUnit(const QString& unit)
{
  _d()->iDevice = unit.toInt();

  return true;
}

bool PiiCurreraIoDriver::close()
{
  PII_D;
  
  // Close driver
  return XiApiExt::xiCloseDevice(d->driverHandle) == XI_OK;
}

int PiiCurreraIoDriver::channelCount() const
{
  return 8;
}

PiiIoChannel* PiiCurreraIoDriver::createChannel(int channel)
{
  PII_D;
  return new PiiCurreraIoChannel(this,channel,d->driverHandle);
}

