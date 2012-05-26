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

#ifndef _PIITHRESHOLDING_TEMPLATES_H
#define _PIITHRESHOLDING_TEMPLATES_H

#include <PiiMatrix.h>
#include "PiiHistogram.h"

namespace PiiImage
{
  template <class T> int otsuThreshold(const PiiMatrix<T>& histogram)
	{
    PiiMatrix<double> matVariances(1,histogram.columns()-1);
    PiiMatrix<double> matCumulative(cumulative(histogram));
    double* varptr = matVariances.row(0);
    
    for ( int k=0; k<matVariances.columns(); k++ )
      {
        double sum1 = 0, sum2 = 0, myy1 = 0, myy2 = 0, var1 = 0, var2 = 0;
        sum1 = matCumulative(0,k);

        if ( sum1 != 0 )
          {
            for ( int i=0; i<=k; i++ )
              myy1 += i*histogram(i);
            myy1 /= sum1;

            for ( int i=0; i<=k; i++ )
              {
                double temp = (i-myy1);
                var1 += temp*temp*histogram(i);
              }
          }

        if ( sum1 != 1 )
          {
            sum2 = 1.0-sum1;
            for ( int i=k+1; i<histogram.columns(); i++ )
              myy2 += i*histogram(i);
            myy2 /= sum2;

            
            for ( int i=k+1; i<histogram.columns(); i++ )
              {
                double temp = (i - myy2);
                var2 += temp*temp*histogram(i);
              }
          }

        
        varptr[k] = var1 + var2;
      }

    int th = 0;
    double min = matVariances(0);
    for ( int c=1; c<matVariances.columns(); c++ )
      if ( varptr[c] < min )
        {
          min = varptr[c];
          th = c;
        }

    return th;
	}

  template <class Matrix, class PixelCounter, class BinaryFunction>
  PiiMatrix<typename BinaryFunction::result_type> adaptiveThresholdImpl(const PiiRandomAccessMatrix& image,
                                                                        const PixelCounter& counter,
                                                                        BinaryFunction func,
                                                                        int windowRows, int windowColumns)
  {
    typedef typename Matrix::value_type T;
    // Use at least int accuracy for the integral image
    typedef typename Pii::Combine<T,int>::Type I;
    // Use at least float for the mean.
    typedef typename Pii::Combine<T,float>::Type M;
    typedef typename Matrix::const_row_iterator Iterator;

    // Calculate an integral image
    PiiMatrix<I> matIntegral = Pii::cumulativeSum<I,Matrix>(image, Pii::ZeroBorderCumulativeSum);
    // Initialize result image
    PiiMatrix<T> matThresholded(PiiMatrix<T>::uninitialized(image.rows(), image.columns()));
    if (windowColumns <= 0)
      windowColumns = windowRows;
    int iHalfRows = windowRows/2, iHalfCols = windowColumns/2;
    const int iRows = image.rows(), iCols = image.columns();
    int c1, c2, r1, r2;

    const I* pPrevRow, *pNextRow;
    T* pTarget;
    for (int r=0; r<iRows; ++r)
      {
        // Check image boundaries
        r1 = qMax(r-iHalfRows, 0);
        r2 = qMin(r+iHalfRows+1, iRows);
        pPrevRow = matIntegral[r1];
        pNextRow = matIntegral[r2];
        pTarget = matThresholded[r];
        Iterator pSource = image[r];
        for (int c=0; c<iCols; ++c)
          {
            c1 = qMax(c-iHalfCols, 0);
            c2 = qMin(c+iHalfCols+1, iCols);
            // Use the integral image to calculate moving average.
            pTarget[c] = func(M(pSource[c]),
                              M(pNextRow[c2] + pPrevRow[c1] - pNextRow[c1] - pPrevRow[c2]) /
                              counter.countPixels(r1, c1, r2, c2));
          }
      }
    return matThresholded;
  }

  template <class T> struct RoiMaskFunction : Pii::BinaryFunction<T,bool,T>
  {
    T operator() (T value, bool mask) const { return mask ? value : T(0); }
  };

  // Uses cumulative sum of mask matrix to quickly count handled
  // pixels in a local window.
  struct RoiMaskPixelCounter
  {
    RoiMaskPixelCounter(const PiiMatrix<bool>& mask) :
      matMaskSum(Pii::cumulativeSum<int>(mask, Pii::ZeroBorderCumulativeSum))
    {}
    
    inline int countPixels(int r1, int c1, int r2, int c2) const
    {
      return matMaskSum(r2,c2) + matMaskSum(r1,c1) - matMaskSum(r2,c1) - matMaskSum(r1,c2);
    }

    PiiMatrix<int> matMaskSum;
  };

  struct DefaultPixelCounter
  {
    inline int countPixels(int r1, int c1, int r2, int c2) const { return (c2-c1) * (r2-r1); }
  };

