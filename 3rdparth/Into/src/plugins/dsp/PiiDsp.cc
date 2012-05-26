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

#include "PiiDsp.h"

namespace PiiDsp
{
  QList<Peak> findPeaks(const PiiMatrix<double>& data,
                        double levelThreshold,
                        double sharpnessThreshold,
                        int smoothWidth,
                        int windowWidth)
  {
    QList<Peak> lstResult;
    
    if (data.columns() < qMax(windowWidth, qMax(smoothWidth, 3)))
      return lstResult;

    // Calculated first derivative (forward differences)
    PiiMatrix<double> matDeriv = Pii::diff<double>(data.rows() == 2 ? data(1,0,1,-1) : data(0,0,1,-1));
    // Smooth the derivative if requested
    if (smoothWidth > 1)
      matDeriv = Pii::movingAverage<double>(matDeriv, smoothWidth, Pii::Horizontally, Pii::ShrinkWindow);

    double *pDeriv = matDeriv[0];
    const double *pDataY, *pDataX;

    int iHalfWindow = windowWidth/2;

    // Take the X coordinate
    PiiMatrix<double> matX(1,data.columns());
    // If X coordinates are provided, copy them
    if (data.rows() == 2)
      {
        matX << data(0,0,1,-1);
        pDataY = data[1];
      }
    // Otherwise generate zero-based indices
    else
      {
        for (int x=0; x<data.columns(); ++x)
          matX(0,x) = x;
        pDataY = data[0];
      }
    pDataX = matX[0];

    for (int i=1; i<data.columns()-1; ++i)
      {
        // Let's see...
        if (pDataY[i] > levelThreshold && // The absolute level of the input signal is high enough ...
            pDeriv[i] < 0 && pDeriv[i-1] >= 0 && // ... derivative changes sign ...
            pDeriv[i-1] - pDeriv[i] > sharpnessThreshold * pDataY[i]) // ... and the change is abrupt enough
          {
            // Well, this must be a peak, then

            // Calculate window boundaries
            int iMin = qMax(0,i-iHalfWindow), iMax = qMin(i+iHalfWindow,data.columns()-1);
            //qDebug("Peak at %d. d[%d]: %lf. d[%d]: %lf", i, i-1, pDeriv[i-1], i, pDeriv[i]);
            //qDebug("data: %lf %lf %lf %lf %lf", pDataY[i-3], pDataY[i-2], pDataY[i-1], pDataY[i], pDataY[i+1]);
            int iWidth = iMax - iMin + 1;
            // Take the corresponding X and Y coordinates
            const PiiMatrix<double> matFitX(1, iWidth, pDataX + iMin);
            const PiiMatrix<double> matFitY(1, iWidth, pDataY + iMin);
            Peak peak;
            if (windowWidth < 5)
              {
                int iTmp;
                peak.height = Pii::maxAll(matFitY, &iTmp, &peak.dataIndex);
                peak.position = matFitX(0,peak.dataIndex);
                peak.dataIndex += iMin;
                //qDebug("max: %lf, dataIndex: %d", peak.height, peak.dataIndex);
              }
            else
              {
                peak.dataIndex = i;
                // Move X to zero mean and divide by std to improve numerical performance
                double dMeanX, dStdX = Pii::stdAll<double>(matFitX, &dMeanX);
                // Make all values at least one
                double dMinY = Pii::minAll(matFitY) - 1;
                // Fit a parabola to log of normalized data
                PiiMatrix<double> matCoeffs = Pii::fitPolynomial(2,
                                                                 PiiMatrix<double>((matFitX-dMeanX)/dStdX),
                                                                 PiiMatrix<double>(Pii::log(matFitY - dMinY)));

                double c = matCoeffs(0), b = matCoeffs(1), a = matCoeffs(2);
                // Zero of 1st derivative is the top.
                double dTopX = -b / (2*a);
                // Denormalize
                peak.position = dMeanX + dStdX * dTopX;
                // Calculate y coordinate
                peak.height = std::exp(a*dTopX*dTopX + b*dTopX + c) + dMinY;
                // Magic equation. Believe this or not. It is taken from T.C. O'Haver's Matlab code.
                peak.width = dStdX * 2.35703 / (M_SQRT2 * std::sqrt(-a));
                
                //qDebug("max: %lf, position: %lf", peak.height, peak.position);
              }
            lstResult << peak;
          }
      }
    return lstResult;
  }
}

