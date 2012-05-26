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

#ifndef _PIIGENERICSERIALIZER_H
#define _PIIGENERICSERIALIZER_H

/**
 * An serialization interface for serializing classes that are not
 * known to the application at compile time.
 */
class PiiGenericSerializer
{
public:
  virtual void serialize(PiiGenericOutputArchive& archive, void* value, const unsigned int version) = 0;
  virtual void serialize(PiiGenericInputArchive& archive, void* value, const unsigned int version) = 0;
};

template <class T> class PiiDefaultGenericSerializer
{
  virtual void serialize(PiiGenericOutputArchive& archive, void* value, const unsigned int version)
  {
    PiiSerialization::Accessor::save(archive, value, version);
  }

  virtual void serialize(PiiGenericInputArchive& archive, void*& value, const unsigned int version)
  {
    PiiSerialization::Accessor::load(archive, value, version);
  }
  
  static PiiDefaultGenericSerializer instance;
};

template <class T> class PiiDefaultGenericSerializer<T> PiiDefaultGenericSerializer<T>::instance;

#endif //_PIIGENERICSERIALIZER_H
