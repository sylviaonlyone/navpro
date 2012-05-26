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

#ifndef _PIICLASSIFICATION_H
# error "Never use <PiiClassification-templates.h> directly; include <PiiClassification.h> instead."
#endif

#include <PiiMathDefs.h>

namespace PiiClassification
{
  template <class SampleSet, class DistanceMeasure>
  QVector<double> fillMissingLabels(const QVector<double>& labels,
                                    const SampleSet& samples,
                                    const DistanceMeasure& measure)
  {
    const int iSamples = PiiSampleSet::sampleCount(samples),
      iFeatures = PiiSampleSet::featureCount(samples);
    QVector<double> newLabels = labels;

    // Handle all samples with a class label
    for (int i=0; i<iSamples; ++i)
      {
        // The class of this sample is already known. Skip.
        if (!Pii::isNan(labels[i]))
          continue;

        // Find closest classified sample
        double minDist = INFINITY;
        int minIndex = -1;
        for (int j=0; j<iSamples; ++j)
          {
            if (!Pii::isNan(labels[j]))
              {
                double distance = measure(PiiSampleSet::sampleAt(samples,i),
                                          PiiSampleSet::sampleAt(samples,j),
                                          iFeatures);
                if (distance < minDist)
                  {
                    minIndex = j;
                    minDist = distance;
                  }
              }
          }
        // Assign the sample the same class as its closest neighbor
        newLabels[i] = labels[minIndex];
      }
    return newLabels;
  }

  template <class SampleSet, class DistanceMeasure>
  PiiMatrix<double> calculateDistanceMatrix(const SampleSet& samples,
                                            const DistanceMeasure& measure,
                                            bool symmetric,
                                            bool calculateDiagonal)
  {
    const int iSamples = PiiSampleSet::sampleCount(samples),
      iFeatures = PiiSampleSet::featureCount(samples);

    PiiMatrix<double> result(iSamples, iSamples);

    // Upper triangle is a reflection of the lower triangle
    if (symmetric)
      {
        for (int r=0; r<iSamples; ++r)
          for (int c=0; c<r; ++c)
            result(r,c) = result(c,r) = measure(PiiSampleSet::sampleAt(samples,r),
                                                PiiSampleSet::sampleAt(samples,c),
                                                iFeatures);
      }
    // Must calculate both triangles separately
    else
      {
        for (int r=0; r<iSamples; ++r)
          for (int c=0; c<r; ++c)
            {
              result(r,c) = measure(PiiSampleSet::sampleAt(samples,r),
                                    PiiSampleSet::sampleAt(samples,c),
                                    iFeatures);
              result(c,r) = measure(PiiSampleSet::sampleAt(samples,c),
                                    PiiSampleSet::sampleAt(samples,r),
                                    iFeatures);
            }
      }

    // Fill the diagonal only on request
    if (calculateDiagonal)
      for (int i=0; i<iSamples; ++i)
        result(i,i) = measure(PiiSampleSet::sampleAt(samples,i),
                              PiiSampleSet::sampleAt(samples,i),
                              iFeatures);

    return result;
  }


  template <class SampleSet, class DistanceMeasure>
  int findClosestMatch(typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator sample,
                       const SampleSet& modelSet,
                       const DistanceMeasure& measure,
                       double* distance)
  {
    const int iModels = PiiSampleSet::sampleCount(modelSet),
      iFeatures = PiiSampleSet::featureCount(modelSet);

    double dMinDistance = INFINITY;
    int minIndex = -1;
    for (int modelIndex = 0; modelIndex < iModels; ++modelIndex)
      {
        double d = measure(sample, modelSet[modelIndex], iFeatures);
        if (d < dMinDistance)
          {
            minIndex = modelIndex;
            dMinDistance = d;
          }
      }
    if (distance != 0)
      *distance = dMinDistance;
    return minIndex;
  }

  template <class SampleSet, class DistanceMeasure>
  MatchList findClosestMatches(typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator sample,
                               const SampleSet& modelSet,
                               const DistanceMeasure& measure,
                               int n)
  {
    const int iModels = PiiSampleSet::sampleCount(modelSet),
      iFeatures = PiiSampleSet::featureCount(modelSet);
    MatchList heap;
    heap.fill(qMin(iModels, n), qMakePair(double(INFINITY), -1));
    // Heap ensures that only shortest distances will be preserved
    for (int modelIndex = 0; modelIndex < iModels; ++modelIndex)
      heap.put(qMakePair(measure(sample, PiiSampleSet::sampleAt(modelSet, modelIndex), iFeatures),
                         modelIndex));
    // Ascending order -> first is the best match
    heap.sort();
    return heap;
  }

