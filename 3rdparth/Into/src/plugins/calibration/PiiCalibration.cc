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

#include "PiiCalibration.h"

#include <cstring>
#include <QCoreApplication>

#ifndef PII_NO_OPENCV
#  include <PiiOpenCv.h>
#endif

#include <PiiOptimization.h>

namespace PiiCalibration
{
  /************************************************************************
   * "Private" utility functions
   ************************************************************************/
  QString tr(const char* message)
  {
    return QCoreApplication::translate("PiiCalibration", message);
  }

#ifndef PII_NO_OPENCV
  
  // A utility function that creates an RelativePosition structure
  // out of rotation and translation vectors. rowIndex specifies the
  // active row in the rotation and translation matrices.
  RelativePosition createRelativePosition(CvMat* rotation, CvMat* translation, int rowIndex = 0)
  {
    return RelativePosition(PiiVector<double,3>(rotation->data.db + rowIndex * rotation->cols),
                            PiiVector<double,3>(translation->data.db + rowIndex * translation->cols));
  }

  CvMat* createIntrinsicMatrix(const CameraParameters& intrinsic)
  {
    // Intrinsic parameters are stored in a 3-by-3 double matrix:
    // [ fx  0 cx ]
    // [  0 fy cy ]
    // [  0  0  1 ]
    CvMat* result = PiiOpenCv::cvMat<double>(3, 3);
    
    // Initialize the intrinsic matrix with the given input values.
    result->data.db[0] = intrinsic.focalLength.x;
    result->data.db[4] = intrinsic.focalLength.y;
    result->data.db[2] = intrinsic.center.x;
    result->data.db[5] = intrinsic.center.y;
    result->data.db[8] = 1;

    return result;
  }

  CvMat* createDistortionCoeffs(const CameraParameters& intrinsic)
  {
    // Distortion coefficients are stored in a 1-by-4 double matrix:
    // [ k1 k2 p1 p2 ]
    CvMat* result = PiiOpenCv::cvMat<double>(1, 4);

    // Initialize distortion coeffs
    result->data.db[0] = intrinsic.k1;
    result->data.db[1] = intrinsic.k2;
    result->data.db[2] = intrinsic.p1;
    result->data.db[3] = intrinsic.p2;

    return result;
  }

  void storeCameraParameters(CameraParameters& intrinsic,
                             CvMat* intrinsicMatrix,
                             CvMat* distortionCoeffs)
  {
    // Pick parameters from the matrices
    intrinsic.focalLength.x = intrinsicMatrix->data.db[0];
    intrinsic.focalLength.y = intrinsicMatrix->data.db[4];
    intrinsic.center.x = intrinsicMatrix->data.db[2];
    intrinsic.center.y = intrinsicMatrix->data.db[5];
    intrinsic.k1 = distortionCoeffs->data.db[0];
    intrinsic.k2 = distortionCoeffs->data.db[1];
    intrinsic.p1 = distortionCoeffs->data.db[2];
    intrinsic.p2 = distortionCoeffs->data.db[3];
  }


  /************************************************************************
   * Main calibration functions
   ************************************************************************/
  void calibrateCamera(const QList<PiiMatrix<double> >& worldPoints,
                       const QList<PiiMatrix<double> >& imagePoints,
                       CameraParameters& intrinsic,
                       QList<RelativePosition>* extrinsic,
                       CalibrationOptions options)

