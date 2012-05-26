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

#ifndef _PIIBINARYOBJECT_H
#define _PIIBINARYOBJECT_H

#include "PiiSerialization.h"

/**
 * @file
 *
 * Declaration of a binary data pair wrapper and a convenience macro.
 */

/**
 * A serialization wrapper for binary data. Note that this wrapper
 * only works with primitive data types.
 *
 * @ingroup Serialization
 */
template <class T> struct PiiBinaryObject
{
  /**
   * Creates a new binary data wrapper with the given pointer and
   * number of elements.
   */
  PiiBinaryObject(T*& p, unsigned int& s) : ptr(p), size(s) {}
  
  /**
   * Creates a new binary data wrapper with the given pointer and
   * number of elements.
   */
  PiiBinaryObject(const T*& p, unsigned int& s) : ptr(const_cast<T*&>(p)), size(s) {}

  PII_SEPARATE_SAVE_LOAD_MEMBERS

  /**
   * Calls @p archive.writeArray().
   */
  template <class Archive> void save(Archive& archive, const unsigned int)
  {
    archive.writeArray(ptr, size);
  }

  /**
   * Calls @p archive.readArray().
   */
  template <class Archive> void load(Archive& archive, const unsigned int)
  {
    archive.readArray(ptr, size);
  }
  
  T*& ptr;
  unsigned int& size;
};

namespace Pii
{
  /**
   * Constructs a PiiBinaryObject.
   *
   * @relates PiiBinaryObject
   */
  template <class T> PiiBinaryObject<T> binaryObject(T*& p, unsigned int& s)
  {
    return PiiBinaryObject<T>(p, s);
  }
  /**
   * Constructs a PiiBinaryObject.
   *
   * @relates PiiBinaryObject
   */
  template <class T> PiiBinaryObject<T> binaryObject(const T*& p, unsigned int& s)
  {
    return PiiBinaryObject<T>(p, s);
  }
}

PII_SERIALIZATION_TRACKING_TEMPLATE(PiiBinaryObject, false);
PII_SERIALIZATION_CLASSINFO_TEMPLATE(PiiBinaryObject, false);

#endif //_PIIBINARYOBJECT_H
