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

#include "PiiAdvantechUsb4761IoChannel.h"
#include <PiiAdvantechUsb4761IoDriver.h>

PiiAdvantechUsb4761IoChannel::Data::Data(LONG handle) :
  driverHandle(handle)
{
}

PiiAdvantechUsb4761IoChannel::PiiAdvantechUsb4761IoChannel(PiiAdvantechUsb4761IoDriver *driver, int channel, LONG driverHandle) :
  PiiDefaultIoChannel(new Data(driverHandle), driver, channel)
{
}

void PiiAdvantechUsb4761IoChannel::checkMode()
{
  ChannelMode mode = channelMode();
  int iChannelIndex = channelIndex();
  
  if (mode == NoMode)
    return;

  if (iChannelIndex < 0 || iChannelIndex > 15)
    {
      qWarning("%s",tr("PiiAdvantechUsb4761IoChannel : wrong channelIndex (%1), must be 0-15").arg(iChannelIndex).toLocal8Bit().constData());
      setChannelMode(NoMode);
      return;
    }

  int port = iChannelIndex / 8;

  
  if ((port == 1 && (mode == TransitionOutput || mode == PulseOutput)) ||
      (port == 0 && (mode == TransitionInput || mode == PulseInput)))
    {
      qWarning("%s", tr("PiiAdvantechUsb4761IoChannel : channelMode(%1) and channelIndex(%2) mismatch. Inputs are 0-7, outputs 8-15.")
               .arg(mode).arg(iChannelIndex).toLocal8Bit().constData());
      setChannelMode(NoMode);
    }
}


int PiiAdvantechUsb4761IoChannel::getCurrentState() const
{
  PII_D;
  
  int iChannelIndex = channelIndex();

  USHORT state;

  PT_DioReadBit ptDioReadBit;
  ptDioReadBit.port = iChannelIndex / 8;
  ptDioReadBit.bit = iChannelIndex % 8;
  ptDioReadBit.state = &state;

  LRESULT ErrCode = DRV_DioReadBit(d->driverHandle, (LPT_DioReadBit)&ptDioReadBit);

  if (ErrCode == SUCCESS)
    return (int)state;
  
  return -1;
}

void PiiAdvantechUsb4761IoChannel::changeOutputState(bool active)
{
  PII_D;
  
  ChannelMode mode = channelMode();
  int iChannelIndex = channelIndex();
  
  if (mode == NoMode)
    {
      qWarning("%s", tr("PiiAdvantechUsb4761IoChannel : cannot change output state (mode == NoMode)").toLocal8Bit().constData());
      return;
    }
  
  int outputState = active ? 1 : 0;

  PT_DioWriteBit ptDioWriteBit;
  ptDioWriteBit.port = iChannelIndex / 8;
  ptDioWriteBit.bit = iChannelIndex % 8;
  ptDioWriteBit.state = outputState;

  LRESULT ErrCode = DRV_DioWriteBit(d->driverHandle, (LPT_DioWriteBit)&ptDioWriteBit);
  
  if (ErrCode != SUCCESS)
    qWarning("%s", tr("PiiAdvantechUsb4761IoChannel: TOGGLE channel %1 failed, ErrCode = %2").arg(iChannelIndex).arg(ErrCode).toLocal8Bit().constData());
}