  {
    if (worldPoints.size() == 0 || imagePoints.size() == 0 ||
        (worldPoints.size() != 1 && worldPoints.size() != imagePoints.size()))
      PII_THROW(PiiCalibrationException,
                tr("Cannot calibrate with non-matching number of views. World views: %1. Image views: %2")
                .arg(worldPoints.size()).arg(imagePoints.size()));

    if (intrinsic.center.x == 0 || intrinsic.center.y == 0)
      PII_THROW(PiiCalibrationException,
                tr("An initial estimate of camera principal point is required."));

    // Initialize and check OpenCv optimization routine flags
    int flags = 0;
    // Estimation of intrinsic parameters won't be done -> must have
    // valid intial values for the parameters.
    if (!(options & EstimateIntrinsic))
      {
        if (intrinsic.focalLength.x <= 0 || intrinsic.focalLength.y <= 0)
          PII_THROW(PiiCalibrationException,
                    tr("Focal lengths must be positive"));
        flags |= CV_CALIB_USE_INTRINSIC_GUESS;
      }
    if (options & FixPrincipalPoint)
      flags |= CV_CALIB_FIX_PRINCIPAL_POINT;
    if (options & FixAspectRatio)
      flags |= CV_CALIB_FIX_ASPECT_RATIO;
    if (options & NoTangentialDistortion)
      flags |= CV_CALIB_ZERO_TANGENT_DIST;


    int viewCount = imagePoints.size();
    int totalPoints = 0;
    // Create a 1-by-N one-channel int matrix
    CvMat* pCounts = cvCreateMat(1, imagePoints.size(), CV_32SC1);

    // Calculate the total number of points and check everything is OK
    // for calibration.
    for (int view=0; view < viewCount; ++view)
      {
        int points = imagePoints[view].rows();
        // Check that there are enough points for calibration
        if (points < 4)
          {
            cvReleaseMat(&pCounts);
            PII_THROW(PiiCalibrationException,
                      tr("The number of calibration points per view must be at least four. View %1 has only %2.").arg(view).arg(points));
          }
        const PiiMatrix<double>& currentWorldPoints = worldPoints.size() == 1 ? worldPoints[0] : worldPoints[view];
        int wPoints = currentWorldPoints.rows();
        // Check that the number of points match
        if (wPoints != points)
          {
            cvReleaseMat(&pCounts);
            PII_THROW(PiiCalibrationException,
                      tr("The number of calibration points per view must match. View %1 has %2 world points and %3 image points.")
                      .arg(view).arg(wPoints).arg(points));
          }
        // Check input dimensions
        if (currentWorldPoints.columns() != 3 || imagePoints[view].columns() != 2)
          {
            cvReleaseMat(&pCounts);
            PII_THROW(PiiCalibrationException,
                      tr("Incorrect point dimensions. View %1 has a %2-dimensional world space and a %3-dimensional image space.")
                      .arg(view).arg(currentWorldPoints.columns()).arg(imagePoints[view].columns()));
          }

        totalPoints += points;
        pCounts->data.i[view] = points;
      }

    // Collect all world points here (64-bit double coordinates)
    CvMat* pWorldPoints = cvCreateMat(totalPoints, 3, CV_64FC1);
    CvMat* pImagePoints = cvCreateMat(totalPoints, 2, CV_64FC1);
    
    // Go through all points in all views and store to the OpenCv
    // matrices.
    for (int view = 0, pointIndex = 0; view < viewCount; ++view)
      {
        const PiiMatrix<double>& currentWorldPoints = worldPoints.size() == 1 ? worldPoints[0] : worldPoints[view];
        for (int r=0; r<imagePoints[view].rows(); ++r, ++pointIndex)
          {
            // Copy world coordinates
            memcpy(pWorldPoints->data.db + pointIndex * 3, currentWorldPoints[r], 3*sizeof(double));
            // Copy image coordinates
            memcpy(pImagePoints->data.db + pointIndex * 2, imagePoints[view][r], 2*sizeof(double));
          }
      }

    // Calculate image size from initial principal point. 1.1 is used
    // instead of 1.0 to ensure rounding errors don't drop the numbers
    // one pixel low.
    CvSize imageSize = { int((intrinsic.center.x*2 + 1.1)), int((intrinsic.center.y*2 + 1.1)) };

    // Convert intrinsics to OpenCv's format
    CvMat* pIntrinsicMatrix = createIntrinsicMatrix(intrinsic);
    CvMat* pDistortionCoeffs = createDistortionCoeffs(intrinsic);

    // Extrinsic parameters for each view are stored in these double matrices.
    CvMat* pRotationVectors = PiiOpenCv::cvMat<double>(viewCount, 3);
    CvMat* pTranslationVectors = PiiOpenCv::cvMat<double>(viewCount, 3);

    cvCalibrateCamera2(pWorldPoints, pImagePoints,
                       pCounts, imageSize,
                       pIntrinsicMatrix, pDistortionCoeffs,
                       pRotationVectors, pTranslationVectors,
                       flags);

    // Store intrinsic parameters back to the structure
    storeCameraParameters(intrinsic, pIntrinsicMatrix, pDistortionCoeffs);

    // Store extrinsic parameters if needed.
    if (extrinsic != 0)
      {
        extrinsic->clear();
        for (int view = 0; view < viewCount; ++view)
          {
            // Initialize the rotation and translation vectors
            // directly from the corresponding matrix rows.
            extrinsic->append(createRelativePosition(pRotationVectors,
                                                     pTranslationVectors,
                                                     view));
          }
      }

    // Destroy all temporary matrices
    cvReleaseMat(&pCounts);
    cvReleaseMat(&pWorldPoints);
    cvReleaseMat(&pImagePoints);
    cvReleaseMat(&pIntrinsicMatrix);
    cvReleaseMat(&pDistortionCoeffs);
    cvReleaseMat(&pRotationVectors);
    cvReleaseMat(&pTranslationVectors);
  }

