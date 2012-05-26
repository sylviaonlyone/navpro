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

#ifndef _PIIINPUTARCHIVE_H
#define _PIIINPUTARCHIVE_H

#include <QList>
#include <cstring>
#include <PiiMetaTemplate.h>
#include "PiiTypeTraits.h"
#include "PiiSerializationTraits.h"
#include "PiiSerializationFactory.h"
#include "PiiSerializationException.h"
#include "PiiMetaObject.h"
#include "PiiSerializer.h"
#include "PiiSmartPtr.h"
#include "PiiDynamicTypeFunctions.h"

/// @internal
struct PiiArchivePointerInfo
{
  PiiArchivePointerInfo(void*p, const QList<void**>& a, bool b) :
    ptr(p), addresses(a), objectStored(b) {}
  void* ptr;
  QList<void**> addresses;
  bool objectStored;
};

namespace PiiSerialization
{
  /**
   * This function is called by PiiInputArchive each time a
   * tracked object is deserialized by a reference to a previously
   * deserialized object. This allows one to do reference counting on
   * tracked objects when reading an archive. The default
   * implementation does nothing. Override this function to perform
   * any action required to restore reference counts.
   *
   * @code
   * namespace PiiSerialization
   * {
   *   inline void rereferencePointer(MyRefCountedObj* ptr)
   *   {
   *      ptr->increaseRefCount();
   *   }
   * }
   * @endcode
   *
   * @relates PiiInputArchive
   */
  inline void rereferencePointer(void* /*ptr*/) {}
}

/**
 * A base class for input archive implementations. This class
 * provides functions for saving pointers.
 *
 * @ingroup Serialization
 */
