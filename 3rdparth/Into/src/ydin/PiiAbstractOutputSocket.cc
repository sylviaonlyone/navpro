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

#include "PiiAbstractOutputSocket.h"

#include "PiiAbstractInputSocket.h"
#include "PiiInputController.h"

PiiAbstractOutputSocket::Data::Data(PiiAbstractOutputSocket* owner) :
  q(owner)
{
}

PiiAbstractOutputSocket::Data::~Data() 
{
}

bool PiiAbstractOutputSocket::Data::setOutputConnected(bool connected)
{
  bool bBranchConnected = false;
  // An output is forward-connected only if at least one of its child
  // branches is connected.
  for (int i=lstInputs.size(); i--; )
    bBranchConnected |= lstInputs.inputAt(i)->d->setInputConnected(connected);

  // The output is fully connected if it is both backward and forward
  // connected.
  return bBranchConnected && connected;
}

PiiAbstractOutputSocket::PiiAbstractOutputSocket(Data* data) :
  d(data)
{
}

PiiAbstractOutputSocket::~PiiAbstractOutputSocket()
{
  // Invalidate owner
  d->q = 0;
  // Break all connections
  disconnectInput();
  delete d;
}

PiiAbstractOutputSocket* PiiAbstractOutputSocket::rootOutput() const
{
  return d->rootOutput();
}

QList<PiiAbstractInputSocket*> PiiAbstractOutputSocket::connectedInputs() const
{
  QList<PiiAbstractInputSocket*> lstInputs;
  for (int i=0; i<d->lstInputs.size(); ++i)
    lstInputs << d->lstInputs.inputAt(i);
  return lstInputs;
}

void PiiAbstractOutputSocket::updateInput(PiiAbstractInputSocket* socket)
{
  if (d->lstInputs.indexOf(socket) != -1)
    {
      d->lstInputs.updateController(socket);
      inputUpdated(socket);
    }
}

void PiiAbstractOutputSocket::connectInput(PiiAbstractInputSocket* input)
{
  if (input != 0 && d->lstInputs.indexOf(input) == -1)
    {
      // Disconnect old output
      if (input->connectedOutput() != 0)
        input->disconnectOutput();

      // Store the connected input and its controller
      d->lstInputs.append(input);
      
      input->d->pConnectedOutput = this;

      PiiAbstractOutputSocket* pRootOutput = d->rootOutput();
      if (pRootOutput != 0)
        pRootOutput->d->setOutputConnected(true);

      inputConnected(input);
    }
}

void PiiAbstractOutputSocket::disconnectInput(PiiAbstractInputSocket* input)
{
  if (input != 0)
    disconnectInputAt(d->lstInputs.indexOf(input));
  else //disconnect all if input == 0
    {
      for (int i=d->lstInputs.size(); i--;)
        disconnectInputAt(i);
    }
}

void PiiAbstractOutputSocket::disconnectInputAt(int index)
{
  if (index >= 0 && index < d->lstInputs.size())
    {
      PiiAbstractInputSocket* input = d->lstInputs.takeInputAt(index);

      input->d->pConnectedOutput = 0;
      input->d->setInputConnected(false);

      PiiAbstractOutputSocket* pRootOutput = d->rootOutput();
      if (pRootOutput != 0)
        pRootOutput->d->setOutputConnected(true);

      if (d->q != 0)
        inputDisconnected(input);
    }
}

void PiiAbstractOutputSocket::reconnect(PiiAbstractOutputSocket* output, PiiAbstractInputSocket* input)
{
  // Store connected inputs
  InputList lstInputs = d->lstInputs;
  // Disconnect them all
  disconnectInput();
  // Reconnect inputs to the new output
  for (int i=0; i<lstInputs.size(); ++i)
    output->connectInput(lstInputs.inputAt(i));
  // Reconnect this output to new input
  if (input != 0)
    connectInput(input);
}

void PiiAbstractOutputSocket::inputConnected(PiiAbstractInputSocket*)
{
}

void PiiAbstractOutputSocket::inputUpdated(PiiAbstractInputSocket*)
{
}

void PiiAbstractOutputSocket::inputDisconnected(PiiAbstractInputSocket*)
{
}


int PiiAbstractOutputSocket::InputList::indexOf(PiiAbstractInputSocket* input) const
{
  int i = size();
  while (i--)
    if (at(i).first == input)
      break;
  return i;
}

PiiAbstractInputSocket* PiiAbstractOutputSocket::InputList::takeInputAt(int index)
{
  PiiAbstractInputSocket* pInput = inputAt(index);
  remove(index);
  return pInput;
}

PiiAbstractInputSocket* PiiAbstractOutputSocket::InputList::inputAt(int index) const
{
  return at(index).first;
}

PiiInputController* PiiAbstractOutputSocket::InputList::controllerAt(int index) const
{
  return at(index).second;
}

void PiiAbstractOutputSocket::InputList::append(PiiAbstractInputSocket* input)
{
  InputListParent::append(qMakePair(input, input->controller()));
}

void PiiAbstractOutputSocket::InputList::updateController(PiiAbstractInputSocket* input)
{
  int i = size();
  while (i--)
    if (at(i).first == input)
      {
        operator[](i).second = input->controller();
        return;
      }
}
