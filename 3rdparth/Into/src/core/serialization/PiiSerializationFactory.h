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

#ifndef _PIISERIALIZATIONFACTORY_H
#define _PIISERIALIZATIONFACTORY_H

#include <PiiMetaTemplate.h>
#include <PiiTemplateExport.h>
#include <QHash>
#include <QMutex>
#include <QString>
#include <PiiConstCharWrapper.h>
#include "PiiSerializationTraits.h"
#include "PiiSerializationGlobal.h"
#include "PiiSerialization.h"
#include <PiiGlobal.h>
#include <cstring>

/**
 * @file
 *
 * Object factory stuff for serializable classes.
 *
 * @ingroup Serialization
 */

/// @cond null
#define PII_FACTORY_TEMPLATE_CLASS(T, ARCHIVE) PiiSerializationFactory::Template<T,ARCHIVE>
#define PII_FACTORY_TEMPLATE_FUNC(T, ARCHIVE) PiiSerializationFactory::create<T,ARCHIVE>
#define PII_DECLARE_FACTORY_MAP(ARCHIVE) \
  PII_DECLARE_EXPORTED_FUNCTION_TEMPLATE(PiiSerializationFactory::MapType*, \
                                         PiiSerializationFactory::map<ARCHIVE >, (), \
                                         PII_BUILDING_SERIALIZATION)
#define PII_DEFINE_FACTORY_MAP(ARCHIVE) \
  PII_DEFINE_EXPORTED_FUNCTION_TEMPLATE(PiiSerializationFactory::MapType*, \
                                        PiiSerializationFactory::map<ARCHIVE >, ())
/// @endcond


#define PII_INSTANTIATE_ARCHIVE_FACTORY(T, ARCHIVE) \
  PII_DECLARE_EXPORTED_CLASS_TEMPLATE(class, PII_FACTORY_TEMPLATE_CLASS(T,ARCHIVE), 1); \
  PII_DECLARE_EXPORTED_FUNCTION_TEMPLATE(T*, PII_FACTORY_TEMPLATE_FUNC(T,ARCHIVE), (const char*,ARCHIVE&), 1); \
  PII_DEFINE_EXPORTED_CLASS_TEMPLATE(class, PII_FACTORY_TEMPLATE_CLASS(T,ARCHIVE)); \
  PII_DEFINE_EXPORTED_FUNCTION_TEMPLATE(T*, PII_FACTORY_TEMPLATE_FUNC(T,ARCHIVE), (const char*,ARCHIVE&)); \
  template <> PII_FACTORY_TEMPLATE_CLASS(T,ARCHIVE) PII_FACTORY_TEMPLATE_CLASS(T,ARCHIVE) \
    ::instance(PiiSerializationTraits::ClassName<T>::get())

/* Order matters. As weird as it may sound, the explicitly specialized
   instance will be created only if it is the last thing. Needless to
   say, this only happens with MSVC.
*/

/**
 * Instantiate a PiiSerializationFactory::Template for the given data
 * type. This registers the factory to the global factory map. This
 * macro can only be used in a .cc file.
 */
#define PII_INSTANTIATE_FACTORY(T) PII_INSTANTIATE_ARCHIVE_FACTORY(T, PiiSerialization::Void)


#define PII_DECLARE_ARCHIVE_FACTORY(T, ARCHIVE, BUILDING_LIB) \
  PII_DECLARE_EXPORTED_FUNCTION_TEMPLATE(T*, PII_FACTORY_TEMPLATE_FUNC(T,ARCHIVE), (const char*,ARCHIVE&), BUILDING_LIB)

/**
 * Declares an exported explicit instance of a factory function for
 * type @p T. This is needed if a serializable object is exported from
 * a dll.
 *
 * @code
 * #ifdef BUILD_MYLIB
 * #  define MY_EXPORT __declspec(dllexport)
 * #  define BUILDING_MYLIB 1
 * #else
 * #  define MY_EXPORT __declspec(dllimport)
 * #  define BUILDING_MYLIB 0
 * #endif
 *
 * // In MyType.h
 * struct MY_EXPORT MyType {};
 *
 * PII_DECLARE_FACTORY(MyType, BUILDING_MYLIB);
 * @endcode
 *
 * @param T the type to which a factory is to be declared
 *
 * @param BUILDING_MYLIB 1, if the factory object is declared in a
 * library that is currently being built, 0 otherwise.
 */
