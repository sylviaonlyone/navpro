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

#ifndef _PIISHAREDOBJECT_H
#define _PIISHAREDOBJECT_H

#include "PiiGlobal.h"
#include <PiiSerialization.h>
#include <PiiNameValuePair.h>
#include <PiiTrackedPointerHolder.h>
#include <QAtomicInt>

/**
 * A shared object. Shared objects are useful when multiple pointers
 * to a single memory location are concurrently in use. With a shared
 * object, it is possible that the users of the memory location are
 * unaware of each other. Each user just calls reserve() when it
 * starts using the pointer and release() after it no longer needs it. 
 * Each reserve() call increases and each release() call decreases a
 * reference counter. When the counter reaches zero, the object is
 * automatically deleted. This type of explicit sharing provides
 * minimum computational overhead but may not be the most convenient
 * way to the programmer. A typical usage scenario is as follows:
 *
 * @code
 * class MyObject : public PiiSharedObject { ... };
 *
 * // Sender creates the pointer and passes it.
 * MyObject* obj = new MyObject;
 * receiver.takeThis(obj);
 * otherReceiver.takeThis(obj);
 * // This may delete the object. You don't know, so don't fiddle with
 * // it any more.
 * obj->release();
 *
 * //in the receiver, we reserve the pointer for later use:
 * void Receiver::takeThis(MyObject* obj)
 * {
 *   internalVariable = obj->reserve();
 *   doWhatEverNeeded();
 * }
 * @endcode
 *
 * Once "Receiver" does not need the pointer any more, it calls
 * internalVariable->release() and forgets about the pointer.
 *
 * Usually, it is easier to use implicit sharing. For this, the
 * @ref PiiSharedPtr class can be used.
 *
 * PiiSharedObject is thread safe. Its memory overhead is one integer
 * for the reference counter (plus a vtable pointer, which you most
 * likely had anyway).
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiSharedObject
{
public:
  /**
   * Create a new shared object. The reference counter is initialized
   * to one.
   */
  PiiSharedObject() : _ref(1)
  { }

  /**
   * Virtual destructor. This ensures the delete operator always
   * destroys the bottommost derived class.
   */
  virtual ~PiiSharedObject();
  
  /**
   * Reserves the object for use. The reference counter is incremented
   * by one and returned.
   *
   * @return the reference count after the call
   */
  int reserve() const
  {
    _ref.ref();
    return static_cast<int>(_ref);
  }
  
  /**
   * Releases the object. The reference counter is decremented by one. 
   * Once it reaches zero, the object will be deleted.
   */
  int release() const
  {
    if (_ref.deref())
      return static_cast<int>(_ref);

    delete const_cast<PiiSharedObject*>(this);
    return 0;
  }

  /**
   * Get the number of references to this object.
   */
  int references() const { return static_cast<int>(_ref); }

  /**
   * Set the number of references to @p cnt.
   */
  void setReferences(int cnt) { _ref = cnt; }

private:
  mutable QAtomicInt _ref;
};

// Made an idiotic design mistake in version 0.
PII_SERIALIZATION_VERSION(PiiSharedObject, 1);

/// @cond null
class PiiSharedObjectHolder : public PiiTrackedPointerHolder
{
public:
  PiiSharedObjectHolder(const PiiSharedObject* ptr) : PiiTrackedPointerHolder(ptr)
  {
    ptr->reserve();
  }

  ~PiiSharedObjectHolder()
  {
    reinterpret_cast<const PiiSharedObject*>(_pointer)->release();
  }  
};

namespace PiiSerialization
{
  inline PiiTrackedPointerHolder* createTrackedPointerHolder(const PiiSharedObject* ptr)
  {
    return new PiiSharedObjectHolder(ptr);
  }
  
  /// A specialization that increases the refcount of @ptr by one.
  inline void rereferencePointer(PiiSharedObject* ptr)
  {
    ptr->reserve();
  }
}
/// @endcond

#endif //_PIISHAREDOBJECT_H
