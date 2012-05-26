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

#include "PiiCameraDriver.h"

PiiCameraDriver::Data::Data() :
  pListener(0)
{
}

PiiCameraDriver::PiiCameraDriver() : d(new Data)
{
}

PiiCameraDriver::~PiiCameraDriver()
{
  delete d;
}

bool PiiCameraDriver::requiresInitialization(const char* /*name*/) const
{
  return false;
}

int PiiCameraDriver::cameraType() const
{
  return (int)PiiCamera::AreaScan;
}

QVariantMap& PiiCameraDriver::propertyMap() { return d->mapProperties; }

QVariant PiiCameraDriver::property(const char* name)
{
  if (isOpen())
    return QObject::property(name);
  else if (d->mapProperties.contains(name))
    return d->mapProperties[name];
  
  return QVariant();
}

bool PiiCameraDriver::setProperty(const char* name, const QVariant& value)
{
  if (!isOpen() || requiresInitialization(name))
    {
      const QMetaObject *pMetaObject = metaObject();
      QVariant::Type type = pMetaObject->property(pMetaObject->indexOfProperty(name)).type();

      if (value.type() == type)
        d->mapProperties[name] = value;
      else
        {
          QVariant copy(value);
          if (copy.convert(type))
            d->mapProperties[name] = copy;
          else
            return false;
        }

      return true;
    }
  else
    return QObject::setProperty(name, value);
}



void PiiCameraDriver::setListener(Listener* listener) { d->pListener = listener; }
PiiCameraDriver::Listener* PiiCameraDriver::listener() const { return d->pListener; }

PiiCameraDriver::Listener::~Listener() {}
void PiiCameraDriver::Listener::frameCaptured(int /*frameIndex*/, void* /*frameBuffer*/, qint64 /*elapsedTime*/) {}
void PiiCameraDriver::Listener::framesMissed(int /*startIndex*/, int /*endIndex*/) {}
void PiiCameraDriver::Listener::captureFinished() {}
void PiiCameraDriver::Listener::captureError(const QString& /*message*/) {}

