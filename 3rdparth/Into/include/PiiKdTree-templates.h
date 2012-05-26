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

#ifndef _PIIKDTREE_H
# error "Never use <PiiKdTree-templates.h> directly; include <PiiKdTree.h> instead."
#endif


template <class SampleSet>
PiiKdTree<SampleSet>::PiiKdTree() :
  d(new Data)
{
}

template <class SampleSet>
PiiKdTree<SampleSet>::PiiKdTree(const PiiKdTree& other) :
  d(other.data)
{
  d->reserve();
}

template <class SampleSet>
PiiKdTree<SampleSet>::PiiKdTree(const SampleSet& modelSet) :
  d(new Data)
{
  buildTree(modelSet);
}

template <class SampleSet> PiiKdTree<SampleSet>::~PiiKdTree()
{
  d->release();
}

template <class SampleSet>
PiiKdTree<SampleSet>& PiiKdTree<SampleSet>::operator= (const PiiKdTree& other)
{
  other.d->assignTo(d);
  return *this;
}

template <class SampleSet>
void PiiKdTree<SampleSet>::buildTree(const SampleSet& modelSet,
                                     PiiProgressController* controller)
{
  d->release();
  d = new Data;
  const int iSampleCount = PiiSampleSet::sampleCount(modelSet);
  d->iFeatureCount = PiiSampleSet::featureCount(modelSet);
  if (d->iFeatureCount == 0 || iSampleCount == 0)
    return;
  
  QVector<FeatureSorter> vecSorters(iSampleCount);
  QVector<double> vecMeans(d->iFeatureCount);
  d->pMeans = vecMeans.data();
  QVector<double> vecVars(d->iFeatureCount);
  d->pVars = vecVars.data();
  for (int i=0; i<iSampleCount; ++i)
    vecSorters[i].second = i;

  d->modelSet = modelSet;
  // May throw a PiiClassificationException
  d->pRoot = createNode(vecSorters.data(), iSampleCount, 0, controller);
}

template <class SampleSet>
int PiiKdTree<SampleSet>::selectDimension(FeatureSorter* sorterArray,
                                          int sampleCount,
                                          int /*depth*/)
{
  Pii::fillN(d->pMeans, d->iFeatureCount, 0.0);
  Pii::fillN(d->pVars, d->iFeatureCount, 0.0);
  // Calculate mean for each dimension.
  for (int i=0; i<sampleCount; ++i)
    Pii::mapN(d->pMeans, d->iFeatureCount, sampleAt(sorterArray[i].second), std::plus<double>());
  Pii::mapN(d->pMeans, d->iFeatureCount, std::bind2nd(std::multiplies<double>(), 1.0/sampleCount));

  // Sum of squared differences to mean = variance
  for (int i=0; i<sampleCount; ++i)
    {
      Sample sample = sampleAt(sorterArray[i].second);
      for (int j=0; j<d->iFeatureCount; ++j)
        d->pVars[j] += Pii::square(sample[j] - d->pMeans[j]);
    }
  Pii::mapN(d->pVars, d->iFeatureCount, std::bind2nd(std::multiplies<double>(), 1.0/sampleCount));

  // Return the index of the dimension with max variance.
  return Pii::findSpecialValue(d->pVars, d->pVars + d->iFeatureCount,
                               std::greater<T>(),
                               Pii::Identity<T>()) - d->pVars;
}

template <class SampleSet>
typename PiiKdTree<SampleSet>::Node* PiiKdTree<SampleSet>::createNode(FeatureSorter* sorterArray,
                                                                      int sampleCount,
                                                                      int depth,
                                                                      PiiProgressController* controller)
{
  if (sampleCount == 0)
    return 0;
  else if (sampleCount == 1)
    return new Node(sorterArray[0].second);
  
  // Select the dimension that best splits the remaining samples.
  int iSplitDimension = selectDimension(sorterArray, sampleCount, depth);

  // Collect the features on the selected dimension.
  for (int i=0; i<sampleCount; ++i)
    sorterArray[i].first = sampleAt(sorterArray[i].second)[iSplitDimension];

  // Partial sort. Median is now at the center of the array.
  Pii::fastMedian(sorterArray, sampleCount);

  int iHalf = (sampleCount-1) / 2;

  ++depth;

  // Exception safety
  PiiSmartPtr<Node> smaller(createNode(sorterArray, iHalf, depth, controller));
  PiiSmartPtr<Node> larger(createNode(sorterArray + iHalf + 1, sampleCount - iHalf - 1, depth, controller));

  PII_TRY_CONTINUE(controller, NAN);

  int iSampleIndex = sorterArray[iHalf].second;
  return new Node(iSampleIndex,
                  iSplitDimension,
                  sampleAt(iSampleIndex)[iSplitDimension],
                  smaller.release(),
                  larger.release());
}

template <class SampleSet>
int PiiKdTree<SampleSet>::findClosestMatch(Sample sample,
                                           double* distance) const
{
  QPair<double,int> minimum(INFINITY, -1);
  if (d->pRoot != 0)
    findClosestMatches(d->pRoot, sample, minimum);
  if (distance != 0)
    *distance = minimum.first;
  return minimum.second;
}

template <class SampleSet>
PiiClassification::MatchList PiiKdTree<SampleSet>::findClosestMatches(Sample sample,
                                                                      int n) const
{
  PiiClassification::MatchList heap;

  if (d->pRoot == 0)
    return heap;

  heap.fill(qMin(sampleCount(), n), qMakePair(double(INFINITY), -1));

  findClosestMatches(d->pRoot, sample, heap);
  // Ascending order -> first is the best match
  heap.sort();
  return heap;
}

