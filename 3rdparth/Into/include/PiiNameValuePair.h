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

#ifndef _PIINAMEVALUEPAIR_H
#define _PIINAMEVALUEPAIR_H

#include "PiiSerializationTraits.h"

/**
 * @file
 *
 * Declaration of a name-value pair wrapper and a few convenience
 * macros.
 */

/**
 * Generate a name-value pair for serialization. Data stored into any
 * archive must be wrapped into name/value pairs because the XML data
 * format expects named values.
 *
 * @param name the name of the value
 *
 * @param the value, typically a class member
 *
 * @code
 * archive & PII_NVP("member", member);
 * @endcode
 */
#define PII_NVP(name, value) ::PiiSerialization::makeNvp(name, value)

/**
 * A convenience macro that writes/retrieves @p member to/from @p
 * archive. The name of the value will be written to the archive as
 * such (if the archive supports name-value pairs).
 */
#define PII_SERIALIZE(archive, member) archive & PII_NVP(#member, member)

/**
 * A convenience macro that writes/retrieves @p member to/from @p
 * archive with a custom name. This is useful if the name of your
 * member variable is not human-friendly.
 */
#define PII_SERIALIZE_NAMED(archive, member, name) archive & PII_NVP(name, member)

/**
 * A template that wraps a name and a value into a pair. XML
 * serialization has special use for the name, other implementations
 * typically ignore it.
 *
 * @ingroup Serialization
 */
template <class T> struct PiiNameValuePair
{
  /**
   * Create a new serialization wrapper for the given value.
   *
   * @param n name
   * @param v data
   */
  PiiNameValuePair(const char* n, const T& v) : name(n), value(const_cast<T&>(v)) {}

  template <class Archive> void serialize(Archive& archive, const unsigned int /*version*/) const
  {
    archive & this->value;
  }
  
  const char* name;
  mutable T& value;
};

namespace PiiSerialization
{
  /**
   * Create a PiiNameValuePair wrapper for a type automatically
   * determined by the @p value parameter.
   *
   * @param name the name of the variable
   *
   * @param value the value
   *
   * @relates PiiNameValuePair
   */
  template <class T> inline PiiNameValuePair<T> makeNvp(const char* name, const T& value) { return PiiNameValuePair<T>(name, value); }
}

PII_SERIALIZATION_TRACKING_TEMPLATE(PiiNameValuePair, false);
PII_SERIALIZATION_CLASSINFO_TEMPLATE(PiiNameValuePair, false);


#endif //_PIINAMEVALUEPAIR_H
