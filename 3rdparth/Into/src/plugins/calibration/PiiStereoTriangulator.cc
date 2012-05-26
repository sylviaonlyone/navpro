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

#include "PiiStereoTriangulator.h"
#include <QCoreApplication>

QString PiiStereoTriangulator::tr(const char* message)
{
  return QCoreApplication::translate("PiiStereoTriangulator", message);
}

PiiStereoTriangulator::Data::Data()
{
}

PiiStereoTriangulator::PiiStereoTriangulator() : d(new Data)
{
}

PiiStereoTriangulator::~PiiStereoTriangulator()
{
  delete d;
}

void PiiStereoTriangulator::addCamera(const PiiCalibration::CameraParameters& intrinsic,
                                      const PiiCalibration::RelativePosition& extrinsic)
{
  d->lstCameraParameters << intrinsic;
  // Calculate relative position wrt all added cameras
  // The lst[0][0] is the relative position of camera 0 wrt the world
  // coordinate system, lst[0][1] is the relative position of camera 1
  // wrt camera 0, lst[1][0] is the relative position of camera 1 wrt
  // world, and so on.
  for (int i=0; i<d->lstRelativePositions.size(); i++)
    d->lstRelativePositions[i] << PiiCalibration::calculateRelativePosition(d->lstRelativePositions[i][0], extrinsic);
  d->lstRelativePositions << (QList<PiiCalibration::RelativePosition>() << extrinsic);
}

PiiMatrix<double> PiiStereoTriangulator::calculate3DPoints(const QList<PiiMatrix<double> >& imagePoints)
{
  if (imagePoints.size() != d->lstCameraParameters.size())
    PII_THROW(PiiCalibrationException,
              tr("Measurement points must be provided for all added cameras. Expected %1, got %2.")
              .arg(d->lstCameraParameters.size()).arg(imagePoints.size()));
  if (d->lstCameraParameters.size() == 0)
    return PiiMatrix<double>(0,3);

  int pointsPerView = imagePoints[0].rows();
  
  // Unapply lens distortions and save normalized points in this list
  QList<PiiMatrix<double> > lstNormalized;
  for (int i=0; i<imagePoints.size(); ++i)
    {
      if (imagePoints[i].rows() != pointsPerView)
        PII_THROW(PiiCalibrationException,
                  tr("Each view must have the same number of measurement points. View 0 has %1, view %2 has %3")
                  .arg(pointsPerView).arg(i).arg(imagePoints[i].rows()));
      if (imagePoints[i].columns() != 2)
        PII_THROW(PiiCalibrationException,
                  tr("Measurement points must be 2-dimensional. View %1 is %2-dimensional.")
                  .arg(i).arg(imagePoints[i].columns()));

      // Transform to normalized image coordinates and add a fixed z coordinate
      lstNormalized << PiiCalibration::undistort(imagePoints[i], d->lstCameraParameters[i], 1.0);
    }

  // Initialize a zero matrix
  PiiMatrix<double> matResult(pointsPerView, 3);
  matResult = NAN;
  PiiMatrix<int> matValidPairs(1, pointsPerView);

  int camCount = d->lstCameraParameters.size();
  // Go through all possible camera pairings
  for (int c1=0; c1<camCount; ++c1)
    {
      for (int c2=c1+1; c2<camCount; ++c2)
        {
          // Triangulated points are in c1's reference frame
          PiiMatrix<double> mat3DPoints = triangulate(c1, c2, lstNormalized[c1], lstNormalized[c2]);
          // If c1 is not the first camera, we need to convert the
          // result to its coordinate frame to ensure the summed
          // points are all in the same coordinate system.
          if (c1 != 0)
            mat3DPoints = PiiCalibration::cameraToWorldCoordinates(mat3DPoints, d->lstRelativePositions[0][c1]);

          // Add successfully triangulated points to the result matrix
          for (int r=0; r<mat3DPoints.rows(); ++r)
            {
              if (!Pii::isNan(mat3DPoints(r,0) + mat3DPoints(r,1) + mat3DPoints(r,2)))
                {
                  // Copy the first point (NAN + x = NAN)
                  if (matValidPairs(r) == 0)
                    matResult(r,0,1,-1) << mat3DPoints(r,0,1,-1);
                  else
                    matResult(r,0,1,-1) += mat3DPoints(r,0,1,-1);
                  ++matValidPairs(r);
                }
            }
        }
    }
  // Divide by the total number of valid pairings to get the mean
  // of all pairwise estimates.
  if (camCount > 2)
    {
      for (int r=0; r<matResult.rows(); ++r)
        {
          if (matValidPairs(r) > 1)
            matResult(r,0,1,-1) /= matValidPairs(r);
        }
    }
  return matResult;
}

#define dot(a,b) a(0)*b(0)+a(1)*b(1)+a(2)*b(2)

PiiMatrix<double> PiiStereoTriangulator::triangulate(int camera1, int camera2,
                                                     const PiiMatrix<double>& normalizedA,
                                                     const PiiMatrix<double>& normalizedB)
{
  // Rotation matrix corresponding to the rigid motion between camera1
  // and camera2. (X2 = R * X1 + T)
  PiiMatrix<double> matRot = PiiCalibration::rotationVectorToMatrix(d->lstRelativePositions[camera1][camera2-camera1].rotation);
  PiiMatrix<double> matInvRot(Pii::transpose(matRot));
  PiiVector<double,3> vecTrans = d->lstRelativePositions[camera1][camera2-camera1].translation;

  // Triangulation of the rays in 3D space. Matlab code ported from
  // camera the calibration toolbox by Jean-Yves Bouguet. Don't ask me.

  // Result matrix is the same size as normalizedA.
  PiiMatrix<double> result(PiiMatrix<double>::uninitialized(normalizedA.rows(), normalizedA.columns()));
  
  for (int row = 0; row < normalizedA.rows(); row++)
    {
      // Treat matrix rows as 3-dimensional vectors
      PiiVector<double,3> ptA = normalizedA.rowAs<PiiVector<double,3> >(row);
      PiiVector<double,3> ptB = normalizedB.rowAs<PiiVector<double,3> >(row);

      // U = A's normalized measurent in B's coordinate system
      PiiVector<double,3> ptU;
      Pii::multiply(matRot, ptA.begin(), ptU.begin());

      double norm2A = dot(ptA,ptA);
      double norm2B = dot(ptB,ptB);

      double DD = norm2A*norm2B - Pii::square(dot(ptU,ptB));

      double dot_uT = dot(ptU,vecTrans);

      double dot_normalizedBT = dot(ptB,vecTrans);
      double dot_normalizedBu = dot(ptU,ptB);

      double NN1 = dot_normalizedBu * dot_normalizedBT - norm2B * dot_uT;
      double NN2 = norm2A * dot_normalizedBT - dot_uT * dot_normalizedBu;

      double zA = NN1 / DD;
      double zB = NN2 / DD;      

      PiiVector<double,3> X1 = ptA * zA;
      PiiVector<double,3> X2;
      Pii::multiply(matInvRot, (ptB * zB - vecTrans).begin(), X2.begin());

      // Coordinates in camera1's reference frame. (We take the mean
      // of two estimates.)
      result.rowAs<PiiVector<double,3> >(row) = (X1 + X2) * 0.5;
    }
  return result;
}
#undef dot
