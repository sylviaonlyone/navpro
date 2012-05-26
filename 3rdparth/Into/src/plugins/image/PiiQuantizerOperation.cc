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

#include "PiiQuantizerOperation.h"

#include <PiiYdinTypes.h>

PiiQuantizerOperation::Data::Data() :
  iLevels(16),
  bTraining(false),
  iTrainingPixels(100000),
  iCollectionIndex(0),
  dSelectionProbability(1.0),
  pCollectedData(0)
{
}

PiiQuantizerOperation::PiiQuantizerOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("image"));
}

PiiQuantizerOperation::~PiiQuantizerOperation()
{
  PII_D;
  delete[] d->pCollectedData;
}

void PiiQuantizerOperation::setLimits(const QVariantList& limits)
{
  PII_D;
  PiiMatrix<double> limitMat(1, limits.size());
  for (int i=limits.size(); i--; ) limitMat(i) = limits[i].toDouble();
  d->quantizer.setLimits(limitMat);
}

QVariantList PiiQuantizerOperation::limits() const
{
  const PII_D;
  QVariantList result;
  PiiMatrix<double> limits = d->quantizer.limits();
  for (int i=0; i<limits.columns(); i++)
    result << limits(i);
  return result;
}

void PiiQuantizerOperation::process()
{
  PiiVariant obj = readInput();
  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(quantize, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

void PiiQuantizerOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);
  if (reset)
    {
      delete[] d->pCollectedData;
      d->pCollectedData = 0;
      d->iCollectionIndex = 0;
    }
}

template <class T> void PiiQuantizerOperation::quantize(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> img = obj.valueAs<PiiMatrix<T> >();

  if (d->bTraining)
    {
      if (!d->pCollectedData)
        d->pCollectedData = new double[d->iTrainingPixels];
      for (int r = img.rows(); r--; )
        {
          const T* row = img.row(r);
          for (int c = img.columns(); c--; )
            {
              // Random number between 0 and 1
              double rnd = (double)std::rand() / RAND_MAX;
              if (rnd < d->dSelectionProbability)
                {
                  d->pCollectedData[d->iCollectionIndex++] = (double)row[c];
                  // All pixels collected ...
                  if (d->iCollectionIndex >= d->iTrainingPixels)
                    {
                      d->iCollectionIndex = 0;
                      d->bTraining = false;
                      learnBoundaries();
                      goto trainingFinished;
                    }
                }
            }
        }
      if (d->iLevels > 256)
        emitObject(PiiMatrix<int>(img.rows(), img.columns()));
      else
        emitObject(PiiMatrix<unsigned char>(img.rows(), img.columns()));
      return;
    }
trainingFinished:
  
  // If 8 bits is enough...
  if (d->iLevels <= 256)
    quantize<unsigned char>(img);
  else // need more
    quantize<int>(img);
}


template <class T, class U> void PiiQuantizerOperation::quantize(const PiiMatrix<U>& img)
{
  PII_D;
  // Create an empty matrix with the same size as the input.
  PiiMatrix<T> matResult(PiiMatrix<T>::uninitialized(img.rows(), img.columns()));
  // Quantize each pixel
  for (int r = matResult.rows(); r--; )
    {
      const U* sourceRow = img.row(r);
      T* targetRow = matResult.row(r);
      for (int c = matResult.columns(); c--; )
        targetRow[c] = d->quantizer.quantize(sourceRow[c]);
    }
  // Emit result
  emitObject(matResult);
}

void PiiQuantizerOperation::learnBoundaries()
{
  PII_D;
  PiiMatrix<double> data(1, d->iTrainingPixels, d->pCollectedData);
  d->quantizer.setLimits(PiiQuantizer<double>::divideEqually(data, d->iLevels));
  qDebug("Quantization limits:");
  for (int i=0; i<d->quantizer.limits().columns(); i++)
    qDebug("%lf", d->quantizer.limits()(i));
  delete d->pCollectedData;
  d->pCollectedData = 0;
}

void PiiQuantizerOperation::setLevels(int levels) { _d()->iLevels = levels; }
int PiiQuantizerOperation::levels() const { return _d()->iLevels; }
void PiiQuantizerOperation::setTraining(bool training) { _d()->bTraining = training; }
bool PiiQuantizerOperation::training() const { return _d()->bTraining; }
void PiiQuantizerOperation::setTrainingPixels(int trainingPixels) { _d()->iTrainingPixels = trainingPixels; }
int PiiQuantizerOperation::trainingPixels() const { return _d()->iTrainingPixels; }
void PiiQuantizerOperation::setSelectionProbability(double selectionProbability) { _d()->dSelectionProbability = selectionProbability; }
double PiiQuantizerOperation::selectionProbability() const { return _d()->dSelectionProbability; }
