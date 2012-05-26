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

#ifndef _PIIEDGEDETECTOR_H
#define _PIIEDGEDETECTOR_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>
#include <PiiMatrixUtil.h>

/**
 * Detect edges on images. The operation works by estimating the
 * strength of local gradient with user-selectable masks. The output
 * is thresholded to produce a binary image.
 *
 * @inputs
 *
 * @in image - any gray-level image.
 * 
 * @outputs
 *
 * @out edges - detected edges as a binary image. The type of the
 * output is the same as the input, but the image only contains zeros
 * and ones.
 *
 * @out magnitude - non-thresholded gradient image. If the input is
 * integer-valued, a PiiMatrix<int> will be emitted. Otherwise,
 * PiiMatrix<float> will be emitted. See @ref
 * PiiImage::gradientMagnitude().
 *
 * @out direction - a PiiMatrix<float> that contains local gradient
 * direction for each pixel in radians. See @ref
 * PiiImage::gradientDirection().
 *
 * @ingroup PiiImagePlugin
 */
class PiiEdgeDetector : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Edge detection method. Except for the Canny detector, the only
   * difference between the detection methods is in the gradient
   * estimation filters. The default is @p CannyDetector.
   */
  Q_PROPERTY(Detector detector READ detector WRITE setDetector);
  Q_ENUMS(Detector);

  /**
   * The value with which the gradient magnitude is thresholded. Set
   * to zero to make the operation find a suitable threshold
   * automatically. A "suitable" value is found by adding two times
   * the standard deviation of the gradient magnitude to the mean
   * gradient magnitude. Default is zero.
   */
  Q_PROPERTY(double threshold READ threshold WRITE setThreshold);
  /**
   * Used in hysteresis thresholding with the Canny edge detector. If
   * set to zero, the low threshold will be 0.4 * #threshold. The
   * default is zero.
   */
  Q_PROPERTY(double lowThreshold READ lowThreshold WRITE setLowThreshold);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Supported edge detectors.
   *
   * @lip SobelDetector - Sobel masks are used to calculate gradient.
   *
   * @lip RobertsDetector - Roberts' masks are used to calculate
   * gradient.
   *
   * @lip PrewittDetector - Prewitt's masks are used to calculate
   * gradient.
   *
   * @lip CannyDetector - The three-step procedure is used to detect
   * edges: 1) Sobel masks are used to measure the gradient 2) the
   * image is processed to contain only local maxima (@ref
   * PiiImage::suppressNonMaxima()) 3) hysteresis thresholding is
   * performed (@ref PiiImage::hysteresisThreshold()). This technique
   * requires two thresholds (#lowThreshold and #threshold).
   *
   * @note The original edge detection technique by Canny actually
   * uses derivatives of 2D Gaussians to calculate the gradient. This
   * implementation uses Sobel masks to estimate gradient. This choice
   * was made to keep the design modular. Typically, one should
   * prepend this operation by a PiiImageFilterOperation and set its
   * filter to "Gaussian" to prevent spurious edges. In fact, this
   * technique does not do much harm to the other edge detectors
   * either.
   */     
  enum Detector { SobelDetector, RobertsDetector, PrewittDetector, CannyDetector };
  PiiEdgeDetector();

  Detector detector() const;
  void setDetector(Detector detector);

  void setThreshold(double threshold);
  double threshold() const;

  void setLowThreshold(double lowThreshold);
  double lowThreshold() const;

  void check(bool reset);

protected:
  void process();

private:
  template <class T> void detectIntEdges(const PiiVariant& obj);
  template <class T> void detectFloatEdges(const PiiVariant& obj);
  template <class T> void detectEdges(const PiiMatrix<T>& gradientX,
                                      const PiiMatrix<T>& gradientY);
  template <class T> void cannyThreshold(const PiiMatrix<T>& gradientX,
                                         const PiiMatrix<T>& gradientY,
                                         PiiMatrix<T>& magnitude,
                                         T highThreshold);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    Detector detector;
    double dThreshold;
    double dLowThreshold;
    PiiMatrix<int> matFilterX, matFilterY;
    bool bDirectionConnected;
  };
  PII_D_FUNC;
};


#endif //_PIIEDGEDETECTOR_H
