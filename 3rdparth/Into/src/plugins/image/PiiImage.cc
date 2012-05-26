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

#include "PiiImage.h"
#include <QDebug>
#include <PiiMatrixUtil.h>

namespace PiiImage
{
  PiiMatrix<int> sobelX(3, 3,
                        -1, 0, 1,
                        -2, 0, 2,
                        -1, 0, 1);
  PiiMatrix<int> sobelY(3, 3,
                        -1,-2,-1,
                        0, 0, 0,
                        1, 2, 1);

  PiiMatrix<int> robertsX(2, 2,
                          1, 0,
                          0, -1);
  PiiMatrix<int> robertsY(2, 2,
                          0, 1,
                          -1, 0);

  PiiMatrix<int> prewittX(3, 3,
                          -1, 0, 1,
                          -1, 0, 1,
                          -1, 0, 1);
  PiiMatrix<int> prewittY(3, 3,
                          -1,-1,-1,
                          0, 0, 0,
                          1, 1, 1);
  
  PiiMatrix<double> makeGaussian(unsigned int size)
  {
    size |= 1; // make odd
    PiiMatrix<double> result(PiiMatrix<double>::uninitialized(size,size));
    int center = size/2;
    // Cut the filter at 3 times std
    double sigma = (double)center/3.0;
    double exponentScale = -0.5/(sigma*sigma);
    double scale = 0.5 / (M_PI*sigma*sigma);
    double sum = 0;
    for (int r=-center; r<=center; r++)
      for (int c=-center; c<=center; c++ )
        {
          double val = scale * exp((r*r + c*c) * exponentScale);
          sum += val;
          result(r+center,c+center) = val;
        }
    result /= sum;
    return result;
  }

  PiiMatrix<double> makeLoGaussian(unsigned int size)
  {
    size |= 1; // make odd
    PiiMatrix<double> result(PiiMatrix<double>::uninitialized(size,size));
    int center = size/2;
    // Cut the filter at 3 times std
    double sigma = (double)center/3.0;
    double exponentScale = -0.5/(sigma*sigma);
    double scale = -1.0 / (M_PI*std::pow(sigma,4));
    double mean = 0;
    for (int r=-center; r<=center; r++)
      for (int c=-center; c<=center; c++ )
        {
          double sqDist = c*c + r*r;
          double val = scale * (1 + sqDist*exponentScale) * exp(sqDist * exponentScale);
          mean += val;
          result(r+center,c+center) = val;
        }
                                               
    result -= mean/(size*size);
    return result;
  }

  static inline bool nonZeroSums(int iSum, double dSum)
  {
    return iSum != 0 && !Pii::almostEqualRel(dSum, 0.0) &&
      Pii::sign(iSum) == Pii::sign(dSum);
  }

  PiiMatrix<int> intFilter(const PiiMatrix<int>& image,
                           const PiiMatrix<double>& doubleFilter,
                           Pii::ExtendMode mode,
                           double scale)
  {
    if (scale == 0)
      {
        double dMax = Pii::maxAbsAll(doubleFilter);
        if (dMax != 0)
          scale = 256.0 / dMax;
      }
    
    PiiMatrix<int> integerFilter(Pii::round<int>(doubleFilter * scale));
    int iSum = Pii::sumAll<int>(integerFilter);
    double dSum = Pii::sumAll<double>(doubleFilter);
    //qDebug("%d %lg", iSum, dSum);
    if (nonZeroSums(iSum, dSum))
      scale = iSum / dSum;
    PiiMatrix<int> filtered = filter<int>(image, integerFilter, mode);
    filtered.map(Pii::unaryCompose(Pii::Round<double,int>(),
                                   std::bind2nd(std::multiplies<double>(),
                                                1.0/scale)));
    return filtered;
  }

