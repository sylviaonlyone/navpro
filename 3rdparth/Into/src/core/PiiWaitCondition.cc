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

#include "PiiWaitCondition.h"

PiiWaitCondition::PiiWaitCondition(QueueMode mode) :
  _bQueue(mode == Queue), _iWaiters(0), _iWakeSignals(0)
{ }
  
bool PiiWaitCondition::wait(unsigned long time)
{
  //serialize access to _iWakeSignals and _iWaiters
  QMutexLocker locker(&_mutex);

  //see if we have already been signalled
  if (_iWakeSignals > 0)
    {
      //we are queuing signals -> just decrement by one
      if (_bQueue)
        _iWakeSignals--;
      //we just need one signal -> clear the queue
      else
        _iWakeSignals = 0;
      return true;
    }
  _iWaiters++;

  //Unlock the mutex and wait for a signal. The mutex is atomically
  //unlocked at the time this thread enters suspended mode and
  //atomically locked again when the suspension ends.
  return _condition.wait(&_mutex, time);
}

void PiiWaitCondition::wakeOne()
{
  _mutex.lock();
  //if no thread is waiting, build up a queue
  if (_iWaiters == 0)
    _iWakeSignals++;
  else
    {
      // Somebody is waiting: wake one up
      _condition.wakeOne();
      // Decrement the waiter count. In some rare cases, wait()
      // doesn't return immediately after wakeOne(), but multiple
      // wakeOne() calls may be invoked before wait() finishes. 
      // Therefore, the waiter count must be decreased before the
      // wait() call actually returns.
      _iWaiters--;
    }
  _mutex.unlock();
}

void PiiWaitCondition::wakeAll()
{
  _mutex.lock();
  //release all waiting threads
  _condition.wakeAll();
  //make sure no signals are left in the queue
  _iWakeSignals = 0;
  _iWaiters = 0;
  _mutex.unlock();
}
