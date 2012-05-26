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

#include "PiiMetaConfigurationValue.h"
#include <cstring> //for strcpy, strlen
#include <cstdlib> //for malloc

PiiMetaConfigurationValue::Data* PiiMetaConfigurationValue::Data::sharedNull()
{
  static PiiMetaConfigurationValue::Data nullData;
  return &nullData;
}

PiiMetaConfigurationValue::Data::Data() :
  pName(0),
  pDescription(0),
  type(QVariant::Invalid),
  typeFlags(0)
{
}

PiiMetaConfigurationValue::Data::Data(const char* name,
                                      const char* description,
                                      QVariant::Type type,
                                      const QVariantList& possibleValues,
                                      const QStringList& descriptions,
                                      TypeFlags flags) :
  pName(name),
  pDescription(description),
  type(type),
  lstPossibleValues(possibleValues),
  lstDescriptions(descriptions),
  typeFlags(flags)
{}

PiiMetaConfigurationValue::Data::Data(const char* name,
                                      const char* description,
                                      QVariant::Type type,
                                      const QVariant& minValue,
                                      const QVariant& maxValue,
                                      TypeFlags flags) :
  pName(name),
  pDescription(description),
  type(type),
  minValue(minValue),
  maxValue(maxValue),
  typeFlags(flags)
{}

PiiMetaConfigurationValue::PiiMetaConfigurationValue() :
  d(Data::sharedNull())
{
  d->reserve();
}

PiiMetaConfigurationValue::PiiMetaConfigurationValue(const char* name,
                                                     const char* description,
                                                     QVariant::Type type,
                                                     const QVariantList& possibleValues,
                                                     const QStringList& descriptions,
                                                     TypeFlags flags) :
  d(new Data(name, description, type, possibleValues, descriptions, flags))
{}

PiiMetaConfigurationValue::PiiMetaConfigurationValue(const char* name,
                                                     const char* description,
                                                     QVariant::Type type,
                                                     const QVariant& minValue,
                                                     const QVariant& maxValue,
                                                     TypeFlags flags) :
  d(new Data(name, description, type, minValue, maxValue, flags))
{}


PiiMetaConfigurationValue::PiiMetaConfigurationValue(const PiiMetaConfigurationValue& other) :
  d(other.d)
{
  d->reserve();
}

PiiMetaConfigurationValue& PiiMetaConfigurationValue::operator= (const PiiMetaConfigurationValue& other)
{
  other.d->assignTo(d);
  return *this;
}

PiiMetaConfigurationValue::~PiiMetaConfigurationValue()
{
  d->release();
}

bool PiiMetaConfigurationValue::isValid() const { return d->type != QVariant::Invalid; }
bool PiiMetaConfigurationValue::isEnumerated() const { return d->lstPossibleValues.size() != 0; }
bool PiiMetaConfigurationValue::isReadable() const { return d->typeFlags & Readable; }
bool PiiMetaConfigurationValue::isWritable() const { return d->typeFlags & Writable; }
bool PiiMetaConfigurationValue::isStored() const { return d->typeFlags & Stored; }
QVariant::Type PiiMetaConfigurationValue::type() const { return d->type; }
QVariant PiiMetaConfigurationValue::maxValue() const { return d->maxValue; }
QVariant PiiMetaConfigurationValue::minValue() const { return d->minValue; }
const char * PiiMetaConfigurationValue::name() const { return d->pName; }
const char * PiiMetaConfigurationValue::description() const { return d->pDescription; }
QVariantList PiiMetaConfigurationValue::possibleValues() const { return d->lstPossibleValues; }
QStringList PiiMetaConfigurationValue::descriptions() const { return d->lstDescriptions; }
