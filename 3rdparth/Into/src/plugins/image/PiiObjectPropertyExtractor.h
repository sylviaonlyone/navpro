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

#ifndef _PIIOBJECTPROPERTYEXTRACTOR_H
#define _PIIOBJECTPROPERTYEXTRACTOR_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>
#include <QPair>

/**
 * Calculates geometric properties for labeled objects.
 *
 * @inputs
 *
 * @in image - a labeled image. Each object in the image must have a
 * unique integer label that distinguishes it from others. (any
 * integer-valued matrix)
 *
 * @in labels - the number of distinct objects. This input is
 * optional. (int)
 *
 * @outputs
 *
 * @out areas - the number of pixels on each object. 
 * PiiMatrix<int>(N,1).
 *
 * @out centroids - the center-of-mass point (x,y) for each object. 
 * PiiMatrix<int>(N,2).
 *
 * @out boundingboxes - The bounding boxes of each object
 * (x,y,width,height). PiiMatrix<int>(N,4).
 *
 * @ingroup PiiImagePlugin
 */
class PiiObjectPropertyExtractor : public PiiDefaultOperation
{
  Q_OBJECT

  PII_OPERATION_SERIALIZATION_FUNCTION
public:

  PiiObjectPropertyExtractor();

protected:
  void process();
  
private:
  template <class T> void operate(const PiiVariant& img, int labels);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiInputSocket* pLabeledImageInput;
    PiiInputSocket* pLabelsInput;
    PiiOutputSocket* pAreasOutput;
    PiiOutputSocket* pCentroidsOutput;
    PiiOutputSocket* pBoundingBoxOutput;
  };
  PII_D_FUNC;
};

#endif //_PIIOBJECTPROPERTYEXTRACTOR_H
