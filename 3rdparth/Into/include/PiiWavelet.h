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
#define _PIIWAVELET_H

#include <PiiMath.h>
#include "PiiDspGlobal.h"

/**
 * @file
 * @ingroup PiiDspPlugin
 *
 * Methods for creating wavelets and performing the wavelet
 * decomposition for 1-D and 2-D signals.
 */

namespace PiiDsp
{
  /**
   * Get the <i>index</i>th Daubechies scaling wavelet. The first
   * filter (index 1) is a two-tap filter and equals to the Haar
   * wavelet. Filter 2 is a four-tap filter, filter 3 a six-tap
   * filter and so on.
   *
   * @param index filter index, must be in the range [1,10]
   *
   * @return the filter coefficients as a row matrix
   **/
  PII_DSP_EXPORT PiiMatrix<double> daubechiesScalingFilter(int index);
  
  /**
   * Known wavelet families. Families currently known are:
   *
   * @li Haar - the Haar wavelet. Not actually a family, but equal to
   * the first Daubechies wavelet.
   *
   * @li Daubechies - Daubechies wavelet family
   */
  enum WaveletFamily { Haar, Daubechies };
  
  /**
   * Compute a quadrature mirror filter for a filter. The qmf is a
   * reversed version of the input filter with every second item
   * negated.
   *
   * @param filter the input filter
   *
   * @param odd if one, odd elements are negated. Otherwise, even
   * elements are negated.
   *
   * @return the qmf filter
   */
  template <class T> PiiMatrix<T> quadratureMirror(const PiiMatrix<T>& filter, int odd=1);

  /**
   * Create an orthogonal wavelet filter set from the given scaling
   * filter.
   *
   * @param filter a scaling filter for a wavelet
   *
   * @return a list of four filters: decomposition low-pass,
   * decomposition high-pass, reconstruction low-pass and
   * reconstruction high-pass in this order.
   */
  template <class T> QList<PiiMatrix<T> > createScalingWavelets(const PiiMatrix<T>& filter);

  /**
   * Performs a two-dimensional one-level discrete wavelet transform
   * on the input matrix.
   *
   * @param mat the input matrix
   *
   * @param lo low-pass decomposition filter
   *
   * @param hi high-pass decomposition filter
   *
   * @param type the type of matrix extension (for handling border effects)
   *
   * @return four matrices, the first one containing approximation
   * coefficients and the last three containing horizontal, vertical
   * and diagonal details in this order.
   */
  template <class T> QList<PiiMatrix<T> > dwt(const PiiMatrix<T>& mat,
                                              const PiiMatrix<T>& lo,
                                              const PiiMatrix<T>& hi);
  /**
   * Performs a two-dimensional one-level discrete wavelet transform
   * on the input matrix.
   *
   * @param mat the input matrix
   *
   * @param wavelet the wavelet family to be used (defaults to Haar).
   *
   * @param familyMember the index of the wavelet within its family
   *
   * @param type the type of matrix extension (for handling border effects)
   *
   * @return four matrices, the first one containing approximation
   * coefficients and the last three containing horizontal, vertical
   * and diagonal details in this order.
   */
  template <class T> QList<PiiMatrix<T> > dwt(const PiiMatrix<T>& mat,
                                              WaveletFamily wavelet = Haar,
                                              int familyMember = 1);
  
  /**
   * Get the wavelet scaling filter for a certain mother wavelet.
   *
   * @param wavelet the wavelet family
   *
   * @param index the index of the wavelet within the family. Only
   * discrete wavelet families are supported.
   */
  PII_DSP_EXPORT PiiMatrix<double> scalingFilter(WaveletFamily wavelet, int member = 1);
    
  /**
   * Perform a dyadic downsampling on the input matrix.
   *
   * @param mat the input matrix
   *
   * @param directions either Pii::Horizontally or Pii::Vertically,
   * or both.
   *
   * @param odd if one, odd rows and/or columns are removed. If
   * zero, even rows and/or columns are removed
   *
   * @return the input matrix with odd or even rows and/or columns
   * removed
   */
  template <class T> PiiMatrix<T> downSample(const PiiMatrix<T>& mat,
                                             Pii::MatrixDirections directions,
                                             int odd = 0);

  /**
   * Take the central part of a matrix.
   *
   * @param mat the matrix whose center is to be taken.
   *
   * @param rows the number of rows in the returned matrix
   *
   * @param cols the number of columns in the returned matrix
   */
  template <class T> PiiMatrix<T> keep(const PiiMatrix<T>& mat,
                                       int rows, int cols)
  {
    return mat((mat.rows()-rows)>>1,(mat.columns()-cols)>>1,rows,cols);
  }
}

#include "PiiWavelet-templates.h"

#endif //_PIIWAVELET_H
