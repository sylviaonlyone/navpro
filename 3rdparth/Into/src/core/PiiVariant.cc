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

#include "PiiVariant.h"
#include <PiiSerializableExport.h>
#include <PiiSerializationTraits.h>
#include <PiiSerialization.h>
#include <PiiQVariantWrapper.h>

PII_REGISTER_VARIANT_TYPE(char);
PII_REGISTER_VARIANT_TYPE(bool);
PII_REGISTER_VARIANT_TYPE(short);
PII_REGISTER_VARIANT_TYPE(int);
PII_REGISTER_VARIANT_TYPE(qint64);
//PII_REGISTER_VARIANT_TYPE(long long);
PII_REGISTER_VARIANT_TYPE(float);
PII_REGISTER_VARIANT_TYPE(double);
//PII_REGISTER_VARIANT_TYPE(long double);
PII_REGISTER_VARIANT_TYPE(unsigned char);
PII_REGISTER_VARIANT_TYPE(unsigned short);
PII_REGISTER_VARIANT_TYPE(unsigned int);
PII_REGISTER_VARIANT_TYPE(quint64);
//PII_REGISTER_VARIANT_TYPE(unsigned long long);

// Register PiiVariant as a Qt metatype.
int iPiiVariantTypeId = qRegisterMetaType<PiiVariant>("PiiVariant");

// Register PiiQVariantMapper::Template<PiiVariant> as a serializable
// type.
#define PII_SERIALIZABLE_CLASS PiiQVariantWrapper::Template<PiiVariant>
#define PII_SERIALIZABLE_CLASS_NAME "PiiQVariantWrapper<PiiVariant>"
#include <PiiSerializableRegistration.h>


// Save a few bytes by not repeating the string.
const char* PiiVariant::pValueStr = "value";

QHash<unsigned int, PiiVariant::VTable*>* PiiVariant::hashVTables()
{
  static QHash<unsigned int, VTable*> hash;
  return &hash;
}

PiiVariant::PiiVariant() :
  _pVTable(0), _uiType(InvalidType)
{
}

PiiVariant::PiiVariant(const PiiVariant& other) :
  _pVTable(other._pVTable), _uiType(other._uiType)
{
  if (other._pVTable != 0)
    other._pVTable->constructCopy(*this, other);
  else
    _value = other._value;
}

PiiVariant& PiiVariant::operator= (const PiiVariant& other)
{
  if (&other != this)
    {
      if (_uiType == other._uiType)
        {
          if (_pVTable == 0)
            _value = other._value;
          else
            _pVTable->copy(*this, other);
        }
      else
        {
          if (_pVTable != 0)
            _pVTable->destruct(*this);
          if (other._pVTable == 0)
            _value = other._value;
          else
            other._pVTable->constructCopy(*this, other);

          _uiType = other._uiType;
          _pVTable = other._pVTable;
        }
    }
  return *this;
}

PiiVariant::~PiiVariant()
{
  if (_pVTable != 0)
    _pVTable->destruct(*this);
}

PiiVariant::VTable* PiiVariant::vTableByType(unsigned int type)
{
  return hashVTables()->value(type);
}
