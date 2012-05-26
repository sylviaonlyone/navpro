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

#ifndef _PIIPEAKDETECTOR_H
#define _PIIPEAKDETECTOR_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>

/**
 * Detect peaks in noisy data. See PiiDsp::findPeaks() for a detailed
 * description of function.
 *
 * @inputs
 *
 * @in signal - input signal. A row vector. (real-valued PiiMatrix)
 * 
 * @outputs
 *
 * @out peaks - detected peaks. A (possibly empty) PiiMatrix<double>
 * with one row for each peak. Each row has three entries, in this
 * order: position, height, width. See PiiDsp::Peak for details.
 *
 * @out indices - zero-based indices of detected peaks in the original
 * signal. A N-by-1 PiiMatrix<int>.
 *
 * @ingroup PiiDspPlugin
 */
class PiiPeakDetector : public PiiDefaultOperation
{
  Q_OBJECT
  /**
   * Mimimum value for a measurement to be regarded as a peak. The
   * default value is 0.
   */
  Q_PROPERTY(double levelThreshold READ levelThreshold WRITE setLevelThreshold);
  /**
   * Minimum relative value of the second derivative that will trigger
   * peak detection. The default value is 0.001.
   */
  Q_PROPERTY(double sharpnessThreshold READ sharpnessThreshold WRITE setSharpnessThreshold);
  /**
   * Width of a smoothing window that will be applied to the
   * derivative before peak detection. The default value is 5.
   */
  Q_PROPERTY(int smoothWidth READ smoothWidth WRITE setSmoothWidth);
  /**
   * Number of measurements considered in fitting a local parabola
   * for accurate peak position. The default value is 7.
   */
  Q_PROPERTY(int windowWidth READ windowWidth WRITE setWindowWidth);
  /**
   * The width of a moving average window used for compensation
   * against level changes. If set to a value larger than one, the
   * input signal will be divided by its moving average, calculated
   * over this many elements. The default value is 0.
   */
  Q_PROPERTY(int levelCorrectionWindow READ levelCorrectionWindow WRITE setLevelCorrectionWindow);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiPeakDetector();

  void setLevelThreshold(double levelThreshold);
  double levelThreshold() const;
  void setSharpnessThreshold(double sharpnessThreshold);
  double sharpnessThreshold() const;
  void setSmoothWidth(int smoothWidth);
  int smoothWidth() const;
  void setWindowWidth(int windowWidth);
  int windowWidth() const;
  void setLevelCorrectionWindow(int levelCorrectionWindow);
  int levelCorrectionWindow() const;

protected:
  void process();

private:
  template <class T> void findPeaks(const PiiVariant& obj);
  template <class T> PiiMatrix<double> adjustLevel(const PiiMatrix<T>& matrix);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    double dLevelThreshold;
    double dSharpnessThreshold;
    int iSmoothWidth;
    int iWindowWidth;
    int iLevelCorrectionWindow;
  };
  PII_D_FUNC;
};


#endif //_PIIPEAKDETECTOR_H
