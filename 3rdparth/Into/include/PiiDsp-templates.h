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

#ifndef _PIIDSP_H
# error "Never use <PiiDsp-templates.h> directly; include <PiiDsp.h> instead."
#endif

#include <PiiMatrixUtil.h>

namespace PiiDsp
{
  
  template <class ResultType, class T, class U>
  PiiMatrix<ResultType> filter(const PiiMatrix<T>& a,
                               const PiiMatrix<U>& b,
                               FilterMode mode)
  {
    PiiMatrix<U> func(Pii::flipped(b, Pii::Horizontally | Pii::Vertically));
    return convolution<ResultType>(a, func, mode);
  }
  
  template <class ResultType, class T, class U>
  PiiMatrix<ResultType> correlation(const PiiMatrix<T>& a,
                                    const PiiMatrix<U>& b,
                                    FilterMode mode)
  {
    PiiMatrix<U> func(b);
    Pii::flip(func, Pii::Horizontally | Pii::Vertically);
    return convolution<ResultType>(Pii::conj(a), func, mode);
  }

  template <class T> struct CorrelationNormalizationFunction
  {
    T operator() (T op1, T op2)
    {
      return op2 != 0 ? op1 / op2 : 0;
    }
  };
  
  template <class ResultType, class T, class U>
  PiiMatrix<ResultType> normalizedCorrelation(const PiiMatrix<T>& a,
                                              const PiiMatrix<U>& b,
                                              FilterMode mode)
  {
    PiiMatrix<U> func = b;
    // Create a normalization mask
    PiiMatrix<U> func2 = b;
    func2 = 1;
    Pii::flip(func, Pii::Horizontally | Pii::Vertically);
    PiiMatrix<typename Pii::Conj<T>::result_type> conjugate(Pii::conj(a));
    PiiMatrix<ResultType> result(convolution<ResultType>(conjugate, func, mode));
    result.binaryOp(CorrelationNormalizationFunction<ResultType>(),
                    convolution<ResultType>(conjugate, func2, mode));
    return result;
  }

  template <class ResultType, class T, class U>
  PiiMatrix<ResultType> convolution(const PiiMatrix<T>& a,
                                    const PiiMatrix<U>& b,
                                    FilterMode mode)
  {
    const int ar = a.rows(), ac = a.columns(), br = b.rows(), bc = b.columns();
    if (ar == 0 || ac == 0 || br == 0 || bc == 0)
      return PiiMatrix<ResultType>(a);
    int rr, rc;

    ResultType* rdata;
    const U* bdata;
    const T* adata;

    if (mode != FilterValidPart)
      {
        rr = ar+br-1, rc = ac+bc-1;
        PiiMatrix<ResultType> result(rr,rc);
    
        for (int brr=0; brr<br; ++brr)
          for (int arr=0; arr<ar; ++arr)
            {
              rdata = result.row(brr+arr);
              bdata = b.row(brr);
              adata = a.row(arr);
              
              for (int i=0; i<bc; ++i)
                for (int j=0; j<ac; ++j)
                  rdata[i+j] += ResultType(adata[j]) * ResultType(bdata[i]);
            }
        if (mode == FilterOriginalSize)
          {
            int cropLeft = bc >> 1;
            int cropRight = (bc - 1) >> 1;
            int cropTop = br >> 1;
            int cropBottom = (br - 1) >> 1;
            return result(cropTop, cropLeft, -cropBottom-1, -cropRight-1);
          }
        return result;
      }
    else
      {
        rr = ar - br + 1, rc = ac - bc + 1;
        if (rr <= 0 || rc <= 0)
          return PiiMatrix<ResultType>();
        PiiMatrix<ResultType> result(rr, rc);

        for (int rrr = 0; rrr<rr; ++rrr)
          {
            rdata = result.row(rrr);
            for (int brr = 0; brr<br; ++brr)
              {
                adata = a.row(rrr + brr);
                bdata = b.row(br-brr-1) - 1 + bc;
                
                for (int i = 0; i<rc; ++i)
                  for (int j = 0; j<bc; ++j)
                    rdata[i] += ResultType(adata[i+j]) * ResultType(bdata[-j]);
              }
          }
        return result;
      }
  }

  template <class T> PiiMatrix<T> fftShift(const PiiMatrix<T>& matrix, bool inverse)
  {
    int rows = matrix.rows(), cols = matrix.columns();
    int cols2 = cols / 2, rows2 = rows / 2;
    if (inverse)
      {
        if (cols & 1)
          cols2++;
        if (rows & 1)
          rows2++;
      }
    
    int cols1 = cols - cols2, rows1 = rows - rows2;
    
    PiiMatrix<T> result(PiiMatrix<T>::uninitialized(rows, cols));

    // Top left, top right, bottom left, bottom right
    result(0,0,rows2,cols2) << matrix(rows1,cols1,rows2,cols2);
    result(0,cols2,rows2,cols1) << matrix(rows1,0,rows2,cols1);
    result(rows2,0,rows1,cols2) << matrix(0,cols1,rows1,cols2);
    result(rows2,cols2,rows1,cols1) << matrix(0,0,rows1,cols1);

    return result;
  }
}
