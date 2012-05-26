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

#include "PiiMultiVariableHistogram.h"
#include <PiiUtil.h>
#include <PiiYdinTypes.h>
#include <PiiMath.h>

#define LENGTH_LIMIT (1<<24)

PiiMultiVariableHistogram::Data::Data() :
  distributionType(JointDistribution),
  bNormalized(false)
{
}

PiiMultiVariableHistogram::PiiMultiVariableHistogram() :
  PiiDefaultOperation(new Data, Threaded)
{
  setInputCount(1);

  addSocket(_d()->pHistogramOutput = new PiiOutputSocket("histogram"));
}

PiiMultiVariableHistogram::~PiiMultiVariableHistogram()
{
}

QVariantList PiiMultiVariableHistogram::levels() const
{
  return Pii::vectorToVariants(_d()->vecLevels);
}

void PiiMultiVariableHistogram::setLevels(const QVariantList& levels)
{
  if (levels.size() < 1 || levels.size() > 8)
    return;
  _d()->vecLevels = Pii::variantsToVector<int>(levels);
  setInputCount(levels.size());
}

void PiiMultiVariableHistogram::setScales(const QVariantList& scales)
{
  _d()->vecScales = Pii::variantsToVector<double>(scales);
}

QVariantList PiiMultiVariableHistogram::scales() const
{
  return Pii::vectorToVariants(_d()->vecScales);
}

void PiiMultiVariableHistogram::check(bool reset)
{
  PII_D;
  
  PiiDefaultOperation::check(reset);
  
  d->vecSteps.clear();

  d->vecSteps << d->vecLevels[0];
  if (d->vecSteps[0] > LENGTH_LIMIT)
    throwTooLong();

  if (d->distributionType == JointDistribution)
    {
      // The first multiplier (1) is omitted. The last one tells the total
      // length of the histogram.
      for (int i=1; i<d->vecLevels.size(); i++)
        {
          d->vecSteps << d->vecSteps[i-1]*d->vecLevels[i];
          if (d->vecLevels[i] > LENGTH_LIMIT || d->vecSteps[i] > LENGTH_LIMIT)
            throwTooLong();
        }
    }
  else
    {
      for (int i=1; i<d->vecLevels.size(); i++)
        {
          d->vecSteps << d->vecSteps[i-1] + d->vecLevels[i];
          if (d->vecLevels[i] > LENGTH_LIMIT || d->vecSteps[i] > LENGTH_LIMIT)
            throwTooLong();
        }
    }

  if (d->vecScales.size() != 0 && d->vecScales.size() != d->vecLevels.size())
    PII_THROW(PiiExecutionException, tr("If scales are given, there must be one for each dimension."));
}

void PiiMultiVariableHistogram::setDistributionType(DistributionType distributionType)
{
  _d()->distributionType = distributionType;
}

PiiMultiVariableHistogram::DistributionType PiiMultiVariableHistogram::distributionType() const
{
  return _d()->distributionType;
}

void PiiMultiVariableHistogram::setInputCount(int cnt)
{
  // Add sockets if needed
  PiiBasicOperation::setNumberedInputs(cnt, 0, "matrix");
}

template <class T> PiiMatrix<int> PiiMultiVariableHistogram::scale(const PiiVariant& obj, double factor)
{
  return obj.valueAs<PiiMatrix<T> >().mapped(Pii::unaryCompose(Pii::Round<int>(),
                                                               std::bind2nd(std::multiplies<double>(),
                                                                            factor)));
}

