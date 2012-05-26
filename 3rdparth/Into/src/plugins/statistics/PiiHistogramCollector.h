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

#ifndef _PIIHISTOGRAMCOLLECTOR_H
#define _PIIHISTOGRAMCOLLECTOR_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>

/**
 * An operation that collects a histogram of any numeric data.
 *
 * @inputs
 *
 * @in sync - an optional sync input. If this input is connected,
 * %PiiHistogramCollector collects data until this and the @p data input
 * are in sync. The histogram will be emitted once for each object
 * read from this input. If this input is not connected, the histogram
 * will be emitted for each object read from @p data.
 *
 * @in data - the data. Any number or a numeric matrix. If a matrix is
 * received, each element in the matrix will be added to the
 * histogram. If a scalar is received, it will be added to the
 * histogram. The data will be converted to integers before adding to
 * the histogram.
 *
 * @outputs
 *
 * @out sync - the object received in the @p sync input. If the @p
 * sync input is connected, the operation stores the object received
 * until the inputs are synchronized. It will then send the stored
 * object to this output before it sends the histogram.
 *
 * @out y - in @p FixedLengthOutput mode: the histogram as a
 * 1-by-#binCount PiiMatrix<int>. Zero bins are present in the
 * histogram. Any value outside of [0, #binCount-1] will be ignored. 
 * In the output, the element at (0,0) is the number of zeros
 * collected, the element at (0,1) the number of ones etc. In @p
 * VariableLengthOutput mode: bins with zero entries will not be
 * present in the output. The output value is a 1-by-N PiiMatrix<int>,
 * where N is the number of non-zero histogram bins. In this mode, the
 * operation places no restrictions on the range of input values.
 *
 * @out x - the x coordinates of the histogram bins. A 1-by-N
 * PiiMatrix<int>. In @p FixedLengthOutput mode this will always be
 * the same: (0, 1, 2, ..., #binCount-1). In @p VariableLengthOutput
 * mode the size of the matrix will be equal to that of @p y. The
 * coordinates will always be in ascending order.
 *
 * @ingroup PiiStatisticsPlugin
 */
class PiiHistogramCollector : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The number of histogram bins. This property must be set to a
   * non-zero value in @p FixedLengthOutput mode. In @p
   * VariableLengthOutput mode, this value works as an initial guess
   * to the number of distinct histogram bins. A good guess
   * (over-estimate) increases performance. The default value is 256.
   */
  Q_PROPERTY(int binCount READ binCount WRITE setBinCount);
   
  /**
   * The output mode. Default is @p FixedLengthOutput.
   */
  Q_PROPERTY(OutputMode outputMode READ outputMode WRITE setOutputMode);
  Q_ENUMS(OutputMode);

  /**
   * Output normalization. If set to @p false (the default), the
   * output will be a PiiMatrix<int> in which each column represents
   * the number of times the index appeared in input. If @p true,
   * output histogram will be a PiiMatrix<float> that is normalized so
   * that it sums up to unity .
   */
  Q_PROPERTY(bool normalized READ normalized WRITE setNormalized);
   
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  /**
   * Output modes.
   *
   * @lip FixedLengthOutput - the size of the output histogram is
   * determined by #binCount. @p x output will always be the same.
   *
   * @lip VariableLengthOutput - only non-zero bins will be output. 
   * The size of the output varies, and @p x records the indices of
   * non-zero bins.
   */
  enum OutputMode { FixedLengthOutput, VariableLengthOutput };
  
  PiiHistogramCollector();


  void check(bool reset);
  
protected:
  void process();
  void syncEvent(SyncEvent* event);

  void setBinCount(int binCount);
  int binCount() const;
  void setOutputMode(const OutputMode& outputMode);
  OutputMode outputMode() const;
  void setNormalized(bool normalized);
  bool normalized() const;

private:
  void addToHistogram(int element);
  template <class T> void addPrimitive(const PiiVariant& obj);
  template <class T> void addMatrix(const PiiVariant& obj);
  void emitHistogram();
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    int iBinCount;
    bool bSyncConnected;
    PiiMatrix<int> matHistogram;
    PiiMatrix<int> matX;
    OutputMode outputMode;
    PiiVariant pSyncObject;
    bool bNormalized;
  };
  PII_D_FUNC;
};


#endif //_PIIHISTOGRAMCOLLECTOR_H
