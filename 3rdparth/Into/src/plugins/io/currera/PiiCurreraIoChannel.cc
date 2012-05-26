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

#include "PiiCurreraIoChannel.h"


#include "PiiCurreraIoChannel.h"
#include <PiiCurreraIoDriver.h>

PiiCurreraIoChannel::Data::Data(HANDLE handle) :
  driverHandle(handle)
{
}

PiiCurreraIoChannel::PiiCurreraIoChannel(PiiCurreraIoDriver *driver, int channel, HANDLE driverHandle) :
  PiiDefaultIoChannel(new Data(driverHandle), driver, channel)
{
  bool bInput = channel / 4 == 0;
  int iIndex = channel % 4 + 1;

  // Initialize input/output
  XI_RETURN ret;
  if (bInput)
    {
      if ((ret = XiApiExt::xiWriteInputMode(driverHandle, iIndex, XI_GPI_EXT_EVENT)) != XI_OK)
        piiWarning(tr("Couldn't initialize an input(%1). ErrorCode: %2").arg(iIndex).arg(ret));
    }
  else
    {
      if ((ret = XiApiExt::xiWriteOutputMode(driverHandle, iIndex, XI_GPO_ON)) != XI_OK)
        piiWarning(tr("Couldn't initialize an output(%1). ErrorCode: %2").arg(iIndex).arg(ret));
    }
}

void PiiCurreraIoChannel::checkMode()
{
  ChannelMode mode = channelMode();
  int iChannelIndex = channelIndex();
  
  if (mode == NoMode)
    return;

  if (iChannelIndex < 0 || iChannelIndex >= _d()->pDriver->channelCount())
    {
      piiWarning(tr("PiiCurreraIoChannel : wrong channelIndex (%1), must be 0-7").arg(iChannelIndex));
      setChannelMode(NoMode);
      return;
    }

  int port = iChannelIndex / 4;

  
  if ((port == 0 && (mode == TransitionOutput || mode == PulseOutput)) ||
      (port == 1 && (mode == TransitionInput || mode == PulseInput)))
    {
      piiWarning(tr("PiiCurreraIoChannel : channelMode(%1) and channelIndex(%2) mismatch. Inputs are 0-3, outputs 4-7.")
               .arg(mode).arg(iChannelIndex));
      setChannelMode(NoMode);
      return;
    }
  
  if (mode == TransitionOutput || mode == PulseOutput)
    {
      XI_RETURN ret = XiApiExt::xiWriteOutputMode(_d()->driverHandle,
                                                  channelIndex() % 4 + 1,
                                                  activeState() ? XI_GPO_ON : XI_GPO_OFF);
      if (ret != XI_OK)
        piiWarning(tr("PiiCurreraIoChannel: TOGGLE channel %1 failed, ErrorCode = %2")
                   .arg(channelIndex()).arg(ret));
    }
}

int PiiCurreraIoChannel::currentState() const
{
  const PII_D;
  
  int iChannelIndex = channelIndex();

  bool bInput = iChannelIndex / 4 == 0;
  int iIndex = iChannelIndex % 4 + 1;
  int iState = 0;
  XI_RETURN ret = XI_OK;
  if (bInput)
    {
      if ((ret = XiApiExt::xiReadInputLevel(d->driverHandle, iIndex, &iState)) == XI_OK)
        return iState;
    }
  else
    {
      if ((ret = XiApiExt::xiReadOutputMode(d->driverHandle, iIndex, &iState)) == XI_OK)
        return iState == 1 ? 0 : 1;
    }
  
  piiWarning(tr("PiiCurreraIoChannel : Cannot read the current state "
                "(channel Index = %1, ErrorCode = %2)").arg(iChannelIndex).arg(ret));
  return -1;
}

void PiiCurreraIoChannel::changeOutputState(bool active)
{
  PII_D;
  
  ChannelMode mode = channelMode();
  
  if (mode == NoMode)
    {
      piiWarning(tr("PiiCurreraIoChannel : cannot change output state (mode == NoMode)"));
      return;
    }
  
  XI_RETURN ret = XiApiExt::xiWriteOutputMode(d->driverHandle,
                                              channelIndex() % 4 + 1,
                                              active ? XI_GPO_OFF : XI_GPO_ON);
  if (ret != XI_OK)
    piiWarning(tr("PiiCurreraIoChannel: TOGGLE channel %1 failed, ErrorCode = %2").arg(channelIndex()).arg(ret));
  
}
