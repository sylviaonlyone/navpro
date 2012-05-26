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

#ifndef _PIIMATRIXDECOMPOSER_H
#define _PIIMATRIXDECOMPOSER_H

#include <PiiDefaultOperation.h>

/**
 * Decomposes input matrix so that each row element corresponds one
 * output. So input matrix must be row matrix and size of columns must
 * be same as output count. You must set output counts with
 * outputCount-property first, default is 1.
 *
 * @inputs
 *
 * @in input - any row matrix
 * 
 * @outputs
 *
 * @out outputX - outputX is element (0,X) in input matrix.
 *
 * @ingroup PiiBasePlugin
 */
class PiiMatrixDecomposer : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Specifies the number of the output sockets.
   */
  Q_PROPERTY(int dynamicOutputCount READ dynamicOutputCount WRITE setDynamicOutputCount);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiMatrixDecomposer();

  int dynamicOutputCount() const;
  void setDynamicOutputCount(int cnt);

protected:
  void process();

private:
  template<class T>  void operate(const PiiVariant& obj);
};


#endif //_PIIMATRIXDECOMPOSER_H