template <class Archive> class PiiInputArchive
{
  struct PointerLoader;
  struct ObjectLoader;
  struct TrackedPointerLoader;
  struct TrackedObjectLoader;
  struct ObjectMover;
  struct PointerMover;
  struct PrimitivePointerLoader;
  struct ComplexPointerLoader;

  friend struct PointerLoader;
  friend struct ObjectLoader;
  friend struct TrackedPointerLoader;
  friend struct TrackedObjectLoader;
  friend struct ObjectMover;
  friend struct PointerMover;
  friend struct PrimitivePointerLoader;
  friend struct ComplexPointerLoader;

public:
  /**
   * This enumeration is used as a static constant for compile-time
   * checking of input/output.
   */
  enum { InputArchive = true, OutputArchive = false };

  /**
   * A design pattern for casting the type of this archive to that of
   * the template parameter.
   */
  Archive* self() { return static_cast<Archive*>(this); }

  /**
   * This operator is defined for both input and output archives,
   * which makes it possible to serialize and deserialize data with a
   * single function. The input archive converts the operator to
   * operator>>.
   */
  template <class T> Archive& operator& (T& value) { return *self() >> value; }
  /// @overload
  template <class T> Archive& operator& (const T& value) { return *self() >> value; }

  /**
   * Reads an array of @p size elements to the memory location pointed
   * to by @2 ptr.
   */
  template <class T> void readArray(T*& ptr, unsigned int& size)
  {
    *self() >> size;
    //PENDING This may eat up all memory. Should we have a limit?
    // Allocate memory and restore data
    if (size != 0)
      {
        PiiSmartPtr<T[]> tmpPtr(new T[size]);
        self()->readRawData(tmpPtr, size*sizeof(T));
        ptr = tmpPtr.release();
      }
    else
      ptr = 0;
  }

  /**
   * Analogous to PiiOutputArchive::operator<<(T&). This function
   * calls Archive::load(value).
   */
  template <class T> Archive& operator>> (T& value)
  {
    load(value);
    return *self();
  }
  
  template <class T> Archive& operator>> (const T& value)
  {
    load(const_cast<T&>(value));
    return *self();
  }
  
  template <class T> void load(T& value)
  {
    // If T is a pointer, load it as a pointer
    Pii::IfClass<Pii::IsPointer<T>, PointerLoader, ObjectLoader>::Type::load(*self(), value);
  }

  /**
   * Tell the archive that you changed the location of an object after
   * saving it.
   *
   * @code
   * QList<MyObj> lst; // filled somehow
   *
   * // In your load() function:
   * for (int i=0; i<elementCount; i++)
   *   {
   *     MyObj obj;
   *     archive >> obj;
   *     lst << obj;
   *     // "archive" now thinks &obj is the address of the deserialized object.
   *     // If a pointer refers to this object, it must be updated
   *     archive.objectMoved(obj, lst.last());
   *   }
   * @endcode
   */
  template <class T> void objectMoved(T& from, T& to)
  {
    Pii::IfClass<Pii::IsPointer<T>, PointerMover, ObjectMover>::Type::move(*self(), from, to);
  }
  
private:
  /**
   * Update all references to the memory address @p from to @p to.
   */
  void moveObject(void* from, void* to)
  {
    for (int i=_lstPointers.size(); i--; )
      {
        if (_lstPointers[i].ptr != from)
          continue;
        
        for (int j=_lstPointers[i].addresses.size(); j--; )
          *_lstPointers[i].addresses[j] = to;

        _lstPointers[i].ptr = to;
      }
  }

  // PENDING The type of the pointer is not used as a key in look-ups
  // (how could it). If a class has no vtable, its first member is in
  // the same address as the class itself. If both are tracked,
  // possible problems appear.
  template <class T> bool trackPointer(T*& value)
  {
    // Read pointer index from the archive
    int ptrIndex;
    *self() >> ptrIndex;

    // -1 is a special value for null pointers
    if (ptrIndex == -1)
      {
        value = 0;
        return true;
      }
    
    if (ptrIndex > _lstPointers.size() || ptrIndex < 0)
      PII_SERIALIZATION_ERROR(InvalidDataFormat);
    
    // We already restored this one
    if (ptrIndex < _lstPointers.size())
      {
        value = reinterpret_cast<T*>(_lstPointers[ptrIndex].ptr);
        // Store the address of the pointer for possible later tuning.
        _lstPointers[ptrIndex].addresses << reinterpret_cast<void**>(&value);
        // Give the outer world a possibility to do refcounting.
        PiiSerialization::rereferencePointer(value);
        return true;
      }
    return false;
  }

  template <class T> bool trackObject(T*& value)
  {
    // Read the pointer index from the archive
    int ptrIndex;
    *self() >> ptrIndex;
    
    if (ptrIndex > _lstPointers.size() || ptrIndex < 0)
      PII_SERIALIZATION_ERROR(InvalidDataFormat);
    
    // We already restored this one
    if (ptrIndex < _lstPointers.size())
      {
        value = (T*)_lstPointers[ptrIndex].ptr;
        
        // Give the outer world a possibility to do refcounting.
        PiiSerialization::rereferencePointer(value);

        // If the object has been already serialized by reference, we
        // don't need to restore it again.
        if (_lstPointers[ptrIndex].objectStored)
          return true;

        // We are going to deserialize this again, so store the fact
        // that it is now in a fixed memory location.
        _lstPointers[ptrIndex].objectStored = true;
      }
    // Not restored yet
    else
      {
        // Store object address (later coming ones may refer to this). 
        // The address list is empty, because there are no pointers to
        // this object yet. "true" means the object has been saved by
        // reference.
        _lstPointers << PiiArchivePointerInfo(value, QList<void**>(), true);
        
        // value == 0 means that the object wasn't stored yet
        value = 0;
      }

    // value != 0 with a return value equal to false means that the
    // object was already restored by a pointer that now points to
    // "value"
    return false;
  }
  
  
  template <class T> void loadPointer(T*& value, bool tracked = false)
  {
    // Read object name
    char* name;
    *self() >> name;
    // Make an exception-safe pointer
    PiiSmartPtr<char[]> namePtr(name);

    // The class name for a null pointer is "0". We can end up here if
    // the pointer is not tracked.
    if (std::strcmp(name, "0") == 0)
      {
        value = 0;
        return;
      }

    // Separate primitive and complex types
    Pii::IfClass<PiiSerializationTraits::IsPrimitive<T>, PrimitivePointerLoader, ComplexPointerLoader>::Type
      ::loadPointer(*self(), name, value, tracked);
  }
  
  template <class T> void loadComplexPointer(const char* name, T*& value, bool tracked = false)
  {
    // Create an instance of the named class
    value = PiiSerializationFactory::create<T>(name, *self());
    PiiSmartPtr<T> valuePtr(value); // Exception safety
    if (value == 0)
      PII_SERIALIZATION_ERROR_INFO(UnregisteredClass, name);

    PiiMetaObject metaObject = PII_GET_METAOBJECT(*value);
    unsigned char version = 0;
    if (metaObject.isClassInfoStored())
      {
        // Read version number
        *self() >> version;
      }

    // Check that the stored version is not higher than the current
    // one. The current version number is requested from the meta
    // object. (Otherwise only the version of the parent class would
    // be available, if virtual meta objects are in use.)
    if (version > metaObject.version())
      PII_SERIALIZATION_ERROR_INFO(ClassVersionMismatch, name);
    
    // Store the pointer and its address to the list of deserialized
    // objects.
    if (tracked)
      _lstPointers << PiiArchivePointerInfo(value, QList<void**>() << reinterpret_cast<void**>(&value), false);

    // Restore
    PiiSerializer<Archive>::serialize(name, *self(), *value, version);
    valuePtr.release();
  }

  template <class T> void loadTrackedPointer(T*& value)
  {
    // Check if we can handle this pointer by reference only
    if (!trackPointer(value)) //no
      loadPointer(value, true);
  }

  template <class T> void loadObject(T& value)
  {
    PiiMetaObject metaObject = PII_GET_METAOBJECT(value);
    // Read version number
    unsigned char version = 0;
    if (metaObject.isClassInfoStored())
      *self() >> version;

    // Check validity of version number
    if (version > metaObject.version())
      PII_SERIALIZATION_ERROR_INFO(ClassVersionMismatch,
                                   metaObject.className());

    PiiSerializer<Archive>::serialize(metaObject.className(), *self(), value, version);
  }

  template <class T> void loadTrackedObject(T& value)
  {
    // Let's check if this object has already been stored
    T* ptr = &value;
    if (!trackObject(ptr)) // no, it wasn't, or maybe it was, via a pointer
      {
        // In either case, we must restore the object (again)
        loadObject(value);
        
        if (ptr != 0)
          {
            // The object has already been restored through a pointer
            // and created by a factory, but now we found that we have
            // had space reserved for it. ptr points to the object
            // created earlier.

            // Delete the old object. It is unnecessary now
            delete ptr;

            moveObject(ptr, &value);
          }
      }
  }

  /**
   * This list stores the addresses of deserialized objects together
   * with the addresses of the pointers that point to these memory
   * locations. (Clear, no?)
   */
  QList<PiiArchivePointerInfo> _lstPointers;
};


