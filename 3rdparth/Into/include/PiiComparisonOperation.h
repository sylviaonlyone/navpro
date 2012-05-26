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

#ifndef _PIICOMPARISONOPERATION_H
#define _PIICOMPARISONOPERATION_H

#include <PiiDefaultOperation.h>
#include <QPair>
#include <PiiMatrix.h>

/**
 * Compare two values and output a binary outcome. Two inputs are
 * compared with a predefined comparison operator. The object read
 * from @p input0 is always used as the left-hand operand. The
 * right-hand operand is either a #constant or the object read from @p
 * input1, if it is connected.
 *
 * @inputs
 *
 * @in input0 - first input object. Any number or numeric matrix.
 *
 * @in input1 - second (optional) input object. Any number or numeric
 * matrix. If @p input0 contains a scalar, this must be a scalar. If
 * @p input0 contains a matrix, this must be either a scalar or a
 * matrix with the same size.
 * 
 * @outputs
 *
 * @out output - The comparison result, bool or PiiMatrix<bool>.
 *
 * @ingroup PiiBasePlugin
 */
class PiiComparisonOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * A constant value that is used instead of @p input1 if it is not
   * connected.
   */
  Q_PROPERTY(double constant READ constant WRITE setConstant);

  /**
   * The comparison function.
   */
  Q_PROPERTY(Function function READ function WRITE setFunction);
  Q_ENUMS(Function);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * @lip Equal - left == right
   * @lip LessThan - left < right
   * @lip GreaterThan - left > right
   * @lip LessEqual - left <= right
   * @lip GreaterEqual - left >= right
   * @lip NotEqual - left != right
   */
  enum Function { Equal, LessThan, GreaterThan, LessEqual, GreaterEqual, NotEqual };
  
  PiiComparisonOperation();

  void setConstant(double constant);
  double constant() const;
  void setFunction(Function op);
  Function function() const;

  void check(bool reset);

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dConstant;
    Function function;
    PiiInputSocket *pInput0, *pInput1;
    bool bInput1Connected;
  };
  PII_D_FUNC;

  template <class T> void operateMatrix(const PiiVariant& obj);
  template <class T, class U> void operateMatrixNumber(const PiiMatrix<U>& matrix,const PiiVariant& obj);
  template <class T, class U> void operateMatrixMatrix(const PiiMatrix<U>& matrix,const PiiVariant& obj);
  template <class T> void operateNumber(const PiiVariant& obj);
  template <class T, class U> void operateNumberNumber(U number, const PiiVariant& obj);
  template <class T, class U> void compare(const T& op1, const U& op2);
};


#endif //_PIICOMPARISONOPERATION_H
