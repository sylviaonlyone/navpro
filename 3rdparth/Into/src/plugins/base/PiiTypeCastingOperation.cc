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

#include <PiiTypeCastingOperation.h>
#include <PiiYdinTypes.h>
#include <complex>
#include <PiiColor.h>
#include <PiiTypeTraits.h>

using namespace Pii;
using namespace PiiYdin;

PiiTypeCastingOperation::Data::Data() :
  outputType(0x48)
{
}


PiiTypeCastingOperation::PiiTypeCastingOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

PiiTypeCastingOperation::~PiiTypeCastingOperation()
{
}

void PiiTypeCastingOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(operate, obj);
    case FloatComplexMatrixType:
      operateC<std::complex<float> >(obj);
      break;                                           
    case DoubleComplexMatrixType: 
      operateC<std::complex<double> >(obj);              
      break;                                           
      /*case LongDoubleComplexMatrixType:  
      operateC<std::complex<long double> >(obj);         
      break;*/

      PII_COLOR_IMAGE_CASES(operateCI, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiTypeCastingOperation::operate(const PiiVariant& obj)
{
  PiiMatrix<T> matrix(obj.valueAs<PiiMatrix<T> >());
  
  switch (_d()->outputType)
    {
      PII_NUMERIC_MATRIX_CASES(operateMatrix, matrix);
      PII_COMPLEX_MATRIX_CASES(operateMatrix, matrix);
      //PII_COLOR_IMAGE_CASES(operateMatrix, matrix);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiTypeCastingOperation::operateC(const PiiVariant& obj)
{
  PiiMatrix<T> matrix(obj.valueAs<PiiMatrix<T> >());
  
  switch (_d()->outputType)
    {
      PII_NUMERIC_MATRIX_CASES(operateComplex, matrix);
    case FloatComplexMatrixType:
      operateMatrix<std::complex<float> >(matrix);
      break;                                           
    case DoubleComplexMatrixType: 
      operateMatrix<std::complex<double> >(matrix);              
      break;                                           
      /*case LongDoubleComplexMatrixType:  
      operateMatrix<std::complex<long double> >(matrix);         
      break;*/
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiTypeCastingOperation::operateCI( const PiiVariant& obj )
{
  PiiMatrix<T> matrix(obj.valueAs<PiiMatrix<T> >());
  
  switch (_d()->outputType)
    {
      PII_NUMERIC_MATRIX_CASES(operateColorToGray, matrix);
      //PII_COLOR_IMAGE_CASES(operateMatrix, matrix);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T, class U> void PiiTypeCastingOperation::operateMatrix(const PiiMatrix<U>& matrix)
{
  emitObject(PiiMatrix<T>(matrix));
}

template <class T, class U> void PiiTypeCastingOperation::operateComplex(const PiiMatrix<U>& matrix)
{
  PiiMatrix<T> matResult(matrix.rows(), matrix.columns());
  T* ptrr;
  const U* ptrm;
  
  for (int r=0; r<matrix.rows(); ++r)
    {
      ptrr = matResult.row(r);
      ptrm = matrix.row(r);
      for (int c=0; c<matrix.columns(); ++c)
        ptrr[c] = T(ptrm[c].real());
    }
  
  emitObject(matResult);
}

template <class T, class U> void PiiTypeCastingOperation::operateColorToGray(const PiiMatrix<U>& matrix)
{
  PiiMatrix<T> matResult(matrix.rows(), matrix.columns());
  T* ptrr;
  U color;
  typedef typename Pii::Combine<typename U::Type, int>::Type C;
  for (int r=0; r<matrix.rows(); ++r)
    {
      ptrr = matResult.row(r);
      for (int c=0; c<matrix.columns(); ++c)
        {
          color = matrix(r,c);
          //gray value is the average of r, g, and b.
          ptrr[c] = (T)((C(color.channels[0]) + C(color.channels[1]) + C(color.channels[2]))/3);
        }
    } 
  
  emitObject(matResult);
}

int PiiTypeCastingOperation::outputType() const { return _d()->outputType; }
void PiiTypeCastingOperation::setOutputType(int outputType) { _d()->outputType = outputType; }
