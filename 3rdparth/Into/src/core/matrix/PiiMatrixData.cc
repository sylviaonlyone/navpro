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

#include "PiiMatrixData.h"
#include <cstdlib>
#include <cstring>
#include <new>

PiiMatrixData* PiiMatrixData::sharedNull()
{
  static PiiMatrixData nullData;
  return &nullData;
}

PiiMatrixData* PiiMatrixData::allocate(int rows, int columns, int stride)
{
  void* bfr = malloc(sizeof(PiiMatrixData) + rows * stride);
  return new (bfr) PiiMatrixData(rows, columns, stride);
}

PiiMatrixData* PiiMatrixData::reallocate(PiiMatrixData* d, int rows)
{
  // This may move the contents of d into a new memory location
  d = static_cast<PiiMatrixData*>(realloc(d, sizeof(PiiMatrixData) + rows * d->iStride));
  // If the data buffer is internal, we need to fix the data pointer
  if (d->bufferType == InternalBuffer)
    d->pBuffer = d->bufferAddress();
  return d;
}

void PiiMatrixData::destroy()
{
  if (bufferType == ExternalOwnBuffer)
    free(pBuffer);
  else if (pSourceData != 0)
    pSourceData->release();
  free(this);
}

PiiMatrixData* PiiMatrixData::createUninitializedData(int rows, int columns, int bytesPerRow, int stride)
{
  if (stride < bytesPerRow)
    stride = alignedWidth(bytesPerRow);
  PiiMatrixData* pData = allocate(rows, columns, stride);
  pData->bufferType = InternalBuffer;
  pData->pBuffer = pData->bufferAddress();
  return pData;
}

PiiMatrixData* PiiMatrixData::createInitializedData(int rows, int columns, int bytesPerRow, int stride)
{
  PiiMatrixData* pData = createUninitializedData(rows, columns, bytesPerRow, stride);
  memset(pData->pBuffer, 0, pData->iStride*rows);
  return pData;
}

PiiMatrixData* PiiMatrixData::createReferenceData(int rows, int columns, int stride, void* buffer)
{
  PiiMatrixData* pData = allocate(0, columns, stride);
  pData->iRows = rows;
  pData->iCapacity = rows;
  pData->bufferType = ExternalBuffer;
  pData->pBuffer = buffer;
  return pData;
}

PiiMatrixData* PiiMatrixData::clone(int capacity, int bytesPerRow)
{
  PiiMatrixData* pData;
  int iNewRows = qMax(capacity, iRows);
  // If this is not a submatrix, retain the full width.
  if (pSourceData == 0)
    pData = createUninitializedData(iNewRows, iColumns, iStride, iStride);
  // Submatrices are truncated to minimum (aligned) width when cloning.
  else
    pData = createUninitializedData(iNewRows, iColumns, bytesPerRow);

  // Equal strides -> can copy the full contents at once
  if (pData->iStride == iStride)
    memcpy(pData->pBuffer, pBuffer, iStride * iRows);
  // Need to copy each row separately
  else
    {
      for (int i=0; i<iRows; ++i)
        memcpy(pData->row(i), row(i), bytesPerRow);
    }
  pData->iRows = iRows;
  return pData;
}
