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

#ifndef _PIIFRACTION_H
#define _PIIFRACTION_H

#include "PiiBits.h"
#include "PiiMath.h"
#include <cmath>

#define OPERATOR(op) bool operator op (PiiFraction other) const { return operator- (other).numerator op 0; } \
  bool operator op (T value) const { return operator- (value).numerator op 0; }


/**
 * A template class that represents fractions. Fractions are composed
 * of a numerator and a denominator, the numerical value given by
 * numerator/denominator. The template accepts any integer type.
 *
 * Fractions are automatically reduced. This imposes a performance
 * penalty, but prevents some overflows that would otherwise occur
 * rather often.
 *
 * @ingroup Core
 */
template <class T> class PiiFraction
{
public:
  /**
   * Create a new fraction with the given numerator and denominator. 
   * The default denominator is one, which makes it possible to use a
   * fraction just like an integer.
   *
   * @param num the numerator
   * @param denom the denominator
   */
  PiiFraction(T num = 0, T denom = 1) :
    numerator(num), denominator(denom)
  {
    fix();
    reduce();
  }

  /**
   * Create a fraction out of a floating-point number. This function
   * uses a recursive formula that uses continued fractions to build
   * an approximation for the floating-point number:
   *
   * @f[
   * f = n_0 + \frac{1}{n_1 + \frac{1}{n_2 + \frac{1}{\cdots}}}
   * @f]
   *
   * @param number a floating point number
   *
   * @param accuracy required accuracy for the approximation. This
   * value does not determine the final accuracy of the estimation,
   * but instead the accuracy of the lowest level in the continued
   * fraction. The error will build up.
   *
   * @param iterations the maximum number of recursions
   */
  static PiiFraction create(double number, double accuracy = 1e-5, unsigned int recursions = 20)
  {
    long n = static_cast<long>(number);
    double f = number - n;
    PiiFraction result(static_cast<T>(n));

    // Not quite cardinal yet -> recurse
    if (recursions > 0 && ::fabs(f) > accuracy)
      {
        // Invert the fractional part and try to form a fraction out
        // of it.
        PiiFraction subFraction = create(1.0/f, accuracy, recursions-1);
        subFraction.invert();
        result += subFraction;
      }
    
    return result;
  }

  /**
   * Return true if the denominator is different from zero.
   */
  bool isValid() const { return denominator != 0; }

  /**
   * Invert the number. Exchanges numerator to denominator.
   */
  void invert() { qSwap(numerator, denominator); }

  /**
   * Return an invert version of the fraction. Exchanges numerator to
   * denominator.
   */
  PiiFraction inverted()
  {
    PiiFraction result(denominator, numerator);
    result.fix();
    return result;
  }

  PiiFraction operator+ (const PiiFraction& other) const
  {
    if (other.denominator != denominator)
      return PiiFraction(numerator*other.denominator + other.numerator*denominator,
                         denominator * other.denominator);
    else
      return PiiFraction(numerator + other.numerator, denominator);
  }
  
  PiiFraction operator- (const PiiFraction& other) const
  {
    if (other.denominator != denominator)
      return PiiFraction(numerator*other.denominator - other.numerator*denominator,
                         denominator * other.denominator);
    else
      return PiiFraction(numerator - other.numerator, denominator);
  }

  PiiFraction operator* (const PiiFraction& other) const
  {
    return PiiFraction(numerator * other.numerator, denominator * other.denominator);
  }

  PiiFraction operator/ (const PiiFraction& other) const
  {
    return PiiFraction(numerator * other.denominator, denominator * other.numerator);
  }

  void operator+= (const PiiFraction& other)
  {
    if (other.denominator != denominator)
      {
        numerator = numerator*other.denominator + other.numerator*denominator;
        denominator *= other.denominator;
        reduce();
      }
    else
      numerator +=  other.numerator;
  }

  void operator-= (const PiiFraction& other)
  {
    if (other.denominator != denominator)
      {
        numerator = numerator*other.denominator - other.numerator*denominator;
        denominator *= other.denominator;
        reduce();
      }
    else
      numerator -= other.numerator;
  }
  
  void operator*= (const PiiFraction& other)
  {
    numerator *= other.numerator;
    denominator *= other.denominator;
    reduce();
  }

  void operator/= (const PiiFraction& other)
  {
    numerator *= other.denominator;
    denominator *= other.numerator;
    fix();
    reduce();
  }

  PiiFraction operator+ (T value) const
  {
    return PiiFraction(numerator + value*denominator, denominator);
  }
  PiiFraction operator- (T value) const
  {
    return PiiFraction(numerator - value*denominator, denominator);
  }
  PiiFraction operator* (T value) const
  {
    return PiiFraction(value*numerator, denominator);
  }
  PiiFraction operator/ (T value) const
  {
    return PiiFraction(numerator, denominator*value);
  }
  void operator+= (T value)
  {
    numerator += value*denominator;
  }
  void operator-= (T value)
  {
    numerator -= value*denominator;
  }
  void operator*= (T value)
  {
    numerator *= value;
  }
  void operator/= (T value)
  {
    denominator *= value;
    fix();
    reduce();
  }

  PiiFraction operator- () const { return PiiFraction(-numerator, denominator); }

  OPERATOR(<)
  OPERATOR(>)
  OPERATOR(>=)
  OPERATOR(<=)
  OPERATOR(==)
  OPERATOR(!=)
   
  /**
   * Reduce the fraction by dividing both components by the greatest
   * common divisor (gcd).
   */
  void reduce()
  {
    T gcd = Pii::gcd(numerator, denominator);
    numerator /= gcd;
    denominator /= gcd;
  }

  /**
   * Create a reduced fraction by dividing both components by the gcd.
   */
  PiiFraction reduced() const
  {
    T gcd = Pii::gcd(numerator, denominator);
    return PiiFraction(numerator / gcd, denominator / gcd);
  }

  /**
   * Return the absolute value of this fraction.
   */
  PiiFraction abs() { return PiiFraction(Pii::abs(numerator), denominator); }
  
  /**
   * Get an integer approximation of the fraction. If the denominator
   * is zero, 0 will be returned.
   */
  operator T() const
  {
    if (denominator == 0) return 0;
    return numerator/denominator;
  }

  /**
   * Get a floating-point approximation of the fraction. If
   * denominator is zero, @p INFINITY will be returned.
   */
  operator double() const { return (double)numerator/denominator; }

  /**
   * Cast this fraction to another type.
   */
  template <class U> operator PiiFraction<U>() const
  {
    return PiiFraction<U>((U)numerator, (U)denominator);
  }

  /**
   * The numerator.
   */
  T numerator;
  /**
   * The denominator.
   */
  T denominator;

private:
  /**
   * Move the (possible) sign bit to the numerator.
   */
  void fix()
  {
    if (denominator < 0)
      {
        numerator = -numerator;
        denominator = -denominator;
      }
  }
};

#undef OPERATOR

#endif //_PIIFRACTION_H
