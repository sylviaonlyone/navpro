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

#include "PiiObjectReplicator.h"
#include <PiiDefaultFlowController.h>

PiiObjectReplicator::PiiObjectReplicator() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("data"));
  addSocket(new PiiInputSocket("trigger"));
  inputAt(1)->setGroupId(1);
  addSocket(new PiiOutputSocket("output"));
  outputAt(0)->setGroupId(1);
}

PiiFlowController* PiiObjectReplicator::createFlowController()
{
  return new PiiDefaultFlowController(inputSockets(),
                                      outputSockets(),
                                      PiiDefaultFlowController::RelationList() <<
                                      PiiDefaultFlowController::strictRelation(0, 1));
}

void PiiObjectReplicator::process()
{
  PII_D;
  // If data input has an object, store it
  if (activeInputGroup() == 0)
    d->pData = readInput();
  // Otherwise it must be the trigger that was received
  else
    emitObject(d->pData);
}