  RelativePosition calculateCameraPosition(const PiiMatrix<double>& worldPoints,
                                           const PiiMatrix<double>& imagePoints,
                                           const CameraParameters& intrinsic)

  {
    if (worldPoints.rows() < 4)
      PII_THROW(PiiCalibrationException,
                tr("The number of calibration points must be at least four."));
    if (worldPoints.rows() != imagePoints.rows())
      PII_THROW(PiiCalibrationException,
                tr("The number of world and image points must match. %1 world and %2 image points were given.")
                .arg(worldPoints.rows()).arg(imagePoints.rows()));
    if (worldPoints.columns() != 3 || imagePoints.columns() != 2)
      PII_THROW(PiiCalibrationException,
                tr("World points must be 3-dimensional (was %1) and image points 2-dimensional (was %2).")
                .arg(worldPoints.columns()).arg(imagePoints.columns()));

    CvMat* pIntrinsicMatrix = createIntrinsicMatrix(intrinsic);
    CvMat* pDistortionCoeffs = createDistortionCoeffs(intrinsic);

    // Initialize empty matrices to hold the extrinsic parameters
    CvMat* pRotation = PiiOpenCv::cvMat<double>(1, 3);
    CvMat* pTranslation = PiiOpenCv::cvMat<double>(1, 3);

    // Call OpenCv's implementation...
    cvFindExtrinsicCameraParams2(PiiCvMat<double>(worldPoints),
                                 PiiCvMat<double>(imagePoints),
                                 pIntrinsicMatrix,
                                 pDistortionCoeffs,
                                 pRotation,
                                 pTranslation);

    RelativePosition extrinsic = createRelativePosition(pRotation, pTranslation);

    // Release temprorary matrices
    cvReleaseMat(&pIntrinsicMatrix);
    cvReleaseMat(&pDistortionCoeffs);
    cvReleaseMat(&pRotation);
    cvReleaseMat(&pTranslation);

    return extrinsic;
  }

#endif // PII_NO_OPENCV

  RelativePosition calculateRelativePosition(const RelativePosition& camera1,
                                             const RelativePosition& camera2)
  {
    PiiMatrix<double> r1 = camera1.rotationMatrix();
    PiiMatrix<double> r2 = camera2.rotationMatrix();
    PiiMatrix<double> t1 = camera1.translationMatrix();
    PiiMatrix<double> t2 = camera2.translationMatrix();
        
    // Rs = R2*R1^-1 = R2*R1^T, because rotation matrices are orthogonal
    PiiMatrix<double> rs(r2 * Pii::transpose(r1));
    PiiMatrix<double> ts(t2 - rs * t1);
    
    return RelativePosition(rs, ts);
  }

  PiiMatrix<double> rotationVectorToMatrix(const PiiVector<double,3>& rotation)
  {
    // Create a copy of the rotation vector.
    PiiMatrix<double> vector(1,3, rotation.values);
    // The rotation angle is encoded into the length of the vector
    double theta = Pii::norm(vector, 2);
    if (Pii::almostEqualRel(theta,0.0, Pii::Numeric<double>::tolerance()))
      return PiiMatrix<double>::identity(3);
    // Normalize the rotation vector
    vector /= theta;

    // Create an antisymmetric matrix according to Rodrigues' formula.
    PiiMatrix<double> M(3, 3,
                        0.0       , -vector(2),  vector(1), //  0 -z  y
                        vector(2) , 0.0       , -vector(0), //  z  0 -x
                        -vector(1), vector(0) , 0.0      ); // -y  x  0
    return Pii::matrix(PiiMatrix<double>::identity(3) + M*sin(theta) + M*M * (1.0 - cos(theta)));
  }

