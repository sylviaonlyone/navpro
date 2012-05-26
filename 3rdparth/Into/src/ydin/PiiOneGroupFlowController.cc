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

#include "PiiOneGroupFlowController.h"
#include "PiiOutputSocket.h"

PiiOneGroupFlowController::Data::Data(const QList<PiiInputSocket*>& inputs,
                                      const QList<PiiOutputSocket*>& outputs)
{
  // Store connected inputs
  for (int i=0; i<inputs.size(); ++i)
    if (inputs[i]->isConnected())
      vecInputs << inputs[i];

  // Store connected outputs
  for (int i=0; i<outputs.size(); ++i)
    if (outputs[i]->isConnected() && outputs[i]->groupId() == vecInputs[0]->groupId())
      vecOutputs << outputs[i];

  if (vecInputs.size() > 0)
    iActiveInputGroup = vecInputs[0]->groupId();
}

PiiOneGroupFlowController::PiiOneGroupFlowController(const QList<PiiInputSocket*>& inputs,
                                                     const QList<PiiOutputSocket*>& outputs) :
  PiiFlowController(new Data(inputs, outputs))
{
}

void PiiOneGroupFlowController::releaseInputs()
{
  PII_D;
  for (int i=0; i<d->vecInputs.size(); ++i)
    d->vecInputs[i]->release();
}

PiiFlowController::FlowState PiiOneGroupFlowController::prepareProcess()
{
  PII_D;
  int typeMask = inputGroupTypeMask(d->vecInputs.begin(), d->vecInputs.end());

  switch (typeMask)
    {
    case NormalObject:
      // All objects are there -> release sockets and process
      releaseInputs();
      return ProcessableState;

    case NoObject:
      // Not all objects are there
      return IncompleteState;

    case StopTag:
      releaseInputs();
      return FinishedState;

    case PauseTag:
      releaseInputs();
      return PausedState;

    case ResumeTag:
      releaseInputs();
      return ResumedState;

    case EndTag:
    case StartTag:
      {
        releaseInputs();
        // Pass the tag
        const PiiVariant& obj = d->vecInputs[0]->firstObject();
        for (int i=0; i<d->vecOutputs.size(); ++i)
          d->vecOutputs[i]->emitObject(obj);
        return SynchronizedState;
      }
      
    default:
      // Oops
      PII_THROW(PiiExecutionException,
                tr("Synchronization error: inputs are in inconsistent state (type mask 0x%2).\n").arg(typeMask, 0, 16) +
                dumpInputObjects(d->vecInputs.begin(), d->vecInputs.end()));
    }
}
