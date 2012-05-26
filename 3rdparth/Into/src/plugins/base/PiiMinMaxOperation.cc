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

#include "PiiMinMaxOperation.h"

#include <PiiYdinTypes.h>
#include <PiiMath.h>
#include <PiiTypeTraits.h>

PiiMinMaxOperation::Data::Data() :
  mode(MaximumMode), iMinMaxCount(1)
{
}

PiiMinMaxOperation::PiiMinMaxOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input0"));
  addSocket(new PiiInputSocket("input1"));
  inputAt(1)->setOptional(true);
  addSocket(new PiiOutputSocket("output"));
  addSocket(new PiiOutputSocket("coordinates"));
}

void PiiMinMaxOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(findMinMaxMatrix, obj);
      PII_NUMERIC_CASES(findMinMaxScalar, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiMinMaxOperation::findMinMaxMatrix(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> mat = obj.valueAs<PiiMatrix<T> >();

  if (d->iMinMaxCount > mat.columns() * mat.rows())
    PII_THROW(PiiExecutionException, tr("The input matrix contains only %1 elements, but "
                                        "%2 minima/maxima were requested.").arg(mat.columns() * mat.rows()).arg(d->iMinMaxCount));

  PiiVariant obj2 = readInput(1);
  // Compare matrix to matrix or scalar
  if (obj2.isValid())
    {
      switch (obj2.type())
        {
          PII_NUMERIC_MATRIX_CASES_M(findMinMaxMatrixMatrix, (mat, obj2));
          PII_NUMERIC_CASES_M(findMinMaxMatrixScalar, (mat, obj2));
        default:
          PII_THROW_UNKNOWN_TYPE(inputAt(1));
        }
    }
  // Find minimum/maximum within the matrix
  else
    {
      PiiMatrix<T> matValues;
      PiiMatrix<int> matCoords;
      // Find the N smallest/largest values
      if (d->mode == MinimumMode)
        matValues = findExtrema(mat, matCoords, std::less<T>(), std::numeric_limits<T>::max());
      else
        matValues = findExtrema(mat, matCoords, std::greater<T>(), std::numeric_limits<T>::min());

      // If only one was needed, output a scalar
      if (d->iMinMaxCount == 1)
        emitObject(matValues(0,0), 0);
      else
        emitObject(matValues, 0);

      emitObject(matCoords, 0);
    }
}

template <class T, class Comparator> PiiMatrix<T> PiiMinMaxOperation::findExtrema(const PiiMatrix<T>& mat,
                                                                                  PiiMatrix<int>& coords,
                                                                                  Comparator cmp,
                                                                                  T initialValue)
{
  PII_D;
  // Allocate one extra column
  PiiMatrix<T> matValues(1, d->iMinMaxCount+1);
  matValues = initialValue;
  T* pValues = matValues[0];

  // Allocate storage for coordinates
  coords.resize(d->iMinMaxCount+1, 2);
  
  for (int r=0; r<mat.rows(); ++r)
    for (int c=0; c<mat.columns(); ++c)
      {
        // Store the current value into the extra column
        pValues[d->iMinMaxCount] = mat(r,c);
        // Store coordinates
        coords(d->iMinMaxCount,0) = c;
        coords(d->iMinMaxCount,1) = r;
        // Maintain sort order
        for (int i=d->iMinMaxCount-1; i>=0 && cmp(pValues[i+1], pValues[i]); --i)
          {
            qSwap(pValues[i+1], pValues[i]);
            coords.swapRows(i+1, i);
          }
      }
  // Get rid of the extra value
  matValues.removeColumn(d->iMinMaxCount);
  coords.removeRow(d->iMinMaxCount);
  return matValues;
}

template <class T> void PiiMinMaxOperation::findMinMaxScalar(const PiiVariant& obj)
{
  PII_D;
  if (d->iMinMaxCount > 1)
    PII_THROW(PiiExecutionException, tr("There is only one minimum/maximum for a pair of scalar numbers. "
                                        "%1 were requested.").arg(d->iMinMaxCount));
  T scalar = obj.valueAs<T>();

  PiiVariant obj2 = readInput(1);
  // Compare scalar to scalar
  if (obj2.isValid())
    {
      switch (obj2.type())
        {
          PII_NUMERIC_CASES_M(findMinMaxScalarScalar, (scalar, obj2));
        default:
          PII_THROW_UNKNOWN_TYPE(inputAt(1));
        }
    }
}

template <class T, class U> void PiiMinMaxOperation::findMinMaxMatrixMatrix(const PiiMatrix<U>& mat1, const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> mat2 = obj.valueAs<PiiMatrix<T> >();

  typedef typename Pii::Combine<T,U>::Type ResultType;
  typedef PiiMatrix<ResultType> ResultMatrixType;

  try
    {
      if (d->mode == MinimumMode)
        emitObject(Pii::min(ResultMatrixType(mat1), ResultMatrixType(mat2)));
      else
        emitObject(Pii::max(ResultMatrixType(mat1), ResultMatrixType(mat2)));
    }
  catch (PiiMathException&)
    {
      PII_THROW_WRONG_SIZE(inputAt(1), mat2, mat1.rows(), mat1.columns());
    }
}

template <class T, class U> void PiiMinMaxOperation::findMinMaxMatrixScalar(const PiiMatrix<U>& mat1, const PiiVariant& obj)
{
  PII_D;
  T scalar = obj.valueAs<T>();

  typedef typename Pii::Combine<T,U>::Type ResultType;
  typedef PiiMatrix<ResultType> ResultMatrixType;

  if (d->mode == MinimumMode)
    emitObject(Pii::min(ResultMatrixType(mat1), ResultType(scalar)));
  else
    emitObject(Pii::max(ResultMatrixType(mat1), ResultType(scalar)));
}

template <class T, class U> void PiiMinMaxOperation::findMinMaxScalarScalar(U scalar1, const PiiVariant& obj)
{
  PII_D;
  T scalar2 = obj.valueAs<T>();

  typedef typename Pii::Combine<T,U>::Type ResultType;

  if (d->mode == MinimumMode)
    emitObject(Pii::min(ResultType(scalar1), ResultType(scalar2)));
  else
    emitObject(Pii::max(ResultType(scalar1), ResultType(scalar2)));
}


void PiiMinMaxOperation::setMode(Mode mode) { _d()->mode = mode; }
PiiMinMaxOperation::Mode PiiMinMaxOperation::mode() const { return _d()->mode; }
void PiiMinMaxOperation::setMinMaxCount(int minMaxCount) { if (minMaxCount >= 1) _d()->iMinMaxCount = minMaxCount; }
int PiiMinMaxOperation::minMaxCount() const { return _d()->iMinMaxCount; }
