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

#error "Documentation.h is not intended for inclusion. Its sole purpose is to provide documentation for a package."

/**
 * @page calibration_overview Overview of Camera Calibration
 *
 * The following secetions give you a broad overview of camera
 * calibration. Please refer to the documentation of calibration
 * functions in the @ref PiiCalibration namespace for real usage
 * examples.
 *
 * @section calibration_intrinsic_parameters Intrinsic Parameters
 *
 * The calibration functions use a pinhole camera model in which a 3D
 * scene is projected to the image plane with a perspective
 * projection. Let @f$X_c=[x_c,y_c,z_c]^T@f$ be a point in a the
 * three-dimensional world with its coodinates expressed in <em>camera
 * reference frame</em>, a coordinate system attached to the camera. 
 * In this coodinate system, the optical axis intersects the image
 * plane at a point called the <em>principal point</em>. The origin of
 * the coordinate system is at the <em>aperture</em> of the camera,
 * which is also called the <em>optical center</em>. The image plane
 * is located at distance @e f from the origin in the negative
 * direction of the @e z axis. The focal length @e f can be thought of
 * as a scaling factor from normalized (unitless) coordinates to
 * pixels. The value of @e f can actually be different for @e x and @p
 * y axes to allow non-square pixels. Physically, focal length is
 * the distance between the aperture (camera origin) and the image
 * plane, in pixels. Thus, smaller pixels mean larger focal length and
 * vice versa.
 *
 * Perspective projection gives us <em>normalized image
 * coordinates</em>, the coordinates we would ideally get without
 * distortion, translation, or scaling on a virtual image plane:
 *
 * @f[
 * X_n = [x_c, y_c]^T / z_c = [x_n, y_n]^T
 * @f]
 *
 * The lens distortion model used in the calibration functions consist
 * of two components: radial and tangential. The radial distortion is
 * the same for both coordinates and it only depends on the point's
 * distance distance from optical axis (@f$r^2 = x_n^2 + y_n^2@f$)
 * whereas tangential distortion can be a bit skewed. After radial
 * distortion, the coodinates become:
 *
 * @f[
 * X_{rd} = (1 + k_1 r^2 + k_2 r^4) X_n,
 * @f]
 *
 * where @f$k_1@f$ and @f$k_2@f$ are <em>radial distortion
 * factors</em>. The radial distortion could be esimated to higher
 * order polynomials (a well-known Matlab toolbox supports @f$r^6@f$),
 * but the fourth-order polynomial is enough in all but extreme cases. 
 * After adding tangential distortion, the distorted coodinates become
 *
 * @f[
 * X_d = X_{rd} + \left[\begin{array}{c}
 *   2p_1 x_n y_n + p_2(r^2 + 2x_{n}^2) \\
 *   p_1(r^2 + 2y_{n}^2) + 2p_2 x_n y_n
 * \end{array} \right],
 * @f]
 *
 * where @f$p_1@f$ and @f$p_2@f$ are <em>tangential distortion
 * factors</em>.

 * Now that the projection and lens distortion have been taken into
 * account we only have to address three minor issues before we get
 * pixel coordinates: scaling to pixel size, correcting the sensor's
 * skew, and translating to the origin of the image coordinate system. 
 * All these can be perfomed with a single affine transform:
 *
 * @f[
 * X_p = \left[ \begin{array}{cc}
 *   f_x & \alpha x \\
 *   0   & f_y
 * \end{array} \right] X_d + C,
 * @f]
 *
 * where @f$C = [c_x, c_y]^T@f$ is called the <em>principal
 * point</em>, the pixel coordinates of the point where the optical
 * axis hits the image sensor. This point is usually at the center of
 * the image. @f$f_x@f$ and @f$f_y@f$ are the <em>focal length</em> of
 * the camera, in pixels. Nowadays, most cameras have square pixels,
 * and these values are almost equal after calibration. @f$\alpha@f$
 * is needed only if the angle between pixel rows and columns is not
 * straight. Since this is always (well, almost) the case, the skew
 * term is simply set to zero.
 *
 * In summary, there are eight <em>intrinsic parameters</em> that
 * define the transformation from camera reference frame coordinates
 * to pixel coordinates. The intrinsic parameters do not depend on the
 * position of the camera or the scene viewed.
 *
 * @li @f$f_x@f$ and @f$f_y@f$ - the focal length in pixels
 *
 * @li @f$c_x@f$ and @f$c_y@f$ - the location of the principal point
 *
 * @li @f$k_1@f$ and @f$k_2@f$ - radial distortion factors
 *
 * @li @f$p_1@f$ and @f$p_2@f$ - tangential distortion factors
 *
 * In the calibration library, intrinsic parameters are stored in a
 * @ref PiiCalibration::CameraParameters "CameraParameters" structure.
 *
 * @note The origin of the pixel coordinate system is at the center of
 * the pixel at the upper left corner. The pixel coordinates (width-1,
 * height-1) are at the center of the pixel at the lower right corner. 
 * Thus, the center of the image is at (width/2-0.5, height/2-0.5).
 *
 *
 * @section calibration_extrinsic_parameters Extrinsic Parameters
 *
 * The calculation of intrinsic parameters assumed that the original
 * 3-dimensional point coordinates are expressed in the camera
 * reference frame. Since the camera can be in an arbitrary position,
 * we need to find a transformation between the camera reference frame
 * a the world coordinates.
 *
 * Let @f$X = [x,y,z]^T@f$ be the location of a point in world
 * coordinates. The transformation from world coordinates to the
 * camera reference frame can be defined by an affine transform as
 * follows:
 *
 * @f[
 * X_c = R X + T,
 * @f]
 *
 * where R is a 3-by-3 rotation matrix (direction cosine matrix, DCM)
 * whose column vectors define the base of the world coordinate system
 * in the camera reference frame. Note that there needs to be neither
 * scaling nor shearing terms in the matrix as those are already taken
 * into account in the intrinsic parameters. T is a 3-by-1 translation
 * vector that defines the location of the world origin in the camera
 * reference frame. Since 3D rotation can be expressed with just three
 * values, there are six extrinsic parameters in total (rotation
 * vector and translation vector). These parameters are dependent on
 * the location of the camera with respect to the scene viewed, and of
 * course the physical location of the world coordinate system's
 * origin.
 *
 * In the calibration functions, extrinsic parameters are stored in a
 * @ref PiiCalibration::RelativePosition "RelativePosition" structure
 * as they define the relative position of the camera with respect to
 * the world coordinate system.
 *
 *
 * @section calibration_calibration Calibration Procedure
 *
 * The process of finding out the intrinsic and extrinsic parameters
 * of a camera is called @e calibration. To accomplish this task one
 * needs to know a set of <em>point correspondents</em> - pairs of
 * points for which both the world and pixel coordinates are known. 
 * This is usually accomplished by taking images of a <em>calibration
 * rig</em> that has some easily distinguishable features arranged on
 * a regular grid. Given a sufficient amount of such points, the
 * calibration algorithm finds the unknown parameters.
 *
 * In a sense, calibration is a chicken-and-egg problem: one needs to
 * know the 3D coordinates of calibration points in the camera
 * reference frame to be able to calculate the intrinsic parameters. 
 * But we know just the world coordinates, and the transformation from
 * world to camera coordinates is unknown. In order to find the
 * transformation we need the intrinsic parameters, because pixel
 * coordinates cannot be projected back without them. For this reason,
 * some assumptions must be made.
 *
 * The easiest way to solve the problem is to use a planar calibration
 * rig. For such a rig, the calibration algorithm can initialize the
 * intrinsic parameters. The focal length can be estimated by finding
 * the <em>vanishing points</em> on which the calibration rig's
 * horizontal and vertical lines intersect. The principal point in
 * turn is near the center of the image. This initial guess is usually
 * good enough for the iterative algorithm to converge.
 *
 * If the calibration rig is non-planar, estimation of intrinsic
 * parameters becomes more difficult. Currently, the library
 * implements no automatic initialization of intrinsic parameters for
 * non-planar objects. Instead, an initial guess of the intrinsic
 * parameters must be given.
 *
 * Calibration can be performed with a single calibration image, but
 * better results are obtained when the calibration rig is pictured
 * from many different viewpoints. The library finds the intrinsic
 * parameters that minimize the projection error over the whole set
 * of poses. The extrinsic parameters are, of course, calculated
 * separately for each pose.
 *
 *
 * @section calibration_summary Summary
 *
 * @par Coordinate Systems
 *
 * The calibration process involves four coordinate systems:
 *
 * @li World coordinates. The real-world 3D coordinates of calibration
 * points.
 *
 * @li Camera reference frame. The 3D coordinates of the calibration
 * points in a coordinate system whose z axis is the optical axis of
 * the camera. The origin is at the aperture of the camera.
 *
 * @li Normalized image coordinates. Non-distorted perspective
 * projection of the camera reference frame to a virtual image plane
 * (2D). The normalized coordinates are unitless.
 *
 * @li Pixel coordinates. The 2D coordinates of the calibration points
 * in the image. Some functions assume the conventional ordering of
 * image axes: @e x points to the right and @e y points down. If you
 * break this convention, prepare for troubles.
 *
 * @par Transformations
 *
 * Transformations between coordinate systems are performed as
 * follows:
 *
 * @li Word coordinates <-> Camera reference frame: use the extrinsic
 * parameters (translation and rotation vectors) and perform an affine
 * transformation. Note that the inverse of the rotation matrix R is
 * equal to its transpose. This is really just a matter of matrix math
 * and easily done with PiiMatrix. For convenience, the @ref
 * PiiCalibration::cameraToWorldCoordinates()
 * "cameraToWorldCoordinates" and @ref
 * PiiCalibration::worldToCameraCoordinates()
 * "worldToCameraCoordinates" functions are provided.
 *
 * @li Camera reference frame -> Normalized image coordinates: use
 * the pinhole camera model (perspective projection). Just divide the
 * x and y coordinates by z. (See the @ref
 * PiiCalibration::perspectiveProjection() "perspectiveProjection"
 * function.)
 *
 * @li Normalized image coordinates -> Pixel coordinates: Apply lens
 * distortions (radian and tangential). Then scale and translate the
 * distorted coordinates according to the focal length and principal
 * point with an affine transformation. (See the @ref
 * PiiCalibration::normalizedToPixelCoordinates()
 * "normalizedToPixelCoordinates" function.)
 *
 * @li Pixel coordinates -> Normalized image coordinates: unapply
 * focal length and principal point (affine transform). Unapply the
 * radial and tangential distortions. Due to the high-degree
 * polynomials in the distortion model, there is no closed-form
 * inverse distortion model. An iterative numerical solution must be
 * used. The @ref PiiCalibration::undistort() "undistort" function
 * performs this inverse mapping.
 *
 * @li Normalized camera coordinates -> Camera reference frame: there
 * is no unique solution. The 3D location of a point cannot be derived
 * from a single 2D measurement. One measurement only limits the
 * possibilities to a line in the 3D space. The points on the line (in
 * the camera reference frame) satisfy the equation @f$X_c = [ x_n z,
 * y_n z, z ]^T@f$, where z is a free variable. To find z, at least
 * two cameras need to be used. Please refer to @ref
 * calibration_stereo for details.
 */

