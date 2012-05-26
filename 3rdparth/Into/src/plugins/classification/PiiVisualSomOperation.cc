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

#include "PiiVisualSomOperation.h"
#include <PiiQImageUtil.h>

PiiVisualSomOperation::Data::Data() :
  dClassification(NAN),
  iFakedClassification(0)
{
}

PiiVisualSomOperation::PiiVisualSomOperation() :
  PiiSomOperation::Template<PiiMatrix<double> >(new Data)
{
  PII_D;
  d->pImageInput = new PiiInputSocket("image");
  addSocket(d->pImageInput);

  d->limiter.setMaxFrequency(10);
}

PiiVisualSomOperation::~PiiVisualSomOperation()
{
}

double PiiVisualSomOperation::classify()
{
  classificationOutput()->emitObject(_d()->dClassification);
  return _d()->dClassification;
}

void PiiVisualSomOperation::collectSample(double label, double weight)
{
  PII_D;

  if (d->limiter.check())
    {
      PiiVariant image = d->pImageInput->firstObject();
      d->dClassification = SuperType::classify();

      int iClassification;
      if (Pii::isNan(d->dClassification))
        {
          iClassification = d->iFakedClassification;
          d->iFakedClassification++;
          d->iFakedClassification %= (width() * height());
        }
      else
        iClassification = (int)d->dClassification;

      d->storeMutex.lock();
      SuperType::collectSample(label, weight);
      storeImage(image, iClassification);
      d->storeMutex.unlock();
    }
}

bool PiiVisualSomOperation::learnBatch()
{
  PII_D;

  d->storeMutex.lock();
  d->lstNewImages = d->lstImages;
  bool bSuccess = SuperType::learnBatch();
  d->storeMutex.unlock();

  if (!bSuccess)
    d->lstNewImages.clear();

  return bSuccess;
}

void PiiVisualSomOperation::replaceClassifier()
{
  PII_D;
  PiiMatrix<double>* pmatFeatures = d->collector.samples();
  // Classify all collected features and send images/classifications
  for (int i=0; i<pmatFeatures->rows(); i++)
    emit addSubImage(d->lstNewImages.at(i), d->pClassifier->classify(pmatFeatures->row(i)));
  d->lstNewImages.clear();
}

void PiiVisualSomOperation::storeImage(const PiiVariant& image, int classification)
{
  PII_D;

  QImagePtr pImage(0);
  switch (image.type())
    {
      PII_ALL_IMAGE_CASES(pImage = PiiYdin::createQImage, image);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }

  d->lstImages << pImage;
  
  emit addSubImage(pImage, classification);
}

void PiiVisualSomOperation::removeImage(QImagePtr image)
{
  PII_D;
  QMutexLocker lock(&d->storeMutex);

  int index = d->lstImages.indexOf(image);
  if (index > -1)
    {
      d->lstImages.removeAt(index);
      d->collector.samples()->removeRow(index);
    }
}

void PiiVisualSomOperation::setMaxFrequency(double maxFrequency) { _d()->limiter.setMaxFrequency(maxFrequency); }
double PiiVisualSomOperation::maxFrequency() { return _d()->limiter.maxFrequency(); }
