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

#ifndef _PIISERIALIZER_H
#define _PIISERIALIZER_H

#include <QHash>
#include <PiiConstCharWrapper.h>
#include <cstring>
#include "PiiSerialization.h"
#include "PiiTemplateExport.h"
#include <PiiGlobal.h>

/**
 * @file
 *
 * Macros and class declarations for serializers.
 *
 * @ingroup Serialization
 */

/*
 * Instantiate a PiiSerializerTemplate for the given archive and data
 * type. This registers the serializer to the global serializer map of
 * the given archive type.
 */
#define PII_INSTANTIATE_SERIALIZER(ARCHIVE, T) \
  PII_DECLARE_EXPORTED_CLASS_TEMPLATE2(class, PiiSerializerTemplate<ARCHIVE, T >, 1); \
  PII_DEFINE_EXPORTED_CLASS_TEMPLATE2(class, PiiSerializerTemplate<ARCHIVE, T >); \
  template <> PiiSerializerTemplate<ARCHIVE, T > PiiSerializerTemplate<ARCHIVE, T >::instance(PiiSerializationTraits::ClassName<T>::get())

// Note that order matters (with MSVC). See PiiSerializationFactory.

/*
 * Declares an explicit serializer instantiation for the given archive
 * type. One needs to call this macro for each archive type defined in
 * the serialization dll (in a header file). This ensures you'll only
 * have one instance of the serializer registry in your application. 
 * If you are not going to provide archive types from shared
 * libraries, you don't need to use this macro.
 *
 * @see PII_DEFINE_SERIALIZER
 */
#define PII_DECLARE_SERIALIZER(ARCHIVE) PII_DECLARE_EXPORTED_CLASS_TEMPLATE(class, PiiSerializer<ARCHIVE >, PII_BUILDING_SERIALIZATION)

/*
 * Creates an explicit instance of a serializer for the given archive
 * type. This macro needs to be used once in a .cc file to instantiate
 * the PiiSerializer class template for a new archive type.
 *
 * @see PII_DECLARE_SERIALIZER
 */
#define PII_DEFINE_SERIALIZER(ARCHIVE) PII_DEFINE_EXPORTED_CLASS_TEMPLATE(class, PiiSerializer<ARCHIVE >)

/**
 * Serializer is an object that takes care of serializing objects. 
 * Each archive type has a static registry of serializers stored in a
 * hash map. The reason serialization goes through PiiSerializer is
 * that this allows one to conveniently create a name-based dictionary
 * of serializers with virtual serialization functions.
 *
 * @ingroup Serialization
 */
template <class Archive> class PiiSerializer
{
public:
  /**
   * Returns the serializer associated with @a className, or 0 if
   * no serializer is found.
   */
  static const PiiSerializer* serializer(const char* className)
  {
    // Return serializer if found in the map, 0 otherwise
    return map()->value(className, 0);
  }

  /**
   * Returns a list of type names for all registered serializers.
   */
  static QList<const char*> keys();

  /**
   * Seializes an object of type @p T. If @p T is a dynamic type, this
   * function fetches a serializer for the type denoted by @a
   * className. If no serializer is found, returns @p false. 
   * Otherwise, it calls the virtual serialize() function of the found
   * serializer and returns @p true. If the type is not dynamic,
   * PiiSerialization::serialize() will be called directly.
   */
  template <class T> static bool serialize(const char* className,
                                           Archive& archive,
                                           T& value,
                                           const unsigned int version);

  /**
   * Subclasses override this function to serialize any custom type.
   *
   * @param archive the archive to store the data into
   *
   * @param value a pointer to the object to be stored. Subclasses
   * should use @p reinterpret_cast to convert the pointer to the
   * correct type.
   *
   * @param version the version of the class
   */
  virtual void serialize(Archive& archive, void* value, const unsigned int version) const = 0;
  
  /**
   * Removes @p this from the archive's serializer map.
   */
  virtual ~PiiSerializer();

  /**
   * The type of the serializer map. Serializers are keyed with the
   * class name.
   */
  typedef QHash<PiiConstCharWrapper, PiiSerializer<Archive>*> MapType;

protected:
  /**
   * Create a new serializer and insert it to the static serializer
   * map. The class is identified by the given name, which must be
   * application-wide unique.
   */
  PiiSerializer(const char* name);

private:
  /**
   * Get a pointer to the serializer map.
   */
  static MapType* map();

  /**
   * Call virtual serialization function of a registered serializer.
   */
  template <class T> static bool doSerialize(const char* className, Archive& archive, T& value, const unsigned int version)
  {
    const PiiSerializer* serializer = PiiSerializer::serializer(className);
    if (serializer == 0)
      return false;
    serializer->serialize(archive, (void*)&value, version);
    return true;
  }
};

template <class Archive> PiiSerializer<Archive>::PiiSerializer(const char* name)
{
  map()->insert(name, this);
}

template <class Archive> PiiSerializer<Archive>::~PiiSerializer()
{
  QMutableHashIterator<PiiConstCharWrapper, PiiSerializer<Archive>* > i(*map());
  while (i.hasNext())
    {
      i.next();
      if (i.value() == this)
        {
          i.remove();
          return;
        }
    }
}

template <class T> typename PiiSerializer<T>::MapType* PiiSerializer<T>::map()
{
  static MapType map = MapType();
  return &map;
}

template <class T> QList<const char*> PiiSerializer<T>::keys()
{
  QList<const char*> lstResult;
  for (typename MapType::const_iterator i = map()->constBegin();
       i != map()->constEnd();
       ++i)
    lstResult << i.key().ptr;
  return lstResult;
}

/**
 * A template class that is statically instantiated to add
 * serializable classes to an archive's serializer registry.
 *
 * @ingroup Serialization
 */
template <class Archive, class T> class PiiSerializerTemplate :
  public PiiSerializer<Archive>
{
public:
  /**
   * Create a new serializer instance that takes the class name from
   * the PiiSerializationTraits::ClassName structure. The name is used
   * as the class ID. Thus, it must be globally unique.
   */
  PiiSerializerTemplate(const char* name) : PiiSerializer<Archive>(name) {}
  virtual void serialize(Archive& archive, void* value, const unsigned int version) const
  {
    PiiSerialization::serialize(archive, *reinterpret_cast<T*>(value), version);
  }

  static PiiSerializerTemplate instance;
};

#endif //_PIISERIALIZER_H
