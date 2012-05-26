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

#include "PiiObjectCounter.h"

#include <PiiYdinTypes.h>

PiiObjectCounter::Data::Data() :
  iCount(0), bAutoReset(true)
{
}

PiiObjectCounter::PiiObjectCounter() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("count"));
}

void PiiObjectCounter::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);
  if (reset && d->bAutoReset)
    d->iCount = 0;
}

void PiiObjectCounter::process()
{
  PII_D;
  d->lstOutputs[0]->emitObject(++d->iCount);
}


void PiiObjectCounter::setCount(int count) { _d()->iCount = count; }
int PiiObjectCounter::count() const { return _d()->iCount; }
void PiiObjectCounter::setAutoReset(bool autoReset) { _d()->bAutoReset = autoReset; }
bool PiiObjectCounter::autoReset() const { return _d()->bAutoReset; }
