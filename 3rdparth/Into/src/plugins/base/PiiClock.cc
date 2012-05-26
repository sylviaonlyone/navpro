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

#include "PiiClock.h"
#include <PiiYdinTypes.h>

PiiClock::Data::Data() :
  inputType(TriggerInput),
  strFormat("hh:mm:ss"),
  timeType(LocalTime),
  bUseTimer(false),
  bTimeOutputConnected(false),
  bTimestampOutputConnected(false)
{
}

PiiClock::PiiClock() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  addSocket(new PiiInputSocket("trigger"));
  inputAt(0)->setOptional(true);
  
  addSocket(d->pTimeOutput = new PiiOutputSocket("time"));
  addSocket(d->pTimeStampOutput = new PiiOutputSocket("timestamp"));

  d->timer.setInterval(1000); //the default value
  connect(&d->timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
}

void PiiClock::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  d->bTimeOutputConnected = d->pTimeOutput->isConnected();
  d->bTimestampOutputConnected = d->pTimeStampOutput->isConnected();
  
  //check if we must use timer
  d->bUseTimer = !inputAt(0)->isConnected() && (d->bTimestampOutputConnected || d->bTimeOutputConnected);

  //be sure that timer is stopped first
  d->timer.stop();
}

void PiiClock::process()
{
  PII_D;
  if (d->inputType == TimeStampInput)
    {
      PiiVariant obj = readInput();

      if (obj.type() != PiiVariant::IntType)
        PII_THROW_UNKNOWN_TYPE(inputAt(0));
      
      d->dateTime.setTime_t(obj.valueAs<unsigned int>());
    }
  else
    d->dateTime = QDateTime::currentDateTime();

  emitObjects();
}

void PiiClock::emitObjects()
{
  PII_D;
  if (d->bTimeOutputConnected)
    {
      QString result = d->dateTime.toTimeSpec(static_cast<Qt::TimeSpec>(d->timeType)).toString(d->strFormat);
      d->pTimeOutput->emitObject(result);
    }
  if (d->bTimestampOutputConnected)
    d->pTimeStampOutput->emitObject(d->dateTime.toTime_t());
}

void PiiClock::aboutToChangeState(State state)
{
  PII_D;
  if (d->bUseTimer && state == Running)
    d->timer.start();
  else
    d->timer.stop();
}

void PiiClock::timerTimeout()
{
  PII_D;
  d->dateTime = QDateTime::currentDateTime();
  emitObjects();
}

void PiiClock::setInputType(const InputType& inputType) { _d()->inputType = inputType; }
PiiClock::InputType PiiClock::inputType() const { return _d()->inputType; }
QString PiiClock::format() const { return _d()->strFormat; }
void PiiClock::setFormat(QString format) { _d()->strFormat = format; }
void PiiClock::setTimeType(const TimeType& timeType) { _d()->timeType = timeType; }
PiiClock::TimeType PiiClock::timeType() const { return _d()->timeType; }
void PiiClock::setInterval(int interval) { _d()->timer.setInterval(interval); }
int PiiClock::interval() const { return _d()->timer.interval(); }
