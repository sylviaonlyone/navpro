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


#include "PiiContrastOperation.h"
#include <PiiYdinTypes.h>
#include <PiiMath.h>

PiiContrastOperation::Data::Data() :
  type(MaxDiff),
  iRadius(1)
{
}

PiiContrastOperation::PiiContrastOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  d->pImageInput = new PiiInputSocket("image");
  addSocket(d->pImageInput);

  d->pImageOutput = new PiiOutputSocket("image");
  addSocket(d->pImageOutput);

}

void PiiContrastOperation::process()
{
  PII_D;
  PiiVariant obj = d->pImageInput->firstObject();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(contrast, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }
}

template <class T> void PiiContrastOperation::contrast(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> image = obj.valueAs<PiiMatrix<T> >();
  int margin = d->iRadius, doubleMargin = margin << 1, windowSize = doubleMargin+1;

  if (image.rows() <= doubleMargin || image.columns() <= doubleMargin)
    PII_THROW(PiiExecutionException, tr("Input image is too small"));

  switch (d->type)
    {
    case MaxDiff:
      {
        PiiMatrix<T> matResult(PiiMatrix<T>::uninitialized(image.rows()-doubleMargin, image.columns()-doubleMargin));
        for (int r=margin; r<image.rows()-margin; r++)
          {
            const T* centerRow = image.row(r);
            T* resultRow = matResult.row(r-margin);
            for (int c=margin; c<image.columns()-margin; c++)
              {
                T center = centerRow[c];
                T maxDiff = 0;
                for (int neighborY=r-margin; neighborY<r+margin; neighborY++)
                  {
                    const T* neighborRow = image.row(neighborY);
                    for (int neighborX=c-margin; neighborX<c+margin; neighborX++)
                      {
                        T neighbor = neighborRow[neighborX];
                        T diff = Pii::abs(neighbor - center);
                        if (diff > maxDiff)
                          maxDiff = diff;
                      }
                  }
                resultRow[c-margin] = maxDiff;
              }
          }
        d->pImageOutput->emitObject(matResult);
      }
      break;
    case LocalVar:
      {
        PiiMatrix<float> matResult(PiiMatrix<float>::uninitialized(image.rows()-doubleMargin,
                                                                   image.columns()-doubleMargin));
        for (int r=matResult.rows(); r--; )
          {
            float *resultRow = matResult.row(r);
            for (int c=matResult.columns(); c--; )
              resultRow[c] = Pii::varAll<float,T>(image(r,c, windowSize, windowSize));
          }
        d->pImageOutput->emitObject(matResult);
      }
      break;
    }
}

PiiContrastOperation::ContrastType PiiContrastOperation::type() const { return _d()->type; }
void PiiContrastOperation::setType(ContrastType type) { _d()->type = type; }
int PiiContrastOperation::radius() const { return _d()->iRadius; }
void PiiContrastOperation::setRadius(int radius) { _d()->iRadius = radius; }