  PiiVector<double,3> rotationMatrixToVector(const PiiMatrix<double>& matrix)
  {
    PiiVector<double,3> result;
    if (matrix.rows() != 3 || matrix.columns() != 3)
      return result;

    // (R - R^T) / 2 = M sin(theta) 
    PiiMatrix<double> M((matrix - Pii::transpose(matrix)) * 0.5);
    /* M is now
       [  0 -z  y ]
       [  z  0 -x ] sin(theta),
       [ -y  x  0 ]
       where (x,y,z) is a unit vector.
    */
    // Pick coordinates from the matrix
    PiiMatrix<double> vector(1,3, M(2,1), M(0,2), M(1,0));

    // Find out what components of the rotation vector are non-zero.
    int typeMask = 0;
    for (int i=0; i<3; i++)
      if (vector(i) != 0)
        typeMask |= 1 << i;

    /* If the rotation vector has only one non-zero component, we can
       pick the sine and cosine directly from the rotation matrix. The
       rotation matrices around x, y, and z axis are:
       
             x            y            z
             
       [  1  0  0 ] [  c  0  s ] [  c -s  0 ]
       [  0  c -s ] [  0  1  0 ] [  s  c  0 ]
       [  0  s  c ] [ -s  0  c ] [  0  0  1 ]

       s = sin(theta), c = cos(theta)
     */

    /* 0 = all components zero
       1 = only x non-zero
       2 = only y non-zero
       4 = only z non-zero
       3 = x and y non-zero
       5 = x and z non-zero
       6 = y and z non-zero
       7 = all non-zero
    */
    if (typeMask == 0)
      return PiiVector<double,3>(vector.row(0));
    
    // The lenght of the rotation vector is now equal to sin(theta)
    double sinTheta = Pii::norm(vector,2);
    vector /= sinTheta; // normalize to unit length
    double cosTheta = 0;

    switch (typeMask)
      {
      case 1:
        cosTheta = matrix(1,1);
        break;
      case 2:
        cosTheta = matrix(0,0);
        break;
      case 4:
        cosTheta = matrix(0,0);
        break;
      default:
        {
          // (R + R^T) / 2 = I + M^2(1-cos(theta))
          // (R + R^T) / 2 - I = M^2*(1-cos(theta))I
          PiiMatrix<double> M2((matrix + Pii::transpose(matrix)) * 0.5 - PiiMatrix<double>::identity(3));
          /* M2 is now
             [ -y^2-z^2    xy       xz    ]
             [    xy    -x^2-z^2    yz    ] (1-cos(theta))
             [    xz       yz    -x^2-y^2 ]
          */
          switch(typeMask)
            {
            case 7:
            case 3:
              cosTheta = M2(0,1) / (vector(0) * vector(1));
              break;
            case 5:
              cosTheta = M2(0,2) / (vector(0) * vector(2));
              break;
            case 6:
              cosTheta = M2(1,2) / (vector(1) * vector(2));
              break;
            }
          cosTheta = 1.0 - cosTheta;
        }
      }
    
    double theta = atan2(sinTheta, cosTheta);
    if (theta < 0) theta += M_PI;

    /*if (typeMask != 7)
      {
        qDebug("TypeMask = %d\n%lf %lf %lf\n%lf %lf %lf\n%lf %lf %lf",
               typeMask,
               matrix(0,0), matrix(0,1), matrix(0,2),
               matrix(1,0), matrix(1,1), matrix(1,2),
               matrix(2,0), matrix(2,1), matrix(2,2));
        qDebug("vector = (%lf %lf %lf)\nsinTheta = %lf, cosTheta = %lf, theta = %lf",
               vector(0), vector(1), vector(2),
               sinTheta, cosTheta, theta);
               }*/
    // Denormalize vector
    vector *= theta;
    return PiiVector<double,3>(vector.row(0));
  }

