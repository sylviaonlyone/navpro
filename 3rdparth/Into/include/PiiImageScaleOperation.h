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

#ifndef _PIIIMAGESCALEOPERATION_H
#define _PIIIMAGESCALEOPERATION_H

#include <PiiDefaultOperation.h>

/**
 * Scale images to arbitrary sizes. The operation supports linear and
 * nearest neighbor interpolation.
 *
 * @inputs
 *
 * @in image - Input image. Any image type.
 * 
 * @outputs
 *
 * @out image - Scaled image. Same type as the input.
 *
 * @ingroup PiiImagePlugin
 */
class PiiImageScaleOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Scaling mode. Default is @p ZoomAccordingToFactor.
   */
  Q_PROPERTY(ScaleMode scaleMode READ scaleMode WRITE setScaleMode);
  Q_ENUMS(ScaleMode);

  /**
   * The amount of scaling, i.e. a "zoom factor". Default is 1.0. The
   * scale ratio also works as the target aspect ratio, depending on
   * @ref scaleMode. The scale ratio is calculated as width/height. 
   * Thus, 2 means an image whose width is two times its height.
   */
  Q_PROPERTY(double scaleRatio READ scaleRatio WRITE setScaleRatio);
  /**
   * Defines the size of output image. Default is (100,100).
   */
  Q_PROPERTY(QSize scaledSize READ scaledSize WRITE setScaledSize);

  /**
   * Interpolation mode. The default is @p LinearInterpolation. @p
   * NearestNeighborInterpolation is faster, but less accurate.
   */
  Q_PROPERTY(Interpolation interpolation READ interpolation WRITE setInterpolation);
  Q_ENUMS(Interpolation);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * A copy of Pii::Interpolation. (Stupid moc.)
   */
  enum Interpolation { NearestNeighborInterpolation, LinearInterpolation };
  
  /**
   * Scaling modes:
   *
   * @lip ZoomAccordingToFactor - zoom both dimensions according to the
   * specified scale ratio (@ref scaleRatio). Retain original aspect
   * ratio.
   *
   * @lip ScaleToSize - scale to the specified size (@ref scaledSize),
   * ignoring original aspect ratio.
   *
   * @lip ScaleToAspectRatioX - scale to a specified aspect ratio
   * (@ref scaleRatio). Scale the image horizontally to obtain this
   * aspect ratio.
   *
   * @lip ScaleToAspectRatioY - same as above, but scale vertically.
   *
   * @lip ScaleDownToAspectRatio - same as above, but scale the
   * direction that results in a smaller output image.
   *
   * @lip ScaleUpToAspectRatio - same as above, but scale the
   * direction that results in a larger output image.
   *
   * @lip ScaleToBox - fit to size retaining aspect ratio.
   *
   * Aspect ratio is always calculated as width/height.
   */
  enum ScaleMode { ZoomAccordingToFactor,
                   ScaleToSize,
                   ScaleToAspectRatioX,
                   ScaleToAspectRatioY,
                   ScaleDownToAspectRatio,
                   ScaleUpToAspectRatio,
                   ScaleToBox };
  
  PiiImageScaleOperation();

  ScaleMode scaleMode() const;
  void setScaleMode(ScaleMode scaleMode);
  
  double scaleRatio() const;
  void setScaleRatio(double scaleRatio);

  QSize scaledSize() const;
  void setScaledSize(QSize scaledSize);

  Interpolation interpolation() const;
  void setInterpolation(Interpolation interpolation);

protected:
  void process();

private:
  template <class T> void scaleImage(const PiiVariant& obj);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    ScaleMode scaleMode;
    double dScaleRatio;
    QSize scaledSize;
    Interpolation interpolation;
  };
  PII_D_FUNC;
};

#endif //_PIIIMAGESCALEOPERATION_H
