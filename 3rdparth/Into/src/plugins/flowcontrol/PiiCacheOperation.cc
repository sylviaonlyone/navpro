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

#include "PiiCacheOperation.h"

#include <PiiYdinTypes.h>

PiiCacheOperation::Data::Data() :
  iMaxBytes(2*1024*1024),
  iMaxObjects(0),
  bAllowOrderChanges(false),
  iConsumedMemory(0)
{
}

PiiCacheOperation::PiiCacheOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  addSocket(d->pKeyInput = new PiiInputSocket("key"));
  addSocket(d->pDataInput = new PiiInputSocket("data"));
  d->pDataInput->setOptional(true);
  
  addSocket(d->pFoundOutput = new PiiOutputSocket("found"));
  addSocket(d->pKeyOutput = new PiiOutputSocket("key"));
  addSocket(d->pDataOutput = new PiiOutputSocket("data"));
}

void PiiCacheOperation::process()
{
  PII_D;
  if (activeInputGroup() == d->pKeyInput->groupId())
    {
      QString strKey = PiiYdin::convertToQString(d->pKeyInput);
      QHash<QString,PiiVariant>::iterator i = d->hashObjects.find(strKey);
      if (i != d->hashObjects.end())
        d->pDataOutput->emitObject(i.value());
      else
        d->hashObjects.insert(strKey, d->pDataInput->firstObject());
    }
  /*
    int iSize = 64;
    if (PiiYdin::isMatrixType(obj.type()))
    iSize += PiiYdin::matrixRows(obj) * PiiYdin::matrixStride(obj);
    else
    PII_THROW_UNKNOWN_TYPE(inputAt(0));
  */
}


void PiiCacheOperation::setMaxBytes(int maxBytes) { _d()->iMaxBytes = maxBytes; }
int PiiCacheOperation::maxBytes() const { return _d()->iMaxBytes; }
void PiiCacheOperation::setMaxObjects(int maxObjects) { _d()->iMaxObjects = maxObjects; }
int PiiCacheOperation::maxObjects() const { return _d()->iMaxObjects; }
void PiiCacheOperation::setAllowOrderChanges(bool allowOrderChanges) { _d()->bAllowOrderChanges = allowOrderChanges; }
bool PiiCacheOperation::allowOrderChanges() const { return _d()->bAllowOrderChanges; }
