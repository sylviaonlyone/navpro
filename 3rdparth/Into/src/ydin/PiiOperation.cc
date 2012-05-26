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

#include "PiiOperation.h"
#include "PiiUtil.h"
#include <PiiSerializationFactory.h>
#include <PiiSerializableExport.h>
#include "PiiYdinResources.h"

PII_DEFINE_VIRTUAL_METAOBJECT_FUNCTION(PiiOperation);
PII_SERIALIZABLE_EXPORT(PiiOperation);
PII_SERIALIZABLE_EXPORT(PiiQVariantWrapper::Template<PiiOperation*>);

static int iOperationMetaType = qRegisterMetaType<PiiOperation*>("PiiOperation*");

PiiOperation::Data::Data() :
  stateMutex(QMutex::Recursive)
{
}

PiiOperation::Data::~Data()
{
}

PiiOperation::PiiOperation() :
  d(new Data)
{
}

PiiOperation::PiiOperation(Data* data) :
  d(data)
{
}

PiiOperation::~PiiOperation()
{
  delete d;
}

int PiiOperation::inputCount() const
{
  return inputs().size();
}

int PiiOperation::outputCount() const
{
  return outputs().size();
}

QStringList PiiOperation::inputNames() const
{
  const QList<PiiAbstractInputSocket*> lstInputs = inputs();
  QStringList lstResult;
  for (int i=0; i<lstInputs.size(); i++)
    lstResult << socketName(lstInputs[i]);
  return lstResult;
}

QStringList PiiOperation::outputNames() const
{
  const QList<PiiAbstractOutputSocket*> lstOutputs = outputs();
  QStringList lstResult;
  for (int i=0; i<lstOutputs.size(); i++)
    lstResult << socketName(lstOutputs[i]);
  return lstResult;
}


bool PiiOperation::connectOutput(const QString& outputName, PiiOperation* other, const QString& inputName)
{
  if (other == 0)
    {
      piiWarning(tr("Cannot connect \"%1\" to the \"%2\" input of a null operation. "
                    "You probably forgot to load a required plug-in").arg(outputName, inputName));
      return false;
    }
  PiiAbstractOutputSocket *out = output(outputName);
  if (out == 0)
    {
      piiWarning(tr("There is no \"%1\" output in %2.").arg(outputName, metaObject()->className()));
      return false;
    }
  PiiAbstractInputSocket *in = other->input(inputName);
  if (in == 0)
    {
      piiWarning(tr("There is no \"%1\" input in %2.").arg(inputName, other->metaObject()->className()));
      return false;
    }
  out->connectInput(in);
  return true;
}

QString PiiOperation::socketName(PiiAbstractSocket* socket) const
{
  return socketProperty(socket, "name").toString();
}

QVariant PiiOperation::socketProperty(PiiAbstractSocket*, const char*) const
{
  return QVariant();
}

QVariant PiiOperation::socketProperty(PiiAbstractSocket* socket, const QString& name) const
{
  return socketProperty(socket, piiPrintable(name));
}

const char* PiiOperation::stateName(State state)
{
  static const char* states[] = { "Stopped", "Starting", "Running", "Pausing", "Paused", "Stopping", "Interrupted" };
  return states[state];
}

bool PiiOperation::setProperty(const char* name, const QVariant& value)
{
  synchronized (&d->stateMutex)
    {
      ProtectionLevel level = protectionLevel(name);
      if (level != WriteAlways)
        {
          State currentState = state();
          if ((level == WriteWhenStoppedOrPaused && currentState not_member_of (Stopped, Paused)) ||
              (level == WriteWhenStopped && currentState != Stopped))
            return false;
        }
      return QObject::setProperty(name, value);
    }
  return true;
}

bool PiiOperation::setProperty(const char* name, const PiiVariant& value)
{
  return setProperty(name, QVariant::fromValue(value));
}

QVariant PiiOperation::property(const char* name)
{
  return QObject::property(name);
}

PiiOperation* PiiOperation::clone() const
{
  // objectName() won't work for template classes, but we have the
  // serialization meta object...
  const char *className = PiiYdin::resourceName(this);
  
  // Use the serialization factory to create an instance of the class
  PiiOperation *op = PiiYdin::createResource<PiiOperation>(className);
  
  if (op != 0)
    {
      //get and set properties from the old to the new one
      QList<QPair<QString, QVariant> > properties = Pii::propertyList(*this);
      Pii::setPropertyList(*op,properties);
    }
  return op;
}

void PiiOperation::disconnectAllInputs()
{
  QList<PiiAbstractInputSocket*> lstInputs = inputs();

  for ( int i=0; i<lstInputs.size(); i++ )
    lstInputs[i]->disconnectOutput();
}

void PiiOperation::disconnectAllOutputs()
{
  QList<PiiAbstractOutputSocket*> lstOutputs = outputs();

  for ( int i=0; i<lstOutputs.size(); i++ )
    lstOutputs[i]->disconnectInput();
}

int PiiOperation::indexOf(const char* property) const
{
  for (int i=0; i<d->lstProtectionLevels.size(); ++i)
    if (!strcmp(d->lstProtectionLevels[i].first, property))
      return i;
  return -1;
}

void PiiOperation::setProtectionLevel(const char* property, ProtectionLevel level)
{
  int iIndex = indexOf(property);
  if (level != WriteAlways)
    {
      // Add new protection level
      if (iIndex == -1)
        d->lstProtectionLevels.append(qMakePair(property, level));
      // Update existing one
      else
        d->lstProtectionLevels[iIndex].second = level;
    }
  else if (iIndex != -1)// Remove protection
    d->lstProtectionLevels.removeAt(iIndex);
}

PiiOperation::ProtectionLevel PiiOperation::protectionLevel(const char* property) const
{
  int iIndex = indexOf(property);
  if (iIndex == -1) return WriteAlways;
  return d->lstProtectionLevels[iIndex].second;
}

QMutex* PiiOperation::stateLock() { return &d->stateMutex; }
