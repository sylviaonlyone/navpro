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

#include <PiiAbsoluteOperation.h>
#include <PiiYdinTypes.h>
#include <PiiMath.h>

using namespace Pii;
using namespace PiiYdin;

PiiAbsoluteOperation::PiiAbsoluteOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

void PiiAbsoluteOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_CASES(operatePrimitive, obj);
      PII_COMPLEX_CASES(operatePrimitive, obj);
      PII_NUMERIC_MATRIX_CASES(operateMatrix, obj);
      PII_COMPLEX_MATRIX_CASES(operateMatrix, obj);
    default:
      emitObject(obj);
    }
}

template <class T> void PiiAbsoluteOperation::operatePrimitive( const PiiVariant& obj )
{
  emitObject(Pii::abs(obj.valueAs<T>()));
}


template <class T> void PiiAbsoluteOperation::operateMatrix( const PiiVariant& obj )
{
  emitObject(Pii::abs(obj.valueAs<PiiMatrix<T> >()));
}
