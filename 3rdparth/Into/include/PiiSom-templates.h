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

#ifndef _PIISOM_H
# error "Never use <PiiSom-templates.h> directly; include <PiiSom.h> instead."
#endif

#include <PiiMath.h>
#include <PiiUtil.h>
#include "PiiClassification.h"


template <class SampleSet> PiiSom<SampleSet>::Data::Data(int width,
                                                         int height) :
  iSizeX(width), iSizeY(height <= 0 ? width : height),
  iIterationNumber(0),
  iLearningLength(10000), dRadius(10), dLearningRate(0.05),
  iCodeBookCollectionIndex(-1),
  initMode(PiiClassification::SomSampleInit),
  topology(PiiClassification::SomHexagonal),
  rateFunction(PiiClassification::SomLinearAlpha),
  neighborhood(PiiClassification::SomBubble),
  algorithm(PiiClassification::SomSequentialAlgorithm),
  dMinQErr(0), dMaxQErr(0), dQErrRange(1),
  dMeanDist(0)
{}


template <class SampleSet> PiiSom<SampleSet>::PiiSom(int width,
                                                     int height) :
  PiiVectorQuantizer<SampleSet>(new Data(width, height))
{
}

template <class SampleSet> bool PiiSom<SampleSet>::converged() const throw ()
{
  const PII_D;
  return d->iIterationNumber >= d->iLearningLength;
}

template <class SampleSet> PiiClassification::LearnerCapabilities PiiSom<SampleSet>::capabilities() const
{
  return PiiClassification::NonSupervisedLearner | PiiClassification::OnlineLearner;
}

template <class SampleSet> QVector<double> PiiSom<SampleSet>::findMostDistantNeighbors(int* vector1Index, int* vector2Index) const
{
  const PII_D;
  const int iSamples = this->sampleCount();
  if (iSamples < d->iSizeX * d->iSizeY)
    return QVector<double>(iSamples, -1);
    
  double distance = 0;
  
  QVector<double> distances(iSamples);
  for (int i=0; i<iSamples; ++i)
    {
      double dist = findMostDistantNeighbor(i, vector2Index);
      distances[i] = dist;
      if (dist > distance)
        {
          distance = dist;
          if (vector1Index != 0)
            *vector1Index = i;
        }
    }

  return distances;
}

template <class SampleSet> double PiiSom<SampleSet>::findMostDistantNeighbor(int vector1Index, int *vector2Index) const
{
  const PII_D;
  const int iSamples = this->sampleCount(), iFeatures = this->featureCount();
  if (iSamples < d->iSizeX * d->iSizeY)
    return QVector<double>(iSamples, -1);
  
  int hX = vector1Index % d->iSizeX;
  int hY = vector1Index / d->iSizeX;

  double distance = 0;
  for (int x=hX-1; x<=hX+1; ++x)
    {
      for (int y=hY-1; y<=hY+1; ++y)
        {
          if ((x >= 0 && x < d->iSizeX) &&
              (y >= 0 && y < d->iSizeY))
            {
              int vector2 = y*d->iSizeX + x;
              double dist = (*this->distanceMeasure())(this->model(vector1Index),
                                                       this->model(vector2),
                                                       iFeatures);

              if (dist > distance)
                {
                  distance = dist;
                  if (vector2Index != 0)
                    *vector2Index = vector2;
                }
            }
        }
    }

  return distance;
}

template <class SampleSet> double PiiSom<SampleSet>::currentRadius() const
{
  const PII_D;
  //radius decreases linearly from the initial value to 1
  int index = d->iIterationNumber < d->iLearningLength ? d->iIterationNumber : d->iLearningLength;
  return 1.0 + (d->dRadius-1.0) * (double)(d->iLearningLength-index)/(double)d->iLearningLength;
}

