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

#include "PiiImageGeometryOperation.h"

#include <PiiYdinTypes.h>

const char* PiiImageGeometryOperation::_pFeatureNames[PII_IMAGE_GEOMETRY_OPERATION_FEATURECNT] = {
  "Width",
  "Height",
  "Area",
  "AspectRatio"
};

PiiImageGeometryOperation::PiiImageGeometryOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("features"));
}

QStringList PiiImageGeometryOperation::features() const
{
  QStringList result;
  for (int i=0; i<PII_IMAGE_GEOMETRY_OPERATION_FEATURECNT; i++)
    result << _pFeatureNames[i];
  return result;
}


void PiiImageGeometryOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_ALL_MATRIX_CASES(calculateFeatures, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiImageGeometryOperation::calculateFeatures(const PiiVariant& obj)
{
  const PiiMatrix<T> img = obj.valueAs<PiiMatrix<T> >();
  double h = img.rows(), w = img.columns();

  emitObject(PiiMatrix<double>(1,4, w, h, w*h, w/h));
}
