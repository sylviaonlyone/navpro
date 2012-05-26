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

#ifndef _PIIFEATURERANGELIMITER_H
#define _PIIFEATURERANGELIMITER_H


#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>
#include <QVector>
#include <QPair>


/**
 * Operation cuts features from feature-vectors by using the first one
 * as a reference. Each input is a matrix in which each row represents
 * the features calculated for an object. The first input must be a
 * N-by-1 matrix. Each value in this matrix is evaluated, and if the
 * value is not within the allowed range, the row will be removed. At
 * the same time, the corresponding rows of all other input matrices
 * are removed as well. The truncated matrices are sent to the output.
 *
 * @inputs
 *
 * @in featureX - a feature vectors. X varies from 0 to N-1, where N
 * is the number of different feature vectors. Each input must contain
 * a matrix with the same amount of rows.
 *
 * @outputs
 *
 * @out count - the number of selected features
 *
 * @out featureX - the limited feature matrices.
 *
 * In the following example it is assumed that the acceptance range is
 * set to 2-3 and that we have two inputs.
 *
 * @code
 * // First input:
 * PiiMatrix<int> feature0(3,1, 1, 2, 4); // three rows
 * // Second input:
 * PiiMatrix<float> feature1(3,2,
 *                           0.1, 0.2,
 *                           1.3, 1.4,
 *                           2.5, 3.6);
 *
 * // First output:
 * PiiMatrix<int> feature0(1,1, 2); // only one row
 * // Second output:
 * PiiMatrix<float> feature1(1,2, 1.3, 1.4); // ditto
 * @endcode
 *
 * @ingroup PiiImagePlugin
 */
class PiiFeatureRangeLimiter : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The number of inputs. Must be greater than zero.
   */
  Q_PROPERTY(int dynamicInputCount READ dynamicInputCount WRITE setDynamicInputCount);
  
  /**
   * Lower limit of the acceptance range.
   */
  Q_PROPERTY(double lowerLimit READ lowerLimit WRITE setLowerLimit);
  /**
   * Upper limit of the acceptance range.
   */
  Q_PROPERTY(double upperLimit READ upperLimit WRITE setUpperLimit);
  
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  PiiFeatureRangeLimiter();

  double lowerLimit() const;
  void setLowerLimit(double lowerLimit);

  double upperLimit() const;
  void setUpperLimit(double upperLimit);

  void setDynamicInputCount(int cnt);
  int dynamicInputCount() const;
  
protected:
  void process();
  
private:
  template <class T> void calculateLimiterMatrix(const PiiVariant& obj);
  template <class T> void operateMatrix(const PiiVariant& obj, int outputIndex);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dLowerLimit, dUpperLimit;
    bool isLowerSelected, isUpperSelected;
    int iRows;
    QVector<int> lstRows;
  };
  PII_D_FUNC;
  
};


#endif //_PIIFEATURERANGELIMITER_H
