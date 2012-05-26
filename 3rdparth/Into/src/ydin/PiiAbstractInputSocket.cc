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

#include "PiiAbstractInputSocket.h"
#include "PiiAbstractOutputSocket.h"

PiiAbstractInputSocket::Data::Data() :
  pConnectedOutput(0)
{
}

PiiAbstractInputSocket::Data::~Data()
{
}

bool PiiAbstractInputSocket::Data::setInputConnected(bool connected)
{
  return connected;
}

PiiAbstractInputSocket::PiiAbstractInputSocket(Data* data) :
  d(data)
{
}

PiiAbstractInputSocket::~PiiAbstractInputSocket()
{
  if (d != 0)
    {
      disconnectOutput();
      delete d;
    }
}

void PiiAbstractInputSocket::disconnectOutput()
{
  if (connectedOutput() != 0)
    connectedOutput()->disconnectInput(this);
}

void PiiAbstractInputSocket::connectOutput(PiiAbstractOutputSocket* output)
{
  disconnectOutput();
  if (output != 0)
    output->connectInput(this);
}

PiiAbstractOutputSocket* PiiAbstractInputSocket::connectedOutput() const
{
  return d->pConnectedOutput;
}
