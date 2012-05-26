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

#ifndef _PIIMATRIXSERIALIZATION_H
#define _PIIMATRIXSERIALIZATION_H

#include "PiiMatrix.h"
#include <PiiSmartPtr.h>
#include <PiiSerializationTraits.h>
#include <PiiNameValuePair.h>
#include <PiiBinaryObject.h>
#include <PiiSerializationException.h>
#include <PiiSerialization.h>

/// @cond null

namespace PiiSerialization
{
  template <class Archive, class T> void save(Archive& archive, const PiiMatrix<T>& mat, const unsigned int /*version*/)
  {
    int iRows = mat.rows(), iCols = mat.columns();
    archive << PII_NVP("rows", iRows);
    archive << PII_NVP("cols", iCols);
    unsigned int uiBytes = iCols*sizeof(T);
    for (int r=0; r<iRows; ++r)
      archive.writeRawData(mat[r], uiBytes);
  }

  template <class Archive, class T> void load(Archive& archive, PiiMatrix<T>& mat, const unsigned int /*version*/)
  {
    int iRows, iCols;
    archive >> PII_NVP("rows", iRows);
    archive >> PII_NVP("cols", iCols);

    if (iRows < 0 || iCols < 0)
      PII_SERIALIZATION_ERROR(InvalidDataFormat);
    
    mat.resize(iRows, iCols);

    unsigned int uiBytes = iCols*sizeof(T);
    for (int r=0; r<iRows; ++r)
      archive.readRawData(mat[r], uiBytes);
  }

  template <class Archive, class T> inline void serialize(Archive& archive, PiiMatrix<T>& mat, const unsigned int version)
  {
    separateFunctions(archive, mat, version);
  }
}

PII_SERIALIZATION_TRACKING_TEMPLATE(PiiMatrix, false);

/// @endcond

#endif //_PIIMATRIXSERIALIZATION_H
