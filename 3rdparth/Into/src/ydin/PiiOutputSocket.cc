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

#include "PiiOutputSocket.h"
#include "PiiInputSocket.h"
#include "PiiYdinTypes.h"
#include "PiiOperation.h"

#include <PiiUtil.h>
#include <PiiSerializableExport.h> // MSVC

using namespace PiiYdin;

PiiOutputSocket::Data::Data(PiiOutputSocket* owner) :
  PiiAbstractOutputSocket::Data(owner),
  iGroupId(0),
  outputPending(PiiWaitCondition::NoQueue),
  pFirstInput(0),
  pFirstController(0),
  bInterrupted(false),
  pbInputCompleted(0)
{
}

bool PiiOutputSocket::Data::setOutputConnected(bool connected)
{
  return bConnected = PiiAbstractOutputSocket::Data::setOutputConnected(connected);
}

PiiAbstractOutputSocket* PiiOutputSocket::Data::rootOutput() const
{
  return q;
}

PiiOutputSocket::PiiOutputSocket(const QString& name) :
  PiiAbstractOutputSocket(new Data(this))
{
  setObjectName(name);
}

PiiOutputSocket::PiiOutputSocket(Data* data, const QString& name) :
  PiiAbstractOutputSocket(data)
{
  setObjectName(name);
}

PiiOutputSocket::~PiiOutputSocket()
{
  delete[] _d()->pbInputCompleted;
}

PiiSocket::Type PiiOutputSocket::type() const { return Output; }

void PiiOutputSocket::setGroupId(int id) { _d()->iGroupId = id; }
int PiiOutputSocket::groupId() const { return _d()->iGroupId; }

bool PiiOutputSocket::isConnected() const
{
  return _d()->bConnected;
}

void PiiOutputSocket::interrupt()
{
  PII_D;
  d->bInterrupted = true;
  // Bypass any forthcoming wait() call.
  d->outputPending.wakeOne();
  d->state = PiiSocketState();
}

void PiiOutputSocket::reset()
{
  PII_D;
  d->bInterrupted = false;
  d->outputPending.wakeAll();
}

void PiiOutputSocket::emitObject(const PiiVariant& object)
{
  PII_D;
  const int iCnt = d->lstInputs.size();

  if (!object.isValid())
    PII_THROW(PiiExecutionException, tr("Trying to send an invalid object."));
  // Optimized emission for a single connected input
  if (iCnt == 1)
    {
      // Try to send to the single input until the object is
      // successfully received.
      for (;;)
        {
          if (d->pFirstController->tryToReceive(d->pFirstInput, object))
            {
              d->outputPending.wakeAll();
              if (d->bInterrupted)
                throw PiiExecutionException(PiiExecutionException::Interrupted);
              break;
            }
          else
            {
              d->outputPending.wait();
              if (d->bInterrupted)
                throw PiiExecutionException(PiiExecutionException::Interrupted);
            }
        }
    }
  else if (iCnt == 0)
    return;
  else
    {
      bool allCompleted = false;

      //Flag inputs that already accepted our object
      for (int i=0; i<iCnt; ++i)
        d->pbInputCompleted[i] = false;

      // Try to send to each input until everything is completed
      for (;;)
        {
          allCompleted = true;
          for (int i=0; i<iCnt; ++i)
            {
              if (!d->pbInputCompleted[i])
                allCompleted &= d->pbInputCompleted[i] =
                  d->lstInputs.controllerAt(i)->tryToReceive(d->lstInputs.inputAt(i), object);
            }
          /*qDebug("Emitting object through %s:%s (%d inputs). Everything completed: %s",
            parentOperation()->metaObject()->className(),
            objectName().toAscii().constData(),
            iCnt,
            allCompleted ? "yes" : "no - must wait");
          */
          if (allCompleted)
            {
              // If we succeeded, clear all signals right away.
              d->outputPending.wakeAll();
              if (d->bInterrupted)
                throw PiiExecutionException(PiiExecutionException::Interrupted);
              break;
            }
          // Some inputs weren't ready -> wait until we receive a signal.
          else
            {
              d->outputPending.wait();
              // Check for interruption. If wait() was interrupted by the
              // parent operation calling interrupt(), we abruptly
              // interrupt the parent's emission.
              if (d->bInterrupted)
                throw PiiExecutionException(PiiExecutionException::Interrupted);
            }
        }
    }
}

void PiiOutputSocket::inputReady(PiiAbstractInputSocket* /*input*/)
{
  _d()->outputPending.wakeOne();
}

PiiOutputSocket* PiiOutputSocket::socket() { return this; }
PiiAbstractInputSocket* PiiOutputSocket::asInput() { return 0; }
PiiAbstractOutputSocket* PiiOutputSocket::asOutput() { return this; }

void PiiOutputSocket::synchronizeTo(PiiInputSocket* input)
{
  setGroupId(input->groupId());
}

void PiiOutputSocket::inputConnected(PiiAbstractInputSocket* input)
{
  PII_D;
  // Run-time optimization
  if (d->lstInputs.size() == 1)
    {
      d->pFirstInput = input;
      d->pFirstController = input->controller();
    }
  createFlagArray();
}

void PiiOutputSocket::inputDisconnected(PiiAbstractInputSocket*)
{
  PII_D;
  if (d->lstInputs.size() > 0)
    {
      d->pFirstInput = d->lstInputs.inputAt(0);
      d->pFirstController = d->lstInputs.controllerAt(0);
    }
  else
    {
      d->pFirstInput = 0;
      d->pFirstController = 0;
    }
  createFlagArray();
}

void PiiOutputSocket::inputUpdated(PiiAbstractInputSocket*)
{
  PII_D;
  if (d->lstInputs.size() > 0)
    d->pFirstController = d->lstInputs.controllerAt(0);
}

void PiiOutputSocket::createFlagArray()
{
  PII_D;
  //qDebug("PiiOutputSocket: creating flag array for %d connections.", d->lstInputs.size());
  delete[] d->pbInputCompleted;
  if (d->lstInputs.size() > 0)
    d->pbInputCompleted = new bool[d->lstInputs.size()];
  else
    d->pbInputCompleted = 0;
}

void PiiOutputSocket::resume(PiiSocketState state)
{
  PII_D;
  //qDebug("Resuming %s::%s: (%d, %d) (%d, %d)", parent() ? parent()->metaObject()->className() : "" ,
  //       qPrintable(objectName()), d->state.flowLevel, d->state.delay, state.flowLevel, state.delay);
  emitObject(PiiSocketState(state.flowLevel + d->state.flowLevel,
                            state.delay + d->state.delay));
}

void PiiOutputSocket::startDelay()
{
  ++_d()->state.delay;
}

void PiiOutputSocket::endDelay()
{
  --_d()->state.delay;
}

void PiiOutputSocket::startMany()
{
  emitObject(PiiVariant(1, SynchronizationTagType));
  ++_d()->state.flowLevel;
}

void PiiOutputSocket::endMany()
{
  emitObject(PiiVariant(-1, SynchronizationTagType));
  --_d()->state.flowLevel;
}