/**
 * @page calibration_stereo Stereo Imaging
 *
 * Calibrated cameras can be used to measure accurate 3D locations of
 * objects in world coordinates. As already noted in @ref
 * calibration_summary, one camera can only find a line in a 3D space
 * on which the measurement point must lie. Another camera is needed
 * to locate the exact position of the point on that line. This
 * calculation is often called <em>stereo triangulation</em>.
 *
 * In fact, it is sometimes possible to measure world coordinates
 * from a single view. But this requires prior knowledge of the scene. 
 * For example, when controlling a robot that picks items from a
 * planar surface one can fix the z coordinate to zero. There are also
 * techniques that make use of visual cues such as texture gradient
 * and line orientation to estimate 3D structure.
 *
 * There are a couple of difficulties in finding a unique solution
 * even with two cameras. Obviously, both cameras must see the point
 * to be measured. But they must also know that they are looking at
 * the same object. Even if these prerequisites hold, it is highly
 * unlikely that the two lines the cameras get for the point's
 * location actually intersect in the 3D space. They are almost
 * certainly slightly apart, and the location of the point needs to be
 * estimated from inconsistent information. Usually, the estimate is
 * placed halfway between the 3D lines where they are closest to each
 * other. An even better approximation can be obtained if many cameras
 * are used. PiiStereoTriangulator does just that: it considers all
 * possible pairings of cameras and uses the stereo triangulation
 * technique to find the most likely 3D location by averaging over all
 * stereo pairs.
 */
