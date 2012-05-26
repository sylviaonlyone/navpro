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

#ifndef _PIIGAUSSIANKERNEL_H
#define _PIIGAUSSIANKERNEL_H

/**
 * Gaussian kernel function. The Gaussian kernel is defined as
 * @f$k(x,y) = e^-\fraq{||x-y||^2}{2\sigma^2}@f$, where @e x and @e y
 * are vectors of any dimensionality. It is also known as the radial
 * basis function (RBF) kernel.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class FeatureIterator> class PiiGaussianKernel
{
public:
  /**
   * Constructs a new Gaussian kernel function.
   */
  PiiGaussianKernel() : _dSigma(1), _dNormalizer(0.5) {}

  /**
   * Sets the @f$\sigma@f$ of the Gaussian function to @a sigma. This
   * value controls the "width" of the Gaussian bell curve. A large @a
   * sigma means generates a wide, flat curve. As @a sigma approaches
   * zero, the curve will approach the delta function. The default
   * value is 1.
   */
  void setSigma(double sigma)
  {
    _dSigma = sigma;
    _dNormalizer = 1.0/(2*sigma*sigma);
  }
  
  /**
   * Returns the value of @f$\sigma@f$.
   */
  double sigma() const { return _dSigma; }
  
  inline double operator() (FeatureIterator sample, FeatureIterator model, int length) const throw()
  {
    return Pii::exp(-Pii::squaredDistanceN(sample, length, model, 0.0) * _dNormalizer);
  }
private:
  double _dSigma, _dNormalizer;
};

#endif //_PIIGAUSSIANKERNEL_H
