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

#ifndef _PIIMETAOBJECT_H
#define _PIIMETAOBJECT_H

/**
 * @file
 *
 * Declarations, macros and functions for dealing with meta objects.
 *
 * @ingroup Serialization
 */

#include "PiiSerializationTraits.h"
#include "PiiSerializationGlobal.h"

/**
 * A convenience macro for fetching the meta object for the given
 * object. The parameter @p obj must be a reference type.
 *
 * @relates PiiMetaObject
 */
#define PII_GET_METAOBJECT(obj) PiiSerialization::piiMetaObject(obj)

/**
 * Meta objects store information specific to a certain data type. 
 * Class meta information contains a unique class name and a version
 * number. The tracking flag is an application-wide static constant
 * and not included in the meta object (see
 * PiiSerializationTraits::Tracking).
 *
 * @ingroup Serialization
 */
class PII_SERIALIZATION_EXPORT PiiMetaObject
{
public:
  /**
   * Create a new meta object with the given class name and version
   * number. The @p storeClassInfo flag determines whether the version
   * number is saved or not.
   */
  PiiMetaObject(const char* className,
                unsigned int version,
                bool storeClassInfo) :
    _pClassName(className),
    _uiVersion(version),
    _bStoreClassInfo(storeClassInfo)
  {}

  /**
   * Get the name of the class this meta object represents.
   */
  const char* className() const { return _pClassName; }

  /**
   * Get the current version number of the class.
   */
  unsigned int version() const { return _uiVersion; }

  /**
   * See if the class information (only version number, currently)
   * needs to be stored.
   */
  bool isClassInfoStored() const { return _bStoreClassInfo; }

private:
  const char* _pClassName;
  unsigned int _uiVersion;
  bool _bStoreClassInfo;
};


namespace PiiSerialization
{
  /**
   * Create a default PiiMetaObject instance for the type given as a
   * parameter.
   *
   * @return a meta object that takes class information from
   * PiiSerializationTraits.
   *
   * @relates PiiMetaObject
   */
  template <class T> inline PiiMetaObject defaultMetaObject(const T& /*obj*/)
  {
    return PiiMetaObject(PiiSerializationTraits::ClassName<T>::get(),
                         (unsigned int)PiiSerializationTraits::Version<T>::intValue,
                         PiiSerializationTraits::ClassInfo<T>::boolValue);
  }
}


#endif //_PIIMETAOBJECT_H
