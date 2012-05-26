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

#include <PiiProxySocket.h>
#include <PiiAlgorithm.h>

PiiProxySocket::Data::Data(PiiProxySocket* owner) :
  PiiAbstractOutputSocket::Data(owner),
  pbInputCompleted(0)
{
}

PiiProxySocket::PiiProxySocket() :
  PiiAbstractOutputSocket(new Data(this)),
  PiiAbstractInputSocket(_d())
{
}

PiiProxySocket::~PiiProxySocket()
{
  delete[] _d()->pbInputCompleted;
  // Avoid double delete
  disconnectOutput();
  PiiAbstractInputSocket::d = 0;
}

PiiAbstractOutputSocket* PiiProxySocket::Data::rootOutput() const
{
  if (pConnectedOutput)
    return pConnectedOutput->rootOutput();
  return 0;
}

bool PiiProxySocket::Data::setInputConnected(bool connected)
{
  return setOutputConnected(connected);
}

PiiSocket::Type PiiProxySocket::type() const { return Proxy; }

PiiInputController* PiiProxySocket::controller() const
{
  return const_cast<PiiProxySocket*>(this);
}

bool PiiProxySocket::tryToReceive(PiiAbstractInputSocket* /*sender*/, const PiiVariant& object) throw ()
{
  PII_D;
  bool bAllCompleted = true;
  // Try to send to inputs that haven't already accepted the object.
  for (int i=d->lstInputs.size(); i--; )
    if (!d->pbInputCompleted[i])
      bAllCompleted &= (d->pbInputCompleted[i] = d->lstInputs.controllerAt(i)->tryToReceive(d->lstInputs.inputAt(i), object));

  // Everything is done -> reinitialize completion flags and return
  // true
  if (bAllCompleted)
    {
      Pii::fillN(d->pbInputCompleted, d->lstInputs.size(), false);
      return true;
    }
  // Something is still missing
  return false;
}

void PiiProxySocket::inputReady(PiiAbstractInputSocket*)
{
  // Pass this signal to the proxied output
  _d()->pConnectedOutput->inputReady(this);
}

void PiiProxySocket::inputConnected(PiiAbstractInputSocket*)
{
  reset();
}

void PiiProxySocket::inputDisconnected(PiiAbstractInputSocket*)
{
  reset();
}

void PiiProxySocket::reset()
{
  PII_D;
  //qDebug("PiiOutputSocket: creating flag array for %d connections.", d->lstInputs.size());
  delete[] d->pbInputCompleted;
  if (d->lstInputs.size() > 0)
    {
      d->pbInputCompleted = new bool[d->lstInputs.size()];
      Pii::fillN(d->pbInputCompleted, d->lstInputs.size(), false);
    }
  else
    d->pbInputCompleted = 0;
}

PiiProxySocket* PiiProxySocket::socket() { return this; }
PiiAbstractInputSocket* PiiProxySocket::asInput() { return this; }
PiiAbstractOutputSocket* PiiProxySocket::asOutput() { return this; }
