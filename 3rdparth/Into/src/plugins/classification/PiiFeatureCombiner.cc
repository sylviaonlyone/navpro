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

#include "PiiFeatureCombiner.h"
#include "PiiClassification.h"
#include <PiiYdinResources.h>
#include <PiiYdinTypes.h>
#include <PiiAlgorithm.h>
#include <PiiAsyncCall.h>
#include <PiiMath.h>

PiiFeatureCombiner::Data::Data() :
  iTotalLength(0),
  uiMaxType(0),
  bThreadRunning(false),
  iLearningBatchSize(0),
  fullBufferBehavior(PiiFeatureCombiner::OverwriteRandomSample),
  pLearningThread(0),
  iSampleIndex(0)
{
}

PiiFeatureCombiner::PiiFeatureCombiner() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  setDynamicInputCount(2);

  d->pFeatureOutput = new PiiOutputSocket("features");
  d->pBoundaryOutput = new PiiOutputSocket("boundaries");

  addSocket(d->pFeatureOutput);
  addSocket(d->pBoundaryOutput);

  void (PiiFeatureCombiner::*func)() = &PiiFeatureCombiner::learnBatch;
  d->pLearningThread = Pii::createAsyncCall(this, func);
}

PiiFeatureCombiner::~PiiFeatureCombiner()
{
  PII_D;
  stopLearningThread();
  delete d->pLearningThread;
  qDeleteAll(d->lstDistanceMeasures);
}

void PiiFeatureCombiner::setDynamicInputCount(int cnt)
{
  if (cnt < 1 || cnt > 64)
    return;

  setNumberedInputs(cnt, 0, "features");
}

void PiiFeatureCombiner::check(bool reset)
{
  PII_D;
  if (reset)
    {
      d->iTotalLength = 0;
      d->uiMaxType = 0;
      d->varBoundaries = PiiVariant();
      d->iSampleIndex = 0;
      d->matBuffer.resize(0,0);
    }
  
  if (d->lstDistanceMeasureNames.size() != 0 &&
      d->lstDistanceMeasureNames.size() != dynamicInputCount())
    PII_THROW(PiiExecutionException, tr("The number of distance measures must match the number of feature vectors."));

  qDeleteAll(d->lstDistanceMeasures);
  d->lstDistanceMeasures.clear();
  for (int i=0; i<d->lstDistanceMeasureNames.size(); ++i)
    {
      QString strName = d->lstDistanceMeasureNames[i] + "<double>";
      MeasureType* pMeasure = PiiYdin::createResource<MeasureType>(strName);
      if (pMeasure == 0)
        PII_THROW(PiiExecutionException, tr("Cannot create %1.").arg(strName));
      d->lstDistanceMeasures << pMeasure;
    }

  PiiDefaultOperation::check(reset);
}

void PiiFeatureCombiner::process()
{
  PII_D;
  using namespace PiiYdin;

  if (d->uiMaxType == 0)
    initializeBoundaries();

  // Max type id determines the output type (smaller ids are more "primitive")
  switch (d->uiMaxType)
    {
      PII_PRIMITIVE_MATRIX_CASES(emitCompound, d->iTotalLength);
    }
}

void PiiFeatureCombiner::initializeBoundaries()
{
  PII_D;
  QMutexLocker lock(&d->learningMutex);
  // Maximum type id is the type of the "most accurate" input vector
  d->uiMaxType = 0;
  // The total length of the resulting feature vector
  d->iTotalLength = 0;
  // Boundary matrix will be reused
  d->varBoundaries = PiiVariant(PiiMatrix<int>::uninitialized(1, inputCount()));

  for (int i=0; i<inputCount(); ++i)
    {
      PiiVariant obj = inputAt(i)->firstObject();
      unsigned int uiType = obj.type();

      if (PiiVariant::isPrimitive(uiType))
        {
          uiType += PiiYdin::CharMatrixType;
          ++d->iTotalLength;
        }
      else if (PiiYdin::isMatrixType(uiType))
        d->iTotalLength += PiiYdin::matrixColumns(obj);
      else
        PII_THROW_UNKNOWN_TYPE(inputAt(i));

      // Store the maximum type id
      if (uiType > d->uiMaxType)
        d->uiMaxType = uiType;
    }
}

