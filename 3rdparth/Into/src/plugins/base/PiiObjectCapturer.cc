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

#include "PiiObjectCapturer.h"

#include <PiiYdinTypes.h>

PiiObjectCapturer::Data::Data() :
  iDynamicInputCount(1),
  bSyncInputConnected(false)
{
}

PiiObjectCapturer::PiiObjectCapturer() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  
  addSocket(d->pSyncInput = new PiiInputSocket("sync"));
  d->pSyncInput->setOptional(true);

  setDynamicInputCount(1);

  setProtectionLevel("dynamicInputCount", WriteWhenStoppedOrPaused);
}

PiiObjectCapturer::~PiiObjectCapturer()
{
  clearObjects();
}

PiiInputSocket*	PiiObjectCapturer::input(const QString &name) const
{
  if (name == "input")
    return PiiDefaultOperation::input("input0");
  return PiiDefaultOperation::input(name);
}

void PiiObjectCapturer::check(bool reset)
{
  PII_D;
  d->bSyncInputConnected = d->pSyncInput->isConnected();

  PiiDefaultOperation::check(reset);
}

void PiiObjectCapturer::initObjectList()
{
  PII_D;

  clearObjects();
  
  for (int i=0; i<d->iDynamicInputCount; i++)
    d->lstObjects.insert(i,QVariantList());
}

void PiiObjectCapturer::clearObjects()
{
  PII_D;
  
  d->syncObject = PiiVariant();
  d->lstObjects.clear();
}

void PiiObjectCapturer::process()
{
  PII_D;
  
  if (activeInputGroup() == 0)
    d->syncObject = d->pSyncInput->firstObject();
  else
    {
      if (d->bSyncInputConnected)
        {
          for (int i=0; i<d->iDynamicInputCount; i++)
            d->lstObjects[i].append(QVariant::fromValue(inputAt(i+1)->firstObject()));
        }
      else
        {
          if (d->iDynamicInputCount == 1)
            emit objectCaptured(inputAt(1)->firstObject());
          else
            {
              QVariantList lstObjects;
              for (int i=1; i<=d->iDynamicInputCount; i++)
                lstObjects << QVariant::fromValue(inputAt(i)->firstObject());

              emit objectsCaptured(lstObjects);
            }
        }
    }
}

void PiiObjectCapturer::syncEvent(SyncEvent* event)
{
  PII_D;

  if (event->type() == SyncEvent::StartInput)
    {
      d->syncObject = PiiVariant();
      for (int i=0; i<d->iDynamicInputCount; i++)
        d->lstObjects[i].clear();
    }
  else
    {
      QVariantList lstObjects;
      for (int i=0; i<d->lstObjects.size(); i++)
        lstObjects.insert(i,d->lstObjects[i]);
      emit objectsCaptured(d->syncObject, lstObjects);
    }
}

void PiiObjectCapturer::setDynamicInputCount(int dynamicInputCount)
{
  PII_D;
  
  if (dynamicInputCount < 1)
    return;
  
  d->iDynamicInputCount = dynamicInputCount;
  setNumberedInputs(d->iDynamicInputCount, 1);

  for (int i=1; i<=d->iDynamicInputCount; ++i)
    PiiDefaultOperation::inputAt(i)->setGroupId(1);
  
  initObjectList();
}

int PiiObjectCapturer::dynamicInputCount() const { return _d()->iDynamicInputCount; }
