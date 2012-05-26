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

#include "PiiDemuxOperation.h"
#include <PiiYdinTypes.h>

PiiDemuxOperation::PiiDemuxOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("control"));
  addSocket(new PiiInputSocket("input"));
  setDynamicOutputCount(2);
}

void PiiDemuxOperation::setDynamicOutputCount(int cnt)
{
  if (cnt < 1) return;
  setNumberedOutputs(cnt);
}

int PiiDemuxOperation::dynamicOutputCount() const { return outputCount(); }

void PiiDemuxOperation::process()
{
  int iCtrl = PiiYdin::primitiveAs<int>(input(0));
  if (iCtrl < 0 || iCtrl >= outputCount())
    PII_THROW(PiiExecutionException, tr("Control input value (%1) is out of range (0-%2).").arg(iCtrl).arg(outputCount()-1));

  // Pass the incoming data to the output selected by the control input
  emitObject(readInput(1), iCtrl);
}
