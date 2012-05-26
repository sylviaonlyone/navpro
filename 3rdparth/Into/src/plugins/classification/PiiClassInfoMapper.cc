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

#include "PiiClassInfoMapper.h"

#include <PiiYdinTypes.h>

PiiClassInfoMapper::Data::Data() :
  bTraining(false), iMaxClasses(1024)
{
}

PiiClassInfoMapper::PiiClassInfoMapper() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  addSocket(d->pIndexInput = new PiiInputSocket("class index"));
  addSocket(d->pDataInput = new PiiInputSocket("data"));
  addSocket(new PiiOutputSocket("data"));
}

void PiiClassInfoMapper::check(bool reset)
{
  PII_D;
  if (reset)
    d->pDataInput->setOptional(!d->bTraining);
  
  PiiDefaultOperation::check(reset);

  if (!d->defaultValue.isValid())
    PII_THROW(PiiExecutionException, tr("Default value must be set to a non-zero value."));
}

void PiiClassInfoMapper::process()
{
  PII_D;
  PiiVariant obj = d->pIndexInput->firstObject();

  int index = -1;
  switch (obj.type())
    {
      PII_PRIMITIVE_CASES(index = (int)PiiYdin::primitiveAs, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pIndexInput);
    }

  if (index < 0) // we don't accept negative indices, sorry
    emitObject(d->defaultValue);
  else if (d->bTraining) // in training mode we collect data
    {
      PiiVariant obj = d->pDataInput->firstObject();
      // In training mode we just pass the incoming object
      emitObject(obj);
      // Store the data if index doesn't exceed the limit
      if (index < d->iMaxClasses)
        {
          // Add empty default values if necessary
          while (d->lstData.size() <= index)
            d->lstData << d->defaultValue;
          // Store the actual data
          d->lstData[index] = obj;
        }
    }
  else if (index < d->lstData.size()) // in normal operation we pass the data learned
    emitObject(d->lstData[index]);
  else
    emitObject(d->defaultValue);
}


void PiiClassInfoMapper::setTraining(bool training) { _d()->bTraining = training; }
bool PiiClassInfoMapper::training() const { return _d()->bTraining; }
void PiiClassInfoMapper::setMaxClasses(int maxClasses) { _d()->iMaxClasses = maxClasses; }
int PiiClassInfoMapper::maxClasses() const { return _d()->iMaxClasses; }
void PiiClassInfoMapper::setDefaultValue(const PiiVariant& defaultValue) { _d()->defaultValue = defaultValue; }
PiiVariant PiiClassInfoMapper::defaultValue() const { return _d()->defaultValue; }
