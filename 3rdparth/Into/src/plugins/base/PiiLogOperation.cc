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

#include "PiiLogOperation.h"

#include <PiiMath.h>
#include <PiiYdinTypes.h>

PiiLogOperation::Data::Data() :
  dBase(M_E), dOnePerLogBase(1.0)
{
}

PiiLogOperation::PiiLogOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

void PiiLogOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_CASES(logarithm, obj);
      PII_NUMERIC_MATRIX_CASES(matrixLogarithm, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

void PiiLogOperation::setBase(double base)
{
  PII_D;
  if (base > 0)
    {
      d->dBase = base;
      d->dOnePerLogBase = 1.0/std::log(base);
    }
}

template <class T> void PiiLogOperation::logarithm(const PiiVariant& obj)
{
  PII_D;
  if (d->dBase == M_E)
    emitObject(Pii::log(obj.valueAs<T>()));
  else
    emitObject(Pii::log(obj.valueAs<T>())*d->dOnePerLogBase);
}

template <class T> void PiiLogOperation::matrixLogarithm(const PiiVariant& obj)
{
  PII_D;
  if (d->dBase == M_E)
    emitObject(Pii::log(obj.valueAs<PiiMatrix<T> >()));
  else
    {
      PiiMatrix<double> mat(Pii::log(obj.valueAs<PiiMatrix<T> >()));
      mat *= d->dOnePerLogBase;
      emitObject(mat);
    }
}

double PiiLogOperation::base() const { return _d()->dBase; }
