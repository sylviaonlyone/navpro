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

#ifndef _PIICLASSIFIER_H
#define _PIICLASSIFIER_H

#include "PiiClassificationGlobal.h"
#include "PiiSampleSet.h"

#include <PiiSerialization.h>
#include <PiiVirtualMetaObject.h>

/**
 * An interface for classification and regression algorithms. A
 * classifier is an algorithm that maps an N-dimensional feature
 * vector @e X into to a set of discrete class labels @f$c \in
 * {0,M-1}@f$. In regression, the algorithm maps @e X to a continuous
 * value @f$c \in [0,1]@f$.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class SampleSet> class PiiClassifier
{
  friend struct PiiSerialization::Accessor;
  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION;
  template <class Archive> void serialize(Archive&, const unsigned int) {}
  
public:
  virtual ~PiiClassifier() {}

  /**
   * Classify the given feature vector. The algorithm may vary from a
   * simple nearest neighbor rule to support vector regression. The
   * only requirement is that the classifier yields a class index or a
   * continuous regression as a result. Errors are indicated by
   * returning a NaN.
   *
   * @param featureVector the feature vector of the sample to be
   * classified
   *
   * @return a zero-based class index in classification, or a value in
   * [0,1] in regression. NaN means that the sample could not be
   * classified.
   */
  virtual double classify(typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator featureVector) throw() = 0;
};

PII_SERIALIZATION_ABSTRACT_TEMPLATE(PiiClassifier);
PII_SERIALIZATION_VIRTUAL_METAOBJECT_TEMPLATE(PiiClassifier);
PII_CLASSIFICATION_NAME_ALIAS_AS_T(PiiClassifier, PiiMatrix<T>);
PII_DEFINE_VIRTUAL_METAOBJECT_FUNCTION_TEMPLATE(PiiClassifier);

#endif //_PIICLASSIFIER_H
