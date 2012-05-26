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

#ifndef _PIIBASECLASS_H
#define _PIIBASECLASS_H

#include "PiiSerializationTraits.h"

/**
 * @file
 *
 * Base class serialization stuff.
 *
 * @ingroup Serialization
 */

/**
 * Invoke the serialization of the base class of your class. This
 * macro is used within a serialization function to store the state of
 * a parent class. It works both in member functions and in separate
 * serialization functions.
 *
 * @param archive an instance of an input or output archive
 *
 * @param base the name of the direct base class of the serializable
 * class
 *
 * Assume @p MyDerivedClass is derived from @p MyClass. Then:
 *
 * @code
 * template <class Archive>
 * void MyDerivedClass::serialize(Archive& archive, const unsigned int version)
 * {
 *   PII_SERIALIZE_BASE(archive, MyClass);
 *   archive & member;
 * }
 * @endcode
 */
#define PII_SERIALIZE_BASE(archive, Base) archive & PiiBaseClass<Base >(*this)


/**
 * A wrapper for serializing a base class. See @ref PII_SERIALIZE_BASE.
 *
 * @ingroup Serialization
 */
template <class T> struct PiiBaseClass
{
  /**
   * Create an instance of base class serializer with a reference to
   * the derived class.
   *
   * @code
   * template <class Archive>
   * void Derived::serialize(Archive& archive, const unsigned int)
   * {
   *   archive & PiiBaseClass<Base>(*this);
   * }
   * @endcode
   */
  PiiBaseClass(T& child) : derived(child) {}

  PII_SEPARATE_SAVE_LOAD_MEMBERS
  
  template <class Archive> void load(Archive& archive, const unsigned int /*version*/) const
  {
    unsigned int iBaseClassVersion(0);
    // Needed to increase archive version number to store the version
    // number of a base class.
    if (PiiSerializationTraits::ClassInfo<T>::boolValue &&
        archive.majorVersion() > 0)
      archive >> iBaseClassVersion;
    PiiSerialization::serialize(archive, derived, iBaseClassVersion);
  }

  template <class Archive> void save(Archive& archive, const unsigned int /*version*/) const
  {
    unsigned int iBaseClassVersion(PiiSerializationTraits::Version<T>::intValue);
    if (PiiSerializationTraits::ClassInfo<T>::boolValue)
      archive << iBaseClassVersion;
    PiiSerialization::serialize(archive, derived, iBaseClassVersion);
  }

//Compiling error found with following code
//  mutable T& derived;
  T& derived;
};

PII_SERIALIZATION_TRACKING_TEMPLATE(PiiBaseClass, false);
PII_SERIALIZATION_CLASSINFO_TEMPLATE(PiiBaseClass, false);

#endif //_PIIBASECLASS_H
