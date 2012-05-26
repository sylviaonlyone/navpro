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

#ifndef _PIICALIBRATION_H
#define _PIICALIBRATION_H

#include "PiiCalibrationException.h"
#include <PiiMath.h>
#include <PiiVector.h>
#include <PiiGeometricObjects.h>
#include <PiiImage.h>
#include <QFlags>

/**
 * Functions for camera calibration.
 *
 * @see @ref calibration_overview
 * @see @ref calibration_stereo
 *
 * @ingroup PiiCalibrationPlugin
 */
namespace PiiCalibration
{
  /**
   * A structure that stores all intrinsic camera parameters estimated
   * by the calibration functions. These parameters are related to the
   * camera and its lens and do not change depending on the scene
   * viewed. Please refer to the documentation at @ref
   * calibration_intrinsic_parameters for an explanation.
   */
  struct PII_CALIBRATION_EXPORT CameraParameters
  {
    /**
     * Initialize intrinsic parameters based on known image pixel
     * size. This will set the principal point to the center of the
     * image. All other values will be set to zero.
     *
     * @param imageWidth the width of camera image in pixels
     *
     * @param imageHeight the height of camera image in pixels
     */
    CameraParameters(int imageWidth = 0, int imageHeight = 0) :
      center(double(imageWidth)/2-0.5, double(imageHeight)/2-0.5),
      k1(0), k2(0), p1(0), p2(0)
    {}

    /**
     * Focal length. A 1-by-2 matrix that stores the focal length in x
     * and y directions.
     */
    PiiPoint<double> focalLength;
    /**
     * Principal point. A 1-by-2 matrix that stores the pixel
     * coordinates of the camera's optical center.
     */
    PiiPoint<double> center;
    /**
     * 2nd order radial distortion factor.
     */
    double k1;
    /**
     * 4th order radial distortion factor.
     */
    double k2;
    /**
     * First tangential distortion factor.
     */
    double p1;
    /**
     * Second tangential distortion factor.
     */
    double p2;
  };

  /**
   * A structure that stores the extrinsic parameters related to a
   * scene viewed. These parameterers are specific to a certain view. 
   * Please refer to @ref calibration_extrinsic_parameters for an
   * explanation.
   */
  struct PII_CALIBRATION_EXPORT RelativePosition
  {
    /**
     * Initializes all parameters to zero.
     */
    RelativePosition()
    {}

    /**
     * Initialize rotation and translation vectors to the given
     * values.
     */
    RelativePosition(const PiiVector<double,3>& rot,
                     const PiiVector<double,3>& trans) :
      rotation(rot), translation(trans)
    {}

    /**
     * Initialize a relative position with a rotation matrix and a
     * translation vector.
     *
     * @param rot a 3-by-3 rotation matrix
     *
     * @param trans a 3-by-1 or 1-by-3 traslation vector
     */
    RelativePosition(const PiiMatrix<double>& rot,
                     const PiiMatrix<double>& trans);

    /**
     * Convert the rotation vector to a 3-by-3 rotation matrix.
     */
    PiiMatrix<double> rotationMatrix() const;
    /**
     * Return the translation vector as a 3-by-1 column matrix.
     */
    PiiMatrix<double> translationMatrix() const;
    
    /**
     * The rotation vector. Defines the rotation between the world
     * coordinate system and the camera reference frame. Use the @ref
     * rotationVectorToMatrix() function to convert this vector to a
     * rotation matrix.
     */
    PiiVector<double,3> rotation;
    /**
     * The translation vector. The origin of the world coordinate
     * system in the camera reference frame.
     */
    PiiVector<double,3> translation;
  };

