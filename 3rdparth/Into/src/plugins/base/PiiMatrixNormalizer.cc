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

#include "PiiMatrixNormalizer.h"
#include <PiiMath.h>
#include <PiiYdinTypes.h>

PiiMatrixNormalizer::Data::Data() :
  dMin(0.0), dMax(1.0), dMean(0.0), dVariance(0.0),
  iOutputType(PiiYdin::DoubleMatrixType)
{
}

PiiMatrixNormalizer::PiiMatrixNormalizer() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

void PiiMatrixNormalizer::check(bool reset)
{
  PiiDefaultOperation::check(reset);
  PII_D;
  if (d->iOutputType < PiiYdin::CharMatrixType ||
      d->iOutputType > PiiYdin::DoubleMatrixType)
    throwOutputTypeError();
}

void PiiMatrixNormalizer::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(normalize, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiMatrixNormalizer::normalize(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> mat = obj.valueAs<PiiMatrix<T> >();

  double scale = 1.0, preShift = 0.0, postShift = 0.0;
  if (d->normalizationMode == NormalizeMinMax)
    {
      T minimum;
      T maximum;
      Pii::minMax(mat, &minimum, &maximum);

      preShift = -double(minimum);
      if (minimum != maximum)
        scale = (d->dMax - d->dMin)/(maximum-minimum);
      postShift = d->dMin;
    }
  else
    {
      if (d->dVariance != 0)
        {
          double mean;
          double variance = Pii::varAll<double>(mat, &mean);
          preShift = -mean;
          scale = d->dVariance/variance;
        }
      else
        preShift = -Pii::meanAll<double>(mat);
      
      postShift = d->dMean;
    }

  PiiMatrix<double> matNormalized(normalizeAs<double>(mat, preShift, scale, postShift));
  switch (d->iOutputType)
    {
      PII_NUMERIC_MATRIX_CASES(emitMatrix, matNormalized);
    default:
      throwOutputTypeError();
    }
}

template <class T> PiiMatrix<double> PiiMatrixNormalizer::normalizeAs(const PiiMatrix<T>& matrix,
                                                                      double preShift,
                                                                      double scale,
                                                                      double postShift)
{
  PiiMatrix<double> matResult(matrix);
  if (preShift != 0)
    matResult += preShift;
  if (scale != 1.0)
    matResult *= scale;
  if (postShift != 0)
    matResult += postShift;
  return matResult;
}

void PiiMatrixNormalizer::setMin(double min) { _d()->dMin = min; }
double PiiMatrixNormalizer::min() const { return _d()->dMin; }
void PiiMatrixNormalizer::setMax(double max) { _d()->dMax = max; }
double PiiMatrixNormalizer::max() const { return _d()->dMax; }
void PiiMatrixNormalizer::setOutputType(int outputType) { _d()->iOutputType = outputType; }
int PiiMatrixNormalizer::outputType() const { return _d()->iOutputType; }
void PiiMatrixNormalizer::setMean(double mean) { _d()->dMean = mean; }
double PiiMatrixNormalizer::mean() const { return _d()->dMean; }
void PiiMatrixNormalizer::setVariance(double variance) { _d()->dVariance = variance; }
double PiiMatrixNormalizer::variance() const { return _d()->dVariance; }
void PiiMatrixNormalizer::setNormalizationMode(NormalizationMode normalizationMode) { _d()->normalizationMode = normalizationMode; }
PiiMatrixNormalizer::NormalizationMode PiiMatrixNormalizer::normalizationMode() const { return _d()->normalizationMode; }
