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

#ifndef _PIIIMAGEDISTORTIONS_H
#define _PIIIMAGEDISTORTIONS_H

#include <PiiMatrix.h>

namespace PiiImage
{
  /**
   * Straighten an image that is wrapped around a cylinder.
   *
   * @param warpedImage distorted input image. It is assumed that the
   * optical axis of the camera is perpendicular to the surface of the
   * cylinder and that the cylinder is vertical in the image (image is
   * compressed towards left and right borders). The image must
   * contain the full visible width of the cylinder and no background.
   *
   * @param focalLength the focal length of the camera. See @ref
   * calibration_intrinsic_parameters for more information. Use a
   * large value to approximate a telecentric lens.
   *
   * @param center the location of the optical center in the image. If
   * the optical axis of the camera is perpendicular to the surface,
   * it hits the cylider at its center, and the center of the cylinder
   * is at the center of the image. If the center of the cylinder is
   * not at the center of the image, the translation must be taken
   * into account in the transformation. The location of the optical
   * center is expressed in pixels so that the center of the leftmost
   * pixel is at zero, and the center of the rightmost pixel at image
   * width - 1. The default value (NaN) places the optical center at
   * the center of the image.
   *
   * @param cameraDistance the physical distance between camera
   * aperture and the cylinder center, in user-defined units. This is
   * an input-output parameter. If cameraDistance is a null pointer or
   * zero, it will be automatically calculated.
   *
   * @param radius an optional input/output parameter that specifies
   * the physical radius of the cylinder, in the same units as @p
   * cameraDistance. If radius is a null pointer or zero, it will be
   * automatically calculated. If both cameraDistance and radius are
   * zero, 100 will be used as the radius because both cannot be
   * solved simultaneously.
   *
   * @param sectorAngle an optional input/output parameter. If this
   * parameter is given and its value is positive, the value will be
   * used as the maximum sector angle (in radians). If the value is
   * smaller than the calculated angle of the visible sector, cylinder
   * boundaries will be cut off. Upon return, the value will store the
   * angle of the visible sector, or @p sectorAngle, whichever is
   * smaller.
   *
   * @param startAngle an optional output-value parameter. If this
   * value is non-zero, if the angle at which the left side of the
   * sector is seen will be stored into the pointed memory. If the
   * center of the cylinder is at optical axis, the minimum value is 0
   * (full 180 degrees visible). This value grows as @a sectorAngle
   * decreases.
   *
   * @return the image that was wrapped around a cylinder, as if it
   * was straightened.
   */
  template <class T> PiiMatrix<T> unwarpCylinder(const PiiMatrix<T>& warpedImage,
                                                 double focalLength = 1e100,
                                                 double center = NAN,
                                                 double *cameraDistance = 0,
                                                 double *radius = 0,
                                                 double *sectorAngle = 0,
                                                 double *startAngle = 0);
}

#include "PiiImageDistortions-templates.h"

#endif //_PIIIMAGEDISTORTIONS_H