  static inline void normalizedToDistorted(const CameraParameters& intrinsic,
                                           double x, double y,
                                           double *newX, double *newY)
  {
    double x2 = x*x, y2 = y*y, xy = x*y, r2 = x2 + y2;

    // Radial distortion is approximated up to 4th order
    double radialDistortion = 1.0 + intrinsic.k1 * r2 + intrinsic.k2 * r2 * r2;
    // Tangential distortion model...
    double tangentialX = 2*intrinsic.p1*xy + intrinsic.p2*(r2 + 2*x2);
    double tangentialY = intrinsic.p1*(r2 + 2*y2) + 2*intrinsic.p2*xy;
    
    // Apply the distortion model
    *newX = x*radialDistortion + tangentialX;
    *newY = y*radialDistortion + tangentialY;
  }

  class UndistortFunction : public PiiOptimization::ResidualFunction<double>
  {
  public:
    UndistortFunction(const CameraParameters& intrinsic, double x, double y) :
      _intrinsic(intrinsic),
      _dX(x), _dY(y)
    {}
    
    int functionCount() const
    {
      return 1;
    }

    void residualValues(const double* params, double* residuals) const
    {
      double dX, dY;
      normalizedToDistorted(_intrinsic, params[0], params[1], &dX, &dY);
      residuals[0] = Pii::square(dX - _dX) + Pii::square(dY - _dY);
    }

  private:
    const CameraParameters& _intrinsic;
    double _dX, _dY;
  };
  
  void undistort(const CameraParameters& intrinsic,
                 double x, double y,
                 double *newX, double *newY)
  {
    // Initial guess
    PiiMatrix<double> matParams(1, 2,
                                (x - intrinsic.center.x) / intrinsic.focalLength.x,
                                (y - intrinsic.center.y) / intrinsic.focalLength.y);

    UndistortFunction func(intrinsic, x, y);
    PiiMatrix<double> matResult(PiiOptimization::lmMinimize(&func, matParams));
    *newX = matResult(0,0);
    *newY = matResult(0,1);
    /*
    double radialDistortion, tangentialX, tangentialY, r2, x2, y2, xy, tmpX, tmpY, xDiff, yDiff;
    
    // First translate to the principal point and divide out the
    // focal length.
    x = (x - intrinsic.center.x)/intrinsic.focalLength.x;
    y = (y - intrinsic.center.y)/intrinsic.focalLength.y;

    // Normalized but still distorted location of the point
    PiiPoint<double> distNorm(x, y);

    // Iteratively unapply the lens distortion model
    for (int iteration=20; iteration--; )
      {
        x2 = x*x, y2 = y*y, xy = x*y, r2 = x2 + y2;
        radialDistortion = 1.0 + intrinsic.k1 * r2 + intrinsic.k2 * r2 * r2;
        tangentialX = 2*intrinsic.p1*xy + intrinsic.p2*(r2 + 2*x2);
        tangentialY = intrinsic.p1*(r2 + 2*y2) + 2*intrinsic.p2*xy;

        // Unapply the distortion model
        tmpX = (distNorm.x - tangentialX)/radialDistortion;
        tmpY = (distNorm.y - tangentialY)/radialDistortion;

        // See how much we are still moving
        xDiff = Pii::abs(x - tmpX);
        yDiff = Pii::abs(y - tmpY);
        x = tmpX;
        y = tmpY;

        if (xDiff < 1e-5 && yDiff < 1e-5)
          break;

        // Now we have a better estimate of the undistorted
        // location. On the next round the distortion will be
        // calculated based on the updated location.
      }
    *newX = x;
    *newY = y;
    */
  }

  PiiMatrix<double> undistort(const PiiMatrix<double>& distorted,
                              const CameraParameters& intrinsic,
                              double zValue)

  {
    if (distorted.columns() != 2)
      PII_THROW(PiiCalibrationException,
                tr("Distorted coordinates must be represented by a N-by-2 matrix. %1-by-%2 was given.")
                .arg(distorted.rows()).arg(distorted.columns()));

    // Initial guess is equal to the distrorted points
    PiiMatrix<double> matResult(PiiMatrix<double>::uninitialized(distorted.rows(), Pii::isNan(zValue) ? 2 : 3));
    if (!Pii::isNan(zValue))
      Pii::fill(matResult.columnBegin(2), matResult.columnEnd(2), zValue);
    
    for (int row=0; row<matResult.rows(); ++row)
      {
        const double* pSource = distorted[row];
        double* pTarget = matResult[row];
        undistort(intrinsic, pSource[0], pSource[1], pTarget, pTarget+1);
      }
    return matResult;
  }
    
