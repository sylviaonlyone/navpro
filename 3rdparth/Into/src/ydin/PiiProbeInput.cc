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

#include "PiiProbeInput.h"

PiiProbeInput::PiiProbeInput(const QString& name) :
  PiiAbstractInputSocket(new Data)
{
  setObjectName(name);
}

PiiProbeInput::PiiProbeInput(PiiAbstractOutputSocket* output, const QObject* receiver,
                             const char* slot, Qt::ConnectionType type) :
  PiiAbstractInputSocket(new Data)
{
  connectOutput(output);
  QObject::connect(this, SIGNAL(objectReceived(PiiVariant)), receiver, slot, type);
}

bool PiiProbeInput::tryToReceive(PiiAbstractInputSocket*, const PiiVariant& object) throw ()
{
  emit objectReceived(object);
  _d()->varSavedObject = object;
  return true;
}

PiiSocket::Type PiiProbeInput::type() const { return Input; }


PiiInputController* PiiProbeInput::controller() const
{
  return const_cast<PiiProbeInput*>(this);
}

PiiProbeInput* PiiProbeInput::socket() { return this; }
PiiAbstractInputSocket* PiiProbeInput::asInput() { return this; }
PiiAbstractOutputSocket* PiiProbeInput::asOutput() { return 0; }

PiiVariant PiiProbeInput::savedObject() const { return _d()->varSavedObject; }
void PiiProbeInput::setSavedObject(const PiiVariant& obj) { _d()->varSavedObject = obj; }
bool PiiProbeInput::hasSavedObject() const { return _d()->varSavedObject.isValid(); }
