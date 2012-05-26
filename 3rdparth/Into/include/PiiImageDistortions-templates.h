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
# error "Never use <PiiImageDistortions-templates.h> directly; include <PiiImageDistortions.h> instead."
#endif

#include <PiiMath.h>

namespace PiiImage
{
  template <class T> PiiMatrix<T> unwarpCylinder(const PiiMatrix<T>& warpedImage,
                                                 double focalLength,
                                                 double center,
                                                 double *cameraDistance,
                                                 double *radius,
                                                 double *sectorAngle,
                                                 double *startAngle)
  {
    // Real-valued version of T (works with color types)
    typedef typename Pii::ToFloatingPoint<T>::Type Real;
    // Real-valued version of color channel type
    typedef typename Pii::ToFloatingPoint<T>::PrimitiveType RealScalar;

    int iLastPixel = warpedImage.columns() - 1;
    
    if (Pii::isNan(center))
      center = 0.5 * iLastPixel;

    double
      dXp1 = iLastPixel - center,
      dXp2 = center,
      dHp1 = Pii::hypotenuse(focalLength, dXp1), // hypotenuse in pixels
      dHp2 = Pii::hypotenuse(focalLength, dXp2),
      dXw1, dXw2, dCw, dR, dZ;

    // No camera distance given -> must solve based on R
    if (cameraDistance == 0 || *cameraDistance == 0)
      {
        if (radius != 0 && *radius != 0)
          dR = *radius;
        // If neither distance or radius is given, must guess
        else
          dR = 100;

        dXw1 = dR * (dHp1 + dHp2) / (focalLength * (1 + dXp2/dXp1));
        dXw2 = dXw1 * dXp2 / dXp1;
        dCw = dR * dHp2/focalLength - dXw2;
        dZ = focalLength * dXw1 / dXp1;
      }
    // Camera distance is given -> solve R
    else
      {
        dZ = *cameraDistance;
        // pixels to world at camera distance
        double dScale = dZ / focalLength;
        dXw1 = dScale * dXp1;
        dXw2 = dScale * dXp2;
        dCw = (dHp2*dXw1 - dHp1*dXw2) / (dHp1 + dHp2);
        dR = focalLength * (dXw1 - dCw) / dHp1;
      }

    double
      // The boundaries of the cylinder are seen at these angles. The
      // closer the camera is to the cylinder, the less we see. The
      // angles approach zero as camera distance approaches infinity.
      dAlpha1 = Pii::acos(dR / (dXw1 - dCw)),
      dAlpha2 = M_PI - Pii::acos(dR / (dXw2 + dCw)),
      // The angle of the visible sector in radians
      dSectorAngle = dAlpha2 - dAlpha1,
      // Shortest distance to the surface
      dSurfaceDistance = Pii::hypotenuse(dZ, dCw) - dR;

    //qDebug("Xw1 = %lf, Xw2 = %lf, Z = %lf, dSurfaceDistance = %lf", dXw1, dXw2, dZ, dSurfaceDistance);
    //qDebug("center = %lf, Cw = %lf, R = %lf, a1 = %lf, a2 = %lf", center, dCw, dR, Pii::radToDeg(dAlpha1), Pii::radToDeg(dAlpha2));
    
    // If sector is limited, calculate new boundaries
    if (sectorAngle != 0 && *sectorAngle > 0 && *sectorAngle < dSectorAngle)
      {
        double dCorrection = (dSectorAngle - *sectorAngle) / 2;
        dSectorAngle = *sectorAngle;
        dAlpha1 += dCorrection;
        dAlpha2 -= dCorrection;
      }
    
    // Straightened length = sector angle * r, projected to image plane
    int iStraightenedLength = Pii::round<int>(dSectorAngle * dR / dSurfaceDistance * focalLength);
    if (iStraightenedLength < 2)
      return PiiMatrix<T>(warpedImage.rows(), 1);
    // Each pixel in target image represents this many radians.
    double dAngleStep = dSectorAngle / (iStraightenedLength-1);

    PiiMatrix<T> matResult(PiiMatrix<T>::uninitialized(warpedImage.rows(), iStraightenedLength));

    /*qDebug("dAlpha1 = %lf, iStraightenedLength = %d, dAngleStep = %lf\n"
           "cols = %d, dPixelX0 = %lf, dPixelXN = %lf",
           dAlpha1 * 180 / M_PI, iStraightenedLength, dAngleStep * 180 / M_PI,
           warpedImage.columns(),
           (dR * Pii::cos(dAlpha1) + dCw) / (dZ - dR * Pii::sin(dAlpha1)) * focalLength,
           (dR * Pii::cos(dAlpha2) + dCw) / (dZ - dR *
           Pii::sin(dAlpha2)) * focalLength);
    */

    for (int i=0; i<iStraightenedLength; ++i)
      {
        // Parametric equation of the surface:
        // (x, z) = (r * cos(alpha) + cw, z - r * sin(alpha))
        double dAlpha = dAlpha2 - i * dAngleStep;
        double dCosAlpha = Pii::cos(dAlpha);
        double dSinAlpha = Pii::sin(dAlpha);

        // Perspective projection, scaling and translation
        double dPixelX = (dR * dCosAlpha + dCw) / (dZ - dR * dSinAlpha) * focalLength + center;
        if (dPixelX < 0)
          dPixelX = 0;
        else if (dPixelX > iLastPixel)
          dPixelX = iLastPixel;

        // Floor to nearest int
        int iPixelX = int(dPixelX);
        // Take fraction
        RealScalar dF1(dPixelX - iPixelX);

        // Linear interpolation
        if (dF1 > 0)
          {
            RealScalar dF2(1.0 - dF1);
            for (int r=0; r<warpedImage.rows(); ++r)
              matResult(r,i) = T(Real(warpedImage(r,iPixelX)) * dF2 + Real(warpedImage(r,iPixelX+1)) * dF1);
          }
        else
          {
            for (int r=0; r<warpedImage.rows(); ++r)
              matResult(r,i) = warpedImage(r,iPixelX);
          }
        
        //qDebug("i = %d, dAlpha = %lf, cos = %lf, sin = %lf, dXp = %lf", i, dAlpha * 180 / M_PI, dCosAlpha, dSinAlpha, dPixelX);
      }

    if (radius != 0)
      *radius = dR;

    if (cameraDistance != 0)
      *cameraDistance = dZ;
    
    if (sectorAngle != 0)
      *sectorAngle = dSectorAngle;

    if (startAngle != 0)
      *startAngle = dAlpha1;
    
    return matResult;
  }
}
