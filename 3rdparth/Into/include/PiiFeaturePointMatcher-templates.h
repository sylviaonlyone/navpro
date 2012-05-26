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

#ifndef _PIIFEATUREPOINTMATCHER_H
# error "Never use <PiiFeaturePointMatcher-templates.h> directly; include <PiiFeaturePointMatcher.h> instead."
#endif

template <class T, class SampleSet>
PiiFeaturePointMatcher<T,SampleSet>::PiiFeaturePointMatcher() :
  d(new Data)
{
}

template <class T, class SampleSet>
PiiFeaturePointMatcher<T,SampleSet>::PiiFeaturePointMatcher(const PiiFeaturePointMatcher& other) :
  d(other.d)
{
  d->reserve();
}

template <class T, class SampleSet>
PiiFeaturePointMatcher<T,SampleSet>::~PiiFeaturePointMatcher()
{
  d->release();
}

template <class T, class SampleSet>
PiiFeaturePointMatcher<T,SampleSet>& PiiFeaturePointMatcher<T,SampleSet>::operator= (const PiiFeaturePointMatcher& other)
{
  other.d->assignTo(d);
  return *this;
}

template <class T, class SampleSet>
void PiiFeaturePointMatcher<T, SampleSet>::buildDatabase(const PiiMatrix<T>& points,
                                                         const SampleSet& features,
                                                         const QVector<int>& modelIndices,
                                                         PiiProgressController* controller,
                                                         PiiDistanceMeasure<ConstFeatureIterator>* measure)
{
  const int iSamples = PiiSampleSet::sampleCount(features),
    iFeatures = PiiSampleSet::featureCount(features);
  
  if (points.rows() != iSamples ||
      (modelIndices.size() != 0 && points.rows() != modelIndices.size()))
    PII_THROW(PiiClassificationException,
              QCoreApplication::translate("PiiFeaturePointMatcher",
                                          "There must be an equal number of points, features, and model indices."));
  
  d->release();
  d = new Data;
  
  // We are going to use the K-d tree only if the number of points is
  // much larger than the number of features. This limit would be way
  // too low if we performed exact search, but we won't.
  if (measure == 0 && // K-d tree doesn't work in non-Euclidean spaces.
      points.rows() > 2 * iFeatures)
    {
      PiiSmartPtr<PiiKdTree<SampleSet> > pKdTree(new PiiKdTree<SampleSet>);
      pKdTree->buildTree(features, controller); // may throw
      d->pKdTree = pKdTree.release();
    }
  else
    {
      d->modelFeatures = features;
      d->pDistanceMeasure = measure;
    }
  d->matModelPoints = points;
  d->vecModelIndices = modelIndices;
}

