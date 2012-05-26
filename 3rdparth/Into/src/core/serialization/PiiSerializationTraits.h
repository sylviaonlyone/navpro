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

#ifndef _PIISERIALIZATIONTRAITS_H
#define _PIISERIALIZATIONTRAITS_H

/**
 * @file
 *
 * Traits for serializable classes and macros for altering them.
 *
 * @ingroup Serialization
 */

#include <PiiPreprocessor.h>
#include <PiiMetaTemplate.h>
#include <PiiTypeTraits.h>
#include <PiiSmartPtr.h>
#include "PiiSerializationGlobal.h"

class QString;

/**
 * Mark @p CLASS_NAME as an abstract type. This prevents the
 * serialization library from trying to instantiate the class.
 *
 * @code
 * class MyClass;
 * PII_SERIALIZATION_ABSTRACT(MyClass);
 * @endcode
 */
#define PII_SERIALIZATION_ABSTRACT(CLASS_NAME) \
  namespace PiiSerializationTraits { template <> struct IsAbstract<CLASS_NAME > : Pii::True {}; }

/**
 * Mark all instances of the class template @p CLASS_NAME as abstract
 * classes.
 *
 * @code
 * template <class T> class MyClass;
 * PII_SERIALIZATION_ABSTRACT_TEMPLATE(MyClass);
 * @endcode
 */
#define PII_SERIALIZATION_ABSTRACT_TEMPLATE(CLASS_NAME) \
  namespace PiiSerializationTraits { template <class T> struct IsAbstract<CLASS_NAME<T> > : Pii::True {}; }

/**
 * Set object tracking for the given class to @p on (true/false).
 * Non-tracked objects will be serialized many times if many
 * pointers/references to them are serialized. Use
 * #PII_SERIALIZATION_TRACKING_TEMPLATE() if @p CLASS_NAME is a
 * template class.
 *
 * @code
 * class MyClass;
 * // Disable tracking of MyClass
 * PII_SERIALIZATION_TRACKING(MyClass, false);
 * @endcode
 */
#define PII_SERIALIZATION_TRACKING(CLASS_NAME, ON) \
  namespace PiiSerializationTraits { template <> struct Tracking<CLASS_NAME > { enum { boolValue = ON }; }; }

/**
 * Set object tracking for the given class template to @p on
 * (true/false).
 *
 * @code
 * template <class T> class MyClass;
 * // Disable tracking of all MyClass template instances
 * PII_SERIALIZATION_TRACKING_TEMPLATE(MyClass, false);
 * @endcode
 */
#define PII_SERIALIZATION_TRACKING_TEMPLATE(CLASS_NAME, ON) \
  namespace PiiSerializationTraits { template <class T> struct Tracking<CLASS_NAME<T> > { enum { boolValue = ON }; }; }

/**
 * Enable/disable storing of class information for the given type. Use
 * #PII_SERIALIZATION_CLASSINFO_TEMPLATE() if @p CLASS_NAME is a
 * template class.
 *
 * @code
 * class MyClass;
 * // Do not save class info with MyClass
 * PII_SERIALIZATION_CLASSINFO(MyClass, false);
 * @endcode
 */
#define PII_SERIALIZATION_CLASSINFO(CLASS_NAME, ON) \
  namespace PiiSerializationTraits { template <> struct ClassInfo<CLASS_NAME > { enum { boolValue = ON }; }; }

/**
 * Enable/disable storing of class information for all instances of
 * the given template type.
 *
 * @code
 * template <class T> class MyClass;
 * // Do not save class info with any MyClass template instance
 * PII_SERIALIZATION_CLASSINFO_TEMPLATE(MyClass, false);
 * @endcode
 */
#define PII_SERIALIZATION_CLASSINFO_TEMPLATE(CLASS_NAME, ON) \
  namespace PiiSerializationTraits { template <class T> struct ClassInfo<CLASS_NAME<T> > { enum { boolValue = ON }; }; }

/**
 * Set object version for the given class (int). Use
 * #PII_SERIALIZATION_VERSION_TEMPLATE() if @p CLASS_NAME is a template
 * class.
 *
 * @code
 * class MyClass;
 * // Set the current version number of MyClass to 2
 * PII_SERIALIZATION_VERSION(MyClass, 2);
 * @endcode
 */
#define PII_SERIALIZATION_VERSION(CLASS_NAME, VERSION) \
  namespace PiiSerializationTraits { template <> struct Version<CLASS_NAME > { enum { intValue = VERSION }; }; }

/**
 * Set object version for all instances of the given class template
 * (int).
 *
 * @code
 * template <class T> class MyClass;
 * // Set the current version number of all MyClass template instances to 2
 * PII_SERIALIZATION_VERSION_TEMPLATE(MyClass, 2);
 * @endcode
 */
#define PII_SERIALIZATION_VERSION_TEMPLATE(CLASS_NAME, VERSION) \
  namespace PiiSerializationTraits { template <class T> struct Version<CLASS_NAME<T> > { enum { intValue = VERSION }; }; }

/**
 * Set object name for the given class. This macro is useful if the
 * preprocessor cannot automatically create a textual representation
 * of a type name or you just want a custom name for the class. Commas
 * in type names confuse cpp. Here's how to work around:
 *
 * @code
 * template <class T, class U> class MyClass;
 * typedef MyClass<int,int> MyIntIntClass;
 * PII_SERIALIZATION_NAME_CUSTOM(MyIntIntClass, "MyClass<int,int>");
 * @endcode
 */
