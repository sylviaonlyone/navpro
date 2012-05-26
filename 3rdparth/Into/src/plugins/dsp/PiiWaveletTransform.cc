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

#include "PiiWaveletTransform.h"

#include <PiiYdinTypes.h>

PiiWaveletTransform::Data::Data() :
  waveletFamily(Haar), iFamilyMember(1)
{
}

PiiWaveletTransform::PiiWaveletTransform() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("approximation"));
  addSocket(new PiiOutputSocket("horizontal"));
  addSocket(new PiiOutputSocket("vertical"));
  addSocket(new PiiOutputSocket("diagonal"));
}

void PiiWaveletTransform::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_UNSIGNED_MATRIX_CASES(integerTransform, obj);
      PII_INTEGER_MATRIX_CASES(integerTransform, obj);
      PII_FLOAT_MATRIX_CASES(floatTransform, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiWaveletTransform::floatTransform(const PiiVariant& obj)
{
  transform(obj.valueAs<PiiMatrix<T> >());
}

template <class T> void PiiWaveletTransform::integerTransform(const PiiVariant& obj)
{
  transform(PiiMatrix<float>(obj.valueAs<PiiMatrix<T> >()));
}

template <class T> void PiiWaveletTransform::transform(const PiiMatrix<T>& mat)
{
  PII_D;
  QList<PiiMatrix<T> > lstTransforms(PiiDsp::dwt(mat, (PiiDsp::WaveletFamily)d->waveletFamily, d->iFamilyMember));
  for (int i=0; i<4; ++i)
    emitObject(lstTransforms[i], i);
}

void PiiWaveletTransform::setWaveletFamily(const WaveletFamily& waveletFamily) { _d()->waveletFamily = waveletFamily; }
PiiWaveletTransform::WaveletFamily PiiWaveletTransform::waveletFamily() const { return _d()->waveletFamily; }
void PiiWaveletTransform::setFamilyMember(int familyMember) { _d()->iFamilyMember = familyMember; }
int PiiWaveletTransform::familyMember() const { return _d()->iFamilyMember; }