  PiiMatrix<int> intFilter(const PiiMatrix<int>& image,
                           const PiiMatrix<double>& horizontalFilter,
                           const PiiMatrix<double>& verticalFilter,
                           Pii::ExtendMode mode,
                           double scale)
  {
    if (horizontalFilter.rows() != 1 || verticalFilter.columns() != 1)
      return image;

    double hMax = Pii::maxAbsAll(horizontalFilter), vMax = Pii::maxAbsAll(verticalFilter);
    if (hMax == 0) hMax = 1;
    if (vMax == 0) vMax = 1;
      
    double dHScale = scale == 0 ? 64.0 / hMax : scale;
    double dVScale = scale == 0 ? 64.0 / vMax : scale;

    // Scale the filters and round to int
    PiiMatrix<int> horizontalIntegerFilter(Pii::round<int>(horizontalFilter * dHScale));
    PiiMatrix<int> verticalIntegerFilter(Pii::round<int>(verticalFilter * dVScale));

    // Recalculate the real scale (after rounding)
    int iHSum = Pii::sumAll<int>(horizontalIntegerFilter),
      iVSum = Pii::sumAll<int>(verticalIntegerFilter);
    double dHSum = Pii::sumAll<double>(horizontalFilter),
      dVSum = Pii::sumAll<double>(verticalFilter);

    if (nonZeroSums(iHSum, dHSum))
      dHScale = double(iHSum) / dHSum;
    if (nonZeroSums(iVSum, dVSum))
      dVScale = double(iVSum) / dVSum;

    if (mode == Pii::ExtendZeros)
      {
        PiiMatrix<int> filtered = PiiDsp::filter<int>(PiiDsp::filter<int>(image,
                                                                          horizontalIntegerFilter,
                                                                          PiiDsp::FilterOriginalSize),
                                                      verticalIntegerFilter,
                                                      PiiDsp::FilterOriginalSize);
        // Readable? Not. Scales each element as doubles and rounds
        // the result to an int.
        filtered.map(Pii::unaryCompose(Pii::Round<double,int>(),
                                       std::bind2nd(std::multiplies<double>(),
                                                    1.0/(dVScale*dHScale))));
        return filtered;
      }

    const int rows = verticalFilter.rows() >> 1, cols = horizontalFilter.columns() >> 1;
    PiiMatrix<int> filtered = PiiDsp::filter<int>(PiiDsp::filter<int>(Pii::extend(image, rows, rows, cols, cols, mode),
                                                                      horizontalIntegerFilter,
                                                                      PiiDsp::FilterValidPart),
                                                  verticalIntegerFilter, PiiDsp::FilterValidPart);
    // Scale back, see above
    filtered.map(Pii::unaryCompose(Pii::Round<double,int>(),
                                   std::bind2nd(std::multiplies<double>(),
                                                1.0/(dVScale*dHScale))));
    return filtered;
  }

  PiiMatrix<float> createRotationTransform(float theta)
  {
    float fSin = Pii::sin(theta), fCos = Pii::cos(theta);
    return PiiMatrix<float>(3,3,
                            fCos, -fSin, 0.0,
                            fSin, fCos, 0.0,
                            0.0, 0.0, 1.0);
  }
  
  PiiMatrix<float> createRotationTransform(float theta, float centerX, float centerY)
  {
    // Move origin to the new center, rotate, and move back
    return createTranslationTransform(centerX, centerY) *
      createRotationTransform(theta) *
      createTranslationTransform(-centerX, -centerY);
  }

  PiiMatrix<float> createShearingTransform(float shearX, float shearY)
  {
    return PiiMatrix<float>(3,3,
                            1.0, shearX, 0.0,
                            shearY, 1.0, 0.0,
                            0.0, 0.0, 1.0);
  }

  PiiMatrix<float> createTranslationTransform(float x, float y)
  {
    return PiiMatrix<float>(3,3,
                            1.0, 0.0, x,
                            0.0, 1.0, y,
                            0.0, 0.0, 1.0);
  }

  PiiMatrix<float> createScalingTransform(float scaleX, float scaleY)
  {
    return PiiMatrix<float>(3,3,
                            scaleX, 0.0, 0.0,
                            0.0, scaleY, 0.0,
                            0.0, 0.0, 1.0);
  }
}
