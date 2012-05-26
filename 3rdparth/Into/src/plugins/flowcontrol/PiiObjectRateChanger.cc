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

#include "PiiObjectRateChanger.h"

PiiObjectRateChanger::Data::Data() :
  iRateChange(0), iCounter(0)
{
}

PiiObjectRateChanger::PiiObjectRateChanger() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

void PiiObjectRateChanger::check(bool reset)
{
  PiiDefaultOperation::check(reset);
  if (reset)
    _d()->iCounter = 0;
}

void PiiObjectRateChanger::process()
{
  PII_D;
  PiiVariant obj = readInput();

  if (d->iRateChange >= 0)
    {
      for (int i=0; i<=d->iRateChange; ++i)
        emitObject(obj);
    }
  else
    {
      if (--d->iCounter < d->iRateChange)
        {
          emitObject(obj);
          d->iCounter = 0;
        }
    }
}

void PiiObjectRateChanger::setRateChange(int rateChange) { _d()->iRateChange = rateChange; }
int PiiObjectRateChanger::rateChange() const { return _d()->iRateChange; }