#define PII_DECLARE_FACTORY(T, BUILDING_LIB) PII_DECLARE_ARCHIVE_FACTORY(T, PiiSerialization::Void, BUILDING_LIB)

namespace PiiSerialization
{
  /// @internal
  template <class T> struct NullConstructor { static T* create() { return 0; } };
  /// @internal
  template <class T> struct NewConstructor { static T* create() { return new T; } };

  /**
   * Creates instances of @p T. This class is used by
   * PiiSerializationFactory when objects are created into heap during
   * deserialization. This class can be specialized for types that
   * need constructor parameters.
   *
   * @see PII_SERIALIZATION_CONSTRUCTOR
   * @see PII_SERIALIZATION_NORMAL_CONSTRUCTOR
   */
  template <class T, class Archive> struct Constructor
  {
    /**
     * Creates and returns an instance of @p T.
     *
     * @param archive the archive that is currently being read. The
     * default implementation ignores this parameter, but it can be
     * used to load constructor parameters before creating the class
     * instance.
     */
    static T* create(Archive& archive)
    {
      Q_UNUSED(archive);
      typedef typename Pii::IfClass<PiiSerializationTraits::IsAbstract<T>,
        NullConstructor<T>,
        NewConstructor<T> >::Type C;
      return C::create();
    }
  };

  /// @cond null
  template <class T, class Archive> struct ConstructorVoid
  {
    static T* create(Archive&)
    {
      return Accessor::voidConstruct<T>();
    }
  };

  template <class T, class Archive> struct Constructor0
  {
    static T* create(Archive&)
    {
      return new T;
    }
  };

  template <class T, class Archive, class P1> struct Constructor1
  {
    static T* create(Archive& archive)
    {
      P1 p1;
      archive >> p1;
      return new T(p1);
    }
  };

  template <class T, class Archive, class P1, class P2> struct Constructor2
  {
    static T* create(Archive& archive)
    {
      P1 p1;
      P2 p2;
      archive >> p1 >> p2;
      return new T(p1, p2);
    }
  };

  template <class T, class Archive, class P1, class P2, class P3> struct Constructor3
  {
    static T* create(Archive& archive)
    {
      P1 p1;
      P2 p2;
      P3 p3;
      archive >> p1 >> p2 >> p3;
      return new T(p1, p2, p3);
    }
  };
}

#define PII_REMOVE_PARENS_Void PII_REMOVE_PARENS_0
#define PII_FOR_Void PII_FOR_0
#define PII_ADD_COMMA_BEFORE(PARAM) , PARAM

#define PII_MAKE_TYPE_LIST(CNT, TYPES) \
  PII_FOR_N(PII_ADD_COMMA_BEFORE, CNT, TYPES)

/// @endcond

/**
 * Creates a constructor class for @a CLASS. The constructor created
 * with this macro will be used for all archive types. A constructor
 * created with this macro reads the constructor parameters from an
 * archive in the order they are specified, and passes them to the
 * constructor.
 *
 * @param CLASS the name of the type that needs a custom constructor
 *
 * @param CNT the number of constructor parameters (or Void)
 *
 * @param TYPES the types of constructor parameters
 *
 * @code
 * // No default constructor but
 * // MyClass::MyClass(int, double)
 * PII_SERIALIZATION_CONSTRUCTOR(MyClass, 2, (int, double));
 * @endcode
 *
 * "Void" as the parameter count has a special meaning: it creates a
 * constructor that passes PiiSerialization::Void as a constructor
 * parameter.
 *
 * @code
 * // Use MyClass::MyClass(PiiSerialization::Void)
 * PII_SERIALIZATION_CONSTRUCTOR(MyClass, Void, ());
 * @endcode
 */
#define PII_SERIALIZATION_CONSTRUCTOR(CLASS, CNT, TYPES)                \
  namespace PiiSerialization {                                          \
    template <class Archive> struct Constructor<CLASS,Archive> :        \
      Constructor ## CNT < CLASS, Archive PII_MAKE_TYPE_LIST(CNT, TYPES) > {}; \
  }

/**
 * Creates a constructor for @a CLASS that will be used when no
 * archive-specific constructor is available. Using this macro makes
 * it possible to use a different constructor when the serialization
 * factory is used directly and not through the deserialization
 * mechanism.
 *
 * @code
 * // Use MyClass::MyClass() when an instance is created using
 * // PiiSerializationFactory::create() directly.
 * PII_SERIALIZATION_NORMAL_CONSTRUCTOR(MyClass, 0, ());
 * @endcode
 */
