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

#include "PiiOneInputFlowController.h"
#include "PiiOutputSocket.h"

PiiOneInputFlowController::Data::Data(PiiInputSocket* input,
                                      const QList<PiiOutputSocket*>& outputs) :
  pInput(input), iOutputCount(0)
{
  // Store synchronized outputs
  for (int i=0; i<outputs.size(); ++i)
    if (outputs[i]->groupId() == pInput->groupId())
      vecOutputs << outputs[i];

  iActiveInputGroup = input->groupId();
  iOutputCount = vecOutputs.size();
}

PiiOneInputFlowController::PiiOneInputFlowController(PiiInputSocket* input,
                                                     const QList<PiiOutputSocket*>& outputs) :
  PiiFlowController(new Data(input, outputs))
{
}

PiiFlowController::FlowState PiiOneInputFlowController::prepareProcess()
{
  PII_D;
  using namespace PiiYdin;
  unsigned int uiType = d->pInput->typeAt(1);
  //qDebug("PiiOneInputFlowController::prepareProcess(): queue length = %d, type = 0x%x", d->pInput->queueLength(), uiType);
  if (uiType == PiiVariant::InvalidType)
    return IncompleteState;
  // If the incoming object is an ordinary one
  else if (isNonControlType(uiType))
    {
      // The input is now free and we are ready to process
      d->pInput->release();
      return ProcessableState;
    }
  else
    {
      switch (uiType)
        {
        case SynchronizationTagType:
          {
            d->pInput->release();
            const PiiVariant& tag = d->pInput->firstObject();
            // Pass sync tags to all synchronized outputs
            for (int i=0; i<d->iOutputCount; ++i)
              d->vecOutputs[i]->emitObject(tag);
            return SynchronizedState;
          }
        case StopTagType:
          d->pInput->release();
          return FinishedState;
          
        case PauseTagType:
          d->pInput->release();
          return PausedState;

        case ResumeTagType:
          d->pInput->release();
          return ResumedState;

        default:
          PII_THROW_UNKNOWN_TYPE(d->pInput);
        }
    }
}