  PiiMatrix<double> cameraToWorldCoordinates(const PiiMatrix<double>& coordinatePoints,
                                             const RelativePosition& extrinsic)
  {
    // X_c = [x_c, y_c, z_c]^T  =  coordinates in camera reference frame 
    // X_w = [x_w, y_w, z_w]^T = coordinates in world reference frame
    // R is cameras rotation matrix
    // T is cameras translation matrix --> [t_x,t_y,t_z]^T
    // Now  X_c = RX_w + T --> X_w = R^-1(X_c - T)
    
    
    PiiMatrix<double> worldCoordinates(coordinatePoints.rows(),3);
    PiiMatrix<double> invR(Pii::transpose(extrinsic.rotationMatrix()));

    PiiMatrix<double> tR = extrinsic.translationMatrix();
   
    for(int i = coordinatePoints.rows(); i--;)
      {
        double x = coordinatePoints(i,0) - tR(0,0);
        double y = coordinatePoints(i,1) - tR(1,0);
        double z = coordinatePoints(i,2) - tR(2,0);
       
        worldCoordinates(i,0) = invR(0,0)*x + invR(0,1)*y+invR(0,2)*z;
        worldCoordinates(i,1) = invR(1,0)*x + invR(1,1)*y+invR(1,2)*z;
        worldCoordinates(i,2) = invR(2,0)*x + invR(2,1)*y+invR(2,2)*z;
      }

    return worldCoordinates;
  }

  PiiMatrix<double> worldToCameraCoordinates(const PiiMatrix<double>& worldPoints,
                                             const RelativePosition& extrinsic)
  {
    //Now X_c = R*X_w+T , where R is rotation matrix and T is
    //translation matrix.
    //X_c = [x_c,y_c,z_c]^T is point in camera coordinates,
    //X_w = [x_w,y_w,z_w]^T is point in world coordinates  

    PiiMatrix<double> cameraCoordinates(worldPoints.rows(), worldPoints.columns());
    PiiMatrix<double> matR = extrinsic.rotationMatrix();
    
    for (int r = worldPoints.rows(); r--; )
      {
        double x = worldPoints(r,0);
        double y = worldPoints(r,1);
        double z = worldPoints(r,2);

        cameraCoordinates(r,0) = matR(0,0)*x + matR(0,1)*y + matR(0,2)*z + extrinsic.translation[0];
        cameraCoordinates(r,1) = matR(1,0)*x + matR(1,1)*y + matR(1,2)*z + extrinsic.translation[1];
        cameraCoordinates(r,2) = matR(2,0)*x + matR(2,1)*y + matR(2,2)*z + extrinsic.translation[2];
      }

    return cameraCoordinates;
  }
  
  void normalizedToPixelCoordinates(const CameraParameters& intrinsic,
                                    double x, double y,
                                    double *newX, double *newY)
  {
    normalizedToDistorted(intrinsic, x, y, newX, newY);

    // Multiply with focal length and add translation of principal point
    *newX = intrinsic.focalLength.x * *newX + intrinsic.center.x;
    *newY = intrinsic.focalLength.y * *newY + intrinsic.center.y;
  }


  PiiMatrix<double> normalizedToPixelCoordinates(const PiiMatrix<double>& points,
                                                 const CameraParameters& intrinsic)
  {

    PiiMatrix<double> matResult(PiiMatrix<double>::uninitialized(points.rows(), points.columns()));
    for (int row=0; row < points.rows(); ++row)
      {
        const double* pSource = points[row];
        double* pTarget = matResult[row];
        normalizedToPixelCoordinates(intrinsic, pSource[0], pSource[1], pTarget, pTarget+1);
      }
    return matResult;
  }

  PiiMatrix<double> cameraToPixelCoordinates(const PiiMatrix<double>& points,
                                             const CameraParameters& intrinsic)
  {
    return normalizedToPixelCoordinates(perspectiveProjection(points),intrinsic);
  }

