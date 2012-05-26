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

#ifndef _PIITRANSFORMS_H
# error "Never use <PiiTransforms-templates.h> directly; include <PiiTransforms.h> instead."
#endif

#include <PiiMatrixUtil.h>
#include <PiiImage.h>

namespace PiiTransforms
{
  template <class T, class U, class UnaryOp>
  PiiMatrix<T> linearHough(const PiiMatrix<U>& img,
                           UnaryOp rule,
                           double angleResolution,
                           double distanceResolution,
                           int angleStart, int angleEnd,
                           int distanceStart, int distanceEnd)
  {
    int rows = img.rows();
    int cols = img.columns();
    
    double centerX = double(cols-1)/2.0; // the center point of X coordinates
    double centerY = double(rows-1)/2.0; // the center point of Y coordinates

    // Maximum distance from origin.
    double dMaxDistance = ::sqrt(centerX*centerX + centerY*centerY) / distanceResolution;

    distanceStart = Pii::round<int>(qBound(-dMaxDistance, double(distanceStart)/distanceResolution, dMaxDistance));
    distanceEnd = Pii::round<int>(qBound(-dMaxDistance, double(distanceEnd)/distanceResolution, dMaxDistance));

    int iDistances = distanceEnd - distanceStart + 1;
    int iAngles = Pii::round<int>((angleEnd - angleStart) / angleResolution);

    //qDebug("distances: %d -> %d (%d)", distanceStart, distanceEnd, iDistances);
    //qDebug("angles: %d -> %d (%d)", angleStart, angleEnd, iAngles);
    //qDebug("center: (%lf, %lf)", centerY, centerX);
    
    PiiMatrix<U> result(iDistances, iAngles);
    double* pCosAngleValues = new double[iAngles];
    double* pSinAngleValues = new double[iAngles];

    double dAngleConversion = M_PI / 180 * angleResolution;
    double dStartRadians = double(angleStart) / 180.0 * M_PI;

    // Look-up tables for sin and cos
    for (int omega=0; omega<iAngles; ++omega)
      {
        pSinAngleValues[omega] = sin(double(omega)*dAngleConversion + dStartRadians);
        pCosAngleValues[omega] = cos(double(omega)*dAngleConversion + dStartRadians);
      }
    
    // go through the input matrix
    for (int r=0; r<rows; ++r)
      {
        //qDebug("row %d", r);
        const U* row = img.row(r);
        for (int c=0; c<cols; ++c)
          {
            // Does this pixel match the rule?
            if (rule(row[c]))
              {
                //qDebug("(%d, %d) matches", r, c);
                double coordX = double(c) - centerX;
                double coordY = double(r) - centerY;
                for (int omega=0; omega<iAngles; ++omega)
                  {
                    // Calculate distance to origin
                    int d = Pii::round<int>((coordX*pCosAngleValues[omega] +
                                             coordY*pSinAngleValues[omega])/distanceResolution);
                    //qDebug("Accessing (%d, %d) of %d-by-%d", d-distanceStart, omega, iDistances, iAngles);
                    if (d >= distanceStart && d <= distanceEnd)
                      result(d - distanceStart, omega) += T(row[c]);
                  }
              }
          }
      }
    delete[] pSinAngleValues;
    delete[] pCosAngleValues;
    return result;
  }

  inline void addPixel(int /*magnitude*/, PiiMatrix<int>& result, double r, double c)
  {
    int iR = Pii::round<int>(r), iC = Pii::round<int>(c);
    if (iR >= 0 && iR < result.rows() && iC >=0 && iC < result.columns())
      ++result(iR, iC);
  }

