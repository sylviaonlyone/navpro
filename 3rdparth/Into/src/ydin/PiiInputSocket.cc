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

#include "PiiInputSocket.h"
#include "PiiOutputSocket.h"
#include "PiiYdinTypes.h"

#include <QStringList>

using namespace PiiYdin;

PiiInputSocket::Data::Data() :
  iGroupId(0),
  bConnected(false),
  bOptional(false),
  pController(0),
  pQueue(0),
  iQueueCapacity(0),
  iQueueStart(0),
  iQueueLength(1)
{
}

bool PiiInputSocket::Data::setInputConnected(bool connected)
{
  return bConnected = connected;
}

PiiInputSocket::PiiInputSocket(const QString& name) :
  PiiAbstractInputSocket(new Data)
{
  setObjectName(name);
  setQueueCapacity(3);
}

PiiInputSocket::PiiInputSocket(Data* data, const QString& name) :
  PiiAbstractInputSocket(data)
{
  setObjectName(name);
  setQueueCapacity(3);
}

PiiInputSocket::~PiiInputSocket()
{
  delete[] _d()->pQueue;
}

PiiSocket::Type PiiInputSocket::type() const { return Input; }

bool PiiInputSocket::isConnected() const
{
  return _d()->bConnected;
}

void PiiInputSocket::setGroupId(int id) { _d()->iGroupId = id; }
int PiiInputSocket::groupId() const { return _d()->iGroupId; }

void PiiInputSocket::setQueueCapacity(int queueCapacity)
{
  PII_D;
  if (queueCapacity < 2) return;
  d->iQueueCapacity = queueCapacity;
  delete[] d->pQueue;
  d->pQueue = new PiiVariant[queueCapacity];
  reset();
}

void PiiInputSocket::receive(const PiiVariant& obj)
{
  PII_D;
  d->pQueue[queueIndex(d->iQueueLength)] = obj;
  ++d->iQueueLength;
}

void PiiInputSocket::release()
{
  PII_D;
  if (d->iQueueLength > 1)
    {
      // Destroy old head object
      d->pQueue[d->iQueueStart] = PiiVariant();
      // Rotate the queue
      d->iQueueStart = (d->iQueueStart+1) % d->iQueueCapacity;
      --d->iQueueLength;
      // Signal the sender
      if (d->pConnectedOutput != 0)
        d->pConnectedOutput->inputReady(this);
    } 
}

void PiiInputSocket::jump(int oldIndex, int newIndex)
{
  if (oldIndex != newIndex)
    {
      PiiVariant tmpObj = objectAt(oldIndex);
      for (int i=oldIndex-1; i>=newIndex; --i)
        setObject(i+1, objectAt(i));
      setObject(newIndex, tmpObj);
    }
}

int PiiInputSocket::indexOf(unsigned int type, int startIndex) const
{
  const PII_D;
  for (int i=startIndex; i<d->iQueueLength; ++i)
    {
      int iQueueIndex = queueIndex(i);
      if (d->pQueue[iQueueIndex].type() == type)
        return i;
    }
  return -1;
}

void PiiInputSocket::setObject(int index, const PiiVariant& object)
{
  _d()->pQueue[queueIndex(index)] = object;
}

void PiiInputSocket::reset()
{
  PII_D;
  for (int i=0; i<d->iQueueCapacity; ++i)
    d->pQueue[i] = PiiVariant();
  d->iQueueLength = 1;
  d->iQueueStart = 0;
}

void PiiInputSocket::setController(PiiInputController* controller)
{
  PII_D;
  d->pController = controller;
  if (d->pConnectedOutput != 0)
    d->pConnectedOutput->updateInput(this);
}

PiiInputController* PiiInputSocket::controller() const { return _d()->pController; }
PiiInputSocket* PiiInputSocket::socket() { return this; }
PiiAbstractInputSocket* PiiInputSocket::asInput() { return this; }
PiiAbstractOutputSocket* PiiInputSocket::asOutput() { return 0; }
PiiVariant PiiInputSocket::firstObject() const { return _d()->pQueue[_d()->iQueueStart]; }
PiiVariant& PiiInputSocket::firstObject() { return _d()->pQueue[_d()->iQueueStart]; }
bool PiiInputSocket::isEmpty() const { return _d()->iQueueLength > 0; }
PiiVariant PiiInputSocket::objectAt(int index) const { return _d()->pQueue[queueIndex(index)]; }
PiiVariant& PiiInputSocket::objectAt(int index) { return _d()->pQueue[queueIndex(index)]; }
unsigned int PiiInputSocket::typeAt(int index) const { return _d()->pQueue[queueIndex(index)].type(); }
int PiiInputSocket::queueLength() const { return _d()->iQueueLength; }
int PiiInputSocket::queueCapacity() const { return _d()->iQueueCapacity; }
bool PiiInputSocket::canReceive() const { return _d()->iQueueCapacity > _d()->iQueueLength; }
void PiiInputSocket::setOptional(bool optional) { _d()->bOptional = optional; }
bool PiiInputSocket::isOptional() const { return _d()->bOptional; }


namespace PiiYdin
{
  QString unknownTypeErrorMessage(PiiInputSocket* input)
  {
    return QCoreApplication::translate("PiiInputSocket", "An object of an unknown type (0x%1) was received in \"%2\" input.")
      .arg(input->firstObject().type(), 0, 16)
      .arg(input->objectName());
  }

  QString wrongSizeErrorMessage(PiiInputSocket* input, const QString& received, const QString& assumed)
  {
    return QCoreApplication::translate("PiiInputSocket", "The matrix received in \"%1\" is %2, but %3 was assumed.")
      .arg(input->objectName()).arg(received).arg(assumed);
  }
}
