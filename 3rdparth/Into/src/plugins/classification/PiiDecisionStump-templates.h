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

#ifndef _PIIDECISIONSTUMP_H
# error "Never use <PiiDecisionStump-templates.h> directly; include <PiiDecisionStump.h> instead."
#endif

template <class SampleSet> PiiDecisionStump<SampleSet>::Data::Data() :
  iSelectedFeature(0),
  dLeftLabel(NAN),
  dRightLabel(NAN),
  threshold(0)
{
}

template <class SampleSet> PiiDecisionStump<SampleSet>::PiiDecisionStump() :
  PiiLearningAlgorithm<SampleSet>(new Data)
{}

template <class SampleSet>
double PiiDecisionStump<SampleSet>::optimizeSplit(const QVector<double>& leftWeights,
                                                  const QVector<double>& weightTotals,
                                                  double totalWeightSum,
                                                  int* leftLabel, int* rightLabel)
{
  double dMinError = INFINITY;
  int iLabels = leftWeights.size();
  // Try all combinations of left-right label pairs (N²-N)
  for (int l=0; l<iLabels; ++l)
    for (int r=0; r<iLabels; ++r)
      if (l != r)
        {
          // If the left label was l and the right label was r, then
          // we would get this weighted error.
          double dError = weightTotals[l] - leftWeights[l] + // "left" label on the "right" side
            leftWeights[r]; // "right" label on the "left" side
          // If there are more than two labels, the rest of the
          // classes will always go wrong.
          if (iLabels > 2)
            dError += totalWeightSum - weightTotals[l] - weightTotals[r];
          if (dError < dMinError)
            {
              dMinError = dError;
              *leftLabel = l;
              *rightLabel = r;
            }
        }
  return dMinError;
}


template <class SampleSet>
void PiiDecisionStump<SampleSet>::learn(const SampleSet& samples,
                                        const QVector<double>& labels,
                                        const QVector<double>& weights)
{
  PII_D;
  d->iSelectedFeature = 0;
  d->threshold = 0;
  d->dLeftLabel = d->dRightLabel = NAN;

  const int iSamples = PiiSampleSet::sampleCount(samples),
    iFeatures = PiiSampleSet::featureCount(samples);

  const QVector<double> vecWeights(weights.size() == iSamples ?
                                   weights : QVector<double>(iSamples, 1.0/iSamples));
  

  double dWeightSum = 0;
  // Calculate the sum of weights for each class separately
  QVector<double> vecWeightTotals;
  for (int i=0; i<iSamples; ++i)
    {
      int iLabel = int(labels[i]);
      if (iLabel >= vecWeightTotals.size())
        vecWeightTotals.resize(iLabel+1);
      vecWeightTotals[iLabel] += weights[i];
      dWeightSum += weights[i];
    }

  QVector<Feature> vecFeatures(iSamples);
  QVector<double> vecLeftWeights(vecWeightTotals.size());
  
  double dMinError = INFINITY;

  // For each feature
  for (int f=0; f<iFeatures; ++f)
    {
      for (int i=0; i<iSamples; ++i)
        vecFeatures[i] = Feature(PiiSampleSet::sampleAt(samples,i)[f], int(labels[i]), weights[i]);

      // Sort according to the feature value
      qSort(vecFeatures);
      vecLeftWeights.fill(0);
      
      // Calculate classification error for each threshold.
      for (int i=0; i<iSamples; i++)
        {
          vecLeftWeights[vecFeatures[i].iLabel] += vecFeatures[i].dWeight;

          int iLeftLabel, iRightLabel;
          double dError = optimizeSplit(vecLeftWeights, vecWeightTotals,
                                        dWeightSum,
                                        &iLeftLabel, &iRightLabel);

          if (dError < dMinError)
            {
              dMinError = dError;
              d->dLeftLabel = iLeftLabel;
              d->dRightLabel = iRightLabel;
              d->iSelectedFeature = f;
              d->threshold = vecFeatures[i].value;
            }
        }
    }

  //piiDebug("Selected feature %d, threshold %lf (%d|%d)", d->iSelectedFeature, double(d->threshold), int(d->dLeftLabel), int(d->dRightLabel));
}

template <class SampleSet> double PiiDecisionStump<SampleSet>::classify(ConstFeatureIterator sample) throw()
{
  PII_D;
  return sample[d->iSelectedFeature] <= d->threshold ? d->dLeftLabel : d->dRightLabel;
}

template <class SampleSet> void PiiDecisionStump<SampleSet>::setSelectedFeature(int feature) { return _d()->iSelectedFeature = feature; }
template <class SampleSet> int PiiDecisionStump<SampleSet>::selectedFeature() const { return _d()->iSelectedFeature; }
template <class SampleSet> void PiiDecisionStump<SampleSet>::setThreshold(FeatureType threshold) { _d()->threshold = threshold; }
template <class SampleSet> typename PiiDecisionStump<SampleSet>::FeatureType PiiDecisionStump<SampleSet>::threshold() const { return _d()->threshold; }
template <class SampleSet> void PiiDecisionStump<SampleSet>::setLeftLabel(double leftLabel) { _d()->dLeftLabel = leftLabel; }
template <class SampleSet> double PiiDecisionStump<SampleSet>::leftLabel() const { return _d()->dLeftLabel; }
template <class SampleSet> void PiiDecisionStump<SampleSet>::setRightLabel(double rightLabel) { _d()->dRightLabel = rightLabel; }
template <class SampleSet> double PiiDecisionStump<SampleSet>::rightLabel() const { return _d()->dRightLabel; }
template <class SampleSet> PiiClassification::LearnerCapabilities PiiDecisionStump<SampleSet>::capabilities() const { return PiiClassification::WeightedLearner; }
template <class SampleSet> bool PiiDecisionStump<SampleSet>::converged() const throw () { return true; }
