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

#include "PiiIoThread.h"
#include <PiiDelay.h>
#include <QDateTime>

PiiIoThread::PiiIoThread(QObject *parent) : QThread(parent), _bRunning(true)
{

}

void PiiIoThread::run()
{
  while (_bRunning)
    {
      _mutex.lock();
      
      for ( int i=_lstPollingInputs.size(); i--; )
        _lstPollingInputs[i]->checkInputChange();
      
      //HANDLE OUTPUTS ->
      QDateTime currentDateTime = QDateTime::currentDateTime();
      int msecs = QTime(0,0).msecsTo(currentDateTime.time());
      int day = currentDateTime.toTime_t() / 86400;
      int size = _lstWaitingOutputSignals.size();

      //handle all current output structs
      for ( int i=0; i<size; i++ )
        {
          OutputSignal stru = _lstWaitingOutputSignals[i];
          if ( checkTime(stru, day, msecs) )
            {
              stru.channel->changeOutputState(stru.active);

              //if we have pulseOutput, we must turn the state back later
              if ( stru.pulseWidth > 0 )
                addNewStruct(stru.channel,!stru.active,day,msecs+stru.pulseWidth,0);
              
              _lstWaitingOutputSignals[i].handled = true;
            }
        }

      //remove all handled output structs
      for ( int i=_lstWaitingOutputSignals.size(); i--; )
        if ( _lstWaitingOutputSignals[i].handled )
          _lstWaitingOutputSignals.removeAt(i);
      
      _mutex.unlock();
      
      PiiDelay::msleep(10);
    }
}

void PiiIoThread::removeOutputList(const QVector<PiiIoChannel*>& lstChannels)
{
  _mutex.lock();

  // First handle all waiting output signals depends on lstChannels
  for (int i=0; i<_lstWaitingOutputSignals.size(); i++)
    {
      OutputSignal& stru = _lstWaitingOutputSignals[i];
      if (lstChannels.contains(stru.channel))
        {
          stru.channel->changeOutputState(stru.active);
          stru.handled = true;
        }
    }

  // Second remove all handled output structs
  for ( int i=_lstWaitingOutputSignals.size(); i--; )
    if ( _lstWaitingOutputSignals[i].handled )
      _lstWaitingOutputSignals.removeAt(i);

  _mutex.unlock();
  
}

bool PiiIoThread::checkTime(const OutputSignal& stru, int day, int msecs)
{
  if ( day > stru.day || (stru.day == day && msecs > stru.msecs) )
    return true;
  
  return false;
}

void PiiIoThread::addNewStruct(PiiIoChannel *channel, bool active, int day, int msecs, int width)
{
  if ( msecs >= 86400000 )
    {
      msecs -= 86400000;
      day++;
    }
  OutputSignal stru;
  stru.handled = false;
  stru.channel = channel;
  stru.active = active;
  stru.day = day;
  stru.msecs = msecs;
  stru.pulseWidth = width;
  _lstWaitingOutputSignals << stru;
}

void PiiIoThread::addPollingInput(PiiIoChannel *input)
{
  _mutex.lock();
  if ( !_lstPollingInputs.contains(input) )
    _lstPollingInputs << input;
  _mutex.unlock();
}

void PiiIoThread::removePollingInput(PiiIoChannel *input)
{
  _mutex.lock();
  _lstPollingInputs.removeAll(input);
  _mutex.unlock();
}


void PiiIoThread::sendSignal(PiiIoChannel *channel, bool value, int day, int msecs, int width)
{
  //qDebug("PiiIoThread::sendSignal(%p,%i,%i,%i,%i)", channel, value, day, msecs, width);
  
  _mutex.lock();

  addNewStruct(channel,value,day,msecs,width);
  
  _mutex.unlock();
}