template <class SampleSet> double PiiSom<SampleSet>::currentLearningRate() const
{
  const PII_D;
  //learning rate decreases from the initial value to 0
  //training index cannot exceed the estimated training length
  int index = d->iIterationNumber < d->iLearningLength ? d->iIterationNumber : d->iLearningLength;
  if (d->rateFunction == PiiClassification::SomLinearAlpha)
    return linearAlpha(index);
  else
    return inverseAlpha(index);
}

template <class SampleSet> void PiiSom<SampleSet>::setIterationNumber(int index)
{
  PII_D;
  d->iIterationNumber = index;
  // algorithm initialization
  if (index == 0)
    {
      d->dMaxQErr = d->dMinQErr = d->dMeanDist = 0;
      d->dQErrRange = 1;
      PiiSampleSet::clear(d->previousSample);
      PiiSampleSet::clear(d->meanSample);
    }
}

template <class SampleSet> void PiiSom<SampleSet>::learn(const SampleSet& samples,
                                                         const QVector<double>& /*labels*/,
                                                         const QVector<double>& /*weights*/)
{
  PII_D;
  const int iMapSize = d->iSizeX * d->iSizeY,
    iSamples = PiiSampleSet::sampleCount(samples),
    iFeatures = PiiSampleSet::featureCount(samples);
  if (iSamples == 0)
    return;
  if (this->modelCount() == 0)
    {
      if (d->initMode == PiiClassification::SomSampleInit)
        {
          SampleSet codeBook(PiiSampleSet::create<SampleSet>(0, iFeatures));
          // If there are less samples than code vectors,
          // selectRandomly() cannot fill the code vectors in one
          // round.
          QVector<int> vecIndices;
          while (PiiSampleSet::sampleCount(codeBook) < iMapSize)
            {
              Pii::selectRandomly(vecIndices,
                                  iMapSize - PiiSampleSet::sampleCount(codeBook),
                                  iSamples);
              for (int i=0; i<vecIndices.size(); ++i)
                PiiSampleSet::append(codeBook, PiiSampleSet::sampleAt(samples, vecIndices[i]));
            }
          this->setModels(codeBook);
        }
      else
        {
          typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator sample = PiiSampleSet::sampleAt(samples, 0);
          this->setModels(PiiClassification::createRandomSampleSet<SampleSet>(d->iSizeX * d->iSizeY,
                                                                              iFeatures,
                                                                              Pii::minAll(sample, sample + iFeatures),
                                                                              Pii::maxAll(sample, sample + iFeatures)));
        }
    }

  while (true)
    {
      for (int i=0; i<iSamples; ++i)
        {
          learnOne(PiiSampleSet::sampleAt(samples,i), iFeatures, NAN);
          if (this->converged())
            goto endTraining;
          PII_TRY_CONTINUE(this->controller(), double(d->iIterationNumber+1)/d->iLearningLength);
        }
    }
 endTraining:;
}

template <class SampleSet> double PiiSom<SampleSet>::learnOne(ConstFeatureIterator features,
                                                              int length,
                                                              double /*label*/,
                                                              double /*weight*/) throw()
{
  const int iModels = this->modelCount();
  PII_D;
  // If there is no code book, initialize it
  if (iModels == 0)
    {
      if (d->initMode == PiiClassification::SomRandomInit)
        setModels(PiiClassification::createRandomSampleSet<SampleSet>(d->iSizeX * d->iSizeY,
                                                                      length,
                                                                      Pii::minAll(features, features + length),
                                                                      Pii::maxAll(features, features + length)));
      else
        {
          setModels(PiiSampleSet::create<SampleSet>(d->iSizeX * d->iSizeY,
                                                    length));
          // Start collecting a new code book
          // Ensure that the size has been properly set
          if (d->iSizeX * d->iSizeY > 0)
            d->iCodeBookCollectionIndex = 0;
          else
            d->iCodeBookCollectionIndex = -1;
        }
    }

  // If we are still collecting the code book...
  if (d->iCodeBookCollectionIndex >= 0)
    {
      // Store the incoming vector
      PiiSampleSet::setSampleAt(d->modelSet, d->iCodeBookCollectionIndex, features);
      if (++d->iCodeBookCollectionIndex >= iModels)
        {
          d->iCodeBookCollectionIndex = -1;
          return iModels - 1;
        }
      return d->iCodeBookCollectionIndex-1;
    }

  return adaptTo(features);
}


