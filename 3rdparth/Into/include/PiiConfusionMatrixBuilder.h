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

#ifndef _PIICONFUSIONMATRIXBUILDER_H
#define _PIICONFUSIONMATRIXBUILDER_H

#include <PiiDefaultOperation.h>
#include "PiiConfusionMatrix.h"

/**
 * An operation that builds a @ref PiiConfusionMatrix "confusion matrix"
 * out of classification results. Optionally, it may also
 * build a matrix of mean distances to the closest training sample, if
 * the classifier provides such information.
 *
 * @inputs
 *
 * @in real class - the real (known) class index for a sample. Any
 * primitive type that will be converted to an @p int.
 *
 * @in classification - the result of classification. Any primitive
 * type that will be converted to an @p int.
 *
 * @in distance - an optional input that collects the distances to the
 * closest training sample.
 * 
 * @outputs
 *
 * @out confusion matrix - The current confusion matrix. 
 * (PiiMatrix<int>)
 *
 * @out distance matrix - The current distance matrix. 
 * (PiiMatrix<double>) See the #distanceMatrix property for
 * explanation.
 *
 * @ingroup PiiClassificationPlugin
 */
class PiiConfusionMatrixBuilder : public PiiDefaultOperation
{
  Q_OBJECT
  /**
   * The collected confusion matrix as a PiiVariant. The variant holds
   * a PiiMatrix<int>. See PiiConfusionMatrix for a thorough
   * explanation.
   */
  Q_PROPERTY(PiiVariant confusionMatrix READ confusionMatrix WRITE setConfusionMatrix);

  /**
   * Distance matrix holds average distances between classified
   * samples and their closest training samples. The structure is
   * analogous to a confusion matrix, but instead of the number of
   * occurrences, it holds the average distances. The sum of the
   * matrix divided by the sum of the confusion matrix (the number of
   * samples) is a measure of average quantization error. The distance
   * matrix is stored as a PiiMatrix<double>.
   */
  Q_PROPERTY(PiiVariant distanceMatrix READ distanceMatrix WRITE setDistanceMatrix);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiConfusionMatrixBuilder();

  void setDistanceMatrix(const PiiVariant& distanceMatrix);
  PiiVariant distanceMatrix() const;
  void setConfusionMatrix(const PiiVariant& confusionMatrix);
  PiiVariant confusionMatrix() const;

  /**
   * Returns the confusion matrix as a text string that can be printed
   * on console for illustration. See PiiConfusionMatrix::print().
   */
  Q_INVOKABLE QString matrixAsText(const QStringList& classNames = QStringList(),
                                   int space = 4) const;

protected:
  void process();
  void check(int reset);

private:
  void addDistance(int correctClass, int classification, double distance);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiConfusionMatrix confusionMatrix;
    PiiMatrix<double> distanceMatrix;
  };
  PII_D_FUNC;
};

#endif //_PIICONFUSIONMATRIXBUILDER_H
