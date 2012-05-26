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


#include <PiiObjectPropertyExtractor.h>
#include <PiiMatrix.h>
#include <PiiObjectProperty.h>
#include <PiiYdinTypes.h>
#include <PiiMath.h>

PiiObjectPropertyExtractor::Data::Data()
{
}

PiiObjectPropertyExtractor::PiiObjectPropertyExtractor() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  d->pLabeledImageInput = new PiiInputSocket("image");
  d->pLabelsInput = new PiiInputSocket("labels");
  d->pLabelsInput->setOptional(true);
  
  d->pAreasOutput = new PiiOutputSocket("areas");
  d->pCentroidsOutput = new PiiOutputSocket("centroids");
  d->pBoundingBoxOutput = new PiiOutputSocket("boundingboxes");

  addSocket(d->pLabeledImageInput);
  addSocket(d->pLabelsInput);
  
  addSocket(d->pAreasOutput);
  addSocket(d->pCentroidsOutput);
  addSocket(d->pBoundingBoxOutput);
}

void PiiObjectPropertyExtractor::process()
{
  PII_D;
  PiiVariant obj = d->pLabeledImageInput->firstObject();

  int objects = -1;
  if (!d->pLabelsInput->isEmpty() && d->pLabelsInput->firstObject().type() == PiiVariant::IntType)
    objects = d->pLabelsInput->firstObject().valueAs<int>();

  switch (obj.type())
    {
      PII_UNSIGNED_MATRIX_CASES_M(operate, (obj, objects));
      PII_INTEGER_MATRIX_CASES_M(operate, (obj, objects));
    default:
      PII_THROW_UNKNOWN_TYPE(d->pLabeledImageInput);
    }  
}

template <class T> void PiiObjectPropertyExtractor::operate(const PiiVariant& img, int labels)
{
  PII_D;
  const PiiMatrix<T>& image = img.valueAs<PiiMatrix<T> >();
  if (labels == -1)
    labels = Pii::maxAll(image);

  PiiMatrix<int> areas, centroids, bbox;
  if (labels > 0)
    PiiImage::calculateProperties(image, labels, areas, centroids, bbox);
  else
    {
      areas = PiiMatrix<int>();
      centroids = PiiMatrix<int>();
      bbox = PiiMatrix<int>();
    }

  if (d->pAreasOutput->isConnected())
    d->pAreasOutput->emitObject(areas);
  if (d->pCentroidsOutput->isConnected())
    d->pCentroidsOutput->emitObject(centroids);
  if (d->pBoundingBoxOutput->isConnected())
    d->pBoundingBoxOutput->emitObject(bbox);
}