  template <class Matrix, class BinaryFunction>
  PiiMatrix<typename BinaryFunction::result_type> adaptiveThreshold(const PiiRandomAccessMatrix& image,
                                                                    BinaryFunction func,
                                                                    float relativeThreshold,
                                                                    float absoluteThreshold,
                                                                    int windowRows, int windowColumns)
  {
    return adaptiveThreshold(image,
                             adaptiveThresholdFunction(func, relativeThreshold, absoluteThreshold),
                             windowRows,
                             windowColumns);
  }
  
  template <class Matrix, class BinaryFunction>
  PiiMatrix<typename BinaryFunction::result_type> adaptiveThreshold(const PiiRandomAccessMatrix& image,
                                                                    BinaryFunction func,
                                                                    int windowRows, int windowColumns)
  {
    return adaptiveThresholdImpl(image, DefaultPixelCounter(), func, windowRows, windowColumns);
  }

  template <class Matrix, class BinaryFunction>
  PiiMatrix<typename BinaryFunction::result_type> adaptiveThreshold(const PiiRandomAccessMatrix& image,
                                                                    const PiiMatrix<bool>& roiMask,
                                                                    BinaryFunction func,
                                                                    int windowRows, int windowColumns)
  {
    return adaptiveThresholdImpl(Pii::binaryMatrixTransform(image.selfRef(),
                                                            roiMask,
                                                            RoiMaskFunction<typename Matrix::value_type>()),
                                 RoiMaskPixelCounter(roiMask),
                                 func,
                                 windowRows, windowColumns);
  }

  template <class TernaryFunction, class T>
  PiiMatrix<typename TernaryFunction::result_type> adaptiveThresholdVar(const PiiMatrix<T>& image,
                                                                        TernaryFunction func,
                                                                        int windowRows, int windowColumns)
  {
    /* Variance s² = 1/N sum(x - u)²
     *
     * Ns² = sum(x - u)²
     *     = sum(x² - 2xu + u²)
     *     = sum(x²) - 2u sum(x) + N u²
     *     = sum(x²) - 2 sum(x)/N sum(x) + N (sum(x) / N)²
     *     = sum(x²) - 2 sum²(x) / N + sum²(x) / N
     *     = sum(x²) - sum²(x)/N
     *  s² = sum(x²)/N - sum²(x)/N² = sum(x²)/N - u²
     */

    // Use at least int accuracy for the integral image
    typedef typename Pii::Combine<T,int>::Type I;
    // Use at least long long accuracy for the integral image of squares
    typedef typename Pii::Combine<T,long long>::Type I2;

    // Calculate an integral image
    PiiMatrix<I> matIntegral = Pii::cumulativeSum<I,PiiMatrix<T> >(image, Pii::ZeroBorderCumulativeSum);
    // Integral image of squares
    PiiMatrix<I2> matIntegral2 = Pii::cumulativeSum(image, Pii::Square<I2>(), Pii::ZeroBorderCumulativeSum);
    
    // Initialize result image
    PiiMatrix<T> matThresholded(PiiMatrix<T>::uninitialized(image.rows(), image.columns()));
    if (windowColumns <= 0)
      windowColumns = windowRows;
    int iHalfRows = windowRows/2, iHalfCols = windowColumns/2;
    const int iRows = image.rows(), iCols = image.columns();
    int c1, c2, r1, r2;

    const I* pPrevRow, *pNextRow;
    const I2* pPrevRow2, *pNextRow2;
    const T* pSource;
    T* pTarget;
    for (int r=0; r<iRows; ++r)
      {
        // Check image boundaries
        r1 = qMax(r-iHalfRows, 0);
        r2 = qMin(r+iHalfRows+1, iRows);
        pPrevRow = matIntegral[r1];
        pNextRow = matIntegral[r2];
        pPrevRow2 = matIntegral2[r1];
        pNextRow2 = matIntegral2[r2];
        pTarget = matThresholded[r];
        pSource = image[r];
        for (int c=0; c<iCols; ++c)
          {
            c1 = qMax(c-iHalfCols, 0);
            c2 = qMin(c+iHalfCols+1, iCols);
            int iCount = (c2-c1) * (r2-r1);
            double dMean = double(pNextRow[c2] + pPrevRow[c1] - pNextRow[c1] - pPrevRow[c2]) / iCount;
            double dVar = double(pNextRow2[c2] + pPrevRow2[c1] - pNextRow2[c1] - pPrevRow2[c2]) / iCount // sum(x²)/N
              - Pii::square(dMean);
            
            pTarget[c] = func(pSource[c],
                              dMean,
                              dVar);
          }
      }
    return matThresholded;
    
  }
}

#endif //_PIITHRESHOLDING_TEMPLATES_H
