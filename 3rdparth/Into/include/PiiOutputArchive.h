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

#ifndef _PIIOUTPUTARCHIVE_H
#define _PIIOUTPUTARCHIVE_H

#include <QHash>
#include <PiiMetaTemplate.h>
#include <PiiTypeTraits.h>
#include "PiiSerializationTraits.h"
#include "PiiSerializationException.h"
#include "PiiMetaObject.h"
#include "PiiSerializer.h"
#include "PiiTrackedPointerHolder.h"
#include "PiiDynamicTypeFunctions.h"

/**
 * A base class for output archive implementations. This class
 * provides functions for saving pointers.
 *
 * @ingroup Serialization
 */
template <class Archive> class PiiOutputArchive
{
  struct PointerSaver;
  struct ObjectSaver;
  struct TrackedPointerSaver;
  struct TrackedObjectSaver;
  struct PrimitivePointerSaver;
  struct ComplexPointerSaver;

  friend struct PointerSaver;
  friend struct ObjectSaver;
  friend struct TrackedPointerSaver;
  friend struct TrackedObjectSaver;
  friend struct PrimitivePointerSaver;
  friend struct ComplexPointerSaver;

public:
  /**
   * This enumeration is used as a static constant for compile-time
   * checking of input/output.
   */
  enum { InputArchive = false, OutputArchive = true };

  ~PiiOutputArchive()
  {
    qDeleteAll(_pointerMap);
  }
  
  /**
   * A design pattern for casting the type of this archive to that of
   * the template parameter (i.e. the most derived class). The pattern
   * is known as "Curiously Recurring Template Pattern" (CRTP), in
   * which the most derived class uses itself as the template
   * parameter for its parents.
   */
  Archive* self() { return static_cast<Archive*>(this); }

  /**
   * Writes an array of @a size elements to the archive.
   */
  template <class T> void writeArray(const T* ptr, unsigned int size)
  {
    *self() << size;
    if (size != 0)
      self()->writeRawData(ptr, sizeof(T)*size);
  }

  /**
   * This operator is defined for both input and output archives,
   * which makes it possible to serialize and deserialize data with a
   * single function. The output archive converts the operator to
   * operator<<.
   */
  template <class T> Archive& operator& (const T& obj) { return *self() << obj; }
  /// @overload
  template <class T> Archive& operator& (T& obj) { return *self() << const_cast<const T&>(obj); }

  /**
   * Default implementation for the << operator. If @p Archive does
   * not have any overrides, this will be called by operator&. This
   * function calls save(value). You may provide any overrides
   * applicable to your archive type. At the minimum, an override for
   * all primitive types must be provided.
   */
  template <class T> Archive& operator<< (T& value)
  {
    save(const_cast<const T&>(value));
    return *self();
  }

  template <class T> Archive& operator<< (const T& value)
  {
    save(value);
    return *self();
  }

  /**
   * Separates pointers and references into different serialization
   * functions.
   */
  template <class T> void save(const T& value)
  {
    // If T is a pointer, save it as a pointer
    Pii::IfClass<Pii::IsPointer<T>, PointerSaver, ObjectSaver>::Type::save(*self(), value);
  }

  template <class T> void objectMoved(T& from, T& to) {}

private:
  /**
   * Check if the object at the given memory address has already been
   * stored. If it is, just write a reference index to the previous
   * one. Otherwise, create a new index. Returns true if the pointer
   * was already stored.
   */
  template <class T> bool trackPointer(const T* value)
  {
    // -1 as the pointer index means null pointer
    if (value == 0)
      {
        *self() << static_cast<int>(-1); // make sure it is an int
        return true;
      }
    
    PiiTrackedPointerHolder* pHolder = _pointerMap.value(value,0);
    // Already stored this one ...
    if (pHolder != 0)
      {
        // Store the index of the pointer.
        *self() << pHolder->pointerIndex();
        return true;
      }
    // Not stored yet -> need to serialize the object
    // Create a new index for the stored pointer
    int index = _pointerMap.size();
    // Store to pointer map (false indicates that we haven't
    // serialized the object as a reference)
    _pointerMap.insert(value, createTrackedPointerHolder(value, index, false));
    // Store the index. When reading back, the deserializer first
    // reads the index and compares it to its count of distinct
    // objects. If the index is larger, deserization is needed.
    *self() << index;
    return false;
  }

  /**
   * Same as above, but for objects. Almost identical.
   */
  template <class T> bool trackObject(const T* value)
  {
    // Already stored this one ...
    PiiTrackedPointerHolder* pHolder = _pointerMap.value(value,0);
    if (pHolder != 0)
      {
        // Store the index of the already saved object.
        *self() << pHolder->pointerIndex();
        
        if (pHolder->isSavedByReference())
          return true;

        // If it was stored as a pointer, we need to serialize it again :(
        // PENDING Better suggestions, anyone?
        // Anyway, now we have stored it as a reference
        pHolder->setSavedByReference(true);
      }
    else
      {
        // Create a new index for the stored pointer
        int index = _pointerMap.size();
        // Store to pointer map (true tells that it's an object reference)
        _pointerMap.insert(value, createTrackedPointerHolder(value, index, true));
        *self() << index;
      }
    
    return false;
  }

  /*
   * A convenience function that creates and fills in a
   * PiiTrackedPointerHolder object for type T.
   */
  template <class T> PiiTrackedPointerHolder* createTrackedPointerHolder(const T* ptr,
                                                                         int pointerIndex,
                                                                         bool reference)
  {
    PiiTrackedPointerHolder* pHolder = PiiSerialization::createTrackedPointerHolder(ptr);
    pHolder->setPointerIndex(pointerIndex);
    pHolder->setSavedByReference(reference);
    return pHolder;
  }

  // Non-tracked pointers are always serialized
  template <class T> void savePointer(const T* value)
  {
    // null pointer needs special treatment
    if (value == 0)
      {
        // The class name for a null pointer is "0"
        *self() << "0";
        return;
      }

    // Separate primitive and complex types
    Pii::IfClass<PiiSerializationTraits::IsPrimitive<T>, PrimitivePointerSaver, ComplexPointerSaver>::Type::savePointer(*self(), value);
  }

  // Save non-primitive pointer
  template <class T> void saveComplexPointer(const T* value)
  {
    PiiMetaObject metaObject = PII_GET_METAOBJECT(*value);
    // Store class name (may be empty)
    const char* name = metaObject.className();
    *self() << name;

    // PENDING
    // if (metaObject.hasConstructData())
    
    // 8 bits is enough for a version number
    unsigned char version = 0;
    if (metaObject.isClassInfoStored())
      {
        version = (unsigned char)metaObject.version();
        *self() << version;
      }

    // Store the object itself
    if (!PiiSerializer<Archive>::serialize(name, *self(), *value, version))
      PII_SERIALIZATION_ERROR_INFO(SerializerNotFound, name);
  }
  
  template <class T> void saveTrackedPointer(const T* value)
  {
    // Test if we need to store
    if (!trackPointer(value))
      {
        savePointer(value);
      }
  }

  template <class T> void saveObject(const T& value)
  {
    PiiMetaObject metaObject = PII_GET_METAOBJECT(value);
    unsigned int version = 0;
    if (metaObject.isClassInfoStored())
      {
        // Store version number in 8 bits
        version = metaObject.version();
        *self() << (unsigned char)version;
      }
    
    // Store the object
    if (!PiiSerializer<Archive>::serialize(metaObject.className(), *self(), const_cast<T&>(value), version))
      PII_SERIALIZATION_ERROR_INFO(SerializerNotFound, metaObject.className());
  }

  template <class T> void saveTrackedObject(const T& value)
  {
    // Has this object been already stored by a reference/pointer?
    if (!trackObject(&value))
      {
        saveObject(value);
        // Stores a pointer with its index and memory address
      }
  }
  
  QHash<const void*,PiiTrackedPointerHolder*> _pointerMap;
};

