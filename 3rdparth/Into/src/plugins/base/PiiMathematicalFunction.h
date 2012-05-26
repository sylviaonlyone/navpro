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

#ifndef _PIIMATHEMATICALFUNCTION_H
#define _PIIMATHEMATICALFUNCTION_H

#include <PiiDefaultOperation.h>
#include <PiiMath.h>

/**
 * An operation that applies a mathematical function to its input.
 *
 * @inputs
 *
 * @in input - any scalar or matrix
 * 
 * @outputs
 *
 * @out output - the result of applying a mathematical function to the
 * input. The output type depends both on the input and the function. 
 * For example, the absolute value for a complex number is real. The
 * input type will be preserved whenever possible.
 *
 * @ingroup PiiBasePlugin
 */
class PiiMathematicalFunction : public PiiDefaultOperation
{
  Q_OBJECT
  /**
   * The function to apply. The default value is @p NoFunction.
   */
  Q_PROPERTY(Function function READ function WRITE setFunction);
  Q_ENUMS(Function);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Supported mathematical functions.
   *
   * @lip NoFunction - just pass the input object
   *
   * Functions that will be applied element-wise to matrices and
   * scalars:
   *
   * @lip Abs - absolute value. Output type equals input type for all
   * but complex numbers.
   *
   * @lip Log - natural logarithm. Output type is @p double. Complex
   * numbers cause run-time exception.
   *
   * @lip Sqrt - square root. Output type is @p double. Complex
   * numbers cause run-time exception.
   *
   * @lip Square - square. Output type equals input type. Beware of
   * overflows!
   *
   * @lip Sin - sine of angle. Output type is @p double except for @p
   * float input, for which it is @p float. Complex numbers cause
   * run-time exception.
   *
   * @lip Cos - cosine of angle. Output type is @p double except for
   * @p float input, for which it is @p float. Complex numbers cause
   * run-time exception.
   *
   * @lip Tan - tangent of angle. Output type is @p double except for
   * @p float input, for which it is @p float. Complex numbers cause
   * run-time exception.
   *
   * Functions that calculate a value over all elements in a matrix. 
   * In all these cases the output type is @p double. Scalars and
   * complex-valued matrices as input cause run-time exception:
   *
   * @lip Var - variance
   * @lip Std - standard deviation
   * @lip Mean - mean
   */
  enum Function
  {
    NoFunction,
    Abs, Log, Sqrt, Square, Sin, Cos, Tan,
    Var, Std, Mean
  };
  
  PiiMathematicalFunction();

  void setFunction(Function function);
  Function function() const;

protected:
  void process();

private:
#define EMIT(obj) emitObject(obj)
#define FUNC_DEF(func) \
  template <class T> void func(const PiiVariant& obj) { EMIT(Pii::func(obj.valueAs<T>())); } \
  template <class T> void func##Mat(const PiiVariant& obj) { EMIT(Pii::func(obj.valueAs<PiiMatrix<T> >())); }

  FUNC_DEF(abs)
  FUNC_DEF(log)
  FUNC_DEF(sqrt)
  FUNC_DEF(square)
  FUNC_DEF(sin)
  FUNC_DEF(cos)
  FUNC_DEF(tan)

  template <class T> void stdAll(const PiiVariant& obj) { EMIT(Pii::stdAll<double>(obj.valueAs<PiiMatrix<T> >())); }
  template <class T> void varAll(const PiiVariant& obj) { EMIT(Pii::varAll<double>(obj.valueAs<PiiMatrix<T> >())); }
  template <class T> void meanAll(const PiiVariant& obj) { EMIT(Pii::meanAll<double>(obj.valueAs<PiiMatrix<T> >())); }

#undef EMIT
#undef FUNC_DEF
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    Function function;
  };
  PII_D_FUNC;
};


#endif //_PIIMATHEMATICALFUNCTION_H