template <class T> void PiiFeatureCombiner::emitCompound(int totalLength)
{
  PII_D;
  PiiMatrix<T> matResult(PiiMatrix<T>::uninitialized(1, totalLength));
  T* pResultRow = matResult.row(0);
  T* pBegin = pResultRow;
  
  PiiMatrix<int>& matBoundaries = d->varBoundaries.valueAs<PiiMatrix<int> >();

  for (int i=0; i<inputCount(); ++i)
    {
      PiiVariant obj = inputAt(i)->firstObject();
      try
        {
          // No conversion is needed if input type equals output type
          if (obj.type() == Pii::typeId<PiiMatrix<T> >())
            {
              const PiiMatrix<T>& matFeatures = obj.valueAs<PiiMatrix<T> >();
              Pii::copyN(matFeatures.row(0), matFeatures.columns(), pResultRow);
              pResultRow += matFeatures.columns();
            }
          // Convert the input vector to type T
          else
            {
              int iColumns = 0;
              switch (obj.type())
                {
                  PII_PRIMITIVE_MATRIX_CASES_M(copyMatrixAs, (obj, pResultRow, &iColumns));
                  PII_PRIMITIVE_CASES_M(copyScalarAs, (obj, pResultRow, &iColumns));
                default:
                  PII_THROW_UNKNOWN_TYPE(inputAt(i));
                }
              pResultRow += iColumns;
            }
        }
      catch (PiiMathException& )
        {
          PII_THROW(PiiExecutionException, tr("The feature vector read from the \"%1%0\" input is not a row matrix.").arg("features").arg(i));
        }
      
      matBoundaries(0, i) = pResultRow - pBegin;
    }
  // If batch size is non-zero, store the compound feature vector
  // into our buffer.
  if (d->iLearningBatchSize != 0 && !learningThreadRunning())
    {
      QMutexLocker lock(&d->learningMutex);
      double* pNewRow;
      // There is still room in the batch -> append a new row
      if (d->iLearningBatchSize < 0 || d->matBuffer.rows() < d->iLearningBatchSize)
        {
          // If matrix is empty, we must resize it based on the input
          // vector.
          if (d->matBuffer.columns() == 0)
            {
              d->matBuffer.resize(0, d->iTotalLength);
              // If batch size is known, reserve enough memory now to
              // avoid reallocations altogether.
              if (d->iLearningBatchSize != -1)
                d->matBuffer.reserve(d->iLearningBatchSize);
            }
          // Allocate room for at most 64 samples each time.
          if (d->matBuffer.capacity() == d->matBuffer.rows())
            d->matBuffer.reserve(qBound(1, d->matBuffer.rows() * 2, d->matBuffer.rows() + 64));
          pNewRow = d->matBuffer.appendRow();
        }
      // No room -> overwrite one of the old ones if needed
      else if (d->fullBufferBehavior == DiscardNewSample)
        pNewRow = 0;
      else
        {
          int iOverwriteIndex = (d->fullBufferBehavior == OverwriteRandomSample) ?
            rand() : d->iSampleIndex;
          pNewRow = d->matBuffer[iOverwriteIndex % d->matBuffer.rows()];
        }
      if (pNewRow != 0)
        Pii::copyN(pBegin, d->iTotalLength, pNewRow);
      ++d->iSampleIndex;
    }

  d->pFeatureOutput->emitObject(matResult);
  d->pBoundaryOutput->emitObject(d->varBoundaries);
}

template <class T, class U> void PiiFeatureCombiner::copyMatrixAs(const PiiVariant& obj, U* row, int* columns)
{
  const PiiMatrix<T> mat = obj.valueAs<PiiMatrix<T> >();
  *columns = mat.columns();
  Pii::transformN(mat.row(0), *columns, row, Pii::Cast<T,U>());
}

template <class T, class U> void PiiFeatureCombiner::copyScalarAs(const PiiVariant& obj, U* row, int* columns)
{
  *columns = 1;
  *row = static_cast<U>(obj.valueAs<T>());
}