template <class Archive> struct PiiOutputArchive<Archive>::TrackedPointerSaver
{
  template <class T> static void savePointer(Archive& archive, const T* value) { archive.saveTrackedPointer(value); }
};

template <class Archive> struct PiiOutputArchive<Archive>::PointerSaver
{
  template <class T> static void save(Archive& archive, const T* value)
  {
    // Separate tracked/non-tracked pointers
    // This could have been in Archive::save, but at least GCC can't tolerate that
    Pii::IfClass<PiiSerializationTraits::Tracking<T>, TrackedPointerSaver, PointerSaver>::Type::savePointer(archive, value);
  }
  template <class T> static void savePointer(Archive& archive, const T* value)
  {
    archive.savePointer(value);
  }
};

template <class Archive> struct PiiOutputArchive<Archive>::TrackedObjectSaver
{
  template <class T> static void saveObject(Archive& archive, const T& value) { archive.saveTrackedObject(value); }
};

template <class Archive> struct PiiOutputArchive<Archive>::ObjectSaver
{
  template <class T> static void save(Archive& archive, const T& value)
  {
    //Separate tracked/non-tracked pointers
    Pii::IfClass<PiiSerializationTraits::Tracking<T>, TrackedObjectSaver, ObjectSaver>::Type::saveObject(archive, value);
  }

  template <class T> static void saveObject(Archive& archive, const T& value)
  {
    archive.saveObject(value);
  }
};

template <class Archive> struct PiiOutputArchive<Archive>::PrimitivePointerSaver
{
  template <class T> static void savePointer(Archive& archive, const T* value)
  {
    // Pointers must be always accompanied with a class name.
    // Primitive types don't need a name, so we use an empty string.
    archive << "";
    // Store the value the pointer refers to.
    archive << *value;
  }
};

template <class Archive> struct PiiOutputArchive<Archive>::ComplexPointerSaver
{
  template <class T> static void savePointer(Archive& archive, const T* value)
  {
    archive.saveComplexPointer(value);
  }
};

#endif //_PIIOUTPUTARCHIVE_H