template <class Archive> struct PiiInputArchive<Archive>::TrackedPointerLoader
{
  template <class T> static void loadPointer(Archive& archive, T*& value) { archive.loadTrackedPointer(value); }
};

template <class Archive> struct PiiInputArchive<Archive>::PointerLoader
{
  template <class T> static void load(Archive& archive, T*& value)
  {
    // Separate tracked/non-tracked pointers
    Pii::IfClass<PiiSerializationTraits::Tracking<T>, TrackedPointerLoader, PointerLoader>::Type::loadPointer(archive, value);
  }

  template <class T> static void loadPointer(Archive& archive, T*& value)
  {
    archive.loadPointer(value);
  }
};

template <class Archive> struct PiiInputArchive<Archive>::TrackedObjectLoader
{
  template <class T> static void loadObject(Archive& archive, T& value) { archive.loadTrackedObject(value); }
};

template <class Archive> struct PiiInputArchive<Archive>::ObjectLoader
{
  template <class T> static void load(Archive& archive, T& value)
  {
    // Separate tracked/non-tracked objects
    Pii::IfClass<PiiSerializationTraits::Tracking<T>, TrackedObjectLoader, ObjectLoader>::Type::loadObject(archive, value);
  }

  template <class T> static void loadObject(Archive& archive, T& value)
  {
    archive.loadObject(value);
  }
};

template <class Archive> struct PiiInputArchive<Archive>::ObjectMover
{
  template <class T> static void move(Archive& archive, T& from, T& to) { archive.moveObject(&from, &to); }
};
  
template <class Archive> struct PiiInputArchive<Archive>::PointerMover
{
  template <class T> static void move(Archive& /*archive*/, T* /*from*/, T* /*to*/) {}
};

template <class Archive> struct PiiInputArchive<Archive>::PrimitivePointerLoader
{
  template <class T> static void loadPointer(Archive& archive, const char* /*name*/, T*& value, bool tracked)
  {
    // Create an uninitialized object.
    value = new T;
    // Store the pointer and its address to the list of deserialized
    // objects.
    if (tracked)
      archive._lstPointers << PiiArchivePointerInfo(value, QList<void**>() << reinterpret_cast<void**>(&value), false);
    archive >> *value;
  }
};

template <class Archive> struct PiiInputArchive<Archive>::ComplexPointerLoader
{
  template <class T> static void loadPointer(Archive& archive, const char* name, T*& value, bool tracked)
  {
    archive.loadComplexPointer(name, value, tracked);
  }
};
#endif //_PIIINPUTARCHIVE_H