template <class T, class SampleSet>
template <class Matcher>
PiiMatching::MatchList PiiFeaturePointMatcher<T,SampleSet>::findMatchingModels(const PiiMatrix<T>& points,
                                                                               const SampleSet& features,
                                                                               Matcher& matcher) const
{
  PiiMatching::MatchList lstMatchedModels;

  if (d->matModelPoints.isEmpty())
    return lstMatchedModels;

  using namespace PiiSampleSet;
  const int iPoints = qMin(points.rows(), sampleCount(features)),
    iDimensions = points.columns();

  typedef QHash<int, QList<QPair<int,int> > > MatchHash;
  MatchHash hashMatchIndices;
  
  // Find N closest matches for each point
  for (int i=0; i<iPoints; ++i)
    {
      PiiClassification::MatchList lstMatches;
      if (d->pKdTree != 0)
        {
          if (d->iMaxEvaluations > 0)
            lstMatches = d->pKdTree->findClosestMatches(sampleAt(features, i),
                                                        d->iClosestMatchCount,
                                                        d->iMaxEvaluations);
          else
            lstMatches = d->pKdTree->findClosestMatches(sampleAt(features, i),
                                                        d->iClosestMatchCount);
        }
      else if (d->pDistanceMeasure != 0)
        lstMatches = PiiClassification::findClosestMatches(sampleAt(features, i),
                                                           d->modelFeatures,
                                                           *d->pDistanceMeasure,
                                                           d->iClosestMatchCount);
      else
        lstMatches = PiiClassification::findClosestMatches(sampleAt(features, i),
                                                           d->modelFeatures,
                                                           d->squaredGeometricDistance,
                                                           d->iClosestMatchCount);

      // All matches that are good enough compared to the best one
      // will be accepted as candidates.
      for (int j=0; j<lstMatches.size(); ++j)
        {
          if (lstMatches[j].first == 0 || lstMatches[0].first / lstMatches[j].first > 0.8)
            {
              int iModelIndex = d->vecModelIndices.size() != 0 ?
                int(d->vecModelIndices[lstMatches[j].second]) : 0;
              // Store the indices of matched points for each model
              // class separately.
              hashMatchIndices[iModelIndex] << qMakePair(i, lstMatches[j].second);
            }
          else
            break;
        }
    }

  QList<QPair<int,int> > lstCandidateModels;
  int iMinMatches = 1;
  int iMaxMatches = 1;
  
  // Collect models that have enough matches
  for (MatchHash::iterator i = hashMatchIndices.begin();
       i != hashMatchIndices.end(); ++i)
    {
      int iMatchCount = i.value().size();
      if (iMatchCount >= iMinMatches)
        lstCandidateModels << qMakePair(iMatchCount, i.key());
    }

  // Sort according to match count (the candidate model with most
  // matches will be evaluated first)
  qSort(lstCandidateModels);

  PiiMatrix<T> matQueryPoints(0,iDimensions), matModelPoints(0,iDimensions);

  iMaxMatches = lstCandidateModels.last().first;

  matQueryPoints.reserve(iMaxMatches);
  matModelPoints.reserve(iMaxMatches);
  
  while (!lstCandidateModels.isEmpty())
    {
      // Collect point correspondences in the last candidate model.
      int iCurrentCandidate = lstCandidateModels.last().second;
      QList<QPair<int,int> >& lstMatchedPairs = hashMatchIndices[iCurrentCandidate];
      collectPoints(lstMatchedPairs,
                    points,
                    matQueryPoints,
                    matModelPoints);

      /*qDebug("%d candidate models left, current candidate: %d, matched pairs: %d",
             lstCandidateModels.size(), iCurrentCandidate,
             lstMatchedPairs.size());
      */
      // Try to match the points
      if (matcher.findBestModel(matModelPoints, matQueryPoints))
        {
          QVector<int> vecInliers = matcher.inlyingPoints();
          //qDebug("Model matched with %d inliers", vecInliers.size());
          // removePoints requires a sorted index list
          qSort(vecInliers);
          
          lstMatchedModels << PiiMatching::Match(iCurrentCandidate,
                                                 matcher.bestModel(),
                                                 matchIndices(vecInliers, lstMatchedPairs));

          
          // If only one match is requested, return now
          if (d->matchingMode == PiiMatching::MatchOneModel)
            return lstMatchedModels;

          // If points were successfully matched, remove all inliers
          // from the query set. The current candidate model will not
          // be removed from the candidate to allow multiple matches
          // to the same model ...
          removePoints(vecInliers, lstMatchedPairs);

          // ... unless only one match per model is allowed.
          if (d->matchingMode == PiiMatching::MatchDifferentModels)
            lstCandidateModels.removeLast();
        }
      else
        lstCandidateModels.removeLast();

      matQueryPoints.resize(0, iDimensions);
      matModelPoints.resize(0, iDimensions);
    }

  return lstMatchedModels;
}

template <class T, class SampleSet>
void PiiFeaturePointMatcher<T,SampleSet>::collectPoints(const QList<QPair<int,int> >& indices,
                                                        const PiiMatrix<T>& points,
                                                        PiiMatrix<T>& queryPoints,
                                                        PiiMatrix<T>& modelPoints) const
{
  for (int i=0; i<indices.size(); ++i)
    {
      queryPoints.appendRow(points[indices[i].first]);
      modelPoints.appendRow(d->matModelPoints[indices[i].second]);
    }
}

template <class T, class SampleSet>
void PiiFeaturePointMatcher<T,SampleSet>::removePoints(const QVector<int>& indices,
                                                       QList<QPair<int,int> >& matches)
{
  for (int i=indices.size(); i--; )
    matches.removeAt(indices[i]);
}

template <class T, class SampleSet>
QList<QPair<int,int> > PiiFeaturePointMatcher<T,SampleSet>::matchIndices(const QVector<int>& indices,
                                                                         const QList<QPair<int,int> >& matches)
{
  const int iCnt = indices.size();
  QList<QPair<int,int> > lstResult;
  for (int i=0; i<iCnt; ++i)
    lstResult << matches[indices[i]];
  return lstResult;
}

namespace PiiMatching
{
  template <class Merger> void removeDuplicates(MatchList& matchedModels, Merger& merge)
  {
    for (int i=matchedModels.size()-1; i>0; --i)
      {
        for (int j=i-1; j>=0; --j)
          {
            // If there are two matches to the same model and they can
            // be merged, remove the other entry.
            if (matchedModels[i].modelIndex() == matchedModels[j].modelIndex() &&
                merge(matchedModels[i], matchedModels[j]))
              {
                matchedModels.removeAt(i);
                break;
              }
          }
      }
  }
}
