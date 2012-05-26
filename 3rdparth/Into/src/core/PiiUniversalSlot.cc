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

#include "PiiUniversalSlot.h"
#include "PiiUtil.h"
#include "PiiMetaTypeUtil.h"

PiiUniversalSlot::PiiUniversalSlot() : d(new Data)
{}

PiiUniversalSlot::~PiiUniversalSlot()
{
  delete d;
}

int PiiUniversalSlot::dynamicConnect(QObject* source,
                                     const char* signal,
                                     const char* slot,
                                     const Qt::ConnectionType type)
{
  // The SIGNAL macro adds '2' at the beginning.
  if (*signal == '2') ++signal;
  QByteArray aSignalSignature(QMetaObject::normalizedSignature(signal));
  int iSignalId = source->metaObject()->indexOfSignal(aSignalSignature);
  if (iSignalId < 0)
    {
      piiWarning("%s: no such signal.", signal);
      return -1;
    }

  QByteArray aSlotSignature;
  if (slot != 0)
    {
      if (*slot == '1') ++slot;
      aSlotSignature = QMetaObject::normalizedSignature(slot);
      if (!QMetaObject::checkConnectArgs(aSignalSignature, aSlotSignature))
        {
          piiWarning("Cannot connect \"%s\" to \"%s\".", signal, slot);
          return -1;
        }
    }
  else
    aSlotSignature = aSignalSignature;

  int iSlotId = d->lstSlots.indexOf(aSlotSignature);
  // Create a new slot
  if (iSlotId == -1)
    {
      // Parse parameter types between braces.
      int iFirstBrace = aSlotSignature.indexOf('('),
        iLastBrace = aSlotSignature.lastIndexOf(')');
      QList<int> lstTypes(Pii::parseTypes(aSlotSignature.mid(iFirstBrace+1, iLastBrace - iFirstBrace + 1)));
      iSlotId = d->lstSlots.size();
      d->lstSlots << aSlotSignature;
      d->lstTypes << lstTypes;
    }

  QMetaObject::connect(source, iSignalId, this, iSlotId + metaObject()->methodCount(), type);
  return iSlotId;
}

int PiiUniversalSlot::dynamicDisconnect(QObject* source, const char* signal, const char* slot)
{
  // The SIGNAL macro adds '2' at the beginning.
  if (*signal == '2') ++signal;
  QByteArray aSignalSignature(QMetaObject::normalizedSignature(signal));
  int iSignalId = source->metaObject()->indexOfSignal(aSignalSignature);
  if (iSignalId < 0)
    {
      piiWarning("%s: no such signal.", signal);
      return false;
    }
  // The SLOT macro adds '1' at the beginning.
  QByteArray aSlotSignature(slot != 0 ?
                            QMetaObject::normalizedSignature(*slot == '1' ? slot+1 : slot) :
                            aSignalSignature);
  int iSlotId = d->lstSlots.indexOf(aSlotSignature);
  if (iSlotId == -1)
    {
      piiWarning("%s: no such slot.", aSlotSignature.constData());
      return false;
    }

  return QMetaObject::disconnect(source, iSignalId, this, iSlotId + metaObject()->methodCount());
}

int PiiUniversalSlot::qt_metacall(QMetaObject::Call call, int id, void **arguments)
{
  id = QObject::qt_metacall(call, id, arguments);
  if (id < 0 || call != QMetaObject::InvokeMetaMethod)
    return id;
    
  return invokeSlot(id, arguments) ? -1 : id;
}

QByteArray PiiUniversalSlot::signatureOf(int id) const
{
  return d->lstSlots[id];
}

QVariantList PiiUniversalSlot::argsToList(int id, void** args) const
{
  return Pii::argsToList(d->lstTypes[id], args+1);
}

int PiiUniversalSlot::argumentCount(int id) const
{
  return d->lstTypes[id].size();
}
