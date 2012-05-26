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

#ifndef _PIIDISTRIBUTIONNORMALIZER_H
#define _PIIDISTRIBUTIONNORMALIZER_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>

/**
 * An operation that divides incoming feature vectors by their sum. 
 * This allows one to use differently scaled distributions in
 * classifying statistical features.
 *
 * @inputs
 *
 * @in features - a feature vector. Any numeric matrix.
 *
 * @in boundaries - an optional input that marks the boundaries of
 * multiple feature vectors in a compound feature vector. This input
 * is mostly used in conjunction with PiiFeatureCombiner. If this
 * input is connected, each part of the compound feature vector will
 * be separately normalized.
 *
 * @outputs
 *
 * @out features - a normalized feature vector. PiiMatrix<double> or
 * PiiMatrix<float>, depending on the @p doubleMode property. If the
 * sum of the input vector is zero, it will not be normalized.
 *
 * @ingroup PiiClassificationPlugin
 */
class PiiDistributionNormalizer : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * A flag that determines the format of output vectors. The
   * operation always outputs floating-point vectors. By default, the
   * values are floats. If this flag is true, double accuracy is used.
   */
  Q_PROPERTY(bool doubleMode READ doubleMode WRITE setDoubleMode);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiDistributionNormalizer();

  bool doubleMode() const;
  void setDoubleMode(bool mode);

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    bool bDoubleMode;

    PiiInputSocket* pFeatureInput, *pBoundaryInput;
    PiiOutputSocket* pFeatureOutput;
  };
  PII_D_FUNC;
  
  template <class T> void normalize(const PiiVariant& obj);
  template <class U, class T> void normalizePieces(const PiiMatrix<T>& vector, const PiiMatrix<int>& boundaries)
;
};

#endif //_PIIDISTRIBUTIONNORMALIZER_H
