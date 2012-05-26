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

#ifndef _PIIMATRIXDATA_H
#define _PIIMATRIXDATA_H

#include <QAtomicInt>
#include "PiiGlobal.h"

/// @internal
struct PII_CORE_EXPORT PiiMatrixData
{
  enum BufferType { InternalBuffer, ExternalBuffer, ExternalOwnBuffer };

  // Constructs a null data
  PiiMatrixData() :
    iRefCount(1),
    iLastRef(1),
    iRows(0),
    iColumns(0),
    iStride(0),
    iCapacity(0),
    pSourceData(0),
    pBuffer(0),
    bufferType(InternalBuffer)
  {}
  
  PiiMatrixData(int rows, int columns, int stride) :
    iRefCount(1),
    iLastRef(1),
    iRows(rows),
    iColumns(columns),
    iStride(stride),
    iCapacity(rows),
    pSourceData(0)
  {}
  
  QAtomicInt iRefCount;
  // Destroy data when iRefCount reaches this value. Default is zero.
  // Setting this value to one and increasing iRefCount by one makes
  // referenced data immutable.
  int iLastRef;
  int iRows, iColumns;
  // Number of bytes between beginnings of successive rows.
  int iStride;
  // Size of allocated buffer in rows
  int iCapacity;
  // Points to the source data if this matrix is a subwindow of
  // another matrix.
  PiiMatrixData* pSourceData;
  // Points to the first element of the matrix.
  void* pBuffer;
  BufferType bufferType;

  void* row(int index) { return static_cast<char*>(pBuffer) + iStride * index; }
  const void* row(int index) const { return static_cast<const char*>(pBuffer) + iStride * index; }

  // Aligns row width to a four-byte boundary
  static int alignedWidth(int bytes) { return (bytes + 3) & ~3; }
  // Returns a pointer to the beginning of an internally allocated buffer.
  char* bufferAddress() { return reinterpret_cast<char*>(this) + sizeof(*this); }

  void reserve() { iRefCount.ref(); }
  void release() { if (iRefCount.fetchAndAddRelaxed(-1) == iLastRef) destroy(); }

  PiiMatrixData* clone(int capacity, int bytesPerRow);

  PiiMatrixData* makeImmutable()
  {
    // Increasing refcount forces all non-const functions to clone the
    // data.
    iRefCount.ref();
    iLastRef = 2;
    return this;
  }
  
  static PiiMatrixData* sharedNull();
  static PiiMatrixData* allocate(int rows, int columns, int stride);
  static PiiMatrixData* reallocate(PiiMatrixData* d, int rows);
  static PiiMatrixData* createUninitializedData(int rows, int columns, int bytesPerRow, int stride = 0);
  static PiiMatrixData* createInitializedData(int rows, int columns, int bytesPerRow, int stride = 0);
  static PiiMatrixData* createReferenceData(int rows, int columns, int stride, void* buffer);

  void destroy();
};

#endif //_PIIMATRIXDATA_H
