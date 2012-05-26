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

#include "PiiImageScaleOperation.h"
#include "PiiImage.h"
#include <PiiYdinTypes.h>

PiiImageScaleOperation::Data::Data() :
  scaleMode(ZoomAccordingToFactor),
  dScaleRatio(1.0),
  scaledSize(100,100),
  interpolation(LinearInterpolation)
{
}

PiiImageScaleOperation::PiiImageScaleOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("image"));
}

void PiiImageScaleOperation::process()
{
  PiiVariant image = readInput();

  switch (image.type())
    {
      PII_GRAY_IMAGE_CASES(scaleImage, image);
      PII_COLOR_IMAGE_CASES(scaleImage, image);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiImageScaleOperation::scaleImage(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> image = obj.valueAs<PiiMatrix<T> >();

  int rows = image.rows(), cols = image.columns();
  switch (d->scaleMode)
    {
    case ZoomAccordingToFactor:
      rows = (int)(d->dScaleRatio * rows + 0.5);
      cols = (int)(d->dScaleRatio * cols + 0.5);
      break;
    case ScaleToSize:
      rows = d->scaledSize.height();
      cols = d->scaledSize.width();
      break;
    case ScaleToBox:
      {
        double dScaleRatio = qMin(d->scaledSize.width() / (double)cols, d->scaledSize.height() / (double)rows);
        rows = (int)(dScaleRatio * rows + 0.5);
        cols = (int)(dScaleRatio * cols + 0.5);
        break;
      }
    default:
      {
        double currentAspectRatio = (double)cols / rows;
        if (d->scaleMode == ScaleToAspectRatioX ||
            (d->scaleMode == ScaleDownToAspectRatio && d->dScaleRatio < currentAspectRatio) ||
            (d->scaleMode == ScaleUpToAspectRatio &&  d->dScaleRatio >= currentAspectRatio))
          cols = (int)(rows * d->dScaleRatio + 0.5);
        else
          rows = (int)(cols / d->dScaleRatio + 0.5);
      }
    }
  // Do we actually need to scale the image?
  if ((rows != image.rows() || cols != image.columns()) &&
      rows > 0 && cols > 0)
    emitObject(PiiImage::scale(image, rows, cols, (Pii::Interpolation)d->interpolation));
  else // Pass the image without modification
    emitObject(obj);
}

PiiImageScaleOperation::ScaleMode PiiImageScaleOperation::scaleMode() const { return _d()->scaleMode; }
void PiiImageScaleOperation::setScaleMode(ScaleMode scaleMode) { _d()->scaleMode = scaleMode; }
double PiiImageScaleOperation::scaleRatio() const { return _d()->dScaleRatio; }
void PiiImageScaleOperation::setScaleRatio(double scaleRatio) { if (scaleRatio > 0) _d()->dScaleRatio = scaleRatio; }
QSize PiiImageScaleOperation::scaledSize() const { return _d()->scaledSize; }
void PiiImageScaleOperation::setScaledSize(QSize scaledSize) { _d()->scaledSize = scaledSize; }
PiiImageScaleOperation::Interpolation PiiImageScaleOperation::interpolation() const { return _d()->interpolation; }
void PiiImageScaleOperation::setInterpolation(Interpolation interpolation) { _d()->interpolation = interpolation; }