#define PII_SERIALIZATION_NORMAL_CONSTRUCTOR(CLASS, CNT, TYPES)         \
  namespace PiiSerialization {                                          \
    template <> struct Constructor<CLASS,PiiSerialization::Void> :      \
      Constructor ## CNT < CLASS, PiiSerialization::Void PII_MAKE_TYPE_LIST(CNT, TYPES) > {}; \
  }

/**
 * A factory that creates objects based on their names. When
 * deserialized, instances of objects are created by calling the
 * static #create() function of this class.
 *
 * @ingroup Serialization
 */
class PII_SERIALIZATION_EXPORT PiiSerializationFactory
{
public:
  /**
   * Returns a factory for the named class in an archive-specific
   * factory map. If an instance of a subclass of
   * PiiSerializationFactory that used @p className in its constructor
   * has been created, this function returns the instance. Otherwise
   * it returns 0.
   */
  template <class Archive> static PiiSerializationFactory* factory(const char* className)
  {
    return map<Archive>()->value(className);
  }
  
  /**
   * @overload
   *
   * This function returns a factory object from the default factory
   * map.
   */
  static PiiSerializationFactory* factory(const char* className)
  {
    return map<PiiSerialization::Void>()->value(className);
  }

  /**
   * Returns a list of all class names registered to the default
   * factory.
   */
  static QList<const char*> keys()
  {
    return keys(map<PiiSerialization::Void>());
  }

  /**
   * Returns a list of all class names registered to an
   * archive-specific factory.
   */
  template <class Archive> static QList<const char*> keys()
  {
    return keys(map<Archive>());
  }
  
  /// @internal
  template <class T, class Archive> static T* create(const char* className, Archive& archive);
  // NOTE: the implementation of this function is in
  // PiiDynamicTypeFunctions.h due to restrictions on declaration
  // order.
  
  /**
   * Creates an instance of the named object. If @p T has a virtual
   * metaobject function, a factory object will be fetched from the
   * static factory map and used in creating the object instance. For
   * other types, new @p T is returned. If the class cannot be
   * created, 0 will be returned.
   */
  template <class T> static inline T* create(const char* className)
  {
    return create<T>(className, PiiSerialization::Void::instance);
  }
  
  /**
   * Creates an instance of @p T. Returns <tt>new T</tt> for
   * instantiable types and 0 for abstract ones.
   *
   * @see PiiSerializationTraits::IsAbstract
   */
  template <class T, class Archive> static T* create(Archive& archive)
  {
    return PiiSerialization::Constructor<T,Archive>::create(archive);
  }
  
  /**
   * Subclasses override this function to create an instance of the
   * class this factory object represents. The newly created object is
   * returned as a void pointer.
   *
   * @param archive a typeless pointer to an archive instance that can
   * be used to read construct data for the object instance to be
   * created.
   */
  virtual void* create(void* archive) = 0;

  /**
   * Calls #create(void*) with a null pointer. This is safe with the
   * default factory that ignores the archive.
   */
  inline void* create() { return create(0); }

  virtual ~PiiSerializationFactory();

  template <class T, class Archive> class Template;
  
protected:
  typedef QHash<PiiConstCharWrapper, PiiSerializationFactory*> MapType;
  template <class Archive> static MapType* map();
  // Removes this from the given factory map.
  void removeFrom(MapType* map);

private:
  static QList<const char*> keys(MapType* map);
};

template <class Archive> PiiSerializationFactory::MapType* PiiSerializationFactory::map()
{
  static MapType map = MapType();
  return &map;
}

PII_DECLARE_FACTORY_MAP(PiiSerialization::Void);

/**
 * Default implementation of serialization factory for tracked types. 
 * Static instances of this class are used in inserting factories to
 * the global factory map.
 *
 * @ingroup Serialization
 */
template <class T, class Archive> class PiiSerializationFactory::Template : public PiiSerializationFactory
{
public:
  /**
   * Create a new object factory instance. This factory will create
   * objects of type @p T. The type is identified by its class name as
   * defined by PiiSerializationTraits::ClassName, and must be
   * application-wide unique.
   */
  Template(const char* name)
  {
    this->map<Archive>()->insert(name, this);
  }

  ~Template()
  {
    removeFrom(this->map<Archive>());
  };
  
  /**
   * Returns a new @p T.
   */
  virtual void* create(void* archive) { return PiiSerializationFactory::create<T>(*reinterpret_cast<Archive*>(archive)); }

  static Template instance;
};

#endif //_PIISERIALIZATIONFACTORY_H
