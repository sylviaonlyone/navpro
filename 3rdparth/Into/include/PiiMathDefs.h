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

/**
 * @file
 *
 * Helpful definitions for mathematical routines. Defines mathematical
 * constants missing from some compilers.
 */

#ifndef _PIIMATHDEFS_H
#define _PIIMATHDEFS_H

#define _USE_MATH_DEFINES //for VC
#if _MSC_VER >= 1500
#  include <math.h>
#else
#  include <cmath>
#endif
#include <limits>

#ifndef M_E
#  define M_E            2.7182818284590452354   /* e */
#  define M_LOG2E        1.4426950408889634074   /* log_2 e */
#  define M_LOG10E       0.43429448190325182765  /* log_10 e */
#  define M_LN2          0.69314718055994530942  /* log_e 2 */
#  define M_LN10         2.30258509299404568402  /* log_e 10 */
#  define M_PI           3.14159265358979323846  /* pi */
#  define M_PI_2         1.57079632679489661923  /* pi/2 */
#  define M_PI_4         0.78539816339744830962  /* pi/4 */
#  define M_1_PI         0.31830988618379067154  /* 1/pi */
#  define M_2_PI         0.63661977236758134308  /* 2/pi */
#  define M_2_SQRTPI     1.12837916709551257390  /* 2/sqrt(pi) */
#  define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#  define M_SQRT1_2      0.70710678118654752440  /* 1/sqrt(2) */
#endif

#ifdef min
#  undef min
#endif

#ifdef max
#  undef max
#endif

// Redefine inf and nan, because MinGW has worse definitions.
#ifdef __MINGW32__
#  undef INFINITY
#  undef NAN
#endif

#ifndef INFINITY
/**
 * Double-precision infinity. Some systems don't have the C99-standard
 * macro @p INFINITY defined. Gcc does as an extension, but MSVC
 * doesn't.
 */
#  define INFINITY std::numeric_limits<double>::infinity()
#endif

#ifndef NAN
/**
 * Double-precision not-a-number. Some systems don't have the
 * C99-standard macro @p NAN defined. Gcc does as an extension, but
 * MSVC doesn't.
 */
#  define NAN std::numeric_limits<double>::quiet_NaN()
#endif


namespace Pii
{
  /**
   * Returns true if @p f equals Inf.
   */
  template <class T> inline bool isInf(T f) { return f == std::numeric_limits<T>::infinity(); }
  
  /**
   * Returns true if @p f is NaN.
   */
  template <class T> inline bool isNan(T f) { return f != f; }
  
  /**
   * Utility for miscellaneous numerical property definitions.
   */
  template<class T> struct Numeric
  {
    /// Defines default tolerance for differences when comparing values.
    static T tolerance() {return 0;}

    /// Returns maximum finite value for given type.
    static T maxValue() { return std::numeric_limits<T>::max(); }

    /// Returns minimum finite value for given type.
    static T minValue() { return std::numeric_limits<T>::min(); }

    /// Returns the smallest possible positive value.
    static T smallestPositive() { return 1; }
  };

  #define IMPLEMENT_FLOATING_SPECIALIZATION(Type, tol) \
  template<> struct Numeric<Type> \
  { \
    static Type tolerance() {return tol;} \
    static Type maxValue() {return std::numeric_limits<Type>::max();} \
    static Type minValue() {return -std::numeric_limits<Type>::max();} \
    static Type smallestPositive() {return std::numeric_limits<Type>::min();} \
  };

  IMPLEMENT_FLOATING_SPECIALIZATION(float, 1e-6f)
  IMPLEMENT_FLOATING_SPECIALIZATION(double, 1e-10)
  IMPLEMENT_FLOATING_SPECIALIZATION(long double, 1e-10)

  #undef IMPLEMENT_FLOATING_SPECIALIZATION

} //namespace Pii



#endif //_PIIMATHDEFS_H
