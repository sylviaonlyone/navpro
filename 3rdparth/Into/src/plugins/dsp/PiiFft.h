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

#ifndef _PIIFFT_H
#define _PIIFFT_H

#include <PiiMatrix.h>
#include <PiiFunctional.h>
#include <PiiMatrixValue.h>
#include <complex>

/**
 * A class for performing forward and inverse FFT for 1D and 2D
 * signals. The calculation is optimized by splitting the input into
 * pieces for which an optimized radix-N implementation exists. The
 * class has implementations for radix 2, 3, 4, 5, 8, and 10.
 *
 * @ingroup PiiDspPlugin
 */
template <class T> class PiiFft
{
public:
  PiiFft();
  ~PiiFft();

  /**
   * Perform a forward Fourier transform.
   */
  template <class S> PiiMatrix<std::complex<T> > forwardFft(const PiiMatrix<S>& source);
  /**
   * Perform an inverse Fourier transform.
   */
  template <class S> PiiMatrix<std::complex<T> > inverseFft(const PiiMatrix<std::complex<S> >& source);
    
private:
  template <class S> bool forward1d(const S* source, std::complex<T>* destination, int count);
  template <class S> bool inverse1d(const std::complex<S>* source, std::complex<T>* destination, int count);

  void factorize(int count);
  template <class S> void reorderSeries(const S* source, std::complex<T>* dest);
  void initializeTrigonomials(int radix);
  
  inline void fftPrime(int radix);
  inline void fft2(std::complex<T>* z);
  inline void fft3(std::complex<T>* z);
  inline void fft4(std::complex<T>* z);
  inline void fft5(std::complex<T>* z);
  inline void fft8(std::complex<T>* z);
  inline void fft10(std::complex<T>* z);
  void synthesizeFft(int sofar, int radix, int remain, std::complex<T>* dest);
  bool isPrimeFactor(int radix);
  void reallocateBuffers(int len);

  std::complex<T> _a[5], _b[5];
  std::complex<T> *_trig, *_twiddle, *_z;
  int _sofarRadix[20], _actualRadix[20], _remainRadix[20];
  int _count, _factorCount, _prevPrimeRadix;

  T _pi, c3_1, c3_2, u5, c5_1, c5_2, c5_3, c5_4, c5_5, c8;
  PiiMatrix<std::complex<T> > _tempSource, _tempResult;
};

#include "PiiFft-templates.h"
#include <PiiMath.h>

namespace PiiDsp
{
  /// @internal Takes the real part of a complex number
  template <class T> struct FastCorrelationConverter
  {
    static PiiMatrix<T> apply(const PiiMatrix<std::complex<T> >& matrix) { return Pii::matrix(Pii::real(matrix)); }
  };
  /// @internal Returns the complex number itself
  template <class T> struct FastCorrelationConverter<std::complex<T> >
  {
    static PiiMatrix<std::complex<T> > apply(const PiiMatrix<std::complex<T> >& matrix) { return matrix; }
  };
  
  /**
   * Calculate the correlation of two signals using the Fourier
   * transform. The fast correlation is defined as
   *
   * @f[
   * \mathrm{corr}(a,b) = F^{-1}(F(a)F(b)^*),
   * @f]
   *
   * where @e F stands for the Fourier transform, and @e * marks
   * complex conjugation. The input matrices must be equal in size.
   *
   * @throw PiiMathException& if input matrices are different in size
   * 
   * @relates PiiFft
   */
  template <class T> inline PiiMatrix<T> fastCorrelation(const PiiMatrix<T>& a,
                                                         const PiiMatrix<T>& b)

  {
    PiiFft<T> fft;
    return FastCorrelationConverter<T>::apply(fft.inverseFft(Pii::matrix(Pii::multiplied(fft.forwardFft(a),
                                                                                         Pii::conj(fft.forwardFft(b))))));
  }

  template <class T> PiiMatrixValue<T> findTranslation(const PiiMatrix<T>& correlation)
  {
    PiiMatrixValue<T> result;
    result.value = Pii::maxAll(correlation, &result.row, &result.column);
    int iCenterR = (correlation.rows() + 1) / 2;
    int iCenterC = (correlation.columns() + 1) / 2;
    if (result.row > iCenterR)
      result.row -= correlation.rows();
    if (result.column > iCenterC)
      result.column -= correlation.columns();
    return result;
  }
  
  /**
   * Find the translation of signal @p a with respect to signal @p b. 
   * This function uses #fastCorrelation() to find the maximum
   * correlation.
   *
   * @return A PiiMatrixValue structure whose @p row and @p column
   * members store the vertical and horizontal translation,
   * respectively. The @p value member stores the correlation at that
   * translation.
   */
  template <class T> inline PiiMatrixValue<T> findTranslation(const PiiMatrix<T>& a,
                                                              const PiiMatrix<T>& b)
  {
    return findTranslation(PiiDsp::fastCorrelation(a,b));
  }
};

#endif //_PIIFFT_H
