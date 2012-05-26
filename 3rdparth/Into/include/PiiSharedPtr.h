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

#ifndef _PIISHAREDPTR_H
#define _PIISHAREDPTR_H

#include "PiiPtrHolder.h"

/**
 * An implicitly shared pointer. The PiiSharedPtr class holds a
 * reference-counted pointer (@ref PiiSharedObject) and masquerades as
 * one itself. The class automatically increases and decreases the
 * reference count when copies of it are requested. Thus, you never
 * need to care about deleting the memory. Typical usage (compare this
 * to the explicit sharing example in @ref PiiSharedObject):
 *
 * @code
 * class MyObject : public PiiSharedObject;
 * typedef PiiSharedPtr<MyObject> MyPtr;
 *
 * // Sender creates an explicitly shared pointer and passes it.
 * MyPtr ptr(new MyObject);
 * receiver.takeThis(ptr);
 * otherReceiver.takeThis(ptr);
 *
 * //in the receiver, we just store the pointer. It is automatically
 * //released when the Receiver class is deleted.
 * void Receiver::takeThis(MyPtr ptr)
 * {
 *   //the MyPtr object works just as a pointer
 *   ptr->callMethod();
 *   internalVariable = ptr;
 *   doWhatEverNeeded();
 * }
 * @endcode
 *
 * The class supports all types types, including arrays. If the
 * reference-counted type is not directly derived from
 * PiiSharedObject, a reference-counted wrapper will be automatically
 * created.
 *
 * @code
 * PiiSharedPtr<string> ptr(new string("this is"));
 * *ptr += " a test";
 * cout << ptr->c_str() << endl; //outputs "this is a test"
 * @endcode
 *
 * @code
 * PiiSharedPtr<int> ptr(new int);
 * ptr = new int; //releases the old one
 * *ptr = 3; //use the class just like an int pointer
 * @endcode
 *
 * @code
 * PiiSharedPtr<int[]> ptr(new int[5]);
 * ptr = new int[6]; //releases the old array
 * ptr[0] = 3; //use the class just like an int array
 * @endcode
 *
 * The computational overhead of using a shared pointer instead of a
 * direct one is minimal. Copying a pointer costs one (non-virtual and
 * inlineable) function call, and accessing it is as fast as direct
 * access, provided that the pointer type is derived from
 * PiiSharedObject. If it is not, there will be one more memory
 * indirection.
 *
 * To ensure no memory leaks, one should never store the internal
 * pointer as an ordinary pointer only, although it is possible. Once
 * you initialize a PiiSharedPtr with a newly allocated pointer, just
 * forget about the pointer and make sure you only make assignments
 * between PiiSharedPtrs. You may, however, use the internal pointer
 * directly if you ensure that the PiiSharedPtr instance stays in
 * memory.
 *
 * @code
 * MyPtr ptr(new MyObject);
 * MyObject* ptr2 = ptr;
 * // You may now safely use ptr2 as long as ptr is in memory
 * @endcode
 *
 * One can assign PiiSharedPtrs just like ordinary pointers. That is,
 * a PiiSharedPtr<Base> can hold a pointer to a class derived from
 * Base.
 *
 * @note If the types are not derived from PiiSharedObject, multiple
 * inheritance is not always handled correctly. You can assign derived
 * pointers to base class pointers only if the base class is the first
 * one in iheritance order.
 *
 * @see PiiSharedObject
 *
 * @ingroup Core
 */
