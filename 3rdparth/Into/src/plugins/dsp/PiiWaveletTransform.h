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

#ifndef _PIIWAVELETTRANSFORM_H
#define _PIIWAVELETTRANSFORM_H

#include <PiiDefaultOperation.h>
#include "PiiWavelet.h"

/**
 * Discrete wavelet decomposition in two dimensions.
 *
 * @inputs
 *
 * @in input - input matrix. Any integer or floating-point matrix.
 * 
 * @outputs
 *
 * @out approximation - approximation coefficients (the result of
 * low-pass filtering in both dimensions)
 *
 * @out horizontal - horizontal details (horizontal high-pass,
 * vertical low-pass)
 * 
 * @out vertical - vertical details (vertical high-pass, horizontal
 * low-pass)
 *
 * @out diagonal - diagonal details (high-pass in both dimensions)
 *
 * @ingroup PiiDspPlugin
 */
class PiiWaveletTransform : public PiiDefaultOperation
{
  Q_OBJECT
  /**
   * The family of wavelets used in the decomposition. The default is
   * @p Haar.
   */
  Q_PROPERTY(WaveletFamily waveletFamily READ waveletFamily WRITE setWaveletFamily);
  Q_ENUMS(WaveletFamily);
  
  /**
   * The index of the wavelet within the chosen family. This value is
   * ignored for the @p Haar "family". The operation supports members
   * 1-10 of the @p Daubechies family. Note that @p Daubechies 1 is
   * equal to @p Haar. The default value is 1.
   */
  Q_PROPERTY(int familyMember READ familyMember WRITE setFamilyMember);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Known wavelet families. Families currently known are:
   *
   * @li Haar - the Haar wavelet. Not actually a family, but equal to
   * the first Daubechies wavelet.
   *
   * @li Daubechies - Daubechies wavelet family
   */
  enum WaveletFamily { Haar, Daubechies };
  PiiWaveletTransform();

  void setWaveletFamily(const WaveletFamily& waveletFamily);
  WaveletFamily waveletFamily() const;
  void setFamilyMember(int familyMember);
  int familyMember() const;

protected:
  void process();

private:
  template <class T> void floatTransform(const PiiVariant& obj);
  template <class T> void integerTransform(const PiiVariant& obj);
  template <class T> void transform(const PiiMatrix<T>& mat);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    
    WaveletFamily waveletFamily;
    int iFamilyMember;
  };
  PII_D_FUNC;
};


#endif //_PIIWAVELETTRANSFORM_H