void PiiMultiVariableHistogram::process()
{
  PII_D;
  
  // Input matrices are stored into a list (either as such or scaled).
  QList<PiiMatrix<int> > lstMatrices;

  int iRows = 0, iColumns = 0;
  // Convert all input objects into int matrices
  for (int i=0; i<inputCount(); ++i)
    {
      // If scaling factors have not been set or the current one
      // equals one, no scaling is needed.
      bool bScaled = d->vecScales.size() > i && d->vecScales[i] != 1;
      PiiVariant obj = inputAt(i)->firstObject();
      // If the input matrix is an int matrix, no conversion is needed
      if (obj.type() == PiiYdin::IntMatrixType)
        {
          if (!bScaled)
            lstMatrices << obj.valueAs<PiiMatrix<int> >();
          else
            lstMatrices << scale<int>(obj, d->vecScales[i]);
        }
      else
        {
          PiiMatrix<int> matrix;
          // Convert type but do not scale
          if (!bScaled)
            {
              switch (obj.type())
                {
                  PII_NUMERIC_MATRIX_CASES(matrix = (PiiMatrix<int>)PiiYdin::matrixAs, obj);
                default:
                  PII_THROW_UNKNOWN_TYPE(inputAt(i));
                }
            }
          else
            {
              // Scale and convert to ints
              switch (obj.type())
                {
                  PII_NUMERIC_MATRIX_CASES_M(matrix = scale, (obj, d->vecScales[i]));
                default:
                  PII_THROW_UNKNOWN_TYPE(inputAt(i));
                }
            }
          if (i > 0 && (matrix.rows() != iRows || matrix.columns() != iColumns))
            PII_THROW_WRONG_SIZE(inputAt(i), matrix, iRows, iColumns);
          lstMatrices << matrix;
        }
      
      if (i == 0)
        {
          iRows = lstMatrices[0].rows();
          iColumns = lstMatrices[0].columns();
        }
    }

  // Allocate size for the histogram
  PiiMatrix<int> matResult(1, d->vecSteps.last());
  if (d->distributionType == JointDistribution)
    jointHistogram(lstMatrices, iRows, iColumns, &matResult);
  else
    marginalHistograms(lstMatrices, iRows, iColumns, &matResult);

  if (d->bNormalized)
    d->pHistogramOutput->emitObject(matResult.mapped(std::bind2nd(std::multiplies<double>(),
                                                                  1.0 / Pii::sumAll<double>(matResult))));
  else
    d->pHistogramOutput->emitObject(matResult);
}

void PiiMultiVariableHistogram::jointHistogram(const QList<PiiMatrix<int> >& matrices,
                                               int rows, int columns,
                                               PiiMatrix<int>* result)
{
  PII_D;
  
  const int iDimensions = matrices.size();
  for (int r=0; r<rows; ++r)
    {
      for (int c=0; c<columns; ++c)
        {
          // Fold multi-dimensional values into one dimension. Index 0
          // is treated separately to save an unnecessary
          // multiplication by one. All pixels need to be checked to
          // prevent over/underflows.
          int index = qBound(0, const_cast<const PiiMatrix<int>&>(matrices[0])(r, c), d->vecLevels[0]-1);
          for (int k=1; k<iDimensions; ++k)
            index += d->vecSteps[k-1] * qBound(0, const_cast<const PiiMatrix<int>&>(matrices[k])(r, c), d->vecLevels[k]-1);
          
          // Increase histogram at the folded index
          ++(*result)(0, index);
        }
    }
}

void PiiMultiVariableHistogram::marginalHistograms(const QList<PiiMatrix<int> >& matrices,
                                                   int rows, int columns,
                                                   PiiMatrix<int>* result)
{
  PII_D;
  
  int* pHistogram = result->row(0);
  for (int k=0; k<matrices.size(); ++k)
    {
      int iMaxValue = d->vecLevels[k]-1;
      for (int r=0; r<rows; ++r)
        {
          const int* pRow = const_cast<const PiiMatrix<int>&>(matrices[k])[r];
          for (int c=0; c<columns; ++c)
            ++pHistogram[qBound(0, pRow[c], iMaxValue)];
        }
      pHistogram += d->vecLevels[k];
    }
}

void PiiMultiVariableHistogram::setNormalized(bool normalize) { _d()->bNormalized = normalize; }
bool PiiMultiVariableHistogram::normalized() const { return _d()->bNormalized; }
