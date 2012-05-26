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

#include "PiiMatrixDecomposer.h"
#include <PiiYdinTypes.h>

PiiMatrixDecomposer::PiiMatrixDecomposer() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  setDynamicOutputCount(1);
}

void PiiMatrixDecomposer::setDynamicOutputCount(int cnt)
{
  if (cnt < 1)
    return;
  setNumberedOutputs(cnt);
}

int PiiMatrixDecomposer::dynamicOutputCount() const { return outputCount(); }

void PiiMatrixDecomposer::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(operate, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template<class T>  void PiiMatrixDecomposer::operate(const PiiVariant& obj)
{
  const PiiMatrix<T> matrix = obj.valueAs<PiiMatrix<T> >();
  if (matrix.rows() != 1)
    PII_THROW_WRONG_SIZE(inputAt(0), matrix, 1, tr("N"));
  if (matrix.columns() != outputCount())
    PII_THROW_WRONG_SIZE(inputAt(0), matrix, 1, outputCount());
  
  for (int i=outputCount(); i--; )
    emitObject(matrix(0,i), i);
}
