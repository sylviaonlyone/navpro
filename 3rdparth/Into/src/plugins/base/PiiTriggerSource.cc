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

#include "PiiTriggerSource.h"
#include <PiiYdinTypes.h>
#include <QMutexLocker>

PiiTriggerSource::Data::Data()
{
}

PiiTriggerSource::PiiTriggerSource() : PiiBasicOperation(new Data)
{
  addSocket(new PiiOutputSocket("trigger"));
}

template <class T> void PiiTriggerSource::emitValue(T value)
{
  QMutexLocker lock(&_d()->stateMutex);

  if (state() == Running)
    emitObject(value);
}

void PiiTriggerSource::trigger(const PiiVariant& value)
{
  emitValue(value);
}

void PiiTriggerSource::trigger()
{
  emitValue(1);
}

void PiiTriggerSource::trigger(int value)
{
  emitValue(value);
}

void PiiTriggerSource::trigger(double value)
{
  emitValue(value);
}

void PiiTriggerSource::trigger(const QString& value)
{
  emitValue(value);
}

void PiiTriggerSource::stop()
{
  PII_D;
  
  d->stateMutex.lock();
  if (state() == Running)
    {
      setState(Stopped);
      d->stateMutex.unlock();
      
      emitObject(PiiYdin::createStopTag());
    }
  else
    d->stateMutex.unlock();
}

void PiiTriggerSource::pause()
{
  PII_D;
  
  d->stateMutex.lock();
  if (state() == Running)
    {
      setState(Paused);
      d->stateMutex.unlock();
      
      emitObject(PiiYdin::createPauseTag());
    }
  else
    d->stateMutex.unlock();

}

void PiiTriggerSource::start()
{
  QMutexLocker lock(stateLock());
  
  if (state() == Paused)
    outputAt(0)->resume(PiiSocketState());
  
  setState(Running);
}

void PiiTriggerSource::interrupt()
{
  QMutexLocker lock(stateLock());

  if (state() != Stopped)
    {
      interruptOutputs();
      setState(Stopped);
    }
}
  