  PiiMatrix<double> worldToPixelCoordinates(const PiiMatrix<double>& points,
                                            const RelativePosition& extrinsic,
                                            const CameraParameters& intrinsic)
  {
    return cameraToPixelCoordinates(worldToCameraCoordinates(points, extrinsic), intrinsic);
  }
  
  PiiMatrix<double> perspectiveProjection(const PiiMatrix<double>& points,
                                          double zValue)
  {
    PiiMatrix<double> result(points.rows(), Pii::isNan(zValue) ? 2 : 3);
    for (int r=points.rows(); r--; )
      {
        result(r,0) = points(r,0)/points(r,2);
        result(r,1) = points(r,1)/points(r,2);
      }
    
    // Fill with a constant value to z
    if (!Pii::isNan(zValue))
      for (int r=points.rows(); r--; )
        result(r,2) = zValue;

    return result;
  }

  /************************************************************************
   * RelativePosition functions
   ************************************************************************/
  RelativePosition::RelativePosition(const PiiMatrix<double>& rot,
                                     const PiiMatrix<double>& trans) :
    rotation(rotationMatrixToVector(rot))
  {
    // Copy translation vector elements. Works for row and column
    // matrices.
    for (int i=3; i--; )
      translation[i] = trans(i);
  }


  PiiMatrix<double> RelativePosition::rotationMatrix() const
  {
    return rotationVectorToMatrix(rotation);
  }

  PiiMatrix<double> RelativePosition::translationMatrix() const
  {
    return PiiMatrix<double>(3, 1, translation.values);
  }

  /************************************************************************
   * Remapping functions
   ************************************************************************/
  
  template <class UnaryFunction>
  PiiMatrix<PiiPoint<typename UnaryFunction::result_type > > undistortMap(int rows, int columns,
                                                                          const CameraParameters& intrinsic,
                                                                          const UnaryFunction& func)
  {
    typedef PiiPoint<typename UnaryFunction::result_type> PointType;
    
    // Four corners in (distorted) image coordinates
    PiiMatrix<double> matCorners(4,2,
                                 0.0, 0.0,
                                 0.0, double(rows-1),
                                 double(columns-1), double(rows-1),
                                 double(columns-1), 0.0);

    // The coordinates of the corners in undistorted normalized coordinates
    PiiMatrix<double> matUndistortedCorners(undistort(matCorners, intrinsic));
    // Maximum extents
    PiiMatrix<double> matMinCorners(Pii::min(matUndistortedCorners, Pii::Vertically));
    PiiMatrix<double> matMaxCorners(Pii::max(matUndistortedCorners, Pii::Vertically));

    double dXStep = (matMaxCorners(0) - matMinCorners(0)) / columns;
    double dYStep = (matMaxCorners(1) - matMinCorners(1)) / rows;

    //Pii::matlabPrint(std::cout, matMinCorners);
    //Pii::matlabPrint(std::cout, matMaxCorners);

    //Pii::matlabPrint(std::cout, matUndistortedCorners);

    //std::cout << dXStep << " " << dYStep << "\n";
    //std::cout.flush();
    

    // Result image is the same size as the input
    PiiMatrix<PointType> matResult(rows, columns);
    double dY = matMinCorners(1);
    double dNewX, dNewY;
    for (int r=0; r<rows; ++r, dY += dYStep)
      {
        double dX = matMinCorners(0);
        //std::cout << "(" << r << ") -> (" << dY << "," << dX << ")\n";
        PointType* pResultRow = matResult[r];
        for (int c=0; c<columns; ++c, dX += dXStep)
          {
            // Map normalized coordinates to distorted image
            // coordinates
            normalizedToPixelCoordinates(intrinsic, dX, dY, &dNewX, &dNewY);
            pResultRow[c] = PointType(func(dNewX), func(dNewY));
          }
      }
    return matResult;
  }
  
  PiiImage::DoubleCoordinateMap undistortMap(int rows, int columns,
                                             const CameraParameters& intrinsic)
  {
    return undistortMap(rows, columns, intrinsic, Pii::Identity<double>());
  }

  PiiImage::IntCoordinateMap undistortMapInt(int rows, int columns,
                                             const CameraParameters& intrinsic)
  {
    return undistortMap(rows, columns, intrinsic, Pii::Round<int>());
  }
}
