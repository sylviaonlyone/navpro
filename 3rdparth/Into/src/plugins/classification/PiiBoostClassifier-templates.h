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

#ifndef _PIIBOOSTCLASSIFIER_H
# error "Never use <PiiBoostClassifier-templates.h> directly; include <PiiBoostClassifier.h> instead."
#endif

#include <PiiMath.h>
#include "PiiClassification.h"

template <class SampleSet>
PiiBoostClassifier<SampleSet>::Data::Data(Factory* factory,
                                          PiiClassification::BoostingAlgorithm algorithm) :
  pFactory(factory),
  algorithm(algorithm),
  iClassCount(0),
  iFeatureCount(0),
  iMaxClassifiers(100)
{
}

template <class SampleSet>
PiiBoostClassifier<SampleSet>::PiiBoostClassifier(Factory* factory,
                                                  PiiClassification::BoostingAlgorithm algorithm) :
  PiiLearningAlgorithm<SampleSet>(new Data(factory, algorithm))
{
}

template <class SampleSet> PiiBoostClassifier<SampleSet>::~PiiBoostClassifier()
{
  qDeleteAll(_d()->lstClassifiers);
}

template <class SampleSet>
void PiiBoostClassifier<SampleSet>::formHypothesis(PiiClassifier<SampleSet>* classifier,
                                                   const SampleSet& samples,
                                                   QVector<double>& hypothesis)
{
  const int iSamples = PiiSampleSet::sampleCount(samples);
  for (int i=0; i<iSamples; ++i)
    hypothesis[i] = classifier->classify(PiiSampleSet::sampleAt(samples,i));
}

template <class SampleSet>
double PiiBoostClassifier<SampleSet>::classifyExcluding(ConstFeatureIterator sample,
                                                        int excludedIndex)
{
  PII_D;
  const int iClassifierCnt = d->lstClassifiers.size();
  double dSum = 0;
  int i = 0;
  for (; i < excludedIndex; ++i)
    dSum += d->lstClassifiers[i]->classify(sample) - 0.5;
  for (++i; i < iClassifierCnt; ++i)
    dSum += d->lstClassifiers[i]->classify(sample) - 0.5;
  return dSum > 0 ? 1 : 0;
}

template <class SampleSet>
void PiiBoostClassifier<SampleSet>::formHypothesisExcluding(const SampleSet& samples,
                                                            QVector<double>& hypothesis,
                                                            int excludedIndex)
{
  const int iSamples = PiiSampleSet::sampleCount(samples);
  for (int i=0; i<iSamples; ++i)
    hypothesis[i] = classifyExcluding(PiiSampleSet::sampleAt(samples,i), excludedIndex);
}

extern PII_CLASSIFICATION_EXPORT const char* pBoostFactoryNotSetError;
extern PII_CLASSIFICATION_EXPORT const char* pBoostTooFewClassesError;
extern PII_CLASSIFICATION_EXPORT const char* pBoostTooManyClassesError;
extern PII_CLASSIFICATION_EXPORT const char* pBoostTooWeakClassifierError;

