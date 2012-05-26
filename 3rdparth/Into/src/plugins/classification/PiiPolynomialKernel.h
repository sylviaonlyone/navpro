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

#ifndef _PIIPOLYNOMIALKERNEL_H
#define _PIIPOLYNOMIALKERNEL_H

/**
 * Polynomial kernel function. The polynomial kernel is defined as
 * @f$k(x,y) = (\alpha + \beta \langle x, y \rangle)^d@f$, where @e x
 * and @e y are vectors of any dimensionality. The user-specified
 * constants of the function can be interpreted as follows:
 *
 * @li @f$\alpha@f$ - an offset. Setting this value to 0 (the default)
 * results in a homogeneous polynomial kernel. The offset must be a
 * non-negative value to satisfy Mercer's condition.
 *
 * @li @f$\beta@f$ - a scale. This is a convenient way of normalizing
 * the input vectors without actually modifying them.
 *
 * @li @e d - the degree of the polynomial, a positive integer.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class FeatureIterator> class PiiPolynomialKernel
{
public:
  /**
   * Constructs a new polynomial kernel function.
   */
  PiiPolynomialKernel() : _dAlpha(0), _dBeta(1), _iDegree(2) {}

  /**
   * Sets the value of @f$\alpha@f$ to @a offset. The default value is
   * zero.
   */
  void setOffset(double offset) { _dOffset = offset; }
  /**
   * Returns the current offset.
   */
  double offset() const { return _dOffset; }
  /**
   * Sets the value of @f$\beta@f$ to @a scale. The default value is
   * one.
   */
  void setScale(double scale) { _dScale = scale; }
  /**
   * Returns the current scale.
   */
  double scale() const { return _dScale; }
  /**
   * Sets the degree of the polynomial to @a degree. The default value
   * is two.
   */
  void setDegree(int degree) { _iDegree = degree; }
  /**
   * Returns the current degree of the polynomial.
   */
  int degree() const { return _iDegree; }
  
  inline double operator() (FeatureIterator sample, FeatureIterator model, int length) const throw()
  {
    return Pii::pow(_dAlpha + _dBeta * Pii::innerProductN(sample, length, model, 0.0), _iDegree)
  }
private:
  double _dOffset;
  double _dScale;
  int _iDegree;
}	

#endif //_PIIPOLYNOMIALKERNEL_H