  /**
   * Options for calibration functions.
   *
   * @lip NoCalibrationOptions - none of the options applies.
   *
   * @lip EstimateIntrinsic - derive an initial guess of the intrinsic
   * parameters based on the assumption that the calibration rig is
   * planar. If this is not the case, you must provide an initial
   * guess for intrinsic parameters. An estimate of the principal
   * point (the image center) must always be provided.
   *
   * @lip FixPrincipalPoint - the principal point will be fixed to the
   * initial value and not changed during optimization.
   *
   * @lip FixAspectRatio - the focal ratio between focal length
   * components will be fixed. If @p EstimateIntrinsic is used, the
   * focal lengths will be set to arbitrary values. Only the ratio is
   * meaningful.
   *
   * @lip NoTangentialDistortion - do not estimate the tangetial
   * distortion factors. The factors are set to zero and won't change
   * in optimization.
   */
  enum CalibrationOption
    {
      NoCalibrationOptions   = 0,
      EstimateIntrinsic      = 1,
      FixPrincipalPoint      = 2,
      FixAspectRatio         = 4,
      NoTangentialDistortion = 8
    };

  Q_DECLARE_FLAGS(CalibrationOptions, CalibrationOption);
  Q_DECLARE_OPERATORS_FOR_FLAGS(CalibrationOptions)

#ifndef PII_NO_OPENCV
    
  /**
   * Calibrate a camera. This function calculates the intrinsic
   * parameters that minimize the reprojection error over the whole
   * set of views. It also returns the extrinsic parameters related to
   * each view.
   *
   * @param worldPoints a list of world coordinates of calibration
   * points. Each matrix in this list corresponds to one view of the
   * calibration rig and holds an N-by-3 matrix in which each row
   * represents the 3-dimensional world coordinates of a calibration
   * point. There must be at least three points in each view. If the
   * world coordinates of the calibration points do not change, and
   * all the points are visible in all views, the same world matrix
   * can be reused. In such a case it suffices to store only one
   * matrix in this list.
   *
   * @param imagePoints the corresponding points in image coordinates. 
   * Each matrix in this list corresponds to one view of the
   * calibration rig and holds an N-by-2 matrix in which each row
   * represents the 2-dimensional image coordinates of a calibration
   * point. The number of rows in each matrix must match @p
   * worldPoints.
   *
   * @param intrinsic store the calculated intrinsic parameters to
   * this structure. If the @p EstimateIntrinsic option is not set,
   * this structure must contain valid initial values for the
   * intrinsic parameters. In any case, an initial estimate of the
   * principal point must be provided. Therefore, always initialize
   * the intrinsic parameters with the size of the camera image.
   *
   * @param extrinsic store the extrinsic parameters to this list (if
   * non-zero). The extrinsic parameters are calculated for each view.
   *
   * @param options a logical OR of calibration options, e.g. @p
   * EstimateIntrinsic | @p NoTangentialDistortion.
   *
   * @exception PiiCalibrationException& if the calibration cannot be
   * performed with the given data.
   */
  PII_CALIBRATION_EXPORT void calibrateCamera(const QList<PiiMatrix<double> >& worldPoints,
                                              const QList<PiiMatrix<double> >& imagePoints,
                                              CameraParameters& intrinsic,
                                              QList<RelativePosition>* extrinsic = 0,
                                              CalibrationOptions options = EstimateIntrinsic);

  /**
   * Calculate the position of the camera reference frame with respect
   * to the world coordinate system. This function can be used once
   * the intrinsic camera parameters are known. See #calibrateCamera()
   * for a detailed description of the parameters.
   *
   * @param worldPoints the real-world coordinates of calibration
   * points in one view. (at least four)
   *
   * @param imagePoints the pixel coordinates of the corresponding
   * calibration points.
   *
   * @param intrinsic the intrinsic parameters of the camera.
   *
   * @return the relative position of the camera with respect to the
   * world coordinate system.
   *
   * @exception PiiCalibrationException& if the camera position cannot
   * be calculated with the given data.
   */
  PII_CALIBRATION_EXPORT RelativePosition calculateCameraPosition(const PiiMatrix<double>& worldPoints,
                                                                  const PiiMatrix<double>& imagePoints,
                                                                  const CameraParameters& intrinsic);


#endif //PII_NO_OPENCV