template <class SampleSet>
void PiiBoostClassifier<SampleSet>::learn(const SampleSet& samples,
                                          const QVector<double>& labels,
                                          const QVector<double>& weights)
{
  PII_D;
  const int iSamples = PiiSampleSet::sampleCount(samples);

  if (d->pFactory == 0)
    PII_THROW(PiiClassificationException, tr(pBoostFactoryNotSetError));

  qDeleteAll(d->lstClassifiers);
  d->lstClassifiers.clear();
  d->lstClassifierWeights.clear();

  // Count the number of samples in each class
  QVector<int> vecCounts(PiiClassification::countLabelsInt(labels));
  // Count classes with a non-zero number of samples
  d->iClassCount = 0;
  for (int i=0; i<vecCounts.size(); ++i)
    if (vecCounts[i] > 0)
      ++d->iClassCount;
  if (d->iClassCount < 2)
    PII_THROW(PiiClassificationException, tr(pBoostTooFewClassesError).arg(d->iClassCount));
  if (d->iClassCount > 2 && d->algorithm != PiiClassification::SammeBoost)
    PII_THROW(PiiClassificationException, tr(pBoostTooManyClassesError).arg(d->iClassCount));
  d->iFeatureCount = PiiSampleSet::featureCount(samples);
  double dMaxError = (double(d->iClassCount) - 1.0) / d->iClassCount;
  double dLogClassesMinus1 = 0.5 * log(double(d->iClassCount-1)); // zero in binary classification
  
  QVector<double> vecWeights;
  if (weights.size() == iSamples)
    vecWeights = weights;
  else
    {
      if (d->algorithm == PiiClassification::FloatBoost)
        {
          vecWeights.reserve(iSamples);
          for (int i = 0; i < iSamples; ++i)
            vecWeights << 1.0 / (vecCounts[labels[i]] * d->iClassCount);
        }
      else
        vecWeights.fill(1.0 / iSamples, iSamples);
    }

  QVector<double> vecHypotheses(iSamples);
  double dMinError = 1;
  while (d->lstClassifiers.size() < d->iMaxClassifiers)
    {
      // Create a new weak classifier
      PiiClassifier<SampleSet>* pClassifier = d->pFactory->create(this, samples, labels, vecWeights);
      // Get the hypotheses
      formHypothesis(pClassifier, samples, vecHypotheses);

      // Calculate error and classifier weight
      double dError = PiiClassification::calculateError(labels, vecHypotheses, vecWeights);
      //piiDebug("Round %d. Error = %lf", d->lstClassifiers.size(), dError);
      if (dError >= dMaxError)
        {
          delete pClassifier;
          PII_THROW(PiiClassificationException, tr(pBoostTooWeakClassifierError).arg(dError).arg(dMaxError));
        }
      // Add the weak classifier to our list
      d->lstClassifiers << pClassifier;

      double dErrorRatio = dError != 0 ? (1.0-dError)/dError : INFINITY;

      /* NOTE
       *
       * Most implementations seem to use 1 as the scaling factor
       * because it merely scales the decision function and doesn't
       * change its sign. However, the square root this scaling causes
       * in individual sample weights seems to increase accuracy, and
       * thus 0.5 is intentionally used here. The same factor is used
       * with the ln(classCount-1) term in SammeBoost.
       */
      d->lstClassifierWeights << 0.5 * log(dErrorRatio);
      
      // Update sample weights
      double dCorrectWeight = 1, dIncorrectWeight = 1;
      switch (d->algorithm)
        {
        case PiiClassification::RealBoost:
          // exp(0.5*ln(dErrorRatio))
          // = exp(ln(dErrorRatio))^0.5
          // = sqrt(dErrorRatio)
          dIncorrectWeight = sqrt(dErrorRatio);
          // exp(-0.5*ln(dErrorRatio))
          // = exp(ln(dErrorRatio))^-0.5
          dCorrectWeight = 1/dIncorrectWeight;
          break;
        case PiiClassification::AdaBoost:
          dIncorrectWeight = dErrorRatio; // = exp(log(dErrorRatio))
          break;          
        case PiiClassification::SammeBoost:
          // exp(0.5*(ln(dErrorRatio) + ln(iClassCount-1)))
          // = exp(ln(dErrorRatio * (iClassCount-1)))^0.5
          // = sqrt(dErrorRatio * (iClassCount-1))
          dIncorrectWeight = sqrt(dErrorRatio * (d->iClassCount-1));
          dCorrectWeight = 1/dIncorrectWeight;
          d->lstClassifierWeights.last() += dLogClassesMinus1;
          break;
        case PiiClassification::FloatBoost:
          // FloatBoost is special in that it uses the full ensemble
          // of weak classifiers on each iteration.
          formHypothesisExcluding(samples, vecHypotheses, -1);
          dError = PiiClassification::calculateError(labels, vecHypotheses, vecWeights);
          if (dError < dMinError)
            dMinError = dError;
          dCorrectWeight = 1.0/M_E;
          dIncorrectWeight = M_E;
          break;
        }
      double dWeightSum = updateWeights(samples, labels, vecHypotheses, vecWeights, dCorrectWeight, dIncorrectWeight);
          
      // Start backtracking when three weak classifiers have been
      // selected.
      if (d->algorithm == PiiClassification::FloatBoost)
        {
          bool bClassifierExcluded = false;
          while (d->lstClassifiers.size() >= 3)
            {
              int iExcludedIndex = excludeOne(samples, vecWeights, labels, vecHypotheses, &dMinError);
              if (iExcludedIndex != -1)
                {
                  bClassifierExcluded = true;
                  d->lstClassifiers.removeAt(iExcludedIndex);
                  // Not that we would need the weights, but anyway...
                  d->lstClassifierWeights.removeAt(iExcludedIndex);
                }
              else
                break;
            }
          if (bClassifierExcluded)  
            dWeightSum = updateWeights(samples, labels, vecHypotheses, vecWeights, dCorrectWeight, dIncorrectWeight);
          // if (dWeightSum < J*) break;
        } // FloatBoost
      PII_TRY_CONTINUE(this->controller(), double(d->lstClassifiers.size()+1)/d->iMaxClassifiers);
      if (dError <= d->dMinError)
        break;
    } // while (d->lstClassifiers.size() < d->iMaxClassifiers)
}

template <class SampleSet>
double PiiBoostClassifier<SampleSet>::updateWeights(const SampleSet& samples,
                                                    const QVector<double>& labels,
                                                    const QVector<double>& hypotheses,
                                                    QVector<double>& weights,
                                                    double correctWeight,
                                                    double incorrectWeight)
{
  const int iSamples = PiiSampleSet::sampleCount(samples);
  double dWeightSum = 0;
  for (int i=0; i<iSamples; ++i)
    {
      weights[i] *= (hypotheses[i] == labels[i]) ? correctWeight : incorrectWeight;
      dWeightSum += weights[i];
    }
  if (dWeightSum != 0)
    Pii::mapN(weights.begin(), iSamples, std::bind2nd(std::multiplies<double>(), 1.0 / dWeightSum));
  return dWeightSum;
}


