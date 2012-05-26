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

#ifndef _PIIMATRIXCOMBINER_H
#define _PIIMATRIXCOMBINER_H

#include <PiiDefaultOperation.h>
#include <QSize>

/**
 * An operation that puts multiple matrices into a grid that produces
 * a larger matrix.
 *
 * @inputs
 *
 * @in inputX - any number of input matrices. X is a zero-based index.
 * 
 * @outputs
 *
 * @out compound - a compound matrix on which the input matrices are
 * placed as denoted by the #rows an #columns properties.
 *
 * @ingroup PiiBasePlugin
 */
class PiiMatrixCombiner : public PiiDefaultOperation
{
  Q_OBJECT
  /**
   * The number of rows in the grid. If the number of rows is limited
   * (non-zero), the grid will grow horizontally. Default is 1.
   */
  Q_PROPERTY(int rows READ rows WRITE setRows);
  /**
   * The number of columns in the grid. If the number of columns is
   * limited (non-zero), the grid will grow vertically. Default is 0.
   */
  Q_PROPERTY(int columns READ columns WRITE setColumns);
  /**
   * The number of input sockets. Default is 2.
   */
  Q_PROPERTY(int dynamicInputCount READ dynamicInputCount WRITE setDynamicInputCount);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiMatrixCombiner();

  void setRows(int rows);
  int rows() const;
  void setColumns(int columns);
  int columns() const;
  void setDynamicInputCount(int count);
  int dynamicInputCount() const;

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    int iRows;
    int iColumns;
  };
  PII_D_FUNC;

  template <class T> QSize matrixSize(const PiiVariant& obj);
  template <class T> void buildCompound(QSize maxSize);

  struct PrimitiveBuilder;
  struct ColorBuilder;
  friend struct PrimitiveBuilder;
  friend struct ColorBuilder;
};


#endif //_PIIMATRIXCOMBINER_H
