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

#ifndef _PIILOGOPERATION_H
#define _PIILOGOPERATION_H

#include <PiiDefaultOperation.h>

/**
 * An operation that calculates the logarithm of its input.
 *
 * @inputs
 *
 * @in input - any number or numeric matrix.
 * 
 * @outputs
 *
 * @out output - the logarithm as a double scalar or a double matrix.
 *
 * @ingroup PiiBasePlugin
 */
class PiiLogOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The base of the logarithm. The default value is @e e.
   */
  Q_PROPERTY(double base READ base WRITE setBase);

  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  PiiLogOperation();

  void setBase(double base);
  double base() const;

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dBase, dOnePerLogBase;
  };
  PII_D_FUNC;

  template <class T> void logarithm(const PiiVariant& obj);
  template <class T> void matrixLogarithm(const PiiVariant& obj);
};

#endif //_PIILOGOPERATION_H
