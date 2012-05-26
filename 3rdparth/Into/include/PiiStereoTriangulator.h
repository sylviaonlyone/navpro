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

#ifndef _PIISTEREOTRIANGULATOR_H
#define _PIISTEREOTRIANGULATOR_H

#include "PiiCalibration.h"

/**
 * A class that calculates 3D world coordinates for objects seen from
 * 2 to N viewpoints. See @ref calibration_stereo for an overview. 
 * %PiiStereoTriangulator is able to compute accurate 3D measurements
 * by combining information from many cameras. It enhances the
 * accuracy of stereo triangulation by averaging triangulation results
 * from all possible stereo pairs if more than two cameras are
 * available.
 *
 * @ingroup PiiCalibrationPlugin
 */
class PII_CALIBRATION_EXPORT PiiStereoTriangulator
{
public:

  /**
   * Create a new stereo trangulator.
   */
  PiiStereoTriangulator();
  ~PiiStereoTriangulator();
  
  /**
   * Add a new camera to the triangulator. The triangulator will
   * automatically calculate the relative positions of the cameras. To
   * be able to measure 3D positions, at least two cameras must be
   * added.
   *
   * @param intrinsic the intrinsic parameters of a camera
   *
   * @param extrinsic the relative position of the camera wrt to the
   * world coordinate system
   */
  void addCamera(const PiiCalibration::CameraParameters& intrinsic,
                 const PiiCalibration::RelativePosition& extrinsic);

  /**
   * Given a list of points in 2-dimensional image coordinates
   * calculate the corresponding 3-dimensional coordinates.
   *
   * @param imagePoints the coordinates of the objects to measure, in
   * pixels (N-by-2). All matrices must be of the same size, and there
   * must be one set of coordinates for each camera added to the
   * triangulator. The coordinates from the cameras must be given in
   * the same order the cameras were added. If a measurement point is
   * not available in one of the views, the point's coordinates must
   * be set to NaN.
   *
   * @return 3-dimensional coordinates of the points in the reference
   * frame of the camera first added with #addCamera() (a N-by-3
   * matrix). If a point is not visible in a sufficient number of
   * views or if the estimation error is deemed too large, all of its
   * coordinates will be set to NaN.
   *
   * @exception PiiCalibrationException& if the data is not correctly
   * formatted.
   *
   * To convert the result to world coordinates, use the
   * PiiCalibration::cameraToWorldCoordinates() function with the
   * extrinsic parameters of the first camera.
   */
  PiiMatrix<double> calculate3DPoints(const QList<PiiMatrix<double> >& imagePoints);

private:
  QString tr(const char* message);
  PiiMatrix<double> triangulate(int camera1, int camera2,
                                const PiiMatrix<double>& normalizedA,
                                const PiiMatrix<double>& normalizedB);
  
  /// @internal
  class Data
  {
  public:
    Data();

    /**
     * Relative positions of the cameras with respect to the world
     * coordinate system and each other. If there are N cameras, lst[0]
     * holds N positions, lst[1] holds N-1 positions etc. All positions
     * are stored so that they transform the reference frame of the
     * corresponding camera to that of the first camera.
     */
    QList<QList<PiiCalibration::RelativePosition> > lstRelativePositions;
    
    /**
     * Intrinsic parameters of cameras added so far.
     */
    QList<PiiCalibration::CameraParameters> lstCameraParameters;
  } *d;
};

#endif //_PIISTEREOTRIANGULATOR_H
