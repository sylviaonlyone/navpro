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

#ifndef _PIIPTRHOLDER_H
#define _PIIPTRHOLDER_H

#include "Pii.h"
#include "PiiSharedObject.h"

/// @cond null

/**
 * A reference counting wrapper for any pointer type. The PiiPtrHolder
 * class is used with primitive types, with arrays of them, and with
 * object types that don't support the reserve/release reference
 * counting mechanism. In these scenarios, a pointer to a PiiPtrHolder
 * object is passed instead of the object pointer itself.
 *
 * @ingroup Core
 */
template <class T> class PiiPtrHolder : public PiiSharedObject
{
public:
  PiiPtrHolder() : pointer(0) {}

  /**
   * Creates a PiiPtrHolder that wraps the given pointer @a ptr. The
   * pointer will be owned by %PiiPtrHolder and must not be deleted by
   * the caller.
   */
  PiiPtrHolder(T* ptr) : pointer(ptr) {}

  /**
   * Deletes the wrapped pointer.
   */
  ~PiiPtrHolder() { delete pointer; }

  T* pointer;
private:
  PII_DISABLE_COPY(PiiPtrHolder);
};

template <class T> class PiiPtrHolder<T[]> : public PiiSharedObject
{
public:
  PiiPtrHolder() : pointer(0) {}
  PiiPtrHolder(T* ptr) : pointer(ptr) {}
  ~PiiPtrHolder() { delete[] pointer; }
  T* pointer;
private:
  PII_DISABLE_COPY(PiiPtrHolder);
};

/// @endcond

#endif //_PIIPTRHOLDER_H
