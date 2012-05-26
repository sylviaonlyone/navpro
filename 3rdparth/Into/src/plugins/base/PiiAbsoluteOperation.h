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

#ifndef _PIIABSOLUTEOPERATION_H
#define _PIIABSOLUTEOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>

/**
 * An operation that calculates the absolute value of any numeric or
 * complex type or a matrix containing such types.
 *
 * @inputs
 *
 * @in input - any number or a matrix containing numbers
 *
 * @outputs
 *
 * @out output - absolute value(s)
 *
 * @ingroup PiiBasePlugin
 */
class PiiAbsoluteOperation : public PiiDefaultOperation
{
  Q_OBJECT

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiAbsoluteOperation();
  
protected:
  void process();
  
private:
  template <class T> void operatePrimitive( const PiiVariant& obj );
  template <class T> void operateMatrix( const PiiVariant& obj );
};


#endif //_PIIABSOLUTEOPERATION_H
