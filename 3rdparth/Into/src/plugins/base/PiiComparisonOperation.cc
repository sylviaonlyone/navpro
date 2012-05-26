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

#include "PiiComparisonOperation.h"
#include <PiiYdinTypes.h>

PiiComparisonOperation::Data::Data() :
  dConstant(0), function(Equal),
  pInput0(0), pInput1(0),
  bInput1Connected(false)
{
}

PiiComparisonOperation::PiiComparisonOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  addSocket(d->pInput0 = new PiiInputSocket("input0"));
  addSocket(d->pInput1 = new PiiInputSocket("input1"));
  d->pInput1->setOptional(true);

  addSocket(new PiiOutputSocket("output"));
}

void PiiComparisonOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  d->bInput1Connected = d->pInput1->isConnected();
}

void PiiComparisonOperation::process()
{
  PII_D;
  PiiVariant obj = d->pInput0->firstObject();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(operateMatrix, obj);
      PII_NUMERIC_CASES(operateNumber, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pInput0);
    }
}

template <class T> void PiiComparisonOperation::operateMatrix(const PiiVariant& obj)
{
  PII_D;
  if (d->bInput1Connected)
    {
      PiiVariant obj2 = d->pInput1->firstObject();
      switch (obj2.type())
        {
          PII_NUMERIC_CASES_M(operateMatrixNumber, (obj.valueAs<PiiMatrix<T> >(), obj2));
          PII_NUMERIC_MATRIX_CASES_M(operateMatrixMatrix, (obj.valueAs<PiiMatrix<T> >(), obj2));
        default:
          PII_THROW_UNKNOWN_TYPE(d->pInput1);
        }
    }
  else
    compare(obj.valueAs<PiiMatrix<T> >(), (T)d->dConstant);
}

template <class T, class U> void PiiComparisonOperation::operateMatrixNumber(const PiiMatrix<U>& matrix, const PiiVariant& obj)
{
  compare(matrix, (U)obj.valueAs<T>());
}

template <class T, class U> void PiiComparisonOperation::operateMatrixMatrix(const PiiMatrix<U>& matrix, const PiiVariant& obj)
{
  compare(matrix, PiiMatrix<U>(obj.valueAs<PiiMatrix<T> >()));
}

template <class T> void PiiComparisonOperation::operateNumber(const PiiVariant& obj)
{
  PII_D;
  if (d->bInput1Connected)
    {
      PiiVariant obj2 = d->pInput1->firstObject();
      switch (obj2.type())
        {
          PII_NUMERIC_CASES_M(operateNumberNumber, (obj.valueAs<T>(), obj2));
        default:
          PII_THROW_UNKNOWN_TYPE(d->pInput1);
        }
    }
  else
    compare(obj.valueAs<T>(), (T)d->dConstant);
}

template <class T, class U> void PiiComparisonOperation::operateNumberNumber(U number, const PiiVariant& obj)
{
  compare(number, (U)obj.valueAs<T>());
}

template <class T, class U> void PiiComparisonOperation::compare(const T& op1, const U& op2)
{
  try
    {
      switch (_d()->function)
        {
        case Equal:
          emitObject(op1 == op2);
          break;
        case LessThan:
          emitObject(op1 < op2);
          break;
        case GreaterThan:
          emitObject(op1 > op2);
          break;
        case LessEqual:
          emitObject(op1 <= op2);
          break;
        case GreaterEqual:
          emitObject(op1 >= op2);
          break;
        default:
          PII_THROW(PiiExecutionException, tr("Unknown comparison function."));
        }
    }
  catch (PiiMathException& ex)
    {
      PII_THROW(PiiExecutionException, ex.message());
    }
}

void PiiComparisonOperation::setConstant(double constant) { _d()->dConstant = constant; }
double PiiComparisonOperation::constant() const { return _d()->dConstant; }
void PiiComparisonOperation::setFunction(Function op) { _d()->function = op; }
PiiComparisonOperation::Function PiiComparisonOperation::function() const { return _d()->function; }
