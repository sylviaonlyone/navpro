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

#include "PiiDefaultIoDriver.h"
#include <PiiDefaultIoChannel.h>

int PiiDefaultIoDriver::_iInstanceCounter = 0;
PiiIoThread* PiiDefaultIoDriver::_pSendingThread = 0;

PiiDefaultIoDriver::Data::Data()
{
}

PiiDefaultIoDriver::Data::~Data()
{
  //delete channels
  qDeleteAll(lstChannels);
  lstChannels.clear();
}

PiiDefaultIoDriver::PiiDefaultIoDriver(Data* data) : d(data)
{
  if (_iInstanceCounter == 0)
    {
      _pSendingThread = new PiiIoThread;
      _pSendingThread->start();
    }
  _iInstanceCounter++;
}

PiiDefaultIoDriver::PiiDefaultIoDriver() : d(new Data)
{
  if (_iInstanceCounter == 0)
    {
      _pSendingThread = new PiiIoThread;
      _pSendingThread->start();
    }
  _iInstanceCounter++;
}

PiiDefaultIoDriver::~PiiDefaultIoDriver()
{
  // handle and remove output signals from the sending thread
  _pSendingThread->removeOutputList(d->lstChannels);

  //check if we must stop and delete thread
  _iInstanceCounter--;

  if (_iInstanceCounter == 0)
    {
      if (_pSendingThread->isRunning())
        {
          _pSendingThread->stop();
          while(!_pSendingThread->isFinished());
        }
      delete _pSendingThread;
      _pSendingThread = 0;
    }

  delete d;
}

bool PiiDefaultIoDriver::initialize()
{
  return true;
}

void PiiDefaultIoDriver::sendSignal(PiiIoChannel *channel, bool value, int day, int msecs, int pulseWidth)
{
  if ( _pSendingThread != 0 )
    _pSendingThread->sendSignal(channel,value,day,msecs,pulseWidth);
}

PiiIoChannel* PiiDefaultIoDriver::channel(int channel)
{
  if (channel >= 0 && channel < channelCount())
    {
      if (channel >= d->lstChannels.size())
        d->lstChannels.resize(channel+1);
      
      if (d->lstChannels[channel] == 0)
        d->lstChannels[channel] = createChannel(channel);
      return d->lstChannels[channel];
    }
  return 0;
}