template <class SampleSet> int PiiSom<SampleSet>::adaptTo(ConstFeatureIterator vector)
{
  PII_D;
  // Find the closest code vector
  double distance;
  int iVectorIndex = this->findClosestMatch(vector, &distance);
  if (iVectorIndex == -1)
    return -1;
  
  // Its coordinates ...
  int hX = iVectorIndex % d->iSizeX;
  int hY = iVectorIndex / d->iSizeX;

  // Adapt the neighborhood
  adaptNeighborhood(hX, hY, vector, distance);
  ++d->iIterationNumber;
  return iVectorIndex;
}

/*
 * Adapt the neighborhood of a SOM node towards the given vector. The
 * current learning parameters affect the strength and radius of the
 * adaptation.
 *
 * @param hitX the x coordinate of the closest code vector
 *
 * @param hitY the y coordinate of the closest code vector
 *
 * @param vector the training sample
 *
 * @param distance distance to the closest code vector
 */
template <class SampleSet> void PiiSom<SampleSet>::adaptNeighborhood(int hitX, int hitY,
                                                                     ConstFeatureIterator vector,
                                                                     double distance)
{
  using namespace PiiSampleSet;
  PII_D;
  double radius = currentRadius();
  radius *= radius; // square

  const int iFeatures = this->featureCount();
  double alpha = 0;

  switch (d->algorithm)
    {
    case PiiClassification::SomSequentialAlgorithm:
      alpha = currentLearningRate();
      break;
      
    case PiiClassification::SomBalancedAlgorithm:
      {
        // Iterative calculation of average sample
        double w = 1/(d->iIterationNumber+1);

        if (PiiSampleSet::sampleCount(d->meanSample) == 0)
          {
            d->meanSample = create<SampleSet>(0, iFeatures);
            append(d->meanSample, vector);
          }
        else
          PiiClassification::adaptVector(sampleAt(d->meanSample, 0), vector, iFeatures, w);
    
        // Iterative calculation of mean distance between samples
        if (sampleCount(d->previousSample) != 0)
          {
            w = 1.0/d->iIterationNumber;
            // Distance between current and previous sample
            double dist = (*this->distanceMeasure())(vector, sampleAt(d->previousSample, 0), iFeatures);
            d->dMeanDist = d->dMeanDist * (1.0-w) + dist * w;

            double sampleWeight = (*this->distanceMeasure())(vector, sampleAt(d->meanSample, 0), iFeatures);
            // Weight based on normalized mean distance
            if (d->dMeanDist != 0.0)
              sampleWeight /= d->dMeanDist;

            alpha = currentLearningRate() * sampleWeight;
          }
        else
          {
            d->previousSample = create<SampleSet>(1, iFeatures);
            alpha = currentLearningRate();
          }
        
        setSampleAt(d->previousSample, 0, vector);
      }
      break;
      
    case PiiClassification::SomQErrAlgorithm:
      {
        // Learning constant is determined by the sample's distance to
        // closest code vector.
        if (distance < d->dMinQErr)
          {
            d->dMinQErr = distance;
            d->dQErrRange = d->dMaxQErr - d->dMinQErr;
          }
        else if (distance > d->dMaxQErr)
          {
            d->dMaxQErr = distance;
            d->dQErrRange = d->dMaxQErr - d->dMinQErr;
          }
        
        alpha = (distance - d->dMinQErr) / d->dQErrRange;
      }
      break;
    }

  const int iModels = this->modelCount();
  for (int index=0; index < iModels; ++index)
    {
      // current x,y
      int tX = index % d->iSizeX;
      int tY = index / d->iSizeX;

      //distance to the current node at (hitX, hitY)
      double distance = d->topology == PiiClassification::SomHexagonal ?
        PiiClassification::somHexagonalDistance(hitX, hitY, tX, tY) :
        PiiClassification::somSquareDistance(hitX, hitY, tX, tY);

      /* Note that radius is squared! somXXXDistance functions return
       * a squared distance as well.
       */
      switch (d->neighborhood)
        {
        case PiiClassification::SomBubble:
          // Bubble neighborhood equally adapts all vectors within the
          // current radius
          if (distance <= radius)
            PiiClassification::adaptVector(this->modelAt(index), vector, iFeatures, alpha);
          break;
        case PiiClassification::SomGaussian:
          // Gaussian updates all vectors, and weights the update with
          // a Gaussian function.
          PiiClassification::adaptVector(this->modelAt(index), vector, iFeatures,
                                    alpha*std::exp(-distance/(2*radius)));
          break;
        case PiiClassification::SomCutGaussian:
          // Combination of the two above.
          if (distance <= radius)
            PiiClassification::adaptVector(this->modelAt(index), vector, iFeatures,
                                      alpha*std::exp(-distance/(2*radius)));
          break;
        }
    }
}

