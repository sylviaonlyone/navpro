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

#ifndef _PIISAMPLESET_H
#define _PIISAMPLESET_H

#include <PiiMatrix.h>

/**
 * Contains functions and definitions for accessing sample sets in an
 * abstract way. Into uses PiiMatrix as the default sample set type,
 * but most learning and classification algorithms are written so that
 * they don't expect a specific sample set type. Instead, functions in
 * this namespace are used to access the sample set. If you want to
 * use a different type to hold your sample sets, you need to create
 * an overloaded version for each function in this namespace and
 * specialize the PiiSampleSet::Traits structure.
 *
 * @ingroup PiiClassificationPlugin
 */
namespace PiiSampleSet
{
  /**
   * Defines the traits of a specific type when used as a sample set.
   * There is no default implementation. You need to specialize this
   * structure if you create a new sample set type.
   */
  template <class T> struct Traits;

  /**
   * Defines the traits of any PiiMatrix when used as a sample set.
   */
  template <class T> struct Traits<PiiMatrix<T> >
  {
    typedef PiiMatrix<T> Type;
    typedef T FeatureType;
    typedef T* FeatureIterator;
    typedef const T* ConstFeatureIterator;

    static PiiMatrix<T> create(int sampleCount, int featureCount) { return PiiMatrix<T>(sampleCount, featureCount); }
  };

  /**
   * Creates a new sample set with @a sampleCount samples and @a
   * featureCount features.
   */
  template <class SampleSet> inline SampleSet create(int sampleCount, int featureCount)
  {
    return Traits<SampleSet>::create(sampleCount, featureCount);
  }

  /**
   * Returns the number of samples in a sample set.
   */
  template <class T> inline int sampleCount(const PiiMatrix<T>& samples) { return samples.rows(); }

  /**
   * Returns the number of features in a sample set.
   */
  template <class T> inline int featureCount(const PiiMatrix<T>& samples) { return samples.columns(); }
  /**
   * Resizes a sample set to hold @a sampleCount samples with @a
   * featureCount features. If @a featureCount is -1, the number of
   * features will not be changed.
   */
  template <class T> inline void resize(PiiMatrix<T>& samples, int sampleCount, int featureCount = -1)
  {
    if (featureCount == -1) featureCount = samples.columns();
    samples.resize(sampleCount, featureCount);
  }
  /**
   * Reserves space for @a sampleCount samples with @a featureCount
   * features. If @a featureCount is -1, the number of features will
   * not be changed.
   */
  template <class T> inline void reserve(PiiMatrix<T>& samples, int sampleCount, int featureCount = -1)
  {
    if (featureCount != -1 && samples.columns() != featureCount)
      samples.resize(0, featureCount);
    samples.reserve(sampleCount);
  }

  /**
   * Clears a sample set.
   */
  template <class T> inline void clear(PiiMatrix<T>& samples) { samples.clear(); }
  /**
   * Returns the number of samples a sample set can hold without
   * reallocation.
   */
  template <class T> inline int capacity(const PiiMatrix<T>& samples) { return samples.capacity(); }
  /**
   * Returns the sample at @a index.
   */
  template <class T> inline const T* sampleAt(const PiiMatrix<T>& samples, int index) { return samples[index]; }
  /// @overload
  template <class T> inline T* sampleAt(PiiMatrix<T>& samples, int index) { return samples[index]; }

  /**
   * Replaces the feature vector at @a index in @a samples with @a
   * features.
   */
  template <class T> inline void setSampleAt(PiiMatrix<T>& samples, int index, const T* features)
  {
    memcpy(samples[index], features, sizeof(T) * samples.columns());
  }

  /**
   * Adds @a sample to the and of a sample set.
   */
  template <class T> inline void append(PiiMatrix<T>& samples, const T* sample)
  {
    // Allocate space in at most 64 sample blocks.
    if (samples.capacity() == samples.rows())
      samples.reserve(qBound(1, samples.rows()*2, samples.rows()+64));
    samples.appendRow(sample);
  }

  /**
   * Removes the sample at @a index.
   */
  template <class T> inline void remove(PiiMatrix<T>& samples, int index)
  {
    samples.removeRow(index);
  }
  
  /**
   * Returns @p true if @a set1 is equal to @a set2, and @p false
   * otherwise.
   */
  template <class T> inline bool equals(PiiMatrix<T>& set1, PiiMatrix<T>& set2)
  {
    return Pii::equals(set1, set2);
  }
}

#endif //_PIISAMPLESET_H