  /**
   * Calculate the relative position of @p camera2 with respect to @p
   * camera1. When the positions of the cameras have been calculated
   * with respect to the world coordinate system, this function can be
   * used to find the transformation between the camera reference
   * frames. As parameters, this function expects the relative
   * positions of both cameras with respect to the same world
   * coordinate system. The returned rotation matrix and translation
   * vector are calculated as follows:
   *
   * @f[
   * \begin{array}{rcl}
   * X_1 & = & R_1 X + T_1 \\
   * X_2 & = & R_2 X + T_2 \\
   * X   & = & R_1^{-1}(X_1 - T_1) \\
   * X_2 & = & R_2 R_1^{-1}(X_1 - T_1) + T_2 \\
   *     & = & R_2 R_1^{-1} X_1 + (T_2 - R_2 R_1^{-1} T_1) \\
   *     & = & R_S X_1 + T_S
   * \end{array},
   * @f]
   *
   * where X is a point in world coordinates. @f$R_S@f$ and @f$T_S@f$
   * are the rotation matrix and translation vector from @f$X_1@f$ to
   * @f$X_2@f$. @f$R_S@f$ will be converted to the more compact
   * rotation vector representation before returning.
   *
   * @param camera1 the relative position of the first camera with
   * respect to the world coordinate system. Let us denote the
   * rotation and translation matrices with @f$R_1@f$ and @f$T_1@f$.
   *
   * @param camera2 the relative position of the second camera with
   * respect to the world coordinate system. Let us denote the
   * rotation and translation matrices with @f$R_2@f$ and @f$T_2@f$.
   *
   * @return the relative position of @p camera2 with respect to @p
   * camera1.
   */
  PII_CALIBRATION_EXPORT RelativePosition calculateRelativePosition(const RelativePosition& camera1,
                                                                    const RelativePosition& camera2);

  /**
   * Converts pixel coordinates to normalized image coordinates. This
   * function first translates the input point according to the
   * principal point and divides out the focal length. It then
   * iteratively unapplies the lens distortion model for which there
   * is no closed-form solution.
   *
   * @param intrinsic the intrinsic parameters of the camera.
   *
   * @param x the x coordinate of the input point in pixel coordinates
   *
   * @param y the y coordinate of the input point in pixel coordinates
   *
   * @param newX undistorted x coordinate
   *
   * @param newY undistorted y coordinate
   */
  void undistort(const CameraParameters& intrinsic,
                 double x, double y,
                 double *newX, double *newY);

  /**
   * Converts pixel coordinates to normalized image coordinates. This
   * function undistorts each row in the input matrix.
   *
   * @param distorted 2D pixel coordinates. A N-by-2 or N-by-3 matrix.
   *
   * @param intrinsic the intrinsic parameters of the camera.
   *
   * @param zValue a fixed value for the z coordinate. If this value
   * is set to a valid numeric value, the undistorted pixels will all
   * have it as the z coordinate. This is useful if you need to apply
   * a 3D transformation to the coordinates afterwards.
   *
   * @return pixel coordinates transformed into normalized image
   * coordinates.
   *
   * @exception PiiCalibrationException& if the input data is invalid.
   */
  PII_CALIBRATION_EXPORT PiiMatrix<double> undistort(const PiiMatrix<double>& distorted,
                                                     const CameraParameters& intrinsic,
                                                     double zValue = NAN);

  /**
   * Transform points from camera reference frame to world coordinates.
   *
   * @param points 3-dimensional coordinates in the camera reference
   * frame (N-by-3 matrix).
   *
   * @param extrinsic the location of the camera wrt to the world
   * coordinate system.
   *
   * @return 3-dimensional coordinates in the world coordinate system
   * (N-by-3 matrix)
   */
  PII_CALIBRATION_EXPORT PiiMatrix<double> cameraToWorldCoordinates(const PiiMatrix<double>& points,
                                                                    const RelativePosition& extrinsic);