void PiiFeatureCombiner::learnBatch()
{
  PII_D;
  QList<double> lstDistanceWeights;
  int iFeatureStart = 0;
  const int iSampleCount = d->matBuffer.rows(), iVectorCount = inputCount(),
    iPairCount = (iSampleCount * iSampleCount - iSampleCount)/2, iTotalCount = iPairCount * iVectorCount;
  // Calculate the distances between all possible sample pairs into
  // this matrix.
  PiiMatrix<double> matDistances(1, iPairCount);
  for (int iFeature = 0, iIndex = 0; iFeature < iVectorCount; ++iFeature)
    {
      int iFeatureCount = d->matStoredBoundaries(0, iFeature) - iFeatureStart;
      matDistances = 0;
      for (int iSample1 = 0, iDist = 0; iSample1 < iSampleCount-1; ++iSample1)
        {
          const double* pRow1 = d->matBuffer[iSample1];
          for (int iSample2 = iSample1+1; iSample2 < iSampleCount; ++iSample2, ++iDist, ++iIndex)
            {
              const double* pRow2 = d->matBuffer[iSample2];
              matDistances(0,iDist) = (*d->lstDistanceMeasures[iFeature])(pRow1 + iFeatureStart,
                                                                          pRow2 + iFeatureStart,
                                                                          iFeatureCount);
            }
          if (d->bThreadRunning)
            emit progressed(double(iIndex) / (iTotalCount+1));
          else
            return;
        }
      // Store the inverse of the variance to the distance weight
      // list.
      double dVar = Pii::varAll<double>(matDistances);
      if (dVar != 0)
        dVar = 1.0/dVar;
      else
        dVar = 1.0;
      lstDistanceWeights << dVar;
      iFeatureStart += iFeatureCount;
    }

  d->learningMutex.lock();
  d->lstDistanceWeights = lstDistanceWeights;
  d->learningMutex.unlock();

  d->bThreadRunning = false;
  emit progressed(1.0);
}

void PiiFeatureCombiner::startLearningThread()
{
  PII_D;
  QMutexLocker lock(&d->learningMutex);
  if (learningThreadRunning())
    return;
  if (d->matBuffer.rows() < 2)
    {
      emit errorOccured(this, tr("Not enough buffered samples."));
      return;
    }
  d->matStoredBoundaries = d->varBoundaries.valueAs<PiiMatrix<int> >();
  
  d->bThreadRunning = true;
  d->pLearningThread->start();
}

void PiiFeatureCombiner::stopLearningThread()
{
  PII_D;
  d->bThreadRunning = false;
  d->pLearningThread->wait();
}

int PiiFeatureCombiner::dynamicInputCount() const { return inputCount(); }
int PiiFeatureCombiner::featureCount() const { return _d()->iTotalLength; }
void PiiFeatureCombiner::setDistanceMeasures(const QStringList& names) { _d()->lstDistanceMeasureNames = names; }
QStringList PiiFeatureCombiner::distanceMeasures() const { return _d()->lstDistanceMeasureNames; }
void PiiFeatureCombiner::setDistanceWeights(const QVariantList& weights)
{
  PII_D;
  QMutexLocker lock(&d->learningMutex);
  d->lstDistanceWeights = Pii::variantsToList<double>(weights);
}
QVariantList PiiFeatureCombiner::distanceWeights() const
{
  const PII_D;
  QMutexLocker lock(&d->learningMutex);
  return Pii::listToVariants(d->lstDistanceWeights);
}
void PiiFeatureCombiner::setLearningBatchSize(int learningBatchSize)
{
  PII_D;
  if (learningBatchSize < -1 || learningBatchSize == 1) return;
  d->iLearningBatchSize = learningBatchSize;
  if (learningBatchSize < d->matBuffer.rows())
    d->matBuffer.resize(learningBatchSize, d->matBuffer.rows());
}
int PiiFeatureCombiner::learningBatchSize() const { return _d()->iLearningBatchSize; }
bool PiiFeatureCombiner::learningThreadRunning() const { return _d()->pLearningThread->isRunning(); }

void PiiFeatureCombiner::setFullBufferBehavior(FullBufferBehavior fullBufferBehavior) { _d()->fullBufferBehavior = fullBufferBehavior; }
PiiFeatureCombiner::FullBufferBehavior PiiFeatureCombiner::fullBufferBehavior() const { return _d()->fullBufferBehavior; }
