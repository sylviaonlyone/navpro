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

#ifndef _PIIDIFFOPERATION_H
#define _PIIDIFFOPERATION_H

#include <PiiDefaultOperation.h>

/**
 * Calculates the difference between two successive input objects.
 *
 * @inputs
 *
 * @in input - the input object. Any numeric or complex type, or a
 * matrix containing such types. Note that the difference between
 * unsigned integers may easily underflow.
 * 
 * @outputs
 *
 * @out difference - the difference between current object and the
 * last one. The type is equal to the input type. If there is no
 * previous object, the difference is calculated with the input object
 * itself. Thus, the first difference is always zero.
 *
 * @ingroup PiiBasePlugin
 */
class PiiDiffOperation : public PiiDefaultOperation
{
  Q_OBJECT

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiDiffOperation();

protected:
  void process();
  void check(bool reset);
  
private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiVariant lastObject;
  };
  PII_D_FUNC;

  template <class T> void diff(const PiiVariant& obj);
  template <class T> inline void matrixDiff(const PiiVariant& obj);
};


#endif //_PIIDIFFOPERATION_H
