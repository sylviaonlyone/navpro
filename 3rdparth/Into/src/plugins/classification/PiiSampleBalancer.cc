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

#include "PiiSampleBalancer.h"

#include <PiiYdinTypes.h>
#include <PiiUtil.h>
#include <PiiRandom.h>

class PiiSampleBalancer::Histogram
{
public:
  Histogram() : _iLevels(256), _iCount(0), _iLearningBatchSize(_iLevels), _dpHistogram(0), _dpNormalizedWeights(0), _ipHistogram(0)
  {}
  
  ~Histogram()
  {
    /*
      for (int i=0; i<_iLevels; i++)
      {
        QString bar = QString("%1 ").arg(i,3);
        for (int j=int(_dpNormalizedWeights[i]*100); j--; )
          bar += 'X';
        qDebug(qPrintable(bar));
      }
    */
    delete[] _ipHistogram;
    delete[] _dpHistogram;
    delete[] _dpNormalizedWeights;
  }

  // allocate space for l distinct values in the histogram
  void allocate(int l, int learningBatchSize)
  {
    _iLevels = l;
    _dpHistogram = new double[l];
    _dpNormalizedWeights = new double[l];
    _ipHistogram = new int[l];
    memset(_ipHistogram, 0, sizeof(int) * l);
    // Set weights initially to a uniform distribution and normalize
    // maximum value to one.
    double fraction = 1.0 / l;
    for (int i=l; i--; )
      {
        _dpNormalizedWeights[i] = 1;
        _dpHistogram[i] = fraction;
      }
    _iLearningBatchSize = learningBatchSize;
    _iCount = 0;
    _iBatchIndex = 0;
  }

  double power(double base, int power)
  {
    while (--power)
      base *= base;
    return base;
  }
  
  // mu is the "learning constant", i.e. adaptation ratio
  void addToWeights(int emphasis, double mu)
  {
    double nmu = 1.0 - mu;
    double maxWeight = 0;
    // Calculate Weighted average of previous and current weights
    for (int i=_iLevels; i--; )
      {
        double newWeight = nmu * _dpHistogram[i] +
          mu * _ipHistogram[i] / _iCount;
        // Find the maximum at the same time
        if (newWeight > maxWeight)
          maxWeight = newWeight;
        _dpHistogram[i] = newWeight;
      }

    // Normalize maximum to one
    if (maxWeight != 0)
      for (int i=_iLevels; i--; )
        _dpNormalizedWeights[i] = power(1.0 - _dpHistogram[i] / maxWeight, emphasis);

    // clear histogram
    memset(_ipHistogram, 0, sizeof(int) * _iLevels);
    _iCount = 0;
  }

  double addMeasurement(int value, int emphasis, double mu)
  {
    _ipHistogram[value]++;
    if (++_iCount > _iLearningBatchSize)
      addToWeights(emphasis, mu);
    return _dpNormalizedWeights[value];
  }

  int levels() const { return _iLevels; }

private:
  int _iLevels;
  int _iCount;
  int _iBatchIndex;
  int _iLearningBatchSize;
  double* _dpHistogram;
  double* _dpNormalizedWeights;
  int* _ipHistogram;
};

PiiSampleBalancer::Data::Data() :
  mode(ProbabilitySelection),
  iDefaultLevels(256),
  iEmphasis(1),
  iFeatureCnt(0),
  pHistograms(0),
  dAdaptationRatio(0.1),
  iLearningBatchSize(256)
{
}

PiiSampleBalancer::PiiSampleBalancer() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("features"));
  addSocket(new PiiOutputSocket("features"));
  addSocket(new PiiOutputSocket("weight"));
  addSocket(new PiiOutputSocket("select"));
  outputAt(0)->setGroupId(-1);
}

PiiSampleBalancer::~PiiSampleBalancer()
{
  PII_D;
  delete[] d->pHistograms;
}

void PiiSampleBalancer::setMode(Mode mode)
{
  PII_D;
  d->mode = mode;
  outputAt(0)->setGroupId(mode == WeightCalculation ? 0 : -1);
}

void PiiSampleBalancer::allocateHistograms()
{
  PII_D;
  d->pHistograms = new Histogram[d->iFeatureCnt];
  for (int i=d->iFeatureCnt; i--; )
    {
      int levels = d->lstLevels.size() > i ? d->lstLevels[i] : d->iDefaultLevels;
      d->pHistograms[i].allocate(levels, d->iLearningBatchSize);
    }
}

void PiiSampleBalancer::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);
  if (reset)
    {
      delete[] d->pHistograms;
      d->pHistograms = 0;
    }
}

void PiiSampleBalancer::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(balance, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

double PiiSampleBalancer::weight(int feature, int index)
{
  PII_D;
  if (feature < d->pHistograms[index].levels() &&
      feature >= 0)
    return d->pHistograms[index].addMeasurement(feature, d->iEmphasis, d->dAdaptationRatio);
  else
    return 0;
}


template <class T> void PiiSampleBalancer::balance(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> features = obj.valueAs<PiiMatrix<T> >();
  //qDebug("Got a %d by %d feature vector", features.rows(), features.columns());
  if (d->pHistograms == 0)
    {
      d->iFeatureCnt = features.columns();
      allocateHistograms();
    }
  else if (features.columns() != d->iFeatureCnt)
    PII_THROW_WRONG_SIZE(inputAt(0), features, "1-N", d->iFeatureCnt);
  
  const T* pFeatures = features.row(0);
  double dWeight = weight((int)pFeatures[0], 0);
  for (int f=1; f<d->iFeatureCnt; f++)
    dWeight *= weight((int)pFeatures[f], f);

  if (d->mode == ProbabilitySelection)
    {
      if (Pii::uniformRandom() < dWeight)
        {
          //qDebug("Emitting a sample with selection probability %lf", weight);
          emitObject(obj);
          outputAt(2)->emitObject(true);
        }
      else
        outputAt(2)->emitObject(false);
      outputAt(1)->emitObject(dWeight);
    }
  else
    {
      emitObject(obj);
      outputAt(1)->emitObject(dWeight);
      outputAt(2)->emitObject(true);
    }
}

void PiiSampleBalancer::setLevels(const QVariantList& levels)
{
  PII_D;
  d->lstLevels = Pii::variantsToList<int>(levels);
  for (int i=d->lstLevels.size(); i--; )
    {
      if (d->lstLevels[i] < 1)
        d->lstLevels[i] = 1;
    }
}

QVariantList PiiSampleBalancer::levels() const { return Pii::listToVariants(_d()->lstLevels); }
void PiiSampleBalancer::setDefaultLevels(int defaultLevels) { _d()->iDefaultLevels = defaultLevels; }
int PiiSampleBalancer::defaultLevels() const { return _d()->iDefaultLevels; }
void PiiSampleBalancer::setEmphasis(int emphasis) { _d()->iEmphasis = emphasis; }
int PiiSampleBalancer::emphasis() const { return _d()->iEmphasis; }
PiiSampleBalancer::Mode PiiSampleBalancer::mode() const { return _d()->mode; }
void PiiSampleBalancer::setAdaptationRatio(double adaptationRatio) { _d()->dAdaptationRatio = adaptationRatio; }
double PiiSampleBalancer::adaptationRatio() const { return _d()->dAdaptationRatio; }
void PiiSampleBalancer::setLearningBatchSize(int learningBatchSize) { _d()->iLearningBatchSize = learningBatchSize; }
int PiiSampleBalancer::learningBatchSize() const { return _d()->iLearningBatchSize; }
