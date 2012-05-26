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

#include "PiiClassifierOperation.h"
#include <PiiYdinTypes.h>
#include <PiiAsyncCall.h>

PiiClassifierOperation::Data::Data(PiiClassification::LearnerCapabilities cap) :
  capabilities(cap),
  iLearningBatchSize(0),
  fullBufferBehavior(PiiClassification::OverwriteRandomSample),
  dProgressStep(0.01),
  dCurrentProgress(0),
  bThreadRunning(false)
{
}

PiiClassifierOperation::PiiClassifierOperation(PiiClassification::LearnerCapabilities cap) :
  PiiDefaultOperation(new Data(cap), NonThreaded)
{
  init();
}

PiiClassifierOperation::PiiClassifierOperation(Data* data) :
  PiiDefaultOperation(data, NonThreaded)
{
  init();
}

void PiiClassifierOperation::init()
{
  PII_D;
  addSocket(d->pFeatureInput = new PiiInputSocket("features"));

  addSocket(d->pLabelInput = new PiiInputSocket("label"));
  d->pLabelInput->setOptional(true);

  addSocket(d->pWeightInput = new PiiInputSocket("weight"));
  d->pWeightInput->setOptional(true);
  
  addSocket(d->pClassificationOutput = new PiiOutputSocket("classification"));

  d->pLearningThread = Pii::createAsyncCall(this, &PiiClassifierOperation::learningThread);

  setProtectionLevel("learningBatchSize", WriteWhenStoppedOrPaused);
  setProtectionLevel("fullBufferBehavior", WriteWhenStoppedOrPaused);
}

PiiClassifierOperation::~PiiClassifierOperation()
{
  stopLearningThread();
  delete _d()->pLearningThread;
}

void PiiClassifierOperation::check(bool reset)
{
  PII_D;
  d->pLabelInput->setOptional(d->iLearningBatchSize == 0 ||
                              d->capabilities & PiiClassification::NonSupervisedLearner);
  PiiDefaultOperation::check(reset);
  if (reset)
    {
      stopLearningThread();
      d->dCurrentProgress = 0;
      resizeBatch(0);
    }
}

void PiiClassifierOperation::aboutToChangeState(State newState)
{
  PII_D;
  // Finishing on-line learning
  if (newState == Stopped &&
      d->capabilities & PiiClassification::OnlineLearner &&
      d->iLearningBatchSize == 1)
    finishOnlineLearning();
}

void PiiClassifierOperation::process()
{
  PII_D;
  QMutexLocker lock(&d->learningMutex);

  // Collect samples for training only if requested (by setting batch
  // size to a non-zero value) and if the learning thread is not
  // already running.
  if (d->iLearningBatchSize != 0 && !d->pLearningThread->isRunning())
    {
      double dInputLabel = readLabel();
      double dInputWeight = readWeight();

      // If the learner is capable of on-line learning and batch size
      // is one, send the sample directly to training.
      if (d->capabilities & PiiClassification::OnlineLearner &&
          d->iLearningBatchSize == 1)
        {
          learnOne(dInputLabel, dInputWeight);
          return;
        }
      else
        collectSample(dInputLabel, dInputWeight);
    }
  
  // Emit classification result.
  classify();
}

bool PiiClassifierOperation::learningThread()
{
  _d()->strLearningError.clear();
  
  if (learnBatch())
    {
      QMutexLocker lock(&_d()->learningMutex);
      replaceClassifier();
      emit progressed(1.0);
      emit learningFinished(true);
      return true;
    }
  emit learningFinished(false);
  return false;
}

bool PiiClassifierOperation::needsThread() const
{
  return true;
}

double PiiClassifierOperation::learnOne(double label,double)
{
  _d()->pClassificationOutput->emitObject(label);
  return label;
}

