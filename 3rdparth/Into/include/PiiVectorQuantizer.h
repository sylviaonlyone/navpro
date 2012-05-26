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

#ifndef _PIIVECTORQUANTIZER_H
#define _PIIVECTORQUANTIZER_H

#include <Pii.h>
#include "PiiDistanceMeasure.h"
#include "PiiClassifier.h"

/**
 * A vector quantizer. Vector quantization is perhaps the most
 * primitive way of performing classification. A vector quantizer
 * holds a number of model vectors (a.k.a. the code book) to which it
 * compares any unknown sample. As a classification result, the index
 * of the closest vector is returned. The "closeness" is defined in
 * terms of a @ref classification_distance_measures "distance
 * measure".
 *
 * @ingroup PiiClassificationPlugin
 */
template <class SampleSet> class PiiVectorQuantizer :
  public PiiClassifier<SampleSet>
{
public:
  typedef typename PiiSampleSet::Traits<SampleSet>::FeatureIterator FeatureIterator;
  typedef typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator ConstFeatureIterator;

  /**
   * Create a vector quantizer with PiiSquaredGeometricDistance as the
   * distance measure.
   */
  PiiVectorQuantizer();
  
  /**
   * Create a new vector quantizer with the given distance measure.
   *
   * @param measure the way of measuring distances between vectors. 
   * PiiVectorQuantizer takes the ownership of the pointer. The
   * distance measure must be set before calling #classify().
   */
  PiiVectorQuantizer(PiiDistanceMeasure<ConstFeatureIterator>* measure);
  
  /**
   * Destroy the vector quantizer. Also deletes the distance measure.
   */
  ~PiiVectorQuantizer();
  
  /**
   * Returns the index of the closest vector in the model set. If two
   * vectors are equally close, the one first in the model set will
   * returned. If the feature vector cannot be classified, @p NaN will
   * be returned. Calls #findClosestMatch() by default.
   */
  double classify(ConstFeatureIterator featureVector) throw();

  /**
   * Analogous to #classify(), but also returns the distance to the
   * closest neighbor.
   */
  virtual int findClosestMatch(ConstFeatureIterator featureVector, double* distance) const throw();

  /**
   * Set a distance threshold for rejecting samples. If the distance to
   * the closest code vector is above this threshold, the sample is
   * rejected and #classify(const PiiMatrix<T>&) returns -1.
   */
  void setRejectThreshold(double rejectThreshold);

  /**
   * Get the reject threshold.
   */
  double rejectThreshold() const;
  
  /**
   * Returns the distance measure.
   */
  PiiDistanceMeasure<ConstFeatureIterator>* distanceMeasure() const;

  /**
   * Sets the distance measure. This class takes the ownership of @a
   * measure. The old distance measure will be deleted.
   */
  void setDistanceMeasure(PiiDistanceMeasure<ConstFeatureIterator>* measure);

  /**
   * Returns a modifiable reference to the model set.
   */
  SampleSet& models();
  
  /**
   * Returns the model sample set.
   */
  SampleSet models() const;

  /**
   * Set the model sample set.
   */
  void setModels(const SampleSet& models);

  /**
   * Returns the number of model vectors in the model sample set.
   */
  int modelCount() const;

  /**
   * Returns the number of dimensions in the model samples.
   */
  int featureCount() const;

  /**
   * Returns a modifiable iterator to the beginning of the model
   * sample at @a index.
   */
  FeatureIterator modelAt(int index);
  /**
   * Returns a const iterator to the beginning of the model sample at
   * @a index.
   */
  ConstFeatureIterator modelAt(int index) const;
  
protected:
  /// @internal
  class Data
  {
  public:
    Data();
    Data(PiiDistanceMeasure<ConstFeatureIterator>* measure);
    SampleSet modelSet;
    PiiDistanceMeasure<ConstFeatureIterator>* pMeasure;
    double dRejectThreshold;
  } *d;

  /// @internal
  PiiVectorQuantizer(Data* d);
  PII_DISABLE_COPY(PiiVectorQuantizer);
};


#include "PiiVectorQuantizer-templates.h"

#endif //_PIIVECTORQUANTIZER_H
