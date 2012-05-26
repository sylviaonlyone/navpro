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

#ifndef _PIIKERNELPERCEPTRON_H
# error "Never use <PiiKernelPerceptron-templates.h> directly; include <PiiKernelPerceptron.h> instead."
#endif

#include "PiiGaussianKernel.h"

template <class SampleSet> PiiKernelPerceptron<SampleSet>::Data::Data() :
  pKernel(new PII_POLYMORPHIC_KERNEL(PiiGaussianKernel)),
  bConverged(false),
  iMaxIterations(100)
{
}

template <class SampleSet> PiiKernelPerceptron<SampleSet>::PiiKernelPerceptron() :
  PiiLearningAlgorithm<SampleSet>(new Data)
{
}

template <class SampleSet> PiiKernelPerceptron<SampleSet>::~PiiKernelPerceptron()
{
  delete _d()->pKernel;
}

template <class SampleSet>
void PiiKernelPerceptron<SampleSet>::learn(const SampleSet& samples,
                                           const QVector<double>& labels,
                                           const QVector<double>& /*weights*/)
{
  PII_D;
  const int iSamples = PiiSampleSet::sampleCount(samples),
    iFeatures = PiiSampleSet::featureCount(samples);
  
  PiiMatrix<double> matKernel(PiiClassification::calculateDistanceMatrix(samples,
                                                                         *d->pKernel,
                                                                         true, true));
  QVector<double> vecWeights(iSamples, 0.0);
  d->vecWeights.clear();
  d->supportVectors.clear();
  d->bConverged = false;
  
  int iErrorCount, iIterations = 0;
  do
    {
      // Count incorrect classifications on each round.
      iErrorCount = 0;
      for (int i=0; i<iSamples; ++i)
        {
          const double* pKernelRow = matKernel.constRowBegin(i);
          // Projection to the hyperplane's normal
          double dSum = 0;
          for (int j=0; j<iSamples; ++j, ++pKernelRow)
            dSum += *pKernelRow * vecWeights[j];
          if (dSum > 0)
            dSum = 1;
          else
            dSum = 0;
          // Prediction doesn't match the training label -> update weights
          if (dSum != labels[i])
            {
              if (labels[i] == 1)
                ++vecWeights[i];
              else
                --vecWeights[i];
              ++iErrorCount;
            }
          PII_TRY_CONTINUE(this->controller(), NAN);
        }
    }
  // Stop if everything was correctly classified
  while (iErrorCount > 0 &&
         ++iIterations < d->iMaxIterations);
  
  d->bConverged = (iErrorCount == 0);

  // Retain only the samples with non-zero weights.
  int iBlockSize = qMax(iSamples/8, 8);
  PiiSampleSet::reserve(d->supportVectors, iBlockSize, iFeatures);
  d->vecWeights.reserve(iBlockSize);
  for (int i=0; i<iSamples; ++i)
    {
      if (vecWeights[i] != 0)
        {
          // Reserve room for many samples at once
          int iCapacity = PiiSampleSet::capacity(d->supportVectors);
          if (iCapacity == PiiSampleSet::sampleCount(d->supportVectors))
            {
              PiiSampleSet::reserve(d->supportVectors, iCapacity + iBlockSize);
              d->vecWeights.reserve(iCapacity + iBlockSize);
            }
          PiiSampleSet::append(d->supportVectors, PiiSampleSet::sampleAt(samples,i));
          d->vecWeights.append(vecWeights[i]);
        }
    }
  //qDebug("%d -> %d", samples.sampleCount(), d->supportVectors.sampleCount());
}

template <class SampleSet>
double PiiKernelPerceptron<SampleSet>::classify(ConstFeatureIterator featureVector) throw()
{
  PII_D;
  if (!d->vecWeights.isEmpty())
    {
      const int iSamples = PiiSampleSet::sampleCount(d->supportVectors),
        iFeatures = PiiSampleSet::featureCount(d->supportVectors);
      double dSum = 0;
      for (int i=0; i<iSamples; ++i)
        dSum += d->vecWeights[i] *
          (*d->pKernel)(PiiSampleSet::sampleAt(d->supportVectors, i),
                        featureVector,
                        iFeatures);
      return dSum > 0 ? 1 : 0;
    }
  return NAN;
}

template <class SampleSet>
PiiKernelFunction<typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator>*
PiiKernelPerceptron<SampleSet>::kernelFunction() const
{
  return _d()->pKernel;
}

template <class SampleSet>
void PiiKernelPerceptron<SampleSet>::setKernelFunction(PiiKernelFunction<ConstFeatureIterator>* kernel)
{
  PII_D;
  delete d->pKernel;
  d->pKernel = kernel;
}

template <class SampleSet> PiiClassification::LearnerCapabilities PiiKernelPerceptron<SampleSet>::capabilities() const { return 0; }
template <class SampleSet> int PiiKernelPerceptron<SampleSet>::featureCount() const { return _d()->supportVectors.featureCount(); }
template <class SampleSet> void PiiKernelPerceptron<SampleSet>::setMaxIterations(int maxIterations) { _d()->iMaxIterations = maxIterations; }
template <class SampleSet> int PiiKernelPerceptron<SampleSet>::maxIterations() const { return _d()->iMaxIterations; }
template <class SampleSet> void PiiKernelPerceptron<SampleSet>::setWeights(const QVector<double>& weights) { _d()->vecWeights = weights; }
template <class SampleSet> QVector<double> PiiKernelPerceptron<SampleSet>::weights() const { return _d()->vecWeights; }
template <class SampleSet> bool PiiKernelPerceptron<SampleSet>::converged() const throw() { return _d()->bConverged; }
template <class SampleSet> SampleSet PiiKernelPerceptron<SampleSet>::supportVectors() const { return _d()->supportVectors; }
template <class SampleSet> void PiiKernelPerceptron<SampleSet>::setSupportVectors(const SampleSet& supportVectors) { _d()->supportVectors = supportVectors; }
