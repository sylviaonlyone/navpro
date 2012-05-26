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

#ifndef _PIIWAVELET_H
# error "Never use <PiiWavelet-templates.h> directly; include <PiiWavelet.h> instead."
#endif

#include "PiiDsp.h"

namespace PiiDsp
{
  template <class T> QList<PiiMatrix<T> > dwt(const PiiMatrix<T>& mat,
                                              WaveletFamily type,
                                              int index)
  {
    QList<PiiMatrix<double> > filters(createScalingWavelets(scalingFilter(type, index)));
    return dwt(mat, PiiMatrix<T>(filters[0]), PiiMatrix<T>(filters[1]));
  }

  template <class T> QList<PiiMatrix<T> > dwt(const PiiMatrix<T>& mat,
                                              const PiiMatrix<T>& lo,
                                              const PiiMatrix<T>& hi)
  {
    QList<PiiMatrix<T> > result;
    PiiMatrix<T> loT(Pii::transpose(lo)), hiT(Pii::transpose(hi));

    // Low-pass filter vertically
    PiiMatrix<T> tmp(downSample(PiiDsp::convolution<T>(mat,loT), Pii::Vertically, 1));

    // Low-pass in both directions (approximation)
    result << downSample(PiiDsp::convolution<T>(tmp,lo), Pii::Horizontally, 1);
    // Vertical low-pass, horizontal high-pass (vertical details)
    result << downSample(PiiDsp::convolution<T>(tmp,hi), Pii::Horizontally, 1);

    // High-pass vertically
    tmp = downSample(PiiDsp::convolution<T>(mat,hiT), Pii::Vertically, 1);

    // Vertical high-pass, horizontal low-pass (horizontal details)
    result << downSample(PiiDsp::convolution<T>(tmp,lo), Pii::Horizontally, 1);
    // High-pass in both directions (diagonal details)
    result << downSample(PiiDsp::convolution<T>(tmp,hi), Pii::Horizontally, 1);

    return result;
  }
  
  template <class T> PiiMatrix<T> quadratureMirror(const PiiMatrix<T>& filter, int odd)
  {
    PiiMatrix<T> result(Pii::flipped(filter, Pii::Horizontally));
    T* pRow = result.row(0);
    for (int c=odd&1; c<result.columns(); c+=2)
      pRow[c] = -pRow[c];
    return result;
  }
  
  template <class T> QList<PiiMatrix<T> > createScalingWavelets(const PiiMatrix<T>& filter)
  {
    PiiMatrix<T> decompLo, decompHi, reconstLo, reconstHi;
  
    reconstLo = filter / T(Pii::sumAll<T>(filter) / 1.41421356237309504880); //M_SQRT2
    reconstHi = quadratureMirror(reconstLo);
    decompHi = Pii::flipped(reconstHi, Pii::Horizontally);
    decompLo = Pii::flipped(reconstLo, Pii::Horizontally);

    return QList<PiiMatrix<T> >() << decompLo << decompHi << reconstLo << reconstHi;
  }

  template <class T> PiiMatrix<T> downSample(const PiiMatrix<T>& mat,
                                             Pii::MatrixDirections directions,
                                             int odd)
  {
    int rows = mat.rows(), rStep = 1, oddRow = 0;
    int cols = mat.columns(), cStep = 1, oddCol = 0;
    if (directions & Pii::Vertically)
      {
        rows = mat.rows() / 2 + (~odd & mat.rows() & 1);
        rStep = 2;
        oddRow = odd & 1;
      }
    if (directions & Pii::Horizontally)
      {
        cols = mat.columns() / 2 + (~odd & mat.columns() & 1);
        cStep = 2;
        oddCol = odd & 1;
      }
    
    PiiMatrix<T> result(PiiMatrix<T>::uninitialized(rows, cols));

    for (int r=oddRow, resultRow=0; r<mat.rows(); r+=rStep, ++resultRow)
      {
        const T* sourceRow = mat.row(r);
        T* targetRow = result.row(resultRow);
        for (int c=oddCol, resultCol=0; c<mat.columns(); c+=cStep, ++resultCol)
          {
            Q_ASSERT(resultCol < result.columns());
            Q_ASSERT(c < mat.columns());
            targetRow[resultCol] = sourceRow[c];
          }
      }

    return result;
  }

}
