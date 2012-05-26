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

#include <PiiArithmeticOperation.h>
#include <PiiYdinTypes.h>
#include <QString>
#include <PiiMath.h>

PiiArithmeticOperation::Data::Data() :
  pConstant(PiiVariant(0.0)),
  bInput1Connected(false),
  function(Plus)
{
}

PiiArithmeticOperation::PiiArithmeticOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  addSocket(d->pInput0 = new PiiInputSocket("input0"));
  addSocket(d->pInput1 = new PiiInputSocket("input1"));
  d->pInput1->setOptional(true);
  
  addSocket(d->pOutput = new PiiOutputSocket("output"));
}

PiiVariant PiiArithmeticOperation::constant() const { return _d()->pConstant; }
void PiiArithmeticOperation::setConstant(const PiiVariant& constant) { _d()->pConstant = constant; }

PiiArithmeticOperation::Function PiiArithmeticOperation::function() const { return _d()->function; }
void PiiArithmeticOperation::setFunction(Function function) { _d()->function = function; }


void PiiArithmeticOperation::check(bool reset)
{
  PiiDefaultOperation::check(reset);
  PII_D;
  
  // If the second input is not connected and constant is not set, we
  // cannot do anything.
  d->bInput1Connected = d->pInput1->isConnected();
  if (!d->bInput1Connected && !d->pConstant.isValid())
    PII_THROW(PiiExecutionException, tr("Must have a value for second operand. Set a constant or connect the input."));
}

void PiiArithmeticOperation::wrongTypes(int type0, int type1)
{
  PII_THROW(PiiExecutionException, tr("Don't know how to perform calculations on types %1 and %2.")
            .arg(type0, 0, 16)
            .arg(type1, 0, 16));
}

void PiiArithmeticOperation::process()
{
  PII_D;
  PiiVariant obj0 = d->pInput0->firstObject();
  PiiVariant obj1 = d->bInput1Connected ? d->pInput1->firstObject() : d->pConstant;
  int iType0 = obj0.type(), iType1 = obj1.type();
  int iClass0 = iType0 & ~0x1f, iClass1 = iType1 & ~0x1f;
  int iPrimitive0 = iType0 & 0x1f, iPrimitive1 = iType1 & 0x1f;

  if ((iClass0 != 0 && iClass0 != 0x40) ||
      (iClass1 != 0 && iClass1 != 0x40))
    wrongTypes(iType0, iType1);
  if (iPrimitive0 == PiiVariant::BoolType || iPrimitive1 == PiiVariant::BoolType)
    PII_THROW(PiiExecutionException, tr("Cannot perform arithmetic with boolean values."));

  int iResultType = iPrimitive0;
  
  // If both types are the same, no conversion is needed.
  if (iPrimitive0 != iPrimitive1)
    {
      // If both are unsigned, result is unsigned
      if (PiiVariant::isUnsigned(iPrimitive0) && PiiVariant::isUnsigned(iPrimitive1))
        iResultType = qMax(iPrimitive0, iPrimitive1);
      // If not, result is maximum of signed versions
      else
        iResultType = qMax(iPrimitive0 & ~0x8, iPrimitive1 & ~0x8);
    }

  switch (iResultType)
    {
      PII_NUMERIC_CASES_M(calculate, (obj0, obj1));
    default:
      wrongTypes(iType0, iType1);
    }
}

template <class T> void PiiArithmeticOperation::send(const T& value)
{
  _d()->pOutput->emitObject(value);
}

template <class T> void PiiArithmeticOperation::calculate(const PiiVariant& obj0, const PiiVariant& obj1)
{
  int iType0 = obj0.type(), iType1 = obj1.type();
  int iMatrixMask = ((iType0 & 0x40) >> 6) | ((iType1 & 0x40) >> 5);
  if (iMatrixMask == 0) // Both scalars
    {
      T op0 = 0, op1 = 0;
      switch (iType0)
        {
          PII_NUMERIC_CASES(op0 = (T)PiiYdin::primitiveAs, obj0);
        default:
          wrongTypes(iType0, iType1);
        }
      switch (iType1)
        {
          PII_NUMERIC_CASES(op1 = (T)PiiYdin::primitiveAs, obj1);
        default:
          wrongTypes(iType0, iType1);
        }
      calculate(op0, op1);
    }
  else if (iMatrixMask == 1) // First is matrix, second scalar
    {
      PiiMatrix<T> op0;
      T op1 = 0;
      switch (iType0)
        {
          PII_NUMERIC_MATRIX_CASES(op0 = (PiiMatrix<T>)PiiYdin::matrixAs, obj0);
        default:
          wrongTypes(iType0, iType1);
        }
      switch (iType1)
        {
          PII_NUMERIC_CASES(op1 = (T)PiiYdin::primitiveAs, obj1);
        default:
          wrongTypes(iType0, iType1);
        }
      calculate(op0, op1);
    }
  else if (iMatrixMask == 3) // Both matrices
    {
      PiiMatrix<T> op0, op1;
      switch (iType0)
        {
          PII_NUMERIC_MATRIX_CASES(op0 = (PiiMatrix<T>)PiiYdin::matrixAs, obj0);
        default:
          wrongTypes(iType0, iType1);
        }
      switch (iType1)
        {
          PII_NUMERIC_MATRIX_CASES(op1 = (PiiMatrix<T>)PiiYdin::matrixAs, obj1);
        default:
          wrongTypes(iType0, iType1);
        }
      calculate(op0, op1);
    }
  else
    PII_THROW(PiiExecutionException, tr("%1 must be a matrix if %2 is a matrix.").arg("input0").arg("input1"));
}

template <class T> void PiiArithmeticOperation::calculate(const PiiMatrix<T>& obj0,
                                                          const PiiMatrix<T>& obj1)
{
  try
    {
      switch (_d()->function)
        {
        case Plus: send(obj0 + obj1); break;
        case Minus: send(obj0 - obj1); break;
        case ElementDivision: send(Pii::divided(obj0, obj1)); break;
        case Division: send(obj0 / obj1); break;
        case ElementMultiplication: send(Pii::multiplied(obj0, obj1)); break;
        case Multiplication: send(obj0 * obj1); break;
        }
    }
  catch (PiiMathException& ex)
    {
      PII_THROW(PiiExecutionException, ex.message());
    }  
}

template <class T, class U> void PiiArithmeticOperation::calculate(const T& obj0, const U& obj1)
{
  switch (_d()->function)
    {
    case Plus: send(obj0 + obj1); break;
    case Minus: send(obj0 - obj1); break;
    case ElementDivision:
    case Division: send(obj0 / obj1); break;
    case ElementMultiplication:
    case Multiplication: send(obj0 * obj1);
    }
}
