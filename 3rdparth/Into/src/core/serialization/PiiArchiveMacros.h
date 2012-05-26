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

#ifndef _PIIARCHIVEMACROS_H
#define _PIIARCHIVEMACROS_H

/**
 * @file
 *
 * Contains preprocessor macros for conveniently creating primitive
 * input/output operators for archive classes. Mostly for internal
 * use.
 *
 * @ingroup Serialization
 */

#define PII_PRIMITIVE_OPERATOR(Archive, T, Stream, dir, ref) Archive& operator dir (T ref value) { startDelim(); Stream::operator dir (value); endDelim(); return *this; }

#define PII_INTEGER_OPERATORS(Archive, Stream, dir, ref)   \
  PII_PRIMITIVE_OPERATOR(Archive, short, Stream, dir, ref) \
  PII_PRIMITIVE_OPERATOR(Archive, int, Stream, dir, ref)   \
  PII_PRIMITIVE_OPERATOR(Archive, long long, Stream, dir, ref) \
  PII_PRIMITIVE_OPERATOR(Archive, unsigned short, Stream, dir, ref)  \
  PII_PRIMITIVE_OPERATOR(Archive, unsigned int, Stream, dir, ref)    \
  PII_PRIMITIVE_OPERATOR(Archive, unsigned long long, Stream, dir, ref) \
  Archive& operator dir (bool ref value) { return operator dir ((unsigned char ref) value); } \
  Archive& operator dir (long ref value) { return operator dir ((int ref) value); } \
  Archive& operator dir (unsigned long ref value) { return operator dir ((unsigned int ref) value); }
  
#define PII_PRIMITIVE_OPERATORS(Archive, Stream, dir, ref) \
  PII_INTEGER_OPERATORS(Archive, Stream, dir, ref) \
  PII_PRIMITIVE_OPERATOR(Archive, float, Stream, dir, ref) \
  PII_PRIMITIVE_OPERATOR(Archive, double, Stream, dir, ref)
//PII_PRIMITIVE_OPERATOR(Archive, long double, Stream, dir, ref)

//These are ambiguous
//PII_PRIMITIVE_OPERATOR(Archive, unsigned char, Stream, dir, ref)
//PII_PRIMITIVE_OPERATOR(Archive, char, Stream, dir, ref)
//PII_PRIMITIVE_OPERATOR(Archive, long, Stream, dir, ref)
//PII_PRIMITIVE_OPERATOR(Archive, unsigned long, Stream, dir, ref)

#define PII_PRIMITIVE_OUTPUT_OPERATORS(Archive, Stream) \
  PII_PRIMITIVE_OPERATORS(Archive, Stream, <<, )

#define PII_PRIMITIVE_INPUT_OPERATORS(Archive, Stream) \
  PII_PRIMITIVE_OPERATORS(Archive, Stream, >>, &)
#define PII_INTEGER_INPUT_OPERATORS(Archive, Stream) \
  PII_INTEGER_OPERATORS(Archive, Stream, >>, &)


#define PII_DEFAULT_OUTPUT_OPERATORS(Archive) \
  template <class T> Archive& operator<< (T& value) { return PiiOutputArchive<Archive>::operator<<(value); } \
  template <class T> Archive& operator<< (const T& value) { return PiiOutputArchive<Archive>::operator<<(value); }

#define PII_DEFAULT_INPUT_OPERATORS(Archive) \
  template <class T> Archive& operator>> (T& value) { return PiiInputArchive<Archive>::operator>>(value); } \
  template <class T> Archive& operator>> (const T& value) { return PiiInputArchive<Archive>::operator>>(value); }

#endif //_PIIARCHIVEMACROS_H