#define PII_SERIALIZATION_NAME_CUSTOM(CLASS_NAME, NAME_STR) \
  namespace PiiSerializationTraits { template <> struct ClassName<CLASS_NAME > { static const char* get() { return NAME_STR; } }; }

/**
 * Set default name for the given class. The name for MyClass becomes
 * "MyClass" etc. Template classes with multiple template parameters
 * must be handled with @ref PII_SERIALIZATION_NAME_CUSTOM because the
 * preprocessor cannot handle commas in macro arguments.
 *
 * @code
 * class MyClass;
 * PII_SERIALIZATION_NAME(MyClass);
 * @endcode
 */
#define PII_SERIALIZATION_NAME(CLASS_NAME) PII_SERIALIZATION_NAME_CUSTOM(CLASS_NAME, PII_STRINGIZE(CLASS_NAME))

#define PII_SERIALIZATION_NAME_TEMPLATE(CLASS_NAME) \
  namespace PiiSerializationTraits \
  { \
    template <class T> struct ClassName<CLASS_NAME<T> > \
    { \
      static const char* get() \
      { \
        static PiiSmartPtr<char[]> pName(createTemplateName(CLASS_NAME, ClassName<T>::get())); \
        return static_cast<char*>(pName); \
      } \
    }; \
  }

/**
 * A namespace that contains classes for specifying traits for
 * serializable types. Traits are specific to a type; they do not
 * apply to derived types. For example, the version number of a base
 * class has no effect on its children. The same applies to the
 * ClassInfo and Tracking traits: if you want the class info to be
 * saved or pointers to be tracked, the ClassInfo/Tracking trait must
 * be enabled for the exact type used in serialization. If you disable
 * ClassInfo or Tracking for type A, version number of B will be
 * stored and pointers to it tracked, even if B was a subclass of A.
 *
 * @note Traits (except for Version) cannot be changed between save
 * and load. Doing so will invalidate your archives.
 *
 * @ingroup Serialization
 */
namespace PiiSerializationTraits
{
  /**
   * A type trait for checking the primitiveness of a type. In the
   * serialization library, QString is treated as a primitive type.
   */
  template <class T> struct IsPrimitive : Pii::IsPrimitive<T> {};
  /**
   * A type trait for checking the primitiveness of a type. In the
   * serialization library, QString is treated as a primitive type.
   */
  template <> struct IsPrimitive<QString> : Pii::True {};

  /**
   * A type trait for checking the abstractness of a type. An explicit
   * specializations must be provided for types that cannot be
   * instantiated.
   */
  template <class T> struct IsAbstract : Pii::False {};
  template <class T> struct IsAbstract<const T> : IsAbstract<T> {};
  
  /**
   * Pointer tracking trait. Tracking is enabled by default for all
   * complex types. Pointers to primitive types will also be tracked. 
   * If tracking is disabled, memory addresses will not be tracked and
   * the same object may be serialized many times. Use the @ref
   * PII_SERIALIZATION_TRACKING macro to conveniently set the tracking
   * trait.
   */
  template <class T> struct Tracking : Pii::True {};
  template <class T> struct Tracking<const T> { enum { boolValue = Tracking<T>::boolValue }; };

  /**
   * Class information trait. Class information (currently only
   * version number) is stored by default. Set this trait to @p false
   * with @ref PII_SERIALIZATION_CLASSINFO to disable storing of the
   * version number.
   */
  template <class T> struct ClassInfo : Pii::True {};
  template <class T> struct ClassInfo<const T> { enum { boolValue = ClassInfo<T>::boolValue }; };

  /**
   * Class version trait. The default version number is zero. Change
   * the default with the @ref PII_SERIALIZATION_VERSION macro.
   */
  template <class T> struct Version { enum { intValue = 0 }; };
  template <class T> struct Version<const T> { enum { intValue = Version<T>::intValue }; };

  /**
   * Class name trait. The default instantation returns an empty
   * string. This trait must be defined if the class will be
   * serialized through a base class pointer. Set class name with the
   * @ref PII_SERIALIZATION_NAME macro.
   */
  template <class T> struct ClassName { static const char* get() { return ""; } };
  template <class T> struct ClassName<const T> { static const char* get() { return ClassName<T>::get(); } };

#define PII_NAME_TYPE(TYPE) \
  template <> struct ClassName<TYPE> { static const char* get() { return PII_STRINGIZE(TYPE); } };
  
  PII_NAME_TYPE(char);
  PII_NAME_TYPE(short);
  PII_NAME_TYPE(int);
  PII_NAME_TYPE(long);
  PII_NAME_TYPE(long long);
  PII_NAME_TYPE(float);
  PII_NAME_TYPE(double);
  PII_NAME_TYPE(long double);
  PII_NAME_TYPE(bool);

#undef PII_NAME_TYPE

#define PII_NAME_TYPE(TYPE) \
  template <> struct ClassName<unsigned TYPE> { static const char* get() { return "unsigned "PII_STRINGIZE(TYPE); } };

  PII_NAME_TYPE(char);
  PII_NAME_TYPE(short);
  PII_NAME_TYPE(int);
  PII_NAME_TYPE(long);
  PII_NAME_TYPE(long long);
  
#undef PII_NAME_TYPE

  /// @internal
  PII_SERIALIZATION_EXPORT char* createTemplateName(const char* className, const char* typeName);
}

#endif //_PIISERIALIZATIONTRAITS_H
