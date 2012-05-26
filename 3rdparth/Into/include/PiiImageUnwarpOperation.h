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

#ifndef _PIIIMAGEUNWARPOPERATION_H
#define _PIIIMAGEUNWARPOPERATION_H

#include <PiiDefaultOperation.h>

/**
 * An operation that straightens cylindrically warped images. See
 * PiiImage::unwarpCylinder().
 *
 * @inputs
 *
 * @in image - a distorted input image. Any image type.
 *
 * @in radius - the radius of the cylinder (double). Optional.
 *
 * @in distance - the distance between camera aperture and cylinder
 * center (double). Optional.
 *
 * @in center - the location of the optical center in the image
 * (double). Optional.
 *
 * @outputs
 *
 * @out image - unwarped version of the input. Same type as input.
 *
 * @out radius - the radius of the cylinder, in units used by
 * #cameraDistance.
 *
 * @out distance - the distance between camera aperture and cylinder
 * center.
 *
 * @out start angle - the start angle of the visible sector, in
 * radians. The start angle approaches zero as the distance or focal
 * length grows. If the camera is not perfectly aligned with the
 * cylinder (camera center is not a image center), start angle may be
 * different from end angle.
 *
 * @out sector - the angle of the visible sector, in radians. This
 * value is affected by the #focalLength.
 *
 * @out scale - the physical size of a pixel in the unwarped image. 
 * Pixel scale is represented as a 1-by-2 PiiMatrix<double>. 
 * Horizontal size (first matrix element) is expressed in rad/px, and
 * vertical size (second matrix element) in units determined by
 * #cameraDistance.
 *
 * @ingroup PiiImagePlugin
 */
class PiiImageUnwarpOperation : public PiiDefaultOperation
{
  Q_OBJECT
  
  /**
   * The focal length of the camera in pixels. Default is 1e100.
   */
  Q_PROPERTY(double focalLength READ focalLength WRITE setFocalLength);

  /**
   * The distance between camera aperture and cylinder center. Default
   * is 1. Set this value to 0 to make the operation to automatically
   * calculate distance.
   */
  Q_PROPERTY(double cameraDistance READ cameraDistance WRITE setCameraDistance);

  /**
   * Physical radius of the cylinder. Default is 0, which causes the
   * radius to be calculated automatically.
   */
  Q_PROPERTY(double radius READ radius WRITE setRadius);

  /**
   * The location of the optical center in the image. The default is
   * NaN, which positions the optical center to the center of the
   * input image.
   */
  Q_PROPERTY(double center READ center WRITE setCenter);

  /**
   * The maximum angle of the unwarped sector, in degrees. See
   * PiiImage::unwarpCylinder() for details. The default value is 0
   * (no limit).
   */
  Q_PROPERTY(double maxSectorAngle READ maxSectorAngle WRITE setMaxSectorAngle);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiImageUnwarpOperation();

  void setFocalLength(double focalLength);
  double focalLength() const;
  void setCameraDistance(double cameraDistance);
  double cameraDistance() const;
  void setRadius(double radius);
  double radius() const;
  void setMaxSectorAngle(double maxSectorAngle);
  double maxSectorAngle() const;
  void setCenter(double center);
  double center() const;

  void check(bool reset);

protected:
  void process();

private:
  template <class T> void unwarp(const PiiVariant& obj);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dFocalLength;
    double dCameraDistance;
    double dRadius;
    double dMaxSectorAngle;
    double dCenter;
    
    PiiInputSocket *pImageInput, *pDistanceInput, *pRadiusInput, *pCenterInput, *pSkewInput;
    bool bRadiusConnected, bDistanceConnected, bCenterConnected, bSkewConnected;
    PiiOutputSocket *pImageOutput, *pRadiusOutput, *pDistanceOutput,
      *pAngleOutput, *pSectorOutput, *pScaleOutput;
  };
  PII_D_FUNC;
};

#endif //_PIIIMAGEUNWARPOPERATION_H
