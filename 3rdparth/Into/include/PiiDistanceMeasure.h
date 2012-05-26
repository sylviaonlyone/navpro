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

#ifndef _PIIDISTANCEMEASURE_H
#define _PIIDISTANCEMEASURE_H

#include "PiiClassificationGlobal.h"

#include <PiiMath.h>
#include <PiiSerializationTraits.h>


/// @internal
#define PII_DEFAULT_DISTANCE_MEASURE_DEF(NAME) \
template <class FeatureIterator> class NAME \
{ \
public: \
  inline double operator() (FeatureIterator sample, FeatureIterator model, int length) const throw(); \
}; \
template <class FeatureIterator> double NAME<FeatureIterator>::operator() (FeatureIterator sample, \
                                                                           FeatureIterator model, \
                                                                           int length) const throw()

/**
 * @file
 * @ingroup PiiClassificationPlugin
 *
 * Definitions for distance measure classes.
 */

/**
 * Type definition for a polymorphic implementation of the function
 * object @a MEASURE.
 *
 * @code
 * PiiDistanceMeasure<ConstFeatureIterator>* pMeasure = new PII_POLYMORPHIC_MEASURE(PiiHistogramIntersection);
 * @endcode
 */
#define PII_POLYMORPHIC_MEASURE(MEASURE) typename PiiDistanceMeasure<ConstFeatureIterator>::template Impl<MEASURE<ConstFeatureIterator> >

/**
 * A polymorphic implementation of the @ref
 * classification_distance_measures "distance measure" concept.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class FeatureIterator> class PiiDistanceMeasure
{
public:
  virtual ~PiiDistanceMeasure();

  /**
   * Measure the distance between two vectors. The order of parameters
   * is significant because a distance measure can be asymmetric.
   *
   * @param sample a sample feature vector
   * vector
   *
   * @param model a model feature vector
   *
   * @param length the number of features (dimensions) to consider
   *
   * @return the distance between the vectors, or NaN if the distance
   * could not be calculated
   */
  virtual double operator() (FeatureIterator sample,
                             FeatureIterator model,
                             int length) const throw() = 0;
  

  virtual PiiDistanceMeasure* clone() const = 0;
  
  template <class Measure> class Impl;

protected:
	PiiDistanceMeasure();
  
  PII_DISABLE_COPY(PiiDistanceMeasure);
};

PII_CLASSIFICATION_NAME_ALIAS_AS_T(PiiDistanceMeasure, const T*);

template <class FeatureIterator> PiiDistanceMeasure<FeatureIterator>::PiiDistanceMeasure()
{
}

template <class FeatureIterator> PiiDistanceMeasure<FeatureIterator>::~PiiDistanceMeasure()
{
}

/**
 * A template that implements the PiiDistanceMeasure interface by
 * using @p Measure as the distance measure implementation. The
 * virtual measure() function just passes the call to the given @p
 * Measure class.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class FeatureIterator> template <class Measure>
class PiiDistanceMeasure<FeatureIterator>::Impl :
  public PiiDistanceMeasure<FeatureIterator>,
  public Measure
{
public:
  double operator() (FeatureIterator sample,
                     FeatureIterator model,
                     int length) const throw()
  {
    return Measure::operator() (sample, model, length);
  }
  
  Impl* clone() const
  {
    return new Impl;
  }
};


#endif //_PIIDISTANCEMEASURE_H
