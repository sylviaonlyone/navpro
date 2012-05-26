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

#include "PiiRansac.h"

#include <PiiAlgorithm.h>
#include <PiiFunctional.h>
#include <PiiUtil.h>
#include <PiiMath.h>
#include <QVector>

PiiRansac::Data::Data() :
  iMaxIterations(1000),
  iMaxSamplings(100),
  iMinInliers(0),
  dFittingThreshold(16),
  dSelectionProbability(0.99)
{
}

PiiRansac::PiiRansac() : d(new Data)
{
}

PiiRansac::PiiRansac(Data* data) : d(data)
{
}

PiiRansac::~PiiRansac()
{
  delete d;
}

bool PiiRansac::findBestModel()
{
  const int iSamples = totalSampleCount();
  const int iMinSamples = minSamples();
  const double dLogProp = Pii::log(1.0 - d->dSelectionProbability);

  if (iSamples < iMinSamples)
    return false;
  
  int iIterations = 0, iRequiredIterations = 1;

  d->vecBestInliers.clear();
  d->matBestModel.clear();

  // This vector stores the indices of all points.
  QVector<int> vecIndices(iSamples);
  Pii::generateN(vecIndices.begin(), iSamples, Pii::CountFunction<int>());
  // Randomize order
  Pii::shuffleN(vecIndices.begin(), iSamples);
  int iSubsetStartIndex = 0;

  while (iIterations < qMin(d->iMaxIterations, iRequiredIterations))
    {
      PiiMatrix<double> matModels;
      int iSamplingCount = 0;

      // Try hard to find a non-degenerate model
      while (matModels.isEmpty() && iSamplingCount < d->iMaxSamplings)
        {
          // No more random orderings left -> reshuffle the samples
          // and start over.
          if (iSubsetStartIndex + iMinSamples > vecIndices.size())
            {
              Pii::shuffleN(vecIndices.begin(), iSamples);
              iSubsetStartIndex = 0;
            }
          matModels = findPossibleModels(vecIndices.begin() + iSubsetStartIndex);
          iSubsetStartIndex += iMinSamples;
          ++iSamplingCount;
          // Special case: if there is only one way to select the
          // samples, there is no need to try again.
          if (iSamples == iMinSamples)
            break;
        }

      // We are out of luck. No model could be found.
      if (matModels.isEmpty())
        return false;

      // This vector stores the indices of selected inlying points.
      QVector<int> vecInliers;
      vecInliers.reserve(iSamples);

      // Test all possible models
      for (int iModel=0; iModel<matModels.rows(); ++iModel)
        {
          // Clear the inlier list at the beginning of each round.
          vecInliers.resize(0);
            
          // Match all points against the current model
          for (int iPoint=0; iPoint<iSamples; ++iPoint)
            {
              // Store points that match to the model with an error
              // less than the threshold.
              if (fitToModel(iPoint, matModels.constRowBegin(iModel)) < d->dFittingThreshold)
                vecInliers << iPoint;
            }
          
          // If the number of inliers is the best so far, store the
          // score.
          const int iInlierCount = vecInliers.size();
          if (iInlierCount > d->vecBestInliers.size())
            {
              if (iInlierCount > d->iMinInliers)
                {
                  d->vecBestInliers = vecInliers;
                  d->matBestModel = matModels(iModel,0,1,-1);
                }

              // The fraction of inliers
              double dInlierFraction = double(iInlierCount) / iSamples;
              if (dInlierFraction != 1.0)
                iRequiredIterations = Pii::round<int>(dLogProp /
                                                      Pii::log(1.0 - Pii::pow(dInlierFraction, iMinSamples)));
              else
                iRequiredIterations = 0;
            }
        }
      ++iIterations;
    }
  
  return !d->matBestModel.isEmpty();
}

PiiMatrix<double> PiiRansac::bestModel() const { return d->matBestModel; }
QVector<int> PiiRansac::inlyingPoints() const { return d->vecBestInliers; }
int PiiRansac::inlierCount() const { return d->vecBestInliers.size(); }
void PiiRansac::setMaxIterations(int maxIterations) { d->iMaxIterations = maxIterations; }
int PiiRansac::maxIterations() const { return d->iMaxIterations; }
void PiiRansac::setMaxSamplings(int maxSamplings) { d->iMaxSamplings = maxSamplings; }
int PiiRansac::maxSamplings() const { return d->iMaxSamplings; }
void PiiRansac::setMinInliers(int minInliers) { d->iMinInliers = minInliers; }
int PiiRansac::minInliers() const { return d->iMinInliers; }
void PiiRansac::setFittingThreshold(double fittingThreshold) { d->dFittingThreshold = fittingThreshold; }
double PiiRansac::fittingThreshold() const { return d->dFittingThreshold; }
void PiiRansac::setSelectionProbability(double selectionProbability) { d->dSelectionProbability = selectionProbability; }
double PiiRansac::selectionProbability() const { return d->dSelectionProbability; }