bool PiiClassifierOperation::startLearningThread(bool startThread)
{
  PII_D;
  QMutexLocker lock(&d->learningMutex);
  if (!needsThread())
    {
      replaceClassifier();
      emit progressed(1.0);
      emit learningFinished(true);
      return true;
    }
  // If the learning thread is already running or
  // there is not enough buffered samples, do nothing.
  if (d->pLearningThread->isRunning())
    return false;
  if (bufferedSampleCount() == 0)
    {
      emit errorOccured(this, tr("No buffered samples."));
      return false;
    }

  d->bThreadRunning = true;

  if (startThread)
    d->pLearningThread->start();
  else
    {
      lock.unlock();
      return learningThread();
    }

  return true;
}

bool PiiClassifierOperation::startLearningThread()
{
  return startLearningThread(true);
}

bool PiiClassifierOperation::learn()
{
  return startLearningThread(false);
}

void PiiClassifierOperation::stopLearningThread()
{
  PII_D;
  d->bThreadRunning = false;
  d->pLearningThread->wait();
}

void PiiClassifierOperation::reset()
{
  QMutexLocker lock(&_d()->learningMutex);
  resetClassifier();
}

bool PiiClassifierOperation::canContinue(double progressPercentage)
{
  PII_D;
  if (progressPercentage - d->dCurrentProgress >= d->dProgressStep)
    {
      emit progressed(progressPercentage);
      d->dCurrentProgress = progressPercentage;
    }
  
  return d->bThreadRunning;
}

PiiClassification::LearnerCapabilities PiiClassifierOperation::capabilities() const
{
  return _d()->capabilities;
}


double PiiClassifierOperation::readLabel() const
{
  const PII_D;
  // Incoming class index is read only with supervised learning algorithms.
  return d->capabilities & PiiClassification::NonSupervisedLearner ? NAN :
    PiiYdin::convertPrimitiveTo<double>(d->pLabelInput);
}

double PiiClassifierOperation::readWeight() const
{
  const PII_D;
  return d->pWeightInput->isConnected() ?
    PiiYdin::convertPrimitiveTo<double>(d->pWeightInput) :
    1.0;
}

void PiiClassifierOperation::setProgressStep(double progressStep) { _d()->dProgressStep = progressStep; }
double PiiClassifierOperation::progressStep() const { return _d()->dProgressStep; }

void PiiClassifierOperation::setLearningBatchSize(int learningBatchSize)
{
  if (learningBatchSize < -1) return;
  PII_D;
  QMutexLocker lock(&d->learningMutex);
  if (learningBatchSize != -1 && learningBatchSize < bufferedSampleCount())
    resizeBatch(learningBatchSize);
  d->iLearningBatchSize = learningBatchSize;
}
int PiiClassifierOperation::learningBatchSize() const { return _d()->iLearningBatchSize; }

void PiiClassifierOperation::setFullBufferBehavior(PiiClassification::FullBufferBehavior fullBufferBehavior)
{ _d()->fullBufferBehavior = fullBufferBehavior; }
PiiClassification::FullBufferBehavior PiiClassifierOperation::fullBufferBehavior() const
{ return _d()->fullBufferBehavior; }

QMutex* PiiClassifierOperation::learningMutex() { return &_d()->learningMutex; }
PiiInputSocket* PiiClassifierOperation::featureInput() { return _d()->pFeatureInput; }
PiiInputSocket* PiiClassifierOperation::labelInput() { return _d()->pLabelInput; }
PiiInputSocket* PiiClassifierOperation::weightInput() { return _d()->pWeightInput; }
PiiOutputSocket* PiiClassifierOperation::classificationOutput() { return _d()->pClassificationOutput; }
bool PiiClassifierOperation::learningThreadRunning() const { return _d()->pLearningThread->isRunning(); }
QString PiiClassifierOperation::learningError() const { return _d()->strLearningError; }
void PiiClassifierOperation::setLearningError(const QString& learningError) { _d()->strLearningError = learningError; }
bool PiiClassifierOperation::learnBatch() { return false; }
void PiiClassifierOperation::finishOnlineLearning() {}
