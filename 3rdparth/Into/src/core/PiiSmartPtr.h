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

#ifndef _PIISMARTPTR_H
#define _PIISMARTPTR_H

#include "PiiGlobal.h"

/**
 * A utility class that works as a simple alternative to
 * std::auto_ptr. PiiSmartPtr is used as a stack-allocated object that
 * deletes its pointer upon destruction. It doesn't provide copy
 * semantics. Contrary to std::auto_ptr, PiiSmartPtr also works for
 * arrays.
 *
 * The main use of PiiSmartPtr is in guarding heap-allocated pointers
 * in functions that may throw exceptions. Use like this:
 *
 * @code
 * PiiSmartPtr<int[]> ptr(new int[5]); // arrays
 * PiiSmartPtr<int> ptr(new int); // any other pointers
 * @endcode
 *
 * @ingroup Core
 */
template <class T> class PiiSmartPtr
{
public:
  PiiSmartPtr() : _ptr(0) {}
  /**
   * Create a new PiiSmartPtr that wraps the given pointer. Ownership
   * of the pointer is taken.
   */
  PiiSmartPtr(T* ptr) : _ptr(ptr) {}
  
  /**
   * Delete the wrapped pointer.
   */
  ~PiiSmartPtr() { delete _ptr; }

  /**
   * This allows one to use PiiSmartPtr<T> as a T*.
   */
  operator T* () const { return _ptr; }

  /**
   * Masquerades the class as a pointer.
   */
  T* operator-> () const { return _ptr; }

  PiiSmartPtr& operator= (T* ptr)
  {
    delete _ptr;
    _ptr = ptr;
    return *this;
  }
  
  /**
   * Release the ownership of the pointer. The pointer is returned.
   */
  T* release()
  {
    T* result = _ptr;
    _ptr = 0;
    return result;
  }
  
private:
  T* _ptr;
  PII_DISABLE_COPY(PiiSmartPtr);
};

template <class T> class PiiSmartPtr<T[]>
{
public:
  PiiSmartPtr(T* ptr = 0) : _ptr(ptr) {}
  ~PiiSmartPtr() { delete[] _ptr; }

  operator T* () const { return _ptr; }
  T* release()
  {
    T* result = _ptr;
    _ptr = 0;
    return result;
  }
  
private:
  T* _ptr;
};

#endif //_PIISMARTPTR_H
