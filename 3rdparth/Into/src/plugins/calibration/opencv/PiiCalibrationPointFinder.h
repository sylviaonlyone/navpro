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

#ifndef _PIICALIBRATIONPOINTFINDER_H
#define _PIICALIBRATIONPOINTFINDER_H

#include "PiiCalibrationGlobal.h"
#include "PiiCalibration.h"
#include "PiiCalibrationException.h"
#include <PiiMath.h>
#include <QVector>

/**
 * A class that can be used to find calibration points in a set of
 * unreliable detections. The problem can be stated as follows: Given
 * a set of points in known world coordinates and a possibly larger
 * set of detected points in image coordinates, find the set of
 * detected points that best matches the known arrangement of
 * calibration points and calibrate the camera to them.
 *
 * To speed up the search, the following assumptions are made:
 *
 * @li calibration points are arranged on a planar surface (all z
 * coordinates are either zero or one).
 *
 * @li world points are given in counter-clockwise order with respect
 * to the center of the calibration points in a right-handed
 * coordinate system.
 *
 * @ingroup PiiCalibrationPlugin
 */
class PII_CALIBRATION_EXPORT PiiCalibrationPointFinder
{
public:
  /**
   * Create a new point finder.
   *
   * @param minDistance the minimum allowed distance (in pixels)
   * between two points in the calibration pattern. on the image
   * plane. This value can be used to cut off too small detections.
   *
   * @param maxDistance the maximum allowed extent (in pixels) of the
   * whole calibration pattern on the image plane. This is used to cut
   * off detections that clearly deviate from the others.
   */
  PiiCalibrationPointFinder(double minDistance = 0.0,
                            double maxDistance = INFINITY);
  ~PiiCalibrationPointFinder();
  
  /**
   * Find the set of measurements that best matches the geometric
   * arrangement of @p worldPoints.
   *
   * @param worldPoints known 3D coordinates of calibration points
   *
   * @param imagePoints 2D locations of detected calibration (and
   * possibly other) points in image coordinates. One should try to
   * make this set as small as possible while still including all the
   * calibration points. The order of image points is irrelevant.
   * Conventional pixel coordinate system is assumed.
   *
   * @param intrinsic the intrinsic parameters of the camera.
   *
   * @return the relative position of the camera with respect to the
   * world coordinate system.
   *
   * @exception PiiCalibrationException& if the camera position cannot
   * be calculated with the given data.
   */
  PiiCalibration::RelativePosition calculateCameraPosition(const PiiMatrix<double>& worldPoints,
                                                           const PiiMatrix<double>& imagePoints,
                                                           const PiiCalibration::CameraParameters& intrinsic);

  /**
   * Get the minimum mean square error in matching the geometric
   * pattern to measurements.
   */
  double minError() const;

  /**
   * Get the coordinates of the selected calibration points.
   *
   * @return the image coordinates of the calibration points as a
   * N-by-2 matrix, in the same order they appeared in worldPoints. If
   * calculateCameraPosition() has not been called or an error
   * occured, an empty matrix will be returned.
   */
  PiiMatrix<double> selectedPoints() const;
  
  void setMinDistance(double minDistance);
  double minDistance() const;
  void setMaxDistance(double maxDistance);
  double maxDistance() const;
  
private:
  // We don't want the functor interface to be public.
  template <class UnaryOp, class Storage> friend void Pii::combinations(int,int,UnaryOp,Storage);
 
  void operator()(int* combination);
    
  void calculateProjectionError(const QVector<int>& indices);
  void createDistanceMatrix();
  void createCombinationMatrix();
  
  /// @internal
  class Data
  {
  public:
    Data(double minDistance, double maxDistance);
    
    PiiMatrix<double> matWorldPoints;
    PiiMatrix<double> matImagePoints;
    PiiMatrix<double> matCurrentPoints;
    PiiMatrix<double> matUndistorted;
    PiiMatrix<double> matDistances;
    PiiMatrix<int> matCombinations;
    PiiCalibration::CameraParameters intrinsic;
    double dMinDistance, dMaxDistance;
    int iPointCount; // The number of calibration points, equals
    // matWorldPoints.rows()
    double dMinError;
    QVector<int>lstMinIndices;
    PiiCalibration::RelativePosition minPosition;
  } *d;
};


#endif //_PIICALIBRATIONPOINTFINDER_H
