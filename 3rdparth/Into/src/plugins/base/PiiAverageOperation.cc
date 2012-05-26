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

#include "PiiAverageOperation.h"
#include <PiiYdinTypes.h>
#include <PiiMath.h>

PiiAverageOperation::Data::Data() :
  calculationMode(MeanAll)
{
}

PiiAverageOperation::PiiAverageOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("matrix"));
  addSocket(new PiiOutputSocket("average"));
}

void PiiAverageOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(average, obj);
      PII_COMPLEX_MATRIX_CASES(complexAverage, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiAverageOperation::average(const PiiVariant& obj)
{
  switch (_d()->calculationMode)
    {
    case MeanRows:
      emitObject(Pii::mean<double>(obj.valueAs<PiiMatrix<T> >(), Pii::Horizontally));
      break;
    case MeanColumns:
      emitObject(Pii::mean<double>(obj.valueAs<PiiMatrix<T> >(), Pii::Vertically));
      break;
    default:
      emitObject(Pii::meanAll<double>(obj.valueAs<PiiMatrix<T> >()));
      break;
    }
}

template <class T> void PiiAverageOperation::complexAverage(const PiiVariant& obj)
{
  switch (_d()->calculationMode)
    {
    case MeanRows:
      emitObject(Pii::mean<std::complex<double> >(obj.valueAs<PiiMatrix<T> >(), Pii::Horizontally));
      break;
    case MeanColumns:
      emitObject(Pii::mean<std::complex<double> >(obj.valueAs<PiiMatrix<T> >(), Pii::Vertically));
      break;
    default:
      emitObject(Pii::meanAll<std::complex<double> >(obj.valueAs<PiiMatrix<T> >()));
      break;
    }
}

void PiiAverageOperation::setCalculationMode(const CalculationMode& calculationMode) { _d()->calculationMode = calculationMode; }
PiiAverageOperation::CalculationMode PiiAverageOperation::calculationMode() const { return _d()->calculationMode; }