  /**
   * Transform points from world coordinate system to the camera
   * reference frame.
   *
   * @param points 3-dimensional coordinates in the world coordinate
   * system (N-by-3 matrix).
   *
   * @param extrinsic the location of the camera wrt to the world
   * coordinate system.
   *
   * @return 3-dimensional coordinates in the camera reference frame
   * (N-by-3 matrix)
   */
  PII_CALIBRATION_EXPORT PiiMatrix<double> worldToCameraCoordinates(const PiiMatrix<double>& points,
                                                                    const RelativePosition& extrinsic);

  /**
   * Transforms a point from normalized image coordinates to pixel
   * coordinates.
   *
   * @param intrinsic the intrinsic parameters of the camera.
   *
   * @param x the x coordinate of the input point in normalized image
   * coordinates
   *
   * @param y the y coordinate of the input point in normalized image
   * coordinates
   *
   * @param newX pixel x coordinate
   *
   * @param newY pixel y coordinate
   */
  PII_CALIBRATION_EXPORT void normalizedToPixelCoordinates(const CameraParameters& intrinsic,
                                                           double x, double y,
                                                           double *newX, double* newY);
  /**
   * Transforms points from normalized image coordinates to pixel
   * coordinates.
   *
   * @param points normalized 2-dimensional coordinates (x,y) (N-by-2
   * matrix).
   *
   * @param intrinsic camera parameters.
   *
   * @return a N-by-2 matrix which contains the corresponding
   * 2-dimensional pixel coordinates.
   */
  PII_CALIBRATION_EXPORT PiiMatrix<double> normalizedToPixelCoordinates(const PiiMatrix<double>& points,
                                                                        const CameraParameters& intrinsic);

  /**
   * Transforms points from camera reference frame to pixel
   * coordinates.
   *
   * @param points input points in the camera reference frame. A N-by-3 matrix (x,y,z).
   *
   * @param intrinsic camera parameters
   *
   * @return a N-by-2 matrix which contains 2-dimensional pixel
   * coordinates of the input points
   */

  PII_CALIBRATION_EXPORT PiiMatrix<double> cameraToPixelCoordinates(const PiiMatrix<double>& points,
                                                                    const CameraParameters& intrinsic);

  /**
   * Transform points from world coordinates to pixel coordinates.
   *
   * @param points input points in the world coordinate system. A
   * N-by-3 matrix (x,y,z).
   *
   * @param extrinsic the location of the camera wrt to the world
   * coordinate system.
   * 
   * @param intrinsic camera parameters
   *
   * @return a N-by-2 matrix which contains 2-dimensional pixel
   * coordinates of the input points
   */

  PII_CALIBRATION_EXPORT PiiMatrix<double> worldToPixelCoordinates(const PiiMatrix<double>& points,
                                                                   const RelativePosition& extrinsic,
                                                                   const CameraParameters& intrinsic);

  /**
   * Perform perspective projection on @p points.
   *
   * @param points a N-by-3 matrix, (x,y,z) on each row
   *
   * @param zValue a fixed value for the z coordinate. If this value
   * is set to a valid numeric value, the projected pixels will all
   * have it as the z coordinate. This is useful if you need to apply
   * a 3D transformation to the coordinates afterwards.
   *
   * @return a N-by-2 matrix (x/z,y/z), if zValue is NaN. Otherwise a
   * N-by-3 matrix.
   */

  PII_CALIBRATION_EXPORT PiiMatrix<double> perspectiveProjection(const PiiMatrix<double>& points,
                                                                 double zValue = NAN);

