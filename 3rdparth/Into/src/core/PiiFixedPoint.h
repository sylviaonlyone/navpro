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

#ifndef _PIIFIXEDPOINT_H
#define _PIIFIXEDPOINT_H

/*
 * A structure that defines the type used for intermediate values in
 * calculations that require more precision than the original type.
 */
template <class T> struct PiiFixedPointTraits;
template <> struct PiiFixedPointTraits<char> { typedef short WiderType; };
template <> struct PiiFixedPointTraits<short> { typedef int WiderType; };
// Both int and long are 32 bits on PC
template <> struct PiiFixedPointTraits<int> { typedef long long WiderType; };
template <> struct PiiFixedPointTraits<long> { typedef long long WiderType; };

/**
 * A replacement for float/double numbers. In some occasions,
 * computations are performed faster with fixed point than with
 * floating point numbers. The speed gain is, however, very dependent
 * on processor architecture. In general, fixed point addition and
 * subtraction are always faster, but the same does not necessarily
 * apply to multiplication and division. The Pentium processor, for
 * example, performs floating point multiplication and division faster
 * than their integer correspondents. The downside is that floating
 * point calculations cannot be fully pipelined. Thus, consequtive
 * fixed multiplications may end up to be faster. Converting a fixed
 * point number back to a float costs a division. Therefore, consider
 * your requirements carefully.
 *
 * The @p decimals template parameter tell the number of decimal bits. 
 * Typically, half of the data type is used (16 for int on PC). Use
 * the number to tune the trade-off between precision and range.
 *
 * @ingroup Core
 */
template <int decimals, class T = int> class PiiFixedPoint
{
public:
  /**
   * Initialize the fixed point value to zero.
   */
  PiiFixedPoint() : _value(0) {}
  /**
   * Initialize with an integer value.
   *
   * @param value the value as an integer
   *
   * @param shift the number of decimal bits to add to the number. @p
   * value will be shifted this many times to the left.
   */
  PiiFixedPoint(T value, int shift = decimals) : _value(value << shift) {}
  /**
   * Copy constructor.
   */
  PiiFixedPoint(const PiiFixedPoint& other) : _value(other._value) {}
  /**
   * Initialize the fixed point number with a floating point value. 
   * This may result in rounding errors.
   */
  PiiFixedPoint(double value) : _value(value * (1 << decimals)) {}

  operator float () const { return (float)_value / (1 << decimals); }
  operator double () const { return (double)_value / (1 << decimals); }  
  
  void operator+= (const PiiFixedPoint& other) { _value += other._value; }
  void operator-= (const PiiFixedPoint& other) { _value -= other._value; }
  void operator*= (const PiiFixedPoint& other) { _value = T(PiiFixedPointTraits<T>::WiderType(_value * other._value) >> decimals); }
  void operator/= (const PiiFixedPoint& other) { _value = (_value / other._value) << decimal; }

  PiiFixedPoint operator+ (const PiiFixedPoint& other) { return PiiFixedPoint(_value + other_value, 0); }
  PiiFixedPoint operator- (const PiiFixedPoint& other) { return PiiFixedPoint(_value - other_value, 0); }
  PiiFixedPoint operator* (const PiiFixedPoint& other) { return PiiFixedPoint(T(PiiFixedPointTraits<T>::WiderType(_value * other_value) >> decimals), 0); }
  PiiFixedPoint operator/ (const PiiFixedPoint& other) { return PiiFixedPoint(_value / other_value); }

private:
  T _value;
};

//PENDING conversion to int (requires arithmetic shift), calculations/conversions betwee numbers with different number of decimals

#endif //_PIIFIXEDPOINT_H
