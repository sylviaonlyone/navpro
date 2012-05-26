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

#ifndef _PIITYPECASTINGOPERATION_H
#define _PIITYPECASTINGOPERATION_H

#include <PiiDefaultOperation.h>
#include <QPair>
#include <PiiMatrix.h>

/**
 * An operation that converts the data type of a matrix.
 *
 * @inputs
 *
 * @in input - any matrix.
 *
 * @outputs
 *
 * @out output - a matrix whose type is specified by #outputType.
 *
 * @ingroup PiiBasePlugin
 */
class PiiTypeCastingOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The type id of the output. See PiiYdinTypes for valid type id
   * numbers. The default is 0x48 (PiiMatrix<unsigned char>).
   */
  Q_PROPERTY(int outputType READ outputType WRITE setOutputType);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiTypeCastingOperation();
  ~PiiTypeCastingOperation();
    
protected:
  void process();

  int outputType() const;
  void setOutputType(int outputType);

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    
    int outputType;
  };
  PII_D_FUNC;
  
  template <class T> void operate( const PiiVariant& obj );
  template <class T> void operateC( const PiiVariant& obj );
  template <class T> void operateCI( const PiiVariant& obj );
  template <class T, class U> void operateMatrix( const PiiMatrix<U>& matrix );
  template <class T, class U> void operateComplex( const PiiMatrix<U>& matrix );
  template <class T, class U> void operateColorToGray( const PiiMatrix<U>& matrix );
};

#endif //_PIITYPECASTINGOPERATION_H
