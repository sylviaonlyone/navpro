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

#ifndef _PIIMOVINGAVERAGEOPERATION_H
#define _PIIMOVINGAVERAGEOPERATION_H

#include <PiiDefaultOperation.h>
#include <QLinkedList>

/**
 * Calculate the moving average over a window of a predefined size. 
 * The output will be the average over the last N input values, which
 * must be of the same type (and size, if matrices are averaged).
 *
 * @inputs
 *
 * @in input - the input value. Any numeric or complex type or a
 * matrix containing such types.
 * 
 * @outputs
 *
 * @out average - the average over the last N entries (see
 * #windowSize). The output type is floating point numbers. Double and
 * long int in input result in double output, others result in float
 * output.
 *
 * @ingroup PiiBasePlugin
 */
class PiiMovingAverageOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The size of the averaging window. Note that the operation buffers
   * this many past values. If large matrices are averaged, a large
   * portion of memory may be reserved. The default value is two (2).
   */
  Q_PROPERTY(int windowSize READ windowSize WRITE setWindowSize);
  /**
   * The minimum acceptable value. If @p rangeMin and #rangeMax are
   * not equal, they define the range of acceptable values, and the
   * values are handled circularly. This is especially useful for
   * angles, because the mean of 359 and 1 should be 0, not 180. The
   * default value is 0. The range applies only to scalars. Matrices
   * are not checked for range under/overflow.
   */
  Q_PROPERTY(double rangeMin READ rangeMin WRITE setRangeMin);  
  /**
   * The maximum acceptable value. See #rangeMin for a detailed
   * description. The default value is 0. Use 360 or 2*M_PI for
   * angles.
   */
  Q_PROPERTY(double rangeMax READ rangeMax WRITE setRangeMax);

  /**
   * If this property is true, output type will be forced to input type.
   */
  Q_PROPERTY(bool forceInputType READ forceInputType WRITE setForceInputType);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiMovingAverageOperation();

  void setWindowSize(int windowSize);
  int windowSize() const;
  void setRangeMin(double rangeMin);
  double rangeMin() const;
  void setRangeMax(double rangeMax);
  double rangeMax() const;
  void setForceInputType(bool forceInputType);
  bool forceInputType() const;

protected:
  void process();
  void check(bool reset);

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    int iWindowSize;
    double dRangeMin;
    double dRangeMax;
    double dRange;
    unsigned int uiType;
    QLinkedList<PiiVariant> lstBuffer;
    bool bForceInputType;
  };
  PII_D_FUNC;

  template <class T> void average(const PiiVariant& obj);
  template <class T> void matrixAverage(const PiiVariant& obj);
  template <class T, class ResultType> void averageTemplate(const PiiVariant& obj);
  template <class T> void addImpl(T& op1, T op2, int index);
  template <class T> void scaleImpl(T& result, int cnt);
  template <class T> void add(T& op1, const T& op2, int index);
  template <class T> void scale(T& result, int cnt);
};


#endif //_PIIMOVINGAVERAGEOPERATION_H