template <class T> class PiiSharedPtr
{
  // Make the class serializable
  friend struct PiiSerialization::Accessor;
  template <class Archive> void serialize(Archive& archive, const unsigned int /*version*/)
  {
    archive & PII_NVP("ptr", unwrap(_ptr));
  }

public:
  /// @internal
  template <class U> struct OnlyDerived : Pii::IfClass<Pii::IsBaseOf<T,U>, Pii::Id<int>, Pii::Empty>::Type {};

#define PII_IS_FIRST_BASE(BASE, DERIVED)                                \
  (reinterpret_cast<void*>(static_cast<BASE*>(reinterpret_cast<DERIVED*>((void*)1))) == (void*)1)

  PiiSharedPtr() : _ptr(wrap(0)) {}
  
  /**
   * Creates an implicitly shared pointer that holds the given pointer.
   *
   * @param obj a pointer to the object to be shared. %PiiSharedPtr
   * takes the ownership of the pointer.
   */
  explicit PiiSharedPtr(T* obj) : _ptr(wrap(obj)) {}

  /**
   * Create a copy of a pointer.
   */
  PiiSharedPtr(const PiiSharedPtr& other) : _ptr(other._ptr)
  {
    if (_ptr != 0)
      _ptr->reserve();
  }

  /**
   * Create a copy of a pointer. This constructor only accepts
   * pointers to objects that are derived from the template type @p T
   * of this class.
   */
  template <class U> PiiSharedPtr(const PiiSharedPtr<U>& other, typename OnlyDerived<U>::Type = 0) :
    _ptr(other._ptr)
  {
    Q_ASSERT(PII_IS_FIRST_BASE(T,U)); // traps multiple inheritance problems
    if (_ptr != 0)
      _ptr->reserve();
  }
  
  ~PiiSharedPtr()
  {
    if (_ptr != 0)
      _ptr->release();
  }

  /**
   * Assigns a new value to this pointer and releases the old pointer.
   *
   * @code
   * PiiSharedPtr<MyObject> ptr1(new MyObject), ptr2(new MyObject);
   * ptr1 = ptr2; //releases the old pointer in ptr1, increases refcount in ptr2
   * @endcode
   */
  PiiSharedPtr& operator= (const PiiSharedPtr& other)
  {
    reassign(other._ptr);
    return *this;
  }

  /**
   * Assigns a new value to this pointer and release the old pointer. 
   * This operator only accepts pointers to objects that are derived
   * from the template type @p T of this class.
   */
  template <class U> inline PiiSharedPtr& operator= (const PiiSharedPtr<U>& other)
  {
    copy(other);
    return *this;
  }
  
  /**
   * Assigns a new value to this pointer. The old pointer will be
   * released. %PiiSharedPtr takes the ownership of @a ptr.
   *
   * @code
   * PiiSharedPtr<MyObject> ptr(new MyObject);
   * ptr = new MyObject; //releases the old pointer in ptr
   * @endcode
   */
  PiiSharedPtr& operator= (T* ptr)
  {
    if (unwrap(_ptr) != ptr)
      {
        if (_ptr != 0)
          _ptr->release();
        _ptr = wrap(ptr);
      }
    return *this;
  }

  /**
   * Returns a reference to the element at @a index, if the wrapped
   * pointer is an array.
   */
  inline T& operator[] (int index)
  {
    return elementAt(this, index);
  }

  /**
   * Returns the element at @a index, if the wrapped pointer is an
   * array.
   */
  inline T operator[] (int index) const
  {
    return elementAt(this, index);
  }

  /**
   * Compare two pointers.
   * @return true iff the internal pointers are equal
   */
  bool operator== (const PiiSharedPtr& other) const { return other._ptr == _ptr; }

  /**
   * Compare two pointers.
   * @return true iff the internal pointers are different
   */
  bool operator!= (const PiiSharedPtr& other) const { return other._ptr != _ptr; }

  /**
   * Compare the internal pointer to the given @p ptr.
   */
  bool operator== (const void* ptr) const { return ptr == _ptr; }
  
  /**
   * Compare the internal pointer to the given @p ptr.
   */
  bool operator!= (const void* ptr) const { return ptr != _ptr; }

  /**
   * Returns the internal pointer. This operator makes it possible to
   * use this class directly as a pointer.
   */
  T* operator-> () const { return unwrap(_ptr); }
  /**
   * Converts the shared pointer to a reference.
   */
  T& operator* () const { return *unwrap(_ptr); }
  /**
   * Casts the shared pointer to the internal pointer type.
   */
  operator T* () const { return unwrap(_ptr); }

private:
  template <class U> friend class PiiSharedPtr;

  struct NonWrapped
  {
    static PiiSharedObject* wrap(T* ptr) { return ptr; }
    static T*& unwrap(PiiSharedObject* ptr) { return static_cast<T*&>(ptr); }
  };

  struct Wrapped
  {
    static PiiSharedObject* wrap(T* ptr) { return new PiiPtrHolder<T>(ptr); }
    static T*& unwrap(PiiSharedObject* ptr) { return static_cast<PiiPtrHolder<T>*>(ptr)->pointer; }
  };
  
  // If T is already derived from PiiSharedObject, we don't need a
  // wrapper. Otherwise we do.
  typedef typename Pii::IfClass<Pii::IsBaseOf<PiiSharedObject, T>,
                                NonWrapped,
                                Wrapped>::Type Traits;

  inline static PiiSharedObject* wrap(T* ptr) { return Traits::wrap(ptr); }
  inline static T*& unwrap(const PiiSharedObject* ptr) { return Traits::unwrap(const_cast<PiiSharedObject*>(ptr)); }

  void reassign(PiiSharedObject* ptr)
  {
    if (ptr != 0)
      ptr->reserve();
    if (_ptr != 0)
      _ptr->release();
    _ptr = ptr;
  }

  template <class U> void copy(const PiiSharedPtr<U>& other, typename OnlyDerived<U>::Type = 0)
  {
    Q_ASSERT(PII_IS_FIRST_BASE(T,U)); // traps multiple inheritance problems
    reassign(other._ptr);
  }

  template <class U> struct OnlyArray : Pii::IfClass<Pii::IsArray<U>, Pii::Id<int>, Pii::Empty>::Type {};

  template <class U> static inline T& elementAt(U* thisPtr, typename OnlyArray<U>::Type index)
  {
    return unwrap(thisPtr->_ptr)[index];
  }

  PiiSharedObject* _ptr;
};

PII_SERIALIZATION_TRACKING_TEMPLATE(PiiSharedPtr, false);

#endif //_PIISHAREDPTR_H
