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

#ifndef _PIIUNDISTORTOPERATION_H
#define _PIIUNDISTORTOPERATION_H

#include <PiiDefaultOperation.h>
#include "PiiCalibration.h"

/**
 * Corrects lens distortion.
 *
 * @inputs
 *
 * @in image - distorted input image. Any image type.
 *
 * @outputs
 *
 * @out image - undistorted image. Same type as the input.
 *
 * @ingroup PiiCalibrationPlugin
 */
class PiiUndistortOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The horizontal focal length of the camera in pixels. The default
   * value is 1000.
   */
  Q_PROPERTY(double focalX READ focalX WRITE setFocalX);

  /**
   * The vertical focal length of the camera in pixels. The default
   * value is 1000.
   */
  Q_PROPERTY(double focalY READ focalY WRITE setFocalY);

  /**
   * The x coordinate of the principal point. If this value is NaN
   * (the default), the operation will place the principal point at
   * the center of the image.
   */
  Q_PROPERTY(double centerX READ centerX WRITE setCenterX);

  /**
   * Same as @p centerX, but for the y coordinate.
   */
  Q_PROPERTY(double centerY READ centerY WRITE setCenterY);

  /**
   * 2nd order radial distortion factor. The default value is zero.
   */
  Q_PROPERTY(double radial2nd READ radial2nd WRITE setRadial2nd);

  /**
   * 4th order radial distortion factor. The default value is zero.
   */
  Q_PROPERTY(double radial4th READ radial4th WRITE setRadial4th);

  /**
   * First tangential distortion factor. The default value is zero.
   */
  Q_PROPERTY(double tangential1st READ tangential1st WRITE setTangential1st);

  /**
   * Second tangential distortion factor. The default value is zero.
   */
  Q_PROPERTY(double tangential2nd READ tangential2nd WRITE setTangential2nd);

  /**
   * All intrinsic camera parameters represented as a 1-by-8
   * PiiMatrix<double>. The matrix stores the focal length (x,y),
   * principal point (x,y), radial distortion (k1, k2), and tangential
   * distortion (p1, p2), in this order.
   */
  Q_PROPERTY(PiiVariant cameraParameters READ cameraParameters WRITE setCameraParameters STORED false);

  /**
   * The type of interpolation. The default value is @p
   * Pii::LinearInterpolation, which results in better image quality
   * but slower operation. Set to @p Pii::NearestNeighborInterpolation
   * to speed up calculations at the expense of image quality.
   */
  Q_PROPERTY(Pii::Interpolation interpolation READ interpolation WRITE setInterpolation);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiUndistortOperation();

  void check(bool reset);

protected:
  void process();

  void setFocalX(double focalX);
  double focalX() const;
  void setFocalY(double focalY);
  double focalY() const;
  void setCenterX(double centerX);
  double centerX() const;
  void setCenterY(double centerY);
  double centerY() const;
  void setRadial2nd(double radial2nd);
  double radial2nd() const;
  void setRadial4th(double radial4th);
  double radial4th() const;
  void setTangential1st(double tangential1st);
  double tangential1st() const;
  void setTangential2nd(double tangential2nd);
  double tangential2nd() const;
  void setCameraParameters(const PiiVariant& cameraParameters);
  PiiVariant cameraParameters() const;
  void setInterpolation(Pii::Interpolation interpolation);
  Pii::Interpolation interpolation() const;

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    PiiCalibration::CameraParameters intrinsic;
    PiiImage::DoubleCoordinateMap dmatMap;
    PiiImage::IntCoordinateMap imatMap;
    Pii::Interpolation interpolation;
  };
  PII_D_FUNC;

  template <class T> void undistort(const PiiVariant& obj);
  void invalidate();
};


#endif //_PIIUNDISTORTOPERATION_H
