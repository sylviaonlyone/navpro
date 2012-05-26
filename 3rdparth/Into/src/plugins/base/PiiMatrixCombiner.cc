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

#include "PiiMatrixCombiner.h"

#include <PiiYdinTypes.h>
#include <PiiMath.h>

using namespace PiiYdin;

PiiMatrixCombiner::Data::Data() :
  iRows(1), iColumns(0)
{
}

PiiMatrixCombiner::PiiMatrixCombiner() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  setDynamicInputCount(2);
  addSocket(new PiiOutputSocket("compound"));
}

int PiiMatrixCombiner::dynamicInputCount() const
{
	return inputCount();
}

void PiiMatrixCombiner::setDynamicInputCount(int cnt)
{
  setNumberedInputs(cnt);
}

void PiiMatrixCombiner::process()
{
  int cnt = inputCount();
  
  // Let's see if all inputs are compatible to each other. Rules:
  // 1. If any of the inputs is not a matrix, stop.
  // 2. Primitive matrices are compatible with complex types
  // 3. Color doesn't mix with other types

  int maxType = 0;
  PiiVariant::PrimitiveType maxPrimitive = PiiVariant::CharType;
  bool primitiveFound = false, colorFound = false, complexFound = false;
  QSize maxSize(0,0);
  
  for (int i=0; i<cnt; ++i)
    {
      PiiVariant obj = readInput(i);
      int type = obj.type();
      // Matrices occupy this space
      if (!PiiYdin::isMatrixType(type))
        PII_THROW_UNKNOWN_TYPE(inputAt(i));

      if (type == BoolMatrixType)
        type = UnsignedCharMatrixType;

      if (type < BoolMatrixType)
        {
          PiiVariant::PrimitiveType priType = (PiiVariant::PrimitiveType)(type & 0x1f);
          if (priType > maxPrimitive)
            maxPrimitive = priType;
          primitiveFound = true;
        }
      else if (type >= UnsignedCharColorMatrixType && type <= FloatColorMatrixType)
        colorFound = true;
      else if (type >= IntComplexMatrixType && type <= DoubleComplexMatrixType)
        complexFound = true;
      else
        PII_THROW_UNKNOWN_TYPE(inputAt(i));

      if (type > maxType)
        maxType = type;

      QSize size;
      switch (type)
        {
          PII_ALL_MATRIX_CASES(size = matrixSize, obj);
          PII_COLOR_IMAGE_CASES(size = matrixSize, obj);
        }
      maxSize = maxSize.expandedTo(size);
    }

  if (colorFound)
    {
      if (complexFound || primitiveFound)
        PII_THROW(PiiExecutionException, tr("Cannot mix color matrices with other types."));
    }
  else if (!complexFound)
    maxType = 0x40 + maxPrimitive;

  switch (maxType)
    {
      PII_PRIMITIVE_MATRIX_CASES(buildCompound, maxSize);
      PII_COLOR_IMAGE_CASES(buildCompound, maxSize);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> QSize PiiMatrixCombiner::matrixSize(const PiiVariant& obj)
{
  const PiiMatrix<T> mat = obj.valueAs<PiiMatrix<T> >();
  return QSize(mat.columns(), mat.rows());
}


template <class T> void PiiMatrixCombiner::buildCompound(QSize maxSize)
{
  PII_D;
  int cnt = inputCount();
  int rows = 0, columns = 0;
  
  if (d->iColumns > 0)
    {
      columns = d->iColumns;
      rows = cnt/columns;
      if (cnt % columns != 0)
        ++rows;
    }
  else if (d->iRows > 0)
    {
      rows = d->iRows;
      columns = cnt/rows;
      if (cnt % rows != 0)
        ++columns;
    }
  else
    {
      rows = 1;
      columns = cnt;
    }

  PiiMatrix<T> matResult(rows * maxSize.height(), columns * maxSize.width());

  Pii::IfClass<Pii::IsPrimitive<T>,
    PrimitiveBuilder,
    ColorBuilder>::Type::buildCompound(this, matResult, maxSize, columns);
  emitObject(matResult);
}

struct PiiMatrixCombiner::PrimitiveBuilder
{
  template <class T> static void buildCompound(PiiMatrixCombiner* combiner, PiiMatrix<T>& result, QSize maxSize, int columns)
  {
    for (int i=0; i<combiner->inputCount(); ++i)
      {
        PiiVariant obj = combiner->readInput(i);
        PiiMatrix<T> block;
        switch (obj.type())
          {
            PII_PRIMITIVE_MATRIX_CASES(block = (PiiMatrix<T>)PiiYdin::matrixAs, obj);
          default:
            qDebug("PiiMatrixCombiner: unrecognized object in input %d (type 0x%x)", i, obj.type());
          }
        if (!block.isEmpty())
          result((i/columns)*maxSize.height(), (i%columns)*maxSize.width(), block.rows(), block.columns()) << block;
      }
  }
};

struct PiiMatrixCombiner::ColorBuilder
{
  template <class T> static void buildCompound(PiiMatrixCombiner* combiner, PiiMatrix<T>& result, QSize maxSize, int columns)
  {
    for (int i=0; i<combiner->inputCount(); ++i)
      {
        PiiVariant obj = combiner->readInput(i);
        PiiMatrix<T> block;
        switch (obj.type())
          {
            PII_COLOR_IMAGE_CASES(block = (PiiMatrix<T>)PiiYdin::matrixAs, obj);
          default:
            qDebug("PiiMatrixCombiner: unrecognized object in input %d (type 0x%x)", i, obj.type());
          }
        if (!block.isEmpty())
          result((i/columns)*maxSize.height(), (i%columns)*maxSize.width(), block.rows(), block.columns()) << block;
      }
  }
};

void PiiMatrixCombiner::setRows(int rows) { _d()->iRows = rows; }
int PiiMatrixCombiner::rows() const { return _d()->iRows; }
void PiiMatrixCombiner::setColumns(int columns) { _d()->iColumns = columns; }
int PiiMatrixCombiner::columns() const { return _d()->iColumns; }