  /**
   * Convert a rotation vector @p rotation to a rotation matrix. This
   * function converts the three-dimensional column vector @f$V =
   * [v_x,v_y,v_z]^T@f$ to a rotation matrix @f$R@f$ as follows:
   *
   * @f[
   * R = e^{M\theta} = I + M \sin(\theta) +
   * M^2(1-\cos(\theta)),
   * @f]
   *
   * where M is an antisymmetric matrix:
   *
   * @f[
   * M = \left[ \begin{array}{ccc}
   *   0       & -v_{nz} & v_{ny}  \\
   *   v_{nz}  & 0       & -v_{nx} \\
   *   -v_{ny} & v_{nx}  & 0
   * \end{array} \right],
   * @f]
   *
   * where @f$\theta = ||V||@f$ and @f$V_n = V/\theta = [v_{nx},
   * v_{ny}, v_{nz}]^T@f$.
   *
   * The norm of V (its geometric length) is used as the rotation
   * angle wheras the direction of the vector specifies the rotation
   * axis.
   */
  PII_CALIBRATION_EXPORT PiiMatrix<double> rotationVectorToMatrix(const PiiVector<double,3>& rotation);

  /**
   * Convert a rotation matrix to a rotation vector. The direction of
   * the rotation vector tells the rotation axis, and its length is
   * equal to the rotation angle. Given a rotation matrix R, the
   * rotation vector V is derived as follows (see
   * #rotationVectorToMatrix() for definitions):
   *
   * @f[
   * R^T = I - M\sin(\theta) + M^2(1-\cos(\theta))
   * @f]
   * 
   * This equation makes use of the fact that M is an antisymmetric
   * matrix. Thus, @f$M^T = -M@f$ and @f$(M^2)^T = M^2@f$.
   *
   * @f[
   * (R - R^T) / 2 = M \sin(\theta)
   * @f]
   *
   * Since the vector represented by M is a unit vector, we obtain
   * both @f$M@f$ and @f$\sin(\theta)@f$ from this equation. This
   * doesn't fully resolve the rotation angle yet.
   *
   * @f[
   * (R + R^T) / 2 = I + M^2(1-\cos(\theta))
   * @f]
   * 
   * The only unknown here is @f$\cos(\theta)@f$, which is trivial to
   * solve.
   */
  PII_CALIBRATION_EXPORT PiiVector<double,3> rotationMatrixToVector(const PiiMatrix<double>& matrix);

  /**
   * Creates a coordinate map that converts distorted pixel coordinates
   * to undistorted ones. The returned map can be used with
   * PiiImage::remap() to correct an image against lens distortions. 
   * This function returns the distorted pixel coordinates in double
   * precision.
   */
  PII_CALIBRATION_EXPORT PiiImage::DoubleCoordinateMap undistortMap(int rows, int columns,
                                                                    const CameraParameters& intrinsic);

  /**
   * Creates a coordinate map that converts distorted pixel coordinates
   * to undistorted ones. The returned map can be used with
   * PiiImage::remap() to correct an image against lens distortions. 
   * This function returns the distorted pixel coordinates as
   * integers, rounded to the position of the closest matching pixel.
   */
  PII_CALIBRATION_EXPORT PiiImage::IntCoordinateMap undistortMapInt(int rows, int columns,
                                                                    const CameraParameters& intrinsic);

  /**
   * Removes lens distortions from @a sourceImage. This function first
   * creates an undistortion map with undistortMap() and then applies
   * PiiImage::remap(). If you need to repeat the process for many
   * images, it is more efficient to calculate the undistortion map
   * once and then apply it to all images.
   *
   * @param sourceImage the input image
   *
   * @param intrinsic camera parameters found by calibration
   *
   * @return an undistorted image
   */
  template <class T> PiiMatrix<T> undistortImage(const PiiMatrix<T>& sourceImage,
                                                 const CameraParameters& intrinsic);
}

#include "PiiCalibration-templates.h"

#endif //_PIICALIBRATION_H