template <class SampleSet>
double PiiBoostClassifier<SampleSet>::excludeOne(const SampleSet& samples,
                                                 const QVector<double>& weights,
                                                 const QVector<double>& labels,
                                                 QVector<double>& hypotheses,
                                                 double* minError)
{
  PII_D;
  // Calculate total error with each one of the classifiers excluded.
  for (int i=0; i<d->lstClassifiers.size(); ++i)
    {
      formHypothesisExcluding(samples, hypotheses, i);
      double dError = PiiClassification::calculateError(labels, hypotheses, weights);
      if (dError < *minError)
        {
          *minError = dError;
          return i;
        }
    }
  return -1;
}


template <class SampleSet>
double PiiBoostClassifier<SampleSet>::classify(ConstFeatureIterator sample) throw()
{
  PII_D;
  const int iClassifiers = d->lstClassifiers.size();
  switch (d->algorithm)
    {
    case PiiClassification::SammeBoost:
      {
        // Get hypotheses from each weak classifier
        int* pHypotheses = new int[iClassifiers];
        for (int i = 0; i < iClassifiers; ++i)
          pHypotheses[i] = int(d->lstClassifiers[i]->classify(sample));

        // Weighted voting for the winning class
        double dMaxSum = 0;
        int iMaxHypothesis = -1;
        for (int iHypothesis = 0; iHypothesis < d->iClassCount; ++iHypothesis)
          {
            double dSum = 0;
            for (int c = 0; c < iClassifiers; ++c)
              if (pHypotheses[c] == iHypothesis)
                dSum += d->lstClassifierWeights[c];
            if (dSum > dMaxSum)
              {
                iMaxHypothesis = iHypothesis;
                dMaxSum = dSum;
              }
          }
        delete[] pHypotheses;
        return iMaxHypothesis != -1 ? iMaxHypothesis : NAN;
      }
    case PiiClassification::FloatBoost:
      {
        double dSum = 0;
        for (int i = 0; i < iClassifiers; ++i)
          dSum += d->lstClassifiers[i]->classify(sample) - 0.5;
        return dSum > 0 ? 1 : 0;
      }
    case PiiClassification::AdaBoost:
    case PiiClassification::RealBoost:
    default:
      {
        double dSum = 0;
        for (int i = 0; i < iClassifiers; ++i)
          // Convert {0,1} classification to {-1,1}.
          // Well, actually the result is {-0.5, 0.5}, but scaling by
          // a constant has no effect on the outcome.
          dSum += d->lstClassifierWeights[i] * (d->lstClassifiers[i]->classify(sample) - 0.5);
        return dSum > 0 ? 1 : 0;
      }
    }
}

template <class SampleSet> void PiiBoostClassifier<SampleSet>::setFactory(Factory* factory) { _d()->pFactory = factory; }
template <class SampleSet> typename PiiBoostClassifier<SampleSet>::Factory* PiiBoostClassifier<SampleSet>::factory() const { return _d()->pFactory; }
template <class SampleSet> void PiiBoostClassifier<SampleSet>::setAlgorithm(PiiClassification::BoostingAlgorithm algorithm) { _d()->algorithm = algorithm; }
template <class SampleSet> PiiClassification::BoostingAlgorithm PiiBoostClassifier<SampleSet>::algorithm() const { return _d()->algorithm; }
template <class SampleSet> void PiiBoostClassifier<SampleSet>::setMaxClassifiers(int maxClassifiers) { _d()->iMaxClassifiers = maxClassifiers; }
template <class SampleSet> int PiiBoostClassifier<SampleSet>::maxClassifiers() const { return _d()->iMaxClassifiers; }
template <class SampleSet> QList<PiiClassifier<SampleSet>*> PiiBoostClassifier<SampleSet>::classifiers() const { return _d()->lstClassifiers; }
template <class SampleSet> QList<float> PiiBoostClassifier<SampleSet>::weights() const { return _d()->lstClassifierWeights; }
template <class SampleSet> PiiClassification::LearnerCapabilities PiiBoostClassifier<SampleSet>::capabilities() const { return PiiClassification::WeightedLearner; }
template <class SampleSet> bool PiiBoostClassifier<SampleSet>::converged() const throw () { return true; }
template <class SampleSet> int PiiBoostClassifier<SampleSet>::featureCount() const { return _d()->iFeatureCount; }
template <class SampleSet> int PiiBoostClassifier<SampleSet>::classCount() const { return _d()->iClassCount; }
template <class SampleSet> void PiiBoostClassifier<SampleSet>::setMinError(double minError) { _d()->dMinError = minError; }
template <class SampleSet> double PiiBoostClassifier<SampleSet>::minError() const { return _d()->dMinError; }
