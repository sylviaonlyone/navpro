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

#include "PiiFrequencyCounter.h"
#include "PiiOutputSocket.h"

#include <PiiYdinTypes.h>


#define FREQCOUNTER_NEW_WEIGHT 0.1
#define FREQCOUNTER_OLD_WEIGHT 0.9
// Maximum sleeping time for PiiFreOutputEmitter in milliseconds.
#define FREQOUTPUTEMITTTER_MAX_SLEEP_TIME 1000


PiiFreqOutputEmitter::PiiFreqOutputEmitter(PiiFrequencyCounter *parent) :
  QThread(parent), _pParent(parent), _pParentData(parent->_d()), _bRunning(false)
{
  //qDebug("PiiFreqOutputEmitter, constructor, _pParent->_iMeasurementInterval = %d", _pParent->_iMeasurementInterval);
  _iSleepDuration = qMin(_pParentData->iMeasurementInterval , (unsigned int)FREQOUTPUTEMITTTER_MAX_SLEEP_TIME);
  
  //qDebug("PiiFreqOutputEmitter, constructor, _iSleepDuration = %d", _iSleepDuration);
}

void PiiFreqOutputEmitter::run()
{
  //qDebug("PiiFreqOutputEmitter::run() BEGIN");
  setPriority(LowestPriority);
  _pParentData->measurementTime.start();
  while ( _bRunning )
    {
      //qDebug("PiiFreqOutputEmitter::run(), in the beginning of while");
      int elapsed = _pParentData->measurementTime.elapsed();
      if (elapsed >= (int)_pParentData->iMeasurementInterval)
         {
           emitFrequency(elapsed);
           _pParentData->measurementTime.restart();
         }
      else
        {
          // Let's make sure that the sleeping time is not too long.
          unsigned int timeToNextEmission = _pParentData->iMeasurementInterval - elapsed;
          unsigned int sleepDuration = _iSleepDuration == 0 ? 0 : qMin(_iSleepDuration,  timeToNextEmission);
          //qDebug("PiiFreqOutputEmitter::run() timeToNextEmission = %d", timeToNextEmission);
          //qDebug("PiiFreqOutputEmitter::run(), sleeping = %d mseconds", sleepDuration);
          msleep(sleepDuration);
        }

    }
  //qDebug("PiiFreqOutputEmitter::run() END");
}

void PiiFreqOutputEmitter::emitFrequency(int elapsed)
{
  //qDebug("PiiFreqOutputEmitter::emitFrequency(), BEGIN elapsed = %d", elapsed);

  // This if is is to make sure, that nothing is emitted, once the
  // engine is in paused state. It also avoids a crash after the
  // engine has been interrupted.
  if (_pParent->state() == PiiOperation::Running)
    {
      //qDebug("                                       _pParent->_iFrequencyCounter = %d", _pParent->_iFrequencyCounter);
      int hitsPerSecond = int((double)1000/(double)elapsed*_pParentData->iFrequencyCounter);
      _pParentData->pFreqOutput->emitObject(hitsPerSecond);
      _pParentData->iFrequencyCounter = 0;
      //qDebug("PiiFreqOutputEmitter::emitFrequency(),  hitsPerSecond = %d", hitsPerSecond);
    }
  
  //qDebug("PiiFreqOutputEmitter::emitFrequency(), END");
}

PiiFrequencyCounter::Data::Data(PiiFrequencyCounter*) :
  iMeasurementInterval(1000),
  pEmitter(0),
  iFilterInterval(0),
  dMeasurementFrequency(1),
  dMeanInterval(0.0),
  dMaxMeanInterval(0),
  iFrequencyCounter(0),
  bFrequencyOutputConnected(false)
{
}
  
PiiFrequencyCounter::PiiFrequencyCounter() :
  PiiDefaultOperation(new Data(this), NonThreaded)
{
  PII_D;
  d->pFreqOutput = new PiiOutputSocket("frequency");
  d->pFreqOutput->setGroupId(-1);
  addSocket(d->pFreqOutput);
  setMaxFrequency(100);
  setMeasurementFrequency(1);

  d->iStaticOutputCount = outputCount();
  
  setDynamicInputCount(1);
  
  connect(this, SIGNAL(stateChanged(int)), this, SLOT(stateChangeOccured(int)));
  d->pEmitter = new PiiFreqOutputEmitter(this);
}

PiiFrequencyCounter::~PiiFrequencyCounter()
{
  PII_D;
  d->pEmitter->stop();
  d->pEmitter->wait();
  delete d->pEmitter;
}

void PiiFrequencyCounter::setDynamicInputCount(int inputCount)
{
  PII_D;
  //qDebug("PiiFrequencyCounter::setInputCount() BEGIN");
  // In this function we assume, that amount of filtered outputs
  // (having a name in format "outputX") equals to the amount of inputs.

  setNumberedInputs(inputCount);
  setNumberedOutputs(inputCount, d->iStaticOutputCount);
  
  //qDebug("PiiFrequencyCounter::setInputCount() END");
}

