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

#ifndef _PIIMULTIFEATUREDISTANCE_H
#define _PIIMULTIFEATUREDISTANCE_H

#include "PiiDistanceMeasure.h"
#include "PiiClassification.h"
#include <PiiMath.h>

/**
 * A distance measure that combines many distance measures into one. 
 * PiiMultiFeatureDistance calculates distances between feature
 * vectors that are composed of many feature vectors by concatenating
 * them into one long vector. It is configured with an index matrix
 * that tells the boundaries of sub-vectors within this long vector. 
 * For each sub-vector, a different distance measure may be used. By
 * default, PiiMultiFeatureDistance sums the separate distance
 * together to get the final distance. Other options are minimum,
 * maximum and product.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class FeatureIterator> class PiiMultiFeatureDistance :
  public QList<PiiDistanceMeasure<FeatureIterator>* >
{
public:
  /**
   * Create a new PiiMultiFeatureDistance.
   */
  PiiMultiFeatureDistance();

  ~PiiMultiFeatureDistance();

  /**
   * Set the combination mode.
   */
  void setCombinationMode(PiiClassification::DistanceCombinationMode mode);

  /**
   * Get the combination mode.
   */
  PiiClassification::DistanceCombinationMode combinationMode() const;

  /**
   * Set the boundaries of sub-vectors within the compound feature
   * vector.
   */
  void setBoundaries(const PiiMatrix<int>& boundaries);

  /**
   * Get the sub-vector boundaries.
   */
  PiiMatrix<int> boundaries() const;

  /**
   * Set a weight for each distance measure. The default weight is 1.
   */
  void setWeights(const QVector<double>& weights);

  /**
   * Get the weights for each distance measure.
   */
  QVector<double> weights() const;

  double operator() (const FeatureIterator sample,
                     const FeatureIterator model,
                     int length) const throw();

private:
  PiiClassification::DistanceCombinationMode _mode;
  PiiMatrix<int> _matBoundaries;
  QVector<double> _lstWeights;
};

template <class FeatureIterator> PiiMultiFeatureDistance<FeatureIterator>::PiiMultiFeatureDistance() :
	_mode(PiiClassification::DistanceSum)
{
}

template <class FeatureIterator> PiiMultiFeatureDistance<FeatureIterator>::~PiiMultiFeatureDistance()
{
}

template <class FeatureIterator> void PiiMultiFeatureDistance<FeatureIterator>::setCombinationMode(PiiClassification::DistanceCombinationMode mode) { _mode = mode; }
template <class FeatureIterator> PiiClassification::DistanceCombinationMode PiiMultiFeatureDistance<FeatureIterator>::combinationMode() const { return _mode; }
template <class FeatureIterator> void PiiMultiFeatureDistance<FeatureIterator>::setBoundaries(const PiiMatrix<int>& boundaries) { _matBoundaries = boundaries; }
template <class FeatureIterator> PiiMatrix<int> PiiMultiFeatureDistance<FeatureIterator>::boundaries() const { return _matBoundaries; }
template <class FeatureIterator> void PiiMultiFeatureDistance<FeatureIterator>::setWeights(const QVector<double>& weights) { _lstWeights = weights; }
template <class FeatureIterator> QVector<double> PiiMultiFeatureDistance<FeatureIterator>::weights() const { return _lstWeights; }

template <class FeatureIterator> double PiiMultiFeatureDistance<FeatureIterator>::operator() (FeatureIterator sample,
                                                                                              FeatureIterator model,
                                                                                              int length) const throw()
{
  const int iSize = this->size();
  if (iSize == 0)
    return 0;
  if (_matBoundaries.columns() == 0)
    return (*this->at(0))(sample, model, length);

  PiiDistanceMeasure<FeatureIterator>* measure = 0;
  int start = 0, end;
  // Init a matrix that collects all sub-vector distances
  PiiMatrix<double> distances(1, _matBoundaries.columns());
  // Loop through all sub-vectors
  for (int i=0; i<_matBoundaries.columns(); ++i)
    {
      // If we have a distance measure for this sub-vector, we use it. 
      // Otherwise use the last one.
      if (i < iSize)
        measure = this->at(i);
      end = _matBoundaries(0,i);
      if (end > length) break; // Error

      double weight = (i < _lstWeights.size() ? _lstWeights[i] : 1);
        
      distances(0,i) = weight * (*measure)(sample + start, model + start, end-start);
      start = end;
    }

  switch (_mode)
    {
    case PiiClassification::DistanceSum: return Pii::sumAll<double>(distances);
    case PiiClassification::DistanceProduct:
      {
        double product = distances(0,0);
        for (int c=1; c<distances.columns(); c++)
          product *= distances(0,c);
        return product;
      }
    case PiiClassification::DistanceMin: return Pii::minAll(distances);
    case PiiClassification::DistanceMax: return Pii::maxAll(distances);
      
    default: return Pii::sumAll<double>(distances);
    }
  return 0;
}

#endif //_PIIMULTIFEATUREDISTANCE_H
