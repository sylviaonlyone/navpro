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

#ifndef _PIIFOURIERTRANSFORM_TEMPLATES_H
#define _PIIFOURIERTRANSFORM_TEMPLATES_H

#ifndef _PIIFOURIERTRANSFORM_H
# error "Never use <PiiFourierTransform-templates.h> directly; include <PiiFourierTransform.h> instead."
#endif

#include <PiiYdinTypes.h>
#include <PiiMath.h>
#include "PiiDsp.h"

using namespace PiiYdin;

template <class T> class PiiFourierTransform::Template : public PiiFourierTransform
{
  PII_OPERATION_SERIALIZATION_FUNCTION;

public:
  Template() : PiiFourierTransform(new Data) {}
  
  void check(bool reset)
  {
    PiiFourierTransform::check(reset);

    _d()->bPowerSpectrumConnected = outputAt(1)->isConnected();
  }

protected:
  void process()
  {
    PiiVariant obj = readInput();
    
    if (_d()->direction == Forward)
      {
        switch (obj.type())
          {
            PII_NUMERIC_MATRIX_CASES(operateForward, obj);
          case DoubleComplexMatrixType:
            operateForward<std::complex<double> >(obj);
            break;
          case FloatComplexMatrixType:
            operateForward<std::complex<float> >(obj);
            break;
          default:
            PII_THROW_UNKNOWN_TYPE(inputAt(0));
          }
      }
    else
      {
        if (obj.type() == DoubleComplexMatrixType)
          operateInverse<double>(obj);
        else if (obj.type() == FloatComplexMatrixType)
          operateInverse<float>(obj);
        else
          PII_THROW_UNKNOWN_TYPE(inputAt(0));
      }
  }

private:
  template <class S> void operateForward(const PiiVariant& obj);
  template <class S> void operateInverse(const PiiVariant& obj);

  class Data : public PiiFourierTransform::Data
  {
  public:
    Data() :  bPowerSpectrumConnected(false) {}

    PiiFft<T> fft;
    bool bPowerSpectrumConnected;
  };
  PII_D_FUNC;
};

template <class T>
template <class S> void PiiFourierTransform::Template<T>::operateForward(const PiiVariant& obj)
{
  PII_D;
  typedef PiiMatrix<std::complex<T> > ResultType;
  typedef typename Pii::ToFloatingPoint<S>::Type FloatType;
  
  const PiiMatrix<S>& image = obj.valueAs<PiiMatrix<S> >();

  ResultType result = d->bSubtractMean ?
    d->fft.forwardFft(image.mapped(std::minus<FloatType>(), Pii::meanAll<FloatType>(image))) :
    d->fft.forwardFft(image);

  if (d->bShift)
    result = PiiDsp::fftShift(result);
  
  emitObject(result);

  if (d->bPowerSpectrumConnected)
    emitObject(result.mapped(Pii::AbsPow<std::complex<T> >(), 2), 1);
}

template <class T>
template <class S> void PiiFourierTransform::Template<T>::operateInverse(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<std::complex<S> > image = obj.valueAs<PiiMatrix<std::complex<S> > >();
  emitObject(d->fft.inverseFft(d->bShift ? PiiDsp::fftShift(image, true) : image));
}

#endif //_PIIFOURIERTRANSFORM_TEMPLATES_H
