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

#include "PiiDefaultOperation.h"
#include "PiiYdinTypes.h"
#include "PiiThreadedProcessor.h"
#include "PiiSimpleProcessor.h"
#include "PiiDefaultFlowController.h"
#include "PiiOneInputFlowController.h"
#include "PiiOneGroupFlowController.h"

PiiDefaultOperation::Data::Data() :
  pFlowController(0), pProcessor(0),
  bChecked(false),
  processLock(PiiReadWriteLock::Recursive)
{
}

PiiDefaultOperation::Data::~Data()
{
  delete pFlowController;
  delete pProcessor;
}

PiiDefaultOperation::PiiDefaultOperation(ProcessingMode mode) :
  PiiBasicOperation(new Data)
{
  init(mode);
}

PiiDefaultOperation::PiiDefaultOperation(Data* data, ProcessingMode mode) :
  PiiBasicOperation(data)
{
  init(mode);
}

void PiiDefaultOperation::init(ProcessingMode mode)
{
  createProcessor(mode);
  setProtectionLevel("processingMode", WriteWhenStoppedOrPaused);
}

PiiDefaultOperation::~PiiDefaultOperation()
{
}

void PiiDefaultOperation::createProcessor(ProcessingMode processingMode)
{
  PII_D;
  delete d->pProcessor;
  switch (processingMode)
    {
    case Threaded:
      d->pProcessor = new PiiThreadedProcessor(this);
      break;
    case NonThreaded:
    default:
      d->pProcessor = new PiiSimpleProcessor(this);
      break;
    }
}

void PiiDefaultOperation::setProcessingMode(ProcessingMode processingMode)
{
  PII_D;
  if (d->bChecked)
    return;
  
  if (d->pProcessor == 0 ||
      processingMode != d->pProcessor->processingMode())
    createProcessor(processingMode);
}

PiiDefaultOperation::ProcessingMode PiiDefaultOperation::processingMode() const
{
  return _d()->pProcessor->processingMode();
}

void PiiDefaultOperation::setPriority(int priority)
{
  _d()->pProcessor->setProcessingPriority((QThread::Priority)priority);
}

int PiiDefaultOperation::priority() const
{
  return _d()->pProcessor->processingPriority();
}


void PiiDefaultOperation::syncEvent(SyncEvent* /*event*/)
{
}

void PiiDefaultOperation::interrupt()
{
  interruptOutputs();

  // Interrupt processor
  _d()->pProcessor->interrupt();
}

void PiiDefaultOperation::check(bool reset)
{
  PII_D;
  PiiBasicOperation::check(reset);

  // Install input controller
  for (int i=0; i<d->lstInputs.size(); ++i)
    d->lstInputs[i]->setController(d->pProcessor);
  
  // Install flow controller
  delete d->pFlowController;
  d->pFlowController = createFlowController();

  if (reset)
    PiiFlowController::SyncListener::reset();

  // Store flow controller to the processor
  d->pProcessor->setFlowController(d->pFlowController);
  d->pProcessor->check(reset);
  d->bChecked = true;
}

PiiFlowController* PiiDefaultOperation::createFlowController()
{
  PII_D;
  int iConnectedInputCount = 0, iPositiveGroupIdCount = 0;
  QList<int> lstGroupIds;
  PiiInputSocket* pLastConnectedInput = 0;
  for (int i=0; i<d->lstInputs.size(); ++i)
    {
      PiiInputSocket* pInput = d->lstInputs[i];
      if (!pInput->isConnected()) continue;
      if (!lstGroupIds.contains(pInput->groupId()))
        {
          lstGroupIds << pInput->groupId();
          if (pInput->groupId() >= 0)
            ++iPositiveGroupIdCount;
        }
      ++iConnectedInputCount;
      pLastConnectedInput = pInput;
    }

  if (iConnectedInputCount == 0)
    return 0;
  
  // If there is only one connected input, PiiOneInputFlowController
  // is our choice.
  if (iConnectedInputCount == 1)
    return new PiiOneInputFlowController(pLastConnectedInput, d->lstOutputs);

  // Only one sync group -> PiiOneGroupFlowController.
  if (lstGroupIds.size() == 1)
    return new PiiOneGroupFlowController(d->lstInputs, d->lstOutputs);

  PiiDefaultFlowController::RelationList lstRelations;
  
  // If there are more than one non-negative group id, assign loose
  // relationships between them.
  if (iPositiveGroupIdCount > 1)
    {
      qSort(lstGroupIds.begin(), lstGroupIds.end());

      // Find first non-negative id
      int i=0;
      while (i < lstGroupIds.size()-1 && lstGroupIds[i] < 0) ++i;

      // Assign relationships
      while (i < lstGroupIds.size()-1)
        {
          lstRelations << PiiDefaultFlowController::Relation(lstGroupIds[i], lstGroupIds[i+1], false);
          ++i;
        }
    }

  // No better choice, damnit
  return new PiiDefaultFlowController(d->lstInputs,
                                      d->lstOutputs,
                                      lstRelations);
}

void PiiDefaultOperation::start()
{
  PII_D;
  if (!d->bChecked)
    {
      piiWarning(tr("Tried to call %1::start() without check().").arg(metaObject()->className()));
      return;
    }
  d->bChecked = true;
  d->pProcessor->start();
}

void PiiDefaultOperation::pause()
{
  _d()->pProcessor->pause();
}

void PiiDefaultOperation::stop()
{
  _d()->pProcessor->stop();
}

bool PiiDefaultOperation::wait(unsigned long time)
{
  return _d()->pProcessor->wait(time);
}

int PiiDefaultOperation::activeInputGroup() const
{
  return _d()->pFlowController->activeInputGroup();
}

bool PiiDefaultOperation::isChecked() const
{
  return _d()->bChecked;
}

PiiReadWriteLock* PiiDefaultOperation::processLock()
{
  return &_d()->processLock;
}

bool PiiDefaultOperation::setProperty(const char* name, const QVariant& value)
{
  PiiWriteLocker lock(&_d()->processLock);
  return PiiBasicOperation::setProperty(name, value);
}

QVariant PiiDefaultOperation::property(const char* name)
{
  PiiReadLocker lock(&_d()->processLock);
  return PiiBasicOperation::property(name);
}
