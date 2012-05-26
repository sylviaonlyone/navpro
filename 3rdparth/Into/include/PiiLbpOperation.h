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

#ifndef _PIILBPOPERATION_H
#define _PIILBPOPERATION_H

#include <PiiDefaultOperation.h>
#include <QVector>
#include "PiiLbp.h"

/**
 * The LBP texture feature.
 *
 * @inputs
 *
 * @in image - an image the LBP texture feature is extracted from. 
 * The image may be of any type. Color images are automatically
 * converted to gray scale before processing.
 *
 * @in roi - region-of-interest. See @ref PiiImagePlugin for a
 * description. Optional.
 *
 * @outputs
 *
 * @out features - all extracted features as a concatenated feature
 * vector. In @p ImageOutput mode, this output will emit the same
 * feature image as @p features0.
 *
 * @out featuresX - a feature vector that contains the extracted LBP
 * features or a feature image that contains the LBP feature code for
 * each pixel, depending on #outputType. X stands for the Xth
 * parameter set (zero-based index). The size of the output image is
 * smaller than the size of the input image. By default, one output is
 * created and named features0. It outputs the 256-bin feature vector
 * for LBP 8,1 in Standard mode with nearest neighbor interpolation.
 *
 * @ingroup PiiTexturePlugin
 */
class PiiLbpOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * A list of parameters used in extracting the LBP features. If
   * multiple parameters are given, multiple feature vectors are
   * extracted for each input image. The parameters are represented
   * with
   * <tt>samples,radius[,threshold][,mode][,interpolation][,smoothing]</tt>. 
   * Samples is the number of samples taken around each pixel (int),
   * radius is the sampling radius (float), and threshold a "noise
   * canceller threshold" (float). Valid values for mode are @p
   * Uniform, @p RotationInvariant, @p UniformRotationInvariant, and
   * @p Symmetric. Interpolation can be either @p LinearInterpolation
   * or @p NearestNeighborInterpolation. Smoothing can be either @p
   * Smoothed or @p NonSmoothed. Everything but samples and radius can
   * be omitted. Default values are "Standard",
   * "NearestNeighborInterpolation", and "NonSmoothed". Threshold,
   * mode, interpolation, and smoothing can be given in any order.
   *
   * @par Notes
   *
   * @li If mode is "Symmetric", a "symmetric" LBP is used instead of
   * the traditional one. The symmetric version compares opposing
   * pairs of pixels in a neighborhood and doesn't consider the
   * center. See PiiLbp::genericSymmetricLbp() for details. The amount
   * of samples for the symmetric LBP must be even. If an odd number
   * is given, samples-1 will be used. The symmetric version cannot
   * make use of the threshold parameter.
   *
   * @li If smoothing is set to "Smoothed", the input image will be
   * smoothed prior to applying the LBP operation. The smoothing is
   * performed with Pii::fastMovingAverage() using the distance
   * between neighborhood samples as the size of the smoothing window.
   * Formally, the window size is calculated as $f@s = \ceil{2 \pi R /
   * N}@f$, where @e N is the number of samples and @e N is the
   * neighborhood radius. Multi-scale LBP with smoothing can be seen
   * as a pyramid description of image texture.
   *
   * @li The standard LBP, albeit being largely invariant against
   * illumination changes, is quite sensitive to noise on uniformly
   * colored surfaces. LBP's noise tolerance can be increased by
   * adding a constant value ("noise canceller threshold") to each
   * pixel before comparing it to the neighbors. This way, small
   * variations don't get captured. The downside is that one needs to
   * select a suitable threshold value, which is yet another parameter
   * to decide. Furthermore, the threshold makes LBP more dependent on
   * illumination changes.
   *
   * Examples of valid parameters:
   *
   * @li 8,1
   * @li 8,1,RotationInvariant
   * @li 16,2,4,UniformRotationInvariant,LinearInterpolation
   * @li 12,1.5,Uniform,Smoothed
   * @li 8,1,LinearInterpolation
   * @li 10,1.7,LinearInterpolation,Symmetric
   *
   * @code
   * lbpOperation.setProperty("parameters", QStringList() << "8,1" << "16,2,Uniform");
   * @endcode
   */
  Q_PROPERTY(QStringList parameters READ parameters WRITE setParameters);

  /**
   * A type that determines whether histograms or feature images will
   * be produced. The default value is @p HistogramOutput.
   */
  Q_PROPERTY(OutputType outputType READ outputType WRITE setOutputType);
  Q_ENUMS(OutputType);

  /**
   * The type or the @p roi input, if connected. The default value is
   * @p AutoRoi.
   */
  Q_PROPERTY(PiiImage::RoiType roiType READ roiType WRITE setRoiType);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:

  /**
   * Output types.
   *
   * @lip HistogramOutput - LBP codes are collected to a histogram and
   * output as a row vector (PiiMatrix<int>). In @p HistogramOutput
   * mode, a separate histogram operation is not needed.
   *
   * @lip NormalizedHistogramOutput - same as @p HistogramOutput, but
   * the histogram is normalized to sum up to unity. The output type
   * will be PiiMatrix<float>.
   *
   * @lip ImageOutput - the output will be an image (PiiMatrix<int>)
   * in which each pixel is replaced with its LBP code.
   */
  enum OutputType { HistogramOutput, NormalizedHistogramOutput, ImageOutput };

  /**
   * Create a new LBP operation.
   */
  PiiLbpOperation();
  ~PiiLbpOperation();

  QStringList parameters() const;
  void setParameters(const QStringList& parameters);

  OutputType outputType() const;
  void setOutputType(OutputType type);
  
  void setRoiType(PiiImage::RoiType roiType);
  PiiImage::RoiType roiType() const;

  void check(bool reset);

protected:
  void process();
  void aboutToChangeState(State state);

private:
  class AnyLbp;
  template <class T, class LbpType> class Lbp;
  friend class AnyLbp;
  template <class T, class LbpType> friend class Lbp;
  
  void createOutput(int samples, double radius, PiiLbp::Mode mode = PiiLbp::Standard,
                    Pii::Interpolation interpolation = Pii::NearestNeighborInterpolation,
                    bool smoothed = false, double threshold = 0);
  template <class T> inline void calculate(const PiiVariant& obj);
  template <class LbpType, class T> void calculate(const PiiMatrix<T>& image);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    ~Data();
    OutputType outputType;
    
    QStringList lstParameters;
    QList<PiiLbp*> lstOperators;
    QList<int> lstSmoothingWindows;
    QList<double> lstThresholds;
    bool bMustSmooth;
    
    QVector<bool> vecMustCalculate;
    bool bCompoundConnected;
    int iStaticOutputCount;
    
    PiiInputSocket* pRoiInput;
    PiiImage::RoiType roiType;
    AnyLbp* pLbp;
    unsigned int uiPreviousType;
  };
  PII_D_FUNC;
};


#endif //_PIILBPOPERATION_H
