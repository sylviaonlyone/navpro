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

#include "PiiMathematicalFunction.h"

#include <PiiYdinTypes.h>

PiiMathematicalFunction::Data::Data() :
  function(NoFunction)
{
}

PiiMathematicalFunction::PiiMathematicalFunction() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

void PiiMathematicalFunction::process()
{
  PiiVariant obj = readInput();
  
  switch (_d()->function)
    {
    case NoFunction:
      emitObject(obj);
      break;

    case Abs:
      switch (obj.type())
        {
          PII_NUMERIC_CASES(abs, obj);
          PII_NUMERIC_MATRIX_CASES(absMat, obj);
          PII_COMPLEX_CASES(abs, obj);
          PII_COMPLEX_MATRIX_CASES(absMat, obj);
        }
      return;

    case Log:
      switch (obj.type())
        {
          PII_NUMERIC_CASES(log, obj);
          PII_NUMERIC_MATRIX_CASES(absMat, obj);
        }
      return;

    case Sqrt:
      switch (obj.type())
        {
          PII_NUMERIC_CASES(sqrt, obj);
          PII_NUMERIC_MATRIX_CASES(sqrtMat, obj);
        }
      return;

    case Square:
      switch (obj.type())
        {
          PII_NUMERIC_CASES(square, obj);
          PII_NUMERIC_MATRIX_CASES(squareMat, obj);
          PII_COMPLEX_CASES(square, obj);
          PII_COMPLEX_MATRIX_CASES(squareMat, obj);
        }
      return;

    case Sin:
      switch (obj.type())
        {
          PII_NUMERIC_CASES(sin, obj);
          PII_NUMERIC_MATRIX_CASES(sinMat, obj);
        }
      return;

    case Cos:
      switch (obj.type())
        {
          PII_NUMERIC_CASES(cos, obj);
          PII_NUMERIC_MATRIX_CASES(cosMat, obj);
        }
      return;

    case Tan:
      switch (obj.type())
        {
          PII_NUMERIC_CASES(tan, obj);
          PII_NUMERIC_MATRIX_CASES(tanMat, obj);
        }
      return;

    case Std:
      switch (obj.type())
        {
          PII_NUMERIC_MATRIX_CASES(stdAll, obj);
        }
      return;

    case Var:
      switch (obj.type())
        {
          PII_NUMERIC_MATRIX_CASES(varAll, obj);
        }
      return;

    case Mean:
      switch (obj.type())
        {
          PII_NUMERIC_MATRIX_CASES(meanAll, obj);
        }
      return;
    }
  
  PII_THROW_UNKNOWN_TYPE(inputAt(0));
}

void PiiMathematicalFunction::setFunction(Function function) { _d()->function = function; }
PiiMathematicalFunction::Function PiiMathematicalFunction::function() const { return _d()->function; }
