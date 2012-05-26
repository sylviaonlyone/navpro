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

#ifndef _PIISERIALIZATION_H
#define _PIISERIALIZATION_H

#include <PiiMetaTemplate.h>
#include "PiiSerializationGlobal.h"
#include "PiiMetaObject.h"

/**
 * @file
 *
 * Serialization functions and a couple of macros for separating the
 * template functions into save/load pairs.
 *
 * @ingroup Serialization
 */

/**
 * A macro that separates the serialize() member function template for
 * type @p T into a save/load member function pair.
 */
#define PII_SEPARATE_SAVE_LOAD_MEMBERS                                  \
template <class Archive> void serialize(Archive& archive, const unsigned int version) \
{                                                                       \
  ::PiiSerialization::separateMembers(archive, *this, version);         \
}

/**
 * Declarations for separated save/load functions. Put this inside
 * your class declaration if you use PII_SEPARATE_SAVE_LOAD_MEMBERS.
 */
#define PII_DECLARE_SAVE_LOAD_MEMBERS \
template <class Archive> void save(Archive& archive, const unsigned int version); \
template <class Archive> void load(Archive& archive, const unsigned int version);
  

/**
 * A macro that separates the serialize() function template for type
 * @p T into a save/load function pair.
 */
#define PII_SEPARATE_SAVE_LOAD_FUNCTIONS(T)                             \
namespace PiiSerialization {                                            \
template <class Archive> inline void serialize(Archive& archive, T& value, const unsigned int version) \
{                                                                       \
  separateFunctions(archive, value, version);                           \
}}

#define PII_ENUM(VALUE) PiiSerialization::enumWrapper(VALUE)

/**
 * A namespace that wraps global serialization functions and
 * serialization-related global symbols.
 *
 * @ingroup Serialization
 */
namespace PiiSerialization
{
  struct PII_SERIALIZATION_EXPORT Void
  {
    static Void instance;
  };

  /**
   * A proxy through which calls to serialization member functions are
   * passed. With intrusive serialization (serialize member function
   * as opposed to an external serializer), it is a good practice to
   * hide the serialization function(s) from the outer world by making
   * them private. The serialization system can access these members
   * if you also make this struct a friend of your class.
   *
   * @code
   * class MyClass
   * {
   *   friend struct PiiSerialization::Accessor;
   *   //... private serialization function(s) here ...
   * };
   * @endcode
   */
  struct Accessor
  {
    template <class T> static const PiiMetaObject* virtualMetaObject(const T* obj)
    {
      return obj->piiMetaObject();
    }
    
    template <class Archive, class T> static void serialize(Archive& archive, T& value, const unsigned int version)
    {
      value.serialize(archive, version);
    }
    template <class Archive, class T> static void save(Archive& archive, T& value, const unsigned int version)
    {
      value.save(archive, version);
    }
    template <class Archive, class T> static void load(Archive& archive, T& value, const unsigned int version)
    {
      value.load(archive, version);
    }
    
    template <class T> static T* voidConstruct()
    {
      return new T(PiiSerialization::Void::instance);
    }
  };

  /**
   * Default implementation of a serialization function. Override this
   * for your type to provide non-intrusive serialization facilities.
   */
  template <class Archive, class T> inline void serialize(Archive& archive, T& value, const unsigned int version)
  {
    Accessor::serialize(archive, value, version);
  }
  
  /// @internal Utility structure for separateMembers.
  struct MemberSaver
  {
    template <class Archive, class T> static void serialize(Archive& archive, T& value, const unsigned int version)
    {
      Accessor::save(archive, value, version);
    }
  };

  /// @internal Utility struct for separateMembers
  struct MemberLoader
  {
    template <class Archive, class T> static void serialize(Archive& archive, T& value, const unsigned int version)
    {
      Accessor::load(archive, value, version);
    }
  };

  /// @internal Utility structure for separateFunctions.
  struct FunctionSaver
  {
    template <class Archive, class T> static void serialize(Archive& archive, T& value, const unsigned int version)
    {
      save(archive, value, version);
    }
  };

  /// @internal Utility structure for separateFunctions.
  struct FunctionLoader
  {
    template <class Archive, class T> static void serialize(Archive& archive, T& value, const unsigned int version)
    {
      load(archive, value, version);
    }
  };
  
  /**
   * Separates the @p serialize member function template into a
   * save/load pair.
   */
  template <class Archive, class T> inline void separateMembers(Archive& archive, T& value, const unsigned int version)
  {
    Pii::If<Archive::InputArchive, MemberLoader, MemberSaver>::Type::serialize(archive, value, version);
  }

  /**
   * Separates the @p serialize function template (non-member) into a
   * save/load pair.
   */
  template <class Archive, class T> inline void separateFunctions(Archive& archive, T& value, const unsigned int version)
  {
    Pii::If<Archive::InputArchive, FunctionLoader, FunctionSaver>::Type::serialize(archive, value, version);
  }  

  template <class T> struct EnumWrapper
  {
    EnumWrapper(T& v) : value(v) {}

    PII_SEPARATE_SAVE_LOAD_MEMBERS

    template <class Archive> void save(Archive& archive, const unsigned int)
    {
      archive << static_cast<int>(value);
    }

    template <class Archive> void load(Archive& archive, const unsigned int)
    {
      int iValue;
      archive >> iValue;
      value = static_cast<T>(iValue);
    }
  
    T& value;
  };

  template <class T> EnumWrapper<T> enumWrapper(T& value) { return EnumWrapper<T>(value); }

  /**
   * A structure whose specializations determine how a specific type
   * is converted from void*. The default implementation uses @p
   * reinterpret_cast. This structure may need to be specialized if a
   * class is serialized through a base class pointer, and the base
   * class is not the first one in inheritance order.
   */
  template <class T> struct VoidCast
  {
    static T* toDerived(void* base) { return reinterpret_cast<T*>(base); }
    static T* toBase(void* derived);
  };
} // namespace PiiSerialization

PII_SERIALIZATION_TRACKING_TEMPLATE(PiiSerialization::EnumWrapper, false);
PII_SERIALIZATION_CLASSINFO_TEMPLATE(PiiSerialization::EnumWrapper, false);

#endif //_PIISERIALIZATION_H
