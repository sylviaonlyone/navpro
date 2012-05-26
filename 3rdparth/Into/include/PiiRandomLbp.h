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

#ifndef _PIIRANDOMLBP_H
#define _PIIRANDOMLBP_H

#include <PiiMatrix.h>
#include <PiiGeometricObjects.h>
#include <PiiBits.h>
#include <QPair>
#include <QVector>

/**
 * Random Local Binary Patterns is a keypoint description technique. 
 * Each RLBP consists of a randomly chosen set of N coordinate pairs,
 * bounded by a predefined window that is centered at the keypoint. A
 * keypoint descriptor is calculated by comparing pairs of pixels at
 * the selected locations. Each comparison yields a binary digit whose
 * value depends on which of the two pixels is brighter. N comparisons
 * result in an N-bit binary number analogously to the @ref PiiLbp
 * "LBP operator".
 *
 * M different RLBP codes are calculated for each keypoint, resulting
 * in M N-bit binary numbers. The keypoint detector is trained by
 * applying different geometric transformations to the input image and
 * recalculating the RLBPs for each simulated viewpoint. The resulting
 * binary numbers are collected into M histograms, @f$2^N@f$ levels
 * each. These histograms are concatenated to produce a single
 * histogram of @f$M 2^N@f$ bins.
 *
 * The technique was originally named "random ferns" by its
 * developers. The "random local binary patterns" used here is a more
 * descriptive name, and it emphasizes the fact that the difference
 * between "random ferns" and LBP is just in the way the compared
 * pixel pairs are selected.
 *
 * @ingroup PiiTemplateMatchingPlugin
 */
class PiiRandomLbp
{
public:
  /**
   * Sets parameters for the RLBP and re-randomizes the selected point
   * pairs. The total length of the feature point descriptor will be
   * @a patterns * 2 ^ @a pairs.
   *
   * @param patterns the number of random LBPs to create
   *
   * @param pairs the number of pixel pairs to include in each pattern
   *
   * @param rows the number of rows in the local window
   *
   * @param columns the number of columns in the local window. If this
   * value is zero or negative, the local window will be a @a rows x
   * @a rows square.
   */
  void setParameters(int patterns, int pairs, int rows, int columns = 0);

  /**
   * Returns a 1-by-@f$M 2^N@f$ matrix whose all entries will be
   * initially set to ones.
   */
  PiiMatrix<int> initializeHistogram() const;

  /**
   * Calculates the M N-bit RLBP codes in @a image and adds one to all
   * matching entries in @a histogram. Assume N is 4 and the
   * calculated binary codes become 3, 10, 0, and 15. This function
   * would then increment bins 3, 26, 32, and 63 in @a histogram.
   */
  template <class T> void updateHistogram(int* histogram, const PiiMatrix<T>& image);
  
private:
  typedef QPair<PiiPoint<int>, PiiPoint<int> > PointPair;
  typedef QVector<PointPair> PointPairList;
  class Data
  {
  public:
    Data();
    
    int iPatterns, iPairs;
    PointPairList vecPointPairs;
  } *d;
};

template <class T> void PiiRandomLbp::updateHistogram(int* histogram,
                                                      const PiiMatrix<T>& image)
{
  const PointPair* pPair = d->vecPointPairs.data();
  const int iBinsPerPattern = 1 << d->iPairs;
  const int iFinalShift = sizeof(int)*8 - d->iPairs;

  for (int iPattern=0; iPattern<d->iPatterns; ++iPattern)
    {
      unsigned int iCode = Pii::signBit(image(pPair->first.y,
                                              pPair->first.x),
                                        image(pPair->second.y,
                                              pPair->second.x));
      ++pPair;
      for (int iPair=1; iPair<d->iPairs; ++iPair, ++pPair)
        iCode |= Pii::signBit(image(pPair->first.y,
                                    pPair->first.x),
                              image(pPair->second.y,
                                    pPair->second.x)) >> iPair;
      ++histogram[iPattern * iBinsPerPattern + (iCode >> iFinalShift)];
    }
}

#endif //_PIIRANDOMLBP_H
