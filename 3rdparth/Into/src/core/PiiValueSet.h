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

#ifndef _PIIVALUESET_H
#define _PIIVALUESET_H

#include <cstdarg>
#include <cstring>
#include "PiiTypeTraits.h"

/// @file

/**
 * A macro that allows one to use a convenient syntax for checking if
 * a value is a member of a set. With compiler optimizations turned
 * on, this technique is equally fast as direct comparison.
 *
 * @code
 * if (rand() % 5 member_of (1, 2, 3))
 *   cout << "You had luck.\n";
 * const char* fruit = "Apple";
 * if (fruit member_of ("Apple", "Orange", "Pear"))
 *   cout << "Yes, this works too.\n";
 * @endcode
 *
 * @see not_member_of
 */
#define member_of == Pii::valueSet

/**
 * A macro that allows one to use a convenient syntax for checking if
 * a value is @e not a member of a set. With compiler optimizations
 * turned on, this technique is equally fast as direct comparison.
 *
 * @code
 * if (1 not_member_of (1, 2, 3))
 *   cout << "Now, this is weird.\n";
 * const char* fruit = "Apple";
 * if (fruit not_member_of ("Banana", "Orange", "Pear"))
 *   cout << "Comparing apples to oranges is not fair.\n";
 * @endcode
 *
 * @see member_of
 */
#define not_member_of != Pii::valueSet

// No docs
/// @cond null

namespace Pii
{
  template <class T> inline bool valueSetEqual(T v1, T v2) { return v1 == v2; }
  inline bool valueSetEqual(const char* v1, const char* v2) { return !strcmp(v1, v2); }
}

template <class T, int N> struct PiiValueSet
{
  typedef typename Pii::VaArg<T>::Type VaArgType;
  
  PiiValueSet(VaArgType firstValue, ...)
  {
    values[0] = firstValue;
    va_list argp;
    // initalize var ptr
    va_start(argp, firstValue);
    // repeat for each arg
    for (int i=1; i<N; ++i)
      values[i] = static_cast<T>(va_arg(argp, VaArgType));

    // done with args
    va_end(argp);
  }
  T values[N];
};
template <class T, int N> bool operator== (T v, const PiiValueSet<T,N>& set)
{
  for (int i=0; i<N; ++i)
    if (Pii::valueSetEqual(v, set.values[i]))
      return true;
  return false;
}
template <class T, int N> bool operator!= (T v, const PiiValueSet<T,N>& set)
{
  for (int i=0; i<N; ++i)
    if (Pii::valueSetEqual(v, set.values[i]))
      return false;
  return true;
}

template <class T> struct PiiValueSet<T,2>
{
  PiiValueSet(T val1, T val2) : v1(val1), v2(val2) {}
  T v1, v2;
};
template <class T> bool operator== (T v, const PiiValueSet<T,2>& set)
{
  return Pii::valueSetEqual(v, set.v1) || Pii::valueSetEqual(v, set.v2);
}
template <class T> bool operator!= (T v, const PiiValueSet<T,2>& set)
{
  return !Pii::valueSetEqual(v, set.v1) && !Pii::valueSetEqual(v, set.v2);
}

template <class T> struct PiiValueSet<T,3>
{
  PiiValueSet(T val1, T val2, T val3) : v1(val1), v2(val2), v3(val3) {}
  T v1, v2, v3;
};
template <class T> bool operator== (T v, const PiiValueSet<T,3>& set)
{
  return Pii::valueSetEqual(v, set.v1) || Pii::valueSetEqual(v, set.v2) || Pii::valueSetEqual(v, set.v3);
}
template <class T> bool operator!= (T v, const PiiValueSet<T,3>& set)
{
  return !Pii::valueSetEqual(v, set.v1) && !Pii::valueSetEqual(v, set.v2) && !Pii::valueSetEqual(v, set.v3);
}

template <class T> struct PiiValueSet<T,4>
{
  PiiValueSet(T val1, T val2,
              T val3, T val4) :
    v1(val1), v2(val2), v3(val3), v4(val4) {}
  T v1, v2, v3, v4;
};
template <class T> bool operator== (T v, const PiiValueSet<T,4>& set)
{
  return Pii::valueSetEqual(v, set.v1) || Pii::valueSetEqual(v, set.v2) ||
    Pii::valueSetEqual(v, set.v3) || Pii::valueSetEqual(v, set.v4);
}
template <class T> bool operator!= (T v, const PiiValueSet<T,4>& set)
{
  return !Pii::valueSetEqual(v, set.v1) && !Pii::valueSetEqual(v, set.v2) &&
    !Pii::valueSetEqual(v, set.v3) && !Pii::valueSetEqual(v, set.v4);
}

template <class T> struct PiiValueSet<T,5>
{
  PiiValueSet(T val1, T val2,
              T val3, T val4,
              T val5) :
    v1(val1), v2(val2), v3(val3), v4(val4), v5(val5) {}
  T v1, v2, v3, v4, v5;
};
template <class T> bool operator== (T v, const PiiValueSet<T,5>& set)
{
  return Pii::valueSetEqual(v, set.v1) || Pii::valueSetEqual(v, set.v2) ||
    Pii::valueSetEqual(v, set.v3) || Pii::valueSetEqual(v, set.v4) ||
    Pii::valueSetEqual(v, set.v5);
}
template <class T> bool operator!= (T v, const PiiValueSet<T,5>& set)
{
  return !Pii::valueSetEqual(v, set.v1) && !Pii::valueSetEqual(v, set.v2) &&
    !Pii::valueSetEqual(v, set.v3) && !Pii::valueSetEqual(v, set.v4) &&
    !Pii::valueSetEqual(v, set.v5);
}

namespace Pii
{
  template <class T> PiiValueSet<T,2> inline valueSet(T v1, T v2)
  { return PiiValueSet<T,2>(v1, v2); }
  template <class T> PiiValueSet<T,3> inline valueSet(T v1, T v2, T v3)
  { return PiiValueSet<T,3>(v1, v2, v3); }
  template <class T> PiiValueSet<T,4> inline valueSet(T v1, T v2, T v3, T v4)
  { return PiiValueSet<T,4>(v1, v2, v3, v4); }
  template <class T> PiiValueSet<T,5> inline valueSet(T v1, T v2, T v3, T v4, T v5)
  { return PiiValueSet<T,5>(v1, v2, v3, v4, v5); }
  template <class T> PiiValueSet<T,6> inline valueSet(T v1, T v2, T v3, T v4, T v5, T v6)
  { return PiiValueSet<T,6>(v1, v2, v3, v4, v5, v6); }
  template <class T> PiiValueSet<T,7> inline valueSet(T v1, T v2, T v3, T v4, T v5, T v6, T v7)
  { return PiiValueSet<T,7>(v1, v2, v3, v4, v5, v6, v7); }
  template <class T> PiiValueSet<T,8> inline valueSet(T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8)
  { return PiiValueSet<T,8>(v1, v2, v3, v4, v5, v6, v7, v8); }
  template <class T> PiiValueSet<T,9> inline valueSet(T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8, T v9)
  { return PiiValueSet<T,9>(v1, v2, v3, v4, v5, v6, v7, v8, v9); }
  template <class T> PiiValueSet<T,10> inline valueSet(T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8, T v9, T v10)
  { return PiiValueSet<T,10>(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10); }
}

/// @endcond

#endif //_PIIVALUESET_H