  template <class T> inline void addPixel(T magnitude, PiiMatrix<T>& result, double r, double c,
                                          typename Pii::OnlyFloatingPoint<T>::Type = 0)
  {
    if (r >= 0 && r < result.rows() && c >=0 && c < result.columns())
      Pii::setValueAt(Pii::sqrt(magnitude), result, r, c);
  }
  
 
  template <class T, class Selector, class U>
  PiiMatrix<U> circularHough(const PiiMatrix<T>& gradientX,
                             const PiiMatrix<T>& gradientY,
                             Selector select,
                             U radius,
                             double angleError,
                             GradientSign sign)
{
		const int iRows =  gradientX.rows();
		const int iCols =  gradientX.columns();
    const bool bPositive = sign member_of (PositiveGradient, IgnoreGradientSign);
    const bool bNegative = sign member_of (NegativeGradient, IgnoreGradientSign);

    int iArcLength = 1;
    double dAngleStep = 0, dSinAlpha = 0, dCosAlpha = 0;

		if (angleError > 0)
      {
        dAngleStep = 1.0 / radius;
        iArcLength = Pii::round<int>(angleError * radius);
        // Rotation matrix coefficients
        dSinAlpha = sin(dAngleStep);
        dCosAlpha = cos(dAngleStep);
      }
		
		PiiMatrix<U> matResult(iRows, iCols);

		for (int r=0; r<iRows; ++r)
      {
        const T* pX = gradientX[r];
        const T* pY = gradientY[r];
        
        for (int c=0; c<iCols; ++c)
          {
            U magnitude = U(Pii::square(pX[c]) + Pii::square(pY[c]));
            if (!select(magnitude))
              continue;

            double dHypot = Pii::hypotenuse(pY[c], pX[c]);
            // Form direction vector.
            double dX = pX[c] / dHypot * radius;
            double dY = pY[c] / dHypot * radius;

            // Each edge point in the input adds two points to the
            // transform. (In the gradient direction and its
            // opposite.)
            if (bPositive)
              addPixel(magnitude, matResult, r + dY, c + dX);
            if (bNegative)
              addPixel(magnitude, matResult, r - dY, c - dX);
            
            // If an estimate of the gradient error is given, draw two
            // arcs to the transformation domain.
            if (iArcLength > 1)
              {
                double dX1 = dX, dX2 = dX, dY1 = dY, dY2 = dY;
                for (int i=1; i<iArcLength; ++i)
                  {
                    // Rotate the direction vector clockwise ...
                    double dXTmp = dCosAlpha * dX1 - dSinAlpha * dY1;
                    dY1 = dSinAlpha * dX1 + dCosAlpha * dY1;
                    dX1 = dXTmp;

                    // ... and counter-clockwise
                    dXTmp = dCosAlpha * dX2 + dSinAlpha * dY2;
                    dY2 = dCosAlpha * dY2 - dSinAlpha * dX2;
                    dX2 = dXTmp;

                    if (bPositive)
                      {
                        addPixel(magnitude, matResult, r + dY1, c + dX1);
                        addPixel(magnitude, matResult, r + dY2, c + dX2);
                      }
                    if (bNegative)
                      {
                        addPixel(magnitude, matResult, r - dY1, c - dX1);
                        addPixel(magnitude, matResult, r - dY2, c - dX2);
                      }
                  }
              }
					}
			}
		return matResult;
  }

  template <class T, class Selector, class U>
  QList<PiiMatrix<U> > circularHough(const PiiMatrix<T>& image,
                                     Selector select,
                                     U startRadius,
                                     U endRadius,
                                     U radiusStep,
                                     GradientSign sign)
  {
    typedef typename Pii::Combine<int,T>::Type GradType;
    if (radiusStep <= 0 || startRadius > endRadius)
      PII_THROW(PiiInvalidArgumentException, ""); //PENDING
    QList<PiiMatrix<U> > lstResult;
    PiiMatrix<GradType> matX(PiiImage::filter<GradType>(image, PiiImage::SobelXFilter));
    PiiMatrix<GradType> matY(PiiImage::filter<GradType>(image, PiiImage::SobelYFilter));
    while (startRadius <= endRadius)
      {
        lstResult << circularHough(matX, matY, select, startRadius, M_PI/64, sign);
        startRadius += radiusStep;
      }
    return lstResult;
  }
}
