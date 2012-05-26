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

#include "PiiMorphologyOperation.h"
#include "PiiMorphology.h"
#include <PiiYdinTypes.h>
#include <cmath>

PiiMorphologyOperation::Data::Data() :
  operation(PiiImage::Erode),
  maskType(PiiImage::RectangularMask),
  bHandleBorders(false),
  maskSize(QSize(3,3))
{
}

PiiMorphologyOperation::PiiMorphologyOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  d->pImageInput = new PiiInputSocket("image");
  d->pBinaryImageOutput = new PiiOutputSocket("image");

  addSocket(d->pImageInput);
  addSocket(d->pBinaryImageOutput);

  prepareMask();
}


void PiiMorphologyOperation::process()
{
  PII_D;
  PiiVariant obj = d->pImageInput->firstObject();

  if ( d->maskSize.width() == 0 || d->maskSize.height() == 0 )
    d->pBinaryImageOutput->emitObject(obj);
  
  switch (obj.type())
    {
      PII_GRAY_IMAGE_CASES(morphologyOperation, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }  
}


template <class T> void PiiMorphologyOperation::morphologyOperation(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> image = obj.valueAs<PiiMatrix<T> >();

  // Ensure that the image is large enough
  if (image.rows() < d->matMask.rows() || image.columns() < d->matMask.columns())
    {
      d->pBinaryImageOutput->emitObject(obj);
      return;
    }
  
  d->pBinaryImageOutput->emitObject(PiiImage::morphology(image,
                                                         d->matMask,
                                                         d->operation,
                                                         d->bHandleBorders));
}

void PiiMorphologyOperation::prepareMask()
{
  PII_D;
  d->matMask = PiiImage::createMask(d->maskType, d->maskSize.height(), d->maskSize.width());
}

PiiImage::MorphologyOperation PiiMorphologyOperation::operation() const { return _d()->operation; }
void PiiMorphologyOperation::setOperation(PiiImage::MorphologyOperation operation) { _d()->operation = operation; }
void PiiMorphologyOperation::setHandleBorders(bool handleBorders) { _d()->bHandleBorders = handleBorders; }
bool PiiMorphologyOperation::handleBorders() const { return _d()->bHandleBorders; }
PiiImage::MaskType PiiMorphologyOperation::maskType() const { return _d()->maskType; }
void PiiMorphologyOperation::setMaskType(PiiImage::MaskType maskType) { _d()->maskType = maskType; prepareMask(); }
QSize PiiMorphologyOperation::maskSize() const { return _d()->maskSize; }
void PiiMorphologyOperation::setMaskSize(QSize maskSize) { _d()->maskSize = maskSize; prepareMask(); }
PiiMatrix<int> PiiMorphologyOperation::mask() const { return _d()->matMask; }
template <class T> void PiiMorphologyOperation::setMask(PiiMatrix<T> mask) { _d()->matMask = mask != 0; }
