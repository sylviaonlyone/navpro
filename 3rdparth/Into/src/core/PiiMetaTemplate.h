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

#ifndef _PIIMETATEMPLATE_H
#define _PIIMETATEMPLATE_H

/// @file

namespace Pii
{
  /**
   * @name Template meta-programming
   *
   * Template meta-programs are structures the compiler evaluates. 
   * They can be used in creating efficient code, for example control
   * structures that are evaluated at compile time.
   */

  //@{
  
  /**
   * A conditional template. If @p condition is true, If::Type is
   * equal to @p Then. The template specialization for condition ==
   * false makes @p Type equal to @p Else.
   *
   * @code
   * template <class T> void myFunc()
   * {
   *   // if T is four bytes, MyType is int, otherwise char
   *   typedef Pii::If<sizeof(T) == 4, int, char>::Type MyType;
   *   MyType myVal(0);
   * }
   * @endcode
   */
  template <bool condition, class Then, class Else>
  struct If
  {
    typedef Then Type;
  };

  template <class Then, class Else>
  struct If<false, Then, Else>
  {
    typedef Else Type;
  };

  /**
   * A conditional template that accepts a type name as the condition
   * parameter. There condition class must declare a static constant
   * called @p boolValue. Otherwise, functions like If.
   *
   * @code
   * template <class T, class U> void myFunc()
   * {
   *   // if T is four bytes and U is two bytes, MyType is int, otherwise long
   *   typedef Pii::IfClass<Pii::And<sizeof(T) == 4,
   *                        sizeof(U) == 2>,
   *                        int, long>::Type MyType;
   *   MyType myVal(0);
   * }
   * @endcode
   */
  template <class Condition, class Then, class Else>
  struct IfClass : public If<Condition::boolValue, Then, Else>
  {};

  /**
   * A structure whose constant @p boolValue member evaluates to @p
   * true.
   */
  struct True
  {
    enum { boolValue = true };
  };

  /**
   * A structure whose constant @p boolValue member evaluates to @p
   * false.
   */
  struct False
  {
    enum { boolValue = false };
  };

  /**
   * Logical not operation. The template parameter must be a boolean
   * value.
   *
   * @code
   * ASSERT(Not<false>::boolValue == true);
   * @endcode
   */
  template <bool b> struct Not
  {
    enum { boolValue = !b };
  };

  /**
   * Logical not operation. The template parameter must be a valid
   * static logical operator.
   *
   * @code
   * ASSERT(NotClass<False>::boolValue == true);
   * ASSERT(NotClass<True>::boolValue == false);
   * @endcode
   */
  template <class T> struct NotClass
  {
    enum { boolValue = !T::boolValue };
  };
  
  /**
   * A template that performs a logical AND operation on its boolean
   * template parameters. The @p boolValue member evaluates to @p true
   * if all of the template parameters evaluate to true.
   */
  template <bool b1, bool b2, bool b3 = true, bool b4 = true, bool b5 = true, bool b6 = true, bool b7 = true>
  struct And : False {};

  template <>
  struct And<true, true, true, true, true, true, true> : True {};

  /**
   * A template that performs a logical OR operation on its boolean
   * template parameters. The @p boolValue member evaluates to @p true
   * if any of the template parameters evaluate to true.
   */
  template <bool b1, bool b2, bool b3 = false, bool b4 = false, bool b5 = false, bool b6 = false, bool b7 = false>
  struct Or : True {};

  template <>
  struct Or<false, false, false, false, false, false, false> : False {};

  template <int i> struct IntIdentity
  {
    enum { intValue = i };
  };
  
  template <int a, int b> struct MinInt : If<(a <= b), IntIdentity<a>, IntIdentity<b> >::Type
  {};

  template <int a, int b> struct MaxInt : If<(a >= b), IntIdentity<a>, IntIdentity<b> >::Type
  {};

  //@}
}

#endif //_PIIMETATEMPLATE_H