  template <class SampleSet, class DistanceMeasure>
  double knnClassify(typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator sample,
                     const SampleSet& modelSet,
                     const QVector<double>& labels,
                     const DistanceMeasure& measure,
                     int k,
                     double* distance,
                     int* closestIndex)
  {
    MatchList lstClosest = findClosestMatches(sample, modelSet, measure, k);
    // May be smaller than original if we have less samples in the
    // model set.
    k = lstClosest.size();

    if (k == 0) // empty set
      return NAN;
    PiiSmartPtr<double[]> pClosestLabels = new double[k];
    
    int iMaxMatches = 0, iBestLabel = -1;
    
    // Store class labels corresponding to the closest samples.
    for (int i=0; i<k; ++i)
      pClosestLabels[i] = labels[lstClosest[i].second];
    // Find the class label with the most occurrences.
    for (int i=0; i<k; ++i)
      {
        double label = pClosestLabels[i];
        int iMatchCnt = 1;
        for (int j=i+1; j<k; ++j)
          if (label == pClosestLabels[j])
            ++iMatchCnt;
        // Nearest wins if the number of votes is equal.
        if (iMatchCnt > iMaxMatches)
          {
            iMaxMatches = iMatchCnt;
            iBestLabel = i;
          }
      }
    if (distance != 0)
      *distance = lstClosest[iBestLabel].first;
    if (closestIndex != 0)
      *closestIndex = lstClosest[iBestLabel].second;
    return pClosestLabels[iBestLabel];
  }

  template <class FeatureIterator, class ConstFeatureIterator>
  void adaptVector(FeatureIterator code,
                   ConstFeatureIterator sample,
                   int length,
                   double alpha)
  {
    double tmp = 1.0-alpha;
    for (int i=0; i<length; ++i)
      code[i] = alpha * sample[i] + tmp * code[i];
  }

  template <class SampleSet, class DistanceMeasure>
  SampleSet kMeans(const SampleSet& samples,
                   unsigned int k,
                   const DistanceMeasure& measure,
                   unsigned int maxIterations)
  {
    const int iSamples = PiiSampleSet::sampleCount(samples),
      iFeatures = PiiSampleSet::featureCount(samples);

    SampleSet resultSet(PiiSampleSet::create<SampleSet>(0, iFeatures));
    if (int(k) >= iSamples)
      return resultSet;
    
    // First initialize the centroids by random selection
    for (int i=k; i--; )
      // OK, this is a bad thing, but we don't need a really good random
      // number here.
      PiiSampleSet::append(resultSet, PiiSampleSet::sampleAt(samples,rand() % iSamples));
    
    // Allocate storage for temporary mean vectors
    SampleSet centroidSet(resultSet);

    // Stores the number of samples per centroid.
    QVector<int> hitCounts(k, 0);

    unsigned int iterationCount = 0;
    while (maxIterations == 0 ||
           iterationCount < maxIterations)
      {
        hitCounts.fill(0);
        // Classify all samples to the closest centroid
        for (int i=0; i<iSamples; ++i)
          {
            // This is the closest centroid
            int centroidIndex = findClosestMatch(PiiSampleSet::sampleAt(samples,i), resultSet, measure);

            // Iteratively update the mean
            adaptVector(centroidSet[centroidIndex],
                        PiiSampleSet::sampleAt(samples,i),
                        iFeatures,
                        1.0/++hitCounts[centroidIndex]);
          }
        // Now we have the new centers. Let's see if they equal to the
        // previous ones...
        if (PiiSampleSet::equals(centroidSet, resultSet))
          break;

        resultSet = centroidSet;
        ++iterationCount;
      }
    return resultSet;
  } 

  template <class SampleSet> SampleSet createRandomSampleSet(int samples,
                                                             int features,
                                                             double minimum,
                                                             double maximum)
  {
    typedef typename PiiSampleSet::Traits<SampleSet>::FeatureType T;
    double dRange = maximum - minimum;
    double dScale = dRange/RAND_MAX;

    SampleSet result = PiiSampleSet::create<SampleSet>(samples, features);
    
    for (int s=0; s<samples; ++s)
      {
        typename PiiSampleSet::Traits<SampleSet>::FeatureIterator sample =
          PiiSampleSet::sampleAt(result, s);
        for (int f=0; f<features; ++f)
          sample[f] = T(dScale * rand() + minimum);
      }
    return result;
  }
}