void PiiFrequencyCounter::setMaxFrequency(double frequency)
{
  PII_D;
  // d->iFilterInterval is the inverse of d->dMaxFrequency in
  // milliseconds. The exception is the value zero. If d->dMaxFrequency
  // is zero, d->iFilterInterval is also zero. 
  d->dMaxFrequency = frequency;
  //qDebug("PiiFrequencyCounter::setMaxFrequency(), d->dMaxFrequency set to %f", d->dMaxFrequency);
  if (frequency == 0)
    {
      d->iFilterInterval = 0;
      // This member variable is not used, when filter interval is
      // zero. So in principle we could assign any value in it.
      d->dMaxMeanInterval = 0;
    }
  else
    {
      d->iFilterInterval = (int)(1.0/frequency*1000.0);
      // Rationale:
      // d->dMaxMeanInterval must be larger than d->iFilterInterval -> add
      // to d->iFilterInterval
      // Large object frequency rapidly decrease the mean -> multiply
      // by frequency
      // Large learning rate rapidly decreases the mean -> multiply by
      // learning rate
      d->dMaxMeanInterval = d->iFilterInterval + frequency*FREQCOUNTER_NEW_WEIGHT;
      
    }
  //qDebug("PiiFrequencyCounter::setMaxFrequency(), d->iFilterInterval set to %d", d->iFilterInterval);
  //qDebug("PiiFrequencyCounter::setMaxFrequency(), d->dMaxMeanInterval set to %f", d->dMaxMeanInterval);
}

void PiiFrequencyCounter::setMeasurementFrequency(double frequency)
{
  PII_D;
  // d->iMeasurementInterval is the inverse of d->dMeasurementFrequency in
  // milliseconds. The exception is the value zero. If d->dMeasurementFrequency
  // is zero, d->iFilterInterval is also zero. 
  d->dMeasurementFrequency = frequency;
  if ( d->dMeasurementFrequency < 0 )
    d->dMeasurementFrequency = 0;
  
  //qDebug("PiiFrequencyCounter::setMaxFrequency(), d->dMeasurementFrequency set to %f", d->dMeasurementFrequency);
  if (d->dMeasurementFrequency <= 0)
    d->iMeasurementInterval = 0;
  else
    d->iMeasurementInterval = (unsigned int)(1.0/frequency*1000.0);
  //qDebug("PiiFreqencyCounter::setMaxFrequency(), d->iMeasurementInterval set to %u", d->iMeasurementInterval);
}

void PiiFrequencyCounter::process()
{
  PII_D;
  //qDebug("PiiFrequencyCounter::process() BEGIN");
  d->iFrequencyCounter++;
  
  bool bSendObject = true;
  if (d->iFilterInterval > 0)
    {
      int elapsed = d->filterTime.elapsed();
      d->dMeanInterval = FREQCOUNTER_NEW_WEIGHT * elapsed + FREQCOUNTER_OLD_WEIGHT * d->dMeanInterval;
      if (d->dMeanInterval > d->dMaxMeanInterval)
        d->dMeanInterval = d->dMaxMeanInterval;
      //qDebug("PiiFrequencyCounter::process(), elapsed = %d", elapsed);
      //qDebug("PiiFrequencyCounter::process(), d->dMeanInterval = %f", d->dMeanInterval);
      //if ( d->iFilterInterval > elapsed ) // REMOVE
      if ( (double)d->iFilterInterval > d->dMeanInterval )
        bSendObject = false;
      //else
      //  d->filterTime.restart();
    }


  if (bSendObject)
    {
      d->filterTime.restart();
      for (int i = 0; i<inputCount(); i++)
        {
          // It is assumed, that there is corresponding output for
          // each intput.
          //qDebug("PiiFrequencyCounter::process(), sending output%d", i);  
          emitObject(readInput(i), i + d->iStaticOutputCount);
        }
    }
  //qDebug("PiiFrequencyCounter::process() END");  
}

void PiiFrequencyCounter::start()
{
  PII_D;
  if (d->bFrequencyOutputConnected && d->state == Stopped)
    d->pEmitter->startThread();

  PiiDefaultOperation::start();
}

void PiiFrequencyCounter::check(bool reset)
{
  PII_D;
  //qDebug("PiiFrequencyCounter::check() BEGIN");
  if (reset)
    {
      d->filterTime.start();
      d->measurementTime.start();
    }
  
  PiiDefaultOperation::check(reset);

  d->bFrequencyOutputConnected = d->pFreqOutput->isConnected();
}

/* This private slot takes care of stopping the execution of the
 *  emitter thread. */
void PiiFrequencyCounter::stateChangeOccured(int state)
{
  PII_D;
  if (state == Stopped)
    {
      d->pEmitter->stop();
      d->pEmitter->wait();
    }
}

int PiiFrequencyCounter::dynamicInputCount() const { return inputCount(); }
double PiiFrequencyCounter::maxFrequency() const { return _d()->dMaxFrequency; }
double PiiFrequencyCounter::measurementFrequency() const { return _d()->dMeasurementFrequency; }