template <class SampleSet> void PiiSom<SampleSet>::setSize(int width, int height)
{
  PII_D;
  if (width != d->iSizeX || height != d->iSizeY)
    PiiSampleSet::clear(d->modelSet);
  d->iSizeX = width;
  d->iSizeY = height;
}

template <class SampleSet> double PiiSom<SampleSet>::initialRadius() const { return _d()->dRadius; }
template <class SampleSet> void PiiSom<SampleSet>::setInitialRadius(double radius) { _d()->dRadius = radius; }
template <class SampleSet> PiiClassification::SomTopology PiiSom<SampleSet>::topology() const { return _d()->topology; }
template <class SampleSet> void PiiSom<SampleSet>::setTopology(PiiClassification::SomTopology topology) { _d()->topology = topology; }
template <class SampleSet> double PiiSom<SampleSet>::initialLearningRate() const { return _d()->dLearningRate; }
template <class SampleSet> void PiiSom<SampleSet>::setInitialLearningRate(double rate) { _d()->dLearningRate = rate; }
template <class SampleSet> int PiiSom<SampleSet>::learningLength() const { return _d()->iLearningLength; }
template <class SampleSet> void PiiSom<SampleSet>::setLearningLength(int length) { _d()->iLearningLength = length; }
template <class SampleSet> PiiClassification::SomRateFunction PiiSom<SampleSet>::rateFunction() const { return _d()->rateFunction; }
template <class SampleSet> void PiiSom<SampleSet>::setRateFunction(PiiClassification::SomRateFunction func) { _d()->rateFunction = func; }
template <class SampleSet> int PiiSom<SampleSet>::iterationNumber() const { return _d()->iIterationNumber; }
template <class SampleSet> int PiiSom<SampleSet>::width() const { return _d()->iSizeX; }
template <class SampleSet> int PiiSom<SampleSet>::height() const { return _d()->iSizeY; }
template <class SampleSet> void PiiSom<SampleSet>::setWidth(int width) { setSize(width, _d()->iSizeY); }
template <class SampleSet> void PiiSom<SampleSet>::setHeight(int height) { setSize(_d()->iSizeX, height); }
template <class SampleSet> void PiiSom<SampleSet>::setInitMode(PiiClassification::SomInitMode mode) { _d()->initMode = mode; }
template <class SampleSet> PiiClassification::SomInitMode PiiSom<SampleSet>::initMode() const { return _d()->initMode; }
template <class SampleSet> PiiClassification::SomLearningAlgorithm PiiSom<SampleSet>::learningAlgorithm() const { return _d()->algorithm; }
template <class SampleSet> void PiiSom<SampleSet>::setLearningAlgorithm(PiiClassification::SomLearningAlgorithm algorithm) { _d()->algorithm = algorithm; }
template <class SampleSet> int PiiSom<SampleSet>::codeBookCollectionIndex() { return _d()->iCodeBookCollectionIndex; }
