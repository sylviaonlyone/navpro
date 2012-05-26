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

#ifndef _PIIFOURIERTRANSFORM_H
#define _PIIFOURIERTRANSFORM_H

#include <PiiDefaultOperation.h>
#include "PiiFft.h"

using namespace PiiYdin;

/**
 * An operation that calculates the fast fourier transform for any
 * matrix.
 *
 * @inputs
 *
 * @in input - the input matrix. If #direction == @p Inverse, the type
 * of the input matrix must be a PiiMatrix<complex<float> > or a
 * PiiMatrix<complex<double> >. Otherwise, any numeric or complex
 * matrix will do.
 *
 * @outputs
 *
 * @out output - the (forward or inverse) Fourier transform of the
 * input matrix (PiiMatrix<complex<float> > or
 * PiiMatrix<complex<double> >).
 *
 * @out power spectrum - real-valued power spectrum of the
 * transformed matrix. Used only in @p Forward transformation.
 *
 * @ingroup PiiDspPlugin
 */
class PiiFourierTransform : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The direction of the transform. The default is @p Forward.
   */
  Q_PROPERTY(FftDirection direction READ direction WRITE setDirection);
  Q_ENUMS(FftDirection);

  /**
   * If this flag is set to @p true, the operation shifts the
   * zero-frequency component to the center of the spectrum. For
   * vectors, FftShift swaps the left and right halves. For matrices,
   * it swaps the first and fourth quadrants and the second and third
   * quadrants. FftShift is useful for visualizing the Fourier
   * transform with the zero- frequency component in the middle of the
   * spectrum. The default value is @p false.
   */
  Q_PROPERTY(bool shift READ shift WRITE setShift);

  /**
   * If this flag is @p true, the mean value of the input is
   * subtracted before applying the FFT. This makes the zero-frequency
   * component zero and helps in visualizing the result. The default
   * is @p false. This flag has no effect in inverse transform.
   */
  Q_PROPERTY(bool subtractMean READ subtractMean WRITE setSubtractMean);
  
public:

  PiiFourierTransform();
  
  /**
   * FFT direction.
   *
   * @lip Forward - calculates FFT.
   *
   * @lip Inverse - calculates inverse FFT.
   */
  enum FftDirection { Forward, Inverse };
  
  FftDirection direction() const;
  void setDirection(FftDirection direction);

  bool shift() const;
  void setShift(bool shift);
  void setSubtractMean(bool subtractMean);
  bool subtractMean() const;

  template <class T> class Template;
    
protected:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    FftDirection direction;
    bool bShift;
    bool bSubtractMean;
  };
  PII_D_FUNC;

  PiiFourierTransform(Data* data);

private:
  void init();
};

#include "PiiFourierTransform-templates.h"


#endif //_PIIFOURIERTRANSFORM_H