template <class SampleSet> template <class MatchList>
void PiiKdTree<SampleSet>::findClosestMatches(Node* node,
                                              Sample sample,
                                              MatchList& matchList) const
{
  if (node == 0)
    return;

  // Measure distance to the sample at this node.
  double dDistance = d->measure(sample, sampleAt(node->sampleIndex), d->iFeatureCount);
  /* Update minimum distance if needed. In k-NN search, a priority
     queue of k best matches are maintained. The first element in the
     queue is the current estimate of the kth closest match.
   */
  if (dDistance < distanceLimit(matchList))
    updateLimit(dDistance, node->sampleIndex, matchList);

  // Recursive descend
  T featureValue = sample[node->splitDimension];
  // Decide search order based on the selected feature value.
  if (featureValue <= node->featureValue)
    {
      this->findClosestMatches(node->smaller, sample, matchList);
      /* If the closest child could be on the other side of this
         splitting hyperplane, we need to search the other side too. 
         In the case of a k-NN search take the kth closest node
         instead of the closest one.
      */ 
      if (Pii::square(node->featureValue - featureValue) <= distanceLimit(matchList))
        findClosestMatches(node->larger, sample, matchList);
    }
  else
    {
      findClosestMatches(node->larger, sample, matchList);
      if (Pii::square(node->featureValue - featureValue) <= distanceLimit(matchList))
        findClosestMatches(node->smaller, sample, matchList);
    }
}

template <class SampleSet>
int PiiKdTree<SampleSet>::findClosestMatch(Sample sample,
                                           int maxEvaluations,
                                           double* distance) const
{
  if (d->pRoot == 0)
    return -1;

  QPair<double,int> pair(INFINITY, -1);

  findClosestMatches(sample, maxEvaluations, pair);

  if (distance != 0)
    *distance = pair.first;
  return pair.second;
}

template <class SampleSet>
PiiClassification::MatchList PiiKdTree<SampleSet>::findClosestMatches(Sample sample,
                                                                      int n,
                                                                      int maxEvaluations) const
{
  PiiClassification::MatchList heap;
  if (d->pRoot == 0)
    return heap;

  heap.fill(qMin(sampleCount(), n), qMakePair(double(INFINITY), -1));

  findClosestMatches(sample, maxEvaluations, heap);

  heap.sort();
  return heap;
}

template <class SampleSet> template <class MatchList>
void PiiKdTree<SampleSet>::findClosestMatches(Sample sample,
                                              int maxEvaluations,
                                              MatchList& matches) const
{
  Node* pNode = d->pRoot;

  BranchList lstBranches;

  for (;;)
    {
      // Search recursively to a leaf node. In backtracking,
      // lstBranches will be filled with nodes that need to be
      // evaluated to be sure about the NN.
      findPossibleBranches(pNode, sample, &maxEvaluations, lstBranches, matches);
      // No more choices found -> found the exact NN
      // No more evaluations allowed -> don't know the NN for sure,
      // but who cares?
      if (lstBranches.isEmpty() || maxEvaluations <= 0)
        break;
      // Otherwise find the closest node and continue search from there.
      typename BranchList::iterator i = Pii::findSpecialValue(lstBranches.begin(),
                                                              lstBranches.end(),
                                                              std::less<BranchSorter>(),
                                                              Pii::Identity<BranchSorter>());
      lstBranches.erase(i);
      pNode = i->second;
    }
}


template <class SampleSet> template <class MatchList>
void PiiKdTree<SampleSet>::findPossibleBranches(Node* node,
                                                Sample sample,
                                                int *maxEvaluations,
                                                BranchList& branchList,
                                                MatchList& matches) const
{
  if (node == 0 || *maxEvaluations <= 0)
    return;

  /* This function is equivalent to the normal recursive tree look-up,
     but instead of inspecting each possible branch when unwinding the
     stack, all possible branches are collected into a priority queue
     and inspected in the order of increasing distance from the query
     sample.
   */
  --*maxEvaluations;

  double dDistance = d->measure(sample, sampleAt(node->sampleIndex), d->iFeatureCount);

  if (dDistance < distanceLimit(matches))
    updateLimit(dDistance, node->sampleIndex, matches);
  /* Prune branches that are no longer possible.
     for (typename BranchList::iterator i=branchList.begin(); i != branchList.end(); )
     {
     T diff = sample[i->second->splitDimension] - i->second->featureValue;
     double dSqDiff = Pii::square(diff);
     if (Pii::square(diff) > distanceLimit(matches))
     i = branchList.erase(i);
     else
     ++i;
     }
  */

  T featureValue = sample[node->splitDimension];
  if (featureValue <= node->featureValue)
    {
      findPossibleBranches(node->smaller, sample, maxEvaluations, branchList, matches);
      // The NN could be on the other branch -> store to possible
      // branches and let the caller to evaluate it if needed.
      if (node->larger != 0 && Pii::square(node->featureValue - featureValue) <= distanceLimit(matches))
        branchList.append(BranchSorter(dDistance, node->larger));
    }
  else
    {
      findPossibleBranches(node->larger, sample, maxEvaluations, branchList, matches);
      if (node->smaller != 0 && Pii::square(node->featureValue - featureValue) <= distanceLimit(matches))
        branchList.append(BranchSorter(dDistance, node->smaller));
    }
}
