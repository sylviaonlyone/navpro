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

#ifndef _PIIDYNAMICTYPEFUNCTIONS_H
#define _PIIDYNAMICTYPEFUNCTIONS_H

#include "PiiMetaObject.h"
#include "PiiSerializationFactory.h"
#include "PiiSerializer.h"

/**
 * @file
 *
 * This file contains functions for getting meta information,
 * serializers, and class factories for serializable types. The
 * functions are separated from context to ensure correct declaration
 * order of overloaded functions. If you include this file directly,
 * please ensure that all metaObjectPointer() and isDynamicType()
 * overloads have been declared beforehand.
 *
 * @ingroup Serialization
 */

namespace PiiSerialization
{
  /**
   * This function is used to check if the real type of the class is
   * known at compile time or not. If a class can be serialized
   * through a base class pointer, its type cannot be known at compile
   * time. Thus, its type is @e dynamic. For such, types an overloaded
   * version of this function that returns @p true must be provided. 
   * Typically, such a type will also have an overload for @ref
   * PiiSerialization::metaObjectPointer(). The default
   * implementation returns @p false.
   *
   * @see serialization_serializers
   */
  inline bool isDynamicType(const void*) { return false; }

  /**
   * Default implementation of the meta object getter function. This
   * function returns a null pointer. If a serializable type has a
   * non-default meta object getter (such as a virtual function), one
   * needs to provide an overloaded version of this function.
   *
   * @relates PiiMetaObject
   */
  inline const PiiMetaObject* metaObjectPointer(const void*) { return 0; }

  /**
   * Get a meta object for the given object. This function calls the
   * metaObjectPointer() function, which needs to be overloaded if
   * a serializable type needs a non-default meta object.
   *
   * @see PII_SERIALIZATION_VIRTUAL_METAOBJECT
   *
   * @relates PiiMetaObject
   */
  template <class T> inline PiiMetaObject piiMetaObject(const T& obj)
  {
    /* The following trickery is needed to tweak C++'s resolution
     * order. There seems to be no way to do this statically with
     * templates. Hope the compiler is wise enough to optimize out the
     * unnecessary comparison in the beginning.
     *
     * If a superclass of type T has a virtual metaobject function, we
     * want it to be used by default. The problem is that the
     * overload resolution on C++ prefers template over base class.
     * Suppose we have a metaobject function template f(T) and a
     * specialization f(A). We want f(A) to be called for types
     * derived from A, but C++ would select the template as it matches
     * the derived type exactly.
     *
     * The metaObjectPointer() function is not a template. Therefore,
     * base classes are considered in overload resolution. The default
     * version has void* as the argument type, and will always come
     * last in resolution order. The problem is that void* has no
     * type. Thus, metaObjectPointer() cannot create a default
     * metaobject based on the type. This could be solved by adding
     * the metaObjectPointer() function a template parameter as its
     * second argument and use the first one for resolution only. But
     * this would force all metaObjectPointer() functions to be
     * templates, which is less than elegant.
     *
     * Therefore, we must first find out if there is an overloaded
     * metaObjectPointer() function. If such a function is
     * resolved, a non-null pointer will be returned. If a null
     * pointer is returned, a default metaobject will be constructed
     * based on the type T.
     */
    const PiiMetaObject* pMetaObject = metaObjectPointer(&obj);
    if (pMetaObject == 0)
      return defaultMetaObject(obj);
    return *pMetaObject;
  }
}

template <class T, class Archive> T* PiiSerializationFactory::create(const char* className, Archive& archive)
{
  if (PiiSerialization::isDynamicType((T*)0))
    {
      // Try archive-specific factory first
      PiiSerializationFactory* pFactory = factory<Archive>(className);
      if (pFactory == 0)
        {
          // If this is already the default factory, fail
          if (Pii::IsSame<Archive, PiiSerialization::Void>::boolValue ||
              // Otherwise try the default factory
              (pFactory = factory<PiiSerialization::Void>(className)) == 0)
            return 0;
        }
      return reinterpret_cast<T*>(pFactory->create(&archive));
    }
  return create<T>(archive);
}

template <class Archive>
template <class T> bool PiiSerializer<Archive>::serialize(const char* className,
                                                          Archive& archive,
                                                          T& value,
                                                          const unsigned int version)
{
  if (PiiSerialization::isDynamicType((T*)0))
    {
      const PiiSerializer* pSerializer = serializer(className);
      if (pSerializer == 0)
        return false;
      pSerializer->serialize(archive, (void*)&value, version);
    }
  else
    PiiSerialization::serialize(archive, const_cast<typename Pii::ToNonConst<T>::Type &>(value), version);
  
  return true;
}

#endif //_PIIDYNAMICTYPEFUNCTIONS_H
