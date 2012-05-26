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

#ifndef _PIIVIRTUALMETAOBJECT_H
#define _PIIVIRTUALMETAOBJECT_H

/**
 * @file
 *
 * Macros for dealing with virtual meta object functions.
 *
 * @ingroup Serialization
 */

/**
 * Declare @p CLASS_NAME as a dynamic type. This overrides the default
 * isDynamicType() function to return @p true for @p CLASS_NAME and all
 * derived types.
 */
#define PII_SERIALIZATION_DYNAMIC(CLASS_NAME) \
namespace PiiSerialization { \
  inline bool isDynamicType(const CLASS_NAME*) { return true; } \
}

/**
 * Declare all instances of @p CLASS_NAME template as a dynamic types. 
 * This overrides the default isDynamicType() function to return @p
 * true for @p CLASS_NAME instances and all derived types.
 */
#define PII_SERIALIZATION_DYNAMIC_TEMPLATE(CLASS_NAME) \
  namespace PiiSerialization { \
  template <class T> inline bool isDynamicType(const CLASS_NAME<T>*) { return true; } \
}

/**
 * A macro that tells the serialization library that @p CLASS_NAME has
 * a virtual piiMetaObject() function. This overrides the default
 * implementation of metaObjectPointer() for the given type. It
 * also overrideds the isDynamicType() function to return @p true.
 */
#define PII_SERIALIZATION_VIRTUAL_METAOBJECT(CLASS_NAME) \
PII_SERIALIZATION_DYNAMIC(CLASS_NAME) \
namespace PiiSerialization {                            \
  inline const PiiMetaObject* metaObjectPointer(const CLASS_NAME* obj) \
  {                                                     \
    return Accessor::virtualMetaObject(obj);            \
  }}

/**
 * A macro that tells the serialization library that all instances of
 * the class template @p CLASS_NAME have a virtual piiMetaObject()
 * function. This overrides the default implementation of
 * metaObjectPointer() for instances of the given template type. It
 * also overrides the isDynamicType() function to return @p true.
 */
#define PII_SERIALIZATION_VIRTUAL_METAOBJECT_TEMPLATE(CLASS_NAME) \
PII_SERIALIZATION_DYNAMIC_TEMPLATE(CLASS_NAME)                    \
namespace PiiSerialization {                                      \
  template <class T> inline const PiiMetaObject* metaObjectPointer(const CLASS_NAME<T>* obj) \
  {                                                               \
    return Accessor::virtualMetaObject(obj);                      \
  }}

/**
 * Virtual meta object function declaration. Place this into the
 * @p private section of your class declaration.
 */
#define PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION virtual const PiiMetaObject* piiMetaObject() const

/**
 * Defines meta object function outside of a class declaration.
 */
#define PII_DEFINE_VIRTUAL_METAOBJECT_FUNCTION(CLASS_NAME) \
const PiiMetaObject* CLASS_NAME::piiMetaObject() const     \
{                                                          \
  static PiiMetaObject metaObject(PiiSerialization::defaultMetaObject(*this)); \
  return &metaObject;                                      \
} namespace PiiDummy {}

/**
 * Defines meta object function outside of a class template declaration.
 */
#define PII_DEFINE_VIRTUAL_METAOBJECT_FUNCTION_TEMPLATE(CLASS_NAME) \
template <class T> const PiiMetaObject* CLASS_NAME<T>::piiMetaObject() const \
{ \
  static PiiMetaObject metaObject(PiiSerialization::defaultMetaObject(*this)); \
  return &metaObject; \
} namespace PiiDummy {}

/**
 * Declare a specialized piiMetaObject() function for
 * <tt>CLASS_NAME::Template<TYPE></tt>.
 */
#define PII_DECLARE_SPECIALIZED_VIRTUAL_METAOBJECT_FUNCTION(CLASS_NAME, TYPE, EXPORT) \
template <> EXPORT const PiiMetaObject* CLASS_NAME::Template<TYPE >::piiMetaObject() const

/**
 * Define meta object function outside of a class template
 * declaration. This version defines a specialization <tt>template <>
 * CLASS_NAME::Template<TYPE>::piiMetaObject()</tt> so that it returns
 * "CLASS_NAME<TYPE_NAME>" as the class name.
 */
#define PII_DEFINE_SPECIALIZED_VIRTUAL_METAOBJECT_FUNCTION_NAMED(CLASS_NAME, TYPE, TYPE_NAME) \
template <> const PiiMetaObject* CLASS_NAME::Template<TYPE >::piiMetaObject() const \
{ \
  static PiiMetaObject metaObject(PII_STRINGIZE(CLASS_NAME<TYPE_NAME>), \
                                  (unsigned int)PiiSerializationTraits::Version<CLASS_NAME::Template<TYPE > >::intValue, \
                                  PiiSerializationTraits::ClassInfo<CLASS_NAME::Template<TYPE > >::boolValue); \
  return &metaObject; \
} namespace PiiDummy {}

/**
 * Same as @ref
 * PII_DEFINE_SPECIALIZED_VIRTUAL_METAOBJECT_FUNCTION_NAMED, but uses
 * @p CLASS_NAME<TYPE> as the name of the type.
 */
#define PII_DEFINE_SPECIALIZED_VIRTUAL_METAOBJECT_FUNCTION(CLASS_NAME, TYPE) \
  PII_DEFINE_SPECIALIZED_VIRTUAL_METAOBJECT_FUNCTION_NAMED(CLASS_NAME, TYPE, TYPE)

/**
 * Declare and define a virtual piiMetaObject() function for a class. 
 * This macro is used within a class declaration. It returs a pointer
 * to an instance of PiiDefaultMetaObject instantiated for the type of
 * this object.
 *
 * @code
 * class MyClass
 * {
 *   PII_VIRTUAL_METAOBJECT_FUNCTION
 * };
 *
 * PII_SERIALIZATION_VIRTUAL_METAOBJECT(MyClass)
 * @endcode
 */
#define PII_VIRTUAL_METAOBJECT_FUNCTION \
PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION \
{                                       \
  static PiiMetaObject metaObject(PiiSerialization::defaultMetaObject(*this)); \
  return &metaObject;                   \
}

#endif //_PIIVIRTUALMETAOBJECT_H
