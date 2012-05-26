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

#ifndef _PIIKERNELADATRON_H
# error "Never use <PiiKernelAdatron-templates.h> directly; include <PiiKernelAdatron.h> instead."
#endif

#include "PiiGaussianKernel.h"

template <class SampleSet> PiiKernelAdatron<SampleSet>::Data::Data() :
  pKernel(new PII_POLYMORPHIC_KERNEL(PiiGaussianKernel)),
  bConverged(false),
  iMaxIterations(100),
  dTheta(0),
  dLearningRate(1),
  dConvergenceThreshold(1e-2)
{
}

template <class SampleSet> PiiKernelAdatron<SampleSet>::PiiKernelAdatron() :
  PiiLearningAlgorithm<SampleSet>(new Data)
{
}

template <class SampleSet> PiiKernelAdatron<SampleSet>::~PiiKernelAdatron()
{
  delete _d()->pKernel;
}

template <class SampleSet>
void PiiKernelAdatron<SampleSet>::learn(const SampleSet& samples,
                                        const QVector<double>& labels,
                                        const QVector<double>& /*weights*/)
{
  PII_D;
  const int iSamples = PiiSampleSet::sampleCount(samples),
    iFeatures = PiiSampleSet::featureCount(samples);
  
  PiiMatrix<double> matKernel(PiiClassification::calculateDistanceMatrix(samples,
                                                                         *d->pKernel,
                                                                         true, true));
  QVector<double> vecWeights(iSamples, 1.0);
  QVector<double> vecZ(iSamples);

  d->vecWeights.clear();
  d->vecLabels.clear();
  PiiSampleSet::clear(d->supportVectors);
  d->bConverged = false;
  
  int iIterations = 0;
  double dTheta = 0, dLearningRate = d->dLearningRate, dMinZ, dMaxZ;
  do
    {
      dMinZ = INFINITY, dMaxZ = -INFINITY;
      for (int i=0; i<iSamples; ++i)
        {
          const double* pKernelRow = matKernel.constRowBegin(i);
          double dSum = 0;
          for (int j=0; j<iSamples; ++j, ++pKernelRow)
            dSum += (labels[j] - 0.5) * *pKernelRow * vecWeights[j];
          vecZ[i] = dSum;
          double dDelta;
          if (labels[i] == 1)
            {
              if (dSum < dMinZ)
                dMinZ = dSum;
              dDelta = dLearningRate * (1 - dSum*2 + dTheta);
            }
          else
            {
              if (dSum > dMaxZ)
                dMaxZ = dSum;
              dDelta = dLearningRate * (1 + dSum*2 - dTheta);
            }
          vecWeights[i] = qMax(0.0, vecWeights[i] + dDelta);
          PII_TRY_CONTINUE(this->controller(), NAN);
        }
      dTheta = dMaxZ + dMinZ;
      //qDebug("theta = %lf (diff = %lf)", dTheta, 1-dMinZ+dMaxZ);
    }
  while (++iIterations < d->iMaxIterations &&
         (iIterations < 2 || (1 - dMinZ + dMaxZ) > d->dConvergenceThreshold));

  d->bConverged = (dMinZ > 0 && dMaxZ <= 0);

  d->dTheta = dTheta/2;

  // Retain only the samples with non-zero weights.
  int iBlockSize = qMax(iSamples/8, 8);
  PiiSampleSet::reserve(d->supportVectors, iBlockSize, iFeatures);
  d->vecWeights.reserve(iBlockSize);
  d->vecLabels.reserve(iBlockSize);
  for (int i=0; i<iSamples; ++i)
    {
      if (vecWeights[i] != 0)
        {
          // Reserve room for many samples at once
          if (PiiSampleSet::capacity(d->supportVectors) == PiiSampleSet::sampleCount(d->supportVectors))
            {
              int iNewSize = PiiSampleSet::capacity(d->supportVectors) + iBlockSize;
              PiiSampleSet::reserve(d->supportVectors, iNewSize);
              d->vecWeights.reserve(iNewSize);
              d->vecLabels.reserve(iNewSize);
            }
          PiiSampleSet::append(d->supportVectors, PiiSampleSet::sampleAt(samples,i));
          d->vecWeights.append(vecWeights[i]);
          d->vecLabels.append(labels[i]);
        }
    }
}

template <class SampleSet>
double PiiKernelAdatron<SampleSet>::classify(ConstFeatureIterator featureVector) throw()
{
  PII_D;
  if (!d->vecWeights.isEmpty())
    {
      const int iSamples = PiiSampleSet::sampleCount(d->supportVectors),
        iFeatures = PiiSampleSet::featureCount(d->supportVectors);
      double dSum = 0;
      for (int i=0; i<iSamples; ++i)
        dSum += (d->vecLabels[i]-0.5) * d->vecWeights[i] *
          (*d->pKernel)(PiiSampleSet::sampleAt(d->supportVectors, i),
                        featureVector,
                        iFeatures);
      return dSum > d->dTheta ? 1 : 0;
    }
  return NAN;
}

template <class SampleSet>
PiiKernelFunction<typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator>*
PiiKernelAdatron<SampleSet>::kernelFunction() const
{
  return _d()->pKernel;
}

template <class SampleSet>
void PiiKernelAdatron<SampleSet>::setKernelFunction(PiiKernelFunction<ConstFeatureIterator>* kernel)
{
  PII_D;
  delete d->pKernel;
  d->pKernel = kernel;
}

template <class SampleSet> PiiClassification::LearnerCapabilities PiiKernelAdatron<SampleSet>::capabilities() const { return 0; }
template <class SampleSet> void PiiKernelAdatron<SampleSet>::setMaxIterations(int maxIterations) { _d()->iMaxIterations = maxIterations; }
template <class SampleSet> int PiiKernelAdatron<SampleSet>::maxIterations() const { return _d()->iMaxIterations; }
template <class SampleSet> void PiiKernelAdatron<SampleSet>::setWeights(const QVector<double>& weights) { _d()->vecWeights = weights; }
template <class SampleSet> QVector<double> PiiKernelAdatron<SampleSet>::weights() const { return _d()->vecWeights; }
template <class SampleSet> bool PiiKernelAdatron<SampleSet>::converged() const throw() { return _d()->bConverged; }
template <class SampleSet> void PiiKernelAdatron<SampleSet>::setDecisionThreshold(double decisionThreshold) { _d()->dTheta = decisionThreshold; }
template <class SampleSet> double PiiKernelAdatron<SampleSet>::decisionThreshold() const { return _d()->dTheta; }
template <class SampleSet> void PiiKernelAdatron<SampleSet>::setLearningRate(double learningRate) { _d()->dLearningRate = learningRate; }
template <class SampleSet> double PiiKernelAdatron<SampleSet>::learningRate() const { return _d()->dLearningRate; }
template <class SampleSet> void PiiKernelAdatron<SampleSet>::setConvergenceThreshold(double convergenceThreshold) { _d()->dConvergenceThreshold = convergenceThreshold; }
template <class SampleSet> double PiiKernelAdatron<SampleSet>::convergenceThreshold() const { return _d()->dConvergenceThreshold; }
template <class SampleSet> SampleSet PiiKernelAdatron<SampleSet>::supportVectors() const { return _d()->supportVectors; }
template <class SampleSet> void PiiKernelAdatron<SampleSet>::setSupportVectors(const SampleSet& supportVectors) { _d()->supportVectors = supportVectors; }
