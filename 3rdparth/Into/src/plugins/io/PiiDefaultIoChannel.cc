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

#include "PiiDefaultIoChannel.h"
#include <PiiDefaultIoDriver.h>
#include <QDateTime>
#include <QTime>

PiiDefaultIoChannel::Data::Data() :
  pDriver(0),
  iChannelIndex(0),
  bSignalEnabled(false),
  channelMode(NoMode),
  iPulseWidth(50),
  iPulseDelay(0),
  iPreviousInputState(-1),
  iInputStateCounter(0),
  bActiveState(false),
  bCurrentState(true)
{
}

PiiDefaultIoChannel::Data::~Data()
{
}

PiiDefaultIoChannel::PiiDefaultIoChannel(Data *data, PiiDefaultIoDriver *driver, int channelIndex) : d(data)
{
  d->iChannelIndex = channelIndex;
  d->pDriver = driver;
  
  setParent(driver);
}

PiiDefaultIoChannel::~PiiDefaultIoChannel()
{
  delete d;
}

void PiiDefaultIoChannel::setActiveState(bool activeState)
{
  d->bActiveState = activeState;
  checkMode();
}

void PiiDefaultIoChannel::setChannelMode(const ChannelMode& channelMode)
{
  d->channelMode = channelMode;
  checkMode();
  setSignalEnabled(d->bSignalEnabled);
}

void PiiDefaultIoChannel::setSignalEnabled(bool signalEnabled)
{
  d->bSignalEnabled = signalEnabled;

  if ( d->channelMode == TransitionInput || d->channelMode == PulseInput )
    {
      d->pDriver->removePollingInput(this);

      if ( signalEnabled )
        d->pDriver->addPollingInput(this);
    }
    
}

void PiiDefaultIoChannel::checkInputChange()
{
  int state = currentState();

  if ( state < 0 )
    return;
  if ( d->iPreviousInputState == state )
    return;
  if ( d->iPreviousInputState == -1 )
    {
      d->iPreviousInputState = state;
      return;
    }
    
  d->iInputStateCounter = (d->iInputStateCounter+1) & 1;
  d->iPreviousInputState = state;
      
  if (d->channelMode == TransitionInput || d->iInputStateCounter == 0)
    emit signalReceived(d->iChannelIndex, state == 0 ? 0.0 : 1.0);
}

void PiiDefaultIoChannel::sendSignal(double /*value*/)
{
  if ( d->pDriver != 0 && (d->channelMode == TransitionOutput || d->channelMode == PulseOutput))
    {
      QDateTime currentDateTime = QDateTime::currentDateTime().addMSecs(d->iPulseDelay);
      
      int msecs = QTime(0,0).msecsTo(currentDateTime.time());
      int width = d->iPulseWidth;
      int day = currentDateTime.toTime_t() / 86400;
      if ( d->channelMode == TransitionOutput )
        width = 0;

      if (d->channelMode == TransitionOutput)
        {
          d->bCurrentState = !d->bCurrentState;
          d->pDriver->sendSignal(this, d->bCurrentState, day, msecs, width);
        }
      else
        d->pDriver->sendSignal(this, d->bActiveState, day, msecs, width);
    }
}

PiiDefaultIoDriver* PiiDefaultIoChannel::driver() { return d->pDriver; }
bool PiiDefaultIoChannel::signalEnabled() const { return d->bSignalEnabled; }
void PiiDefaultIoChannel::setChannelName(const QString& channelName) { d->strChannelName = channelName; }
QString PiiDefaultIoChannel::channelName() const { return d->strChannelName; }
PiiDefaultIoChannel::ChannelMode PiiDefaultIoChannel::channelMode() const { return d->channelMode; }
void PiiDefaultIoChannel::setPulseWidth(int pulseWidth) { d->iPulseWidth = pulseWidth; }
int PiiDefaultIoChannel::pulseWidth() const { return d->iPulseWidth; }
void PiiDefaultIoChannel::setPulseDelay(int pulseDelay) { d->iPulseDelay = pulseDelay; }
int PiiDefaultIoChannel::pulseDelay() const { return d->iPulseDelay; }
bool PiiDefaultIoChannel::activeState() const { return d->bActiveState; }
int PiiDefaultIoChannel::channelIndex() const { return d->iChannelIndex; }
