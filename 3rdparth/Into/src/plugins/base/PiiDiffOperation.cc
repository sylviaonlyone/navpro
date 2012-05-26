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

#include "PiiDiffOperation.h"
#include <PiiYdinTypes.h>
#include <complex>

PiiDiffOperation::Data::Data()
{
}

PiiDiffOperation::PiiDiffOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("difference"));
}

void PiiDiffOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);
  if (reset)
    d->lastObject = PiiVariant();
}

void PiiDiffOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_CASES(diff, obj);
      PII_COMPLEX_CASES(diff, obj);
      PII_NUMERIC_MATRIX_CASES(matrixDiff, obj);
      PII_COMPLEX_MATRIX_CASES(matrixDiff, obj);

    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiDiffOperation::matrixDiff(const PiiVariant& obj)
{
  diff<PiiMatrix<T> >(obj);
}

template <class T> void PiiDiffOperation::diff(const PiiVariant& obj)
{
  PII_D;
  // If there is a previous object, emit the difference between this and it
  if (d->lastObject.isValid())
    {
      if (obj.type() != d->lastObject.type())
        PII_THROW(PiiExecutionException, tr("Cannot calculate the difference between objects of different type."));

      try
        {
          emitObject(obj.valueAs<T>() - d->lastObject.valueAs<T>());
        }
      catch (PiiMathException&)
        {
          PII_THROW(PiiExecutionException, tr("Cannot calculate the difference between matrices of different size."));
        }
    }
  // There is no previous object -> emit zero (subtract from itself)
  else
    emitObject(obj.valueAs<T>() - obj.valueAs<T>());
    
  d->lastObject = obj;
}
