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

#include "PiiMultiCamIoChannel.h"
#include <multicam.h>
#include <QDateTime>
#include <PiiMultiCamIoDriver.h>

PiiMultiCamIoChannel::Data::Data(int board) :
  iBoard(board),
  electricalConnection(NoConnection)
{
}

PiiMultiCamIoChannel::PiiMultiCamIoChannel(PiiMultiCamIoDriver *parent, int channel, int board) :
  PiiDefaultIoChannel(new Data(board), parent, channel)
{
}

void PiiMultiCamIoChannel::setElectricalConnection(const ElectricalConnection& electricalConnection)
{
  _d()->electricalConnection = electricalConnection;
  checkMode();
  setSignalEnabled(signalEnabled());
}

PiiMultiCamIoChannel::ElectricalConnection PiiMultiCamIoChannel::electricalConnection() const
{
  return _d()->electricalConnection;
}

int PiiMultiCamIoChannel::getCurrentState() const
{
  const PII_D;
  int state;
  MCSTATUS status = McGetParamInt(MC_BOARD + d->iBoard, MC_InputState + channelIndex(), &state);
  
  if (status == MC_OK)
    {
      switch (state)
        {
        case MC_InputState_LOW:
          return 0;
        case MC_InputState_HIGH:
          return 1;
        default:
          break;
        }
    }
  
  return -1;
}

void PiiMultiCamIoChannel::changeOutputState(bool active)
{
  PII_D;
  int outputState = active ? MC_OutputState_HIGH : MC_OutputState_LOW;
  
  //qDebug("PiiMultiCamIoChannel:changeOutputState(), Setting state of channel %d to %s", channelIndex(), active ? "HIGH" : "LOW");
  MCSTATUS status = McSetParamInt(MC_BOARD + d->iBoard, MC_OutputState + channelIndex(), outputState);
  
  if (status != MC_OK)
    qWarning("PiiMultiCamIoChannel: TOGGLE channel %i failed, status = %i", channelIndex(), status);
}


/**
 * Update channelMode and electricalConnection
 *
 * ChannelMode:
 * - NoMode
 * - TransitionInput,
 * - PulseInput,
 * - TransitionOutput,
 * - PulseOutput

 * ElectricalConnection:
 * - NoConnection
 * - TtlConnection,
 * - IttlConnection,
 * - CmosConnection,
 * - ChannelLinkConnection,
 * - LvdsConnection,
 * - V5Connection,
 * - V12Connection,
 * - IV12Connection,
 * - SsRelayConnection,
 * - RelayConnection,
 * - AutoConnection,
 * - OcConnection,
 * - IocConnection,
 * - IoeConnection,
 * - OptoConnection
 */
void PiiMultiCamIoChannel::checkMode()
{
  PII_D;

  ChannelMode mode = channelMode();
  int iChannelIndex = channelIndex();
  
  if (mode == NoMode || d->electricalConnection == NoConnection)
    return;
  
  int style = -1;
  bool ok = false;
  
  if (mode == TransitionInput || mode == PulseInput)
    {
      MCSTATUS status = McSetParamInt(MC_BOARD + d->iBoard, MC_InputConfig + iChannelIndex, MC_InputConfig_SOFT);
      if ( status == MC_OK )
        {
          switch (d->electricalConnection)
            {
            case TtlConnection: style = MC_InputStyle_TTL; break;
            case IttlConnection: style = MC_InputStyle_ITTL; break;
            case CmosConnection: style = MC_InputStyle_CMOS; break;
            case ChannelLinkConnection: style = MC_InputStyle_CHANNELLINK; break;
            case LvdsConnection: style = MC_InputStyle_LVDS; break;
            case V5Connection: style = MC_InputStyle_5V; break;
            case V12Connection: style = MC_InputStyle_12V; break;
            case IV12Connection: style = MC_InputStyle_I12V; break;
            case RelayConnection: style = MC_InputStyle_RELAY; break;
            case AutoConnection: style = MC_InputStyle_AUTO; break;
            default: break;
            }
          
          if (style > -1)
            {
              status = McSetParamInt(MC_BOARD + d->iBoard, MC_InputStyle + iChannelIndex, style);
              if ( status == MC_OK )
                ok = true;
            }
        }
    }
  else if (mode == TransitionOutput || mode == PulseOutput)
    {
      MCSTATUS status = McSetParamInt(MC_BOARD + d->iBoard, MC_OutputConfig + iChannelIndex, MC_OutputConfig_SOFT);
      if ( status == MC_OK )
        {
          switch (d->electricalConnection )
            {
            case TtlConnection: style = MC_OutputStyle_TTL; break;
            case IttlConnection: style = MC_OutputStyle_ITTL; break;
            case LvdsConnection: style = MC_OutputStyle_LVDS; break;
            case RelayConnection: style = MC_OutputStyle_RELAY; break;
            case SsRelayConnection: style = MC_OutputStyle_SSRLY; break;
            case OcConnection: style = MC_OutputStyle_OC; break;
            case IocConnection: style = MC_OutputStyle_IOC; break;
            case IoeConnection: style = MC_OutputStyle_IOE; break;
            case OptoConnection: style = MC_OutputStyle_OPTO; break;
            default: break;
            }

          if ( style > -1 )
            {
              MCSTATUS status = McSetParamInt(MC_BOARD + d->iBoard, MC_OutputStyle + iChannelIndex, style);
              if ( status == MC_OK )
                ok = true;
            }
        }

      if (mode == PulseOutput)
        {
          /**
           * Initialize state of the channel to the !activeState().
           */
          int outputState = activeState() ? MC_OutputState_LOW : MC_OutputState_HIGH;
          MCSTATUS status = McSetParamInt(MC_BOARD + d->iBoard, MC_OutputState + iChannelIndex, outputState);

          if ( status != MC_OK )
            {
              ok = false;
              qWarning("PiiMultiCamIoChannel: INITIALIZATION channel %i failed, status = %i", iChannelIndex, status);
            }
        }
    }

  if (!ok)
    {
      setChannelMode(NoMode);
      setElectricalConnection(NoConnection);
    }
}
