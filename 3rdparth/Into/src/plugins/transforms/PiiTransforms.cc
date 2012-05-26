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

#include "PiiTransforms.h"

namespace PiiTransforms
{
  PiiMatrix<double> peakToPoints(double d, double theta,
                                 int rows, int columns)
  {
    theta *= M_PI/180;
    double dLeftY, dLeftX, dRightY, dRightX;
    // Vertical line
    if (Pii::almostEqualRel(theta, 0.0, 1e-4))
      {
        dLeftX = dRightX = columns/2.0 + d;
        dLeftY = 0;
        dRightY = rows-1;
      }
    // Horizontal line
    else if (Pii::almostEqualRel(theta, M_PI/2, 1e-4))
      {
        dLeftY = dRightY = rows/2.0 + d;
        dLeftX = 0;
        dRightX = columns-1;
      }
    // Any other line
    else
      {
        double dSinTheta = ::sin(theta), dCosTheta = ::cos(theta);
        double dHalfCols = columns/2.0, dHalfRows = rows/2;
        dLeftX = -dHalfCols;
        dLeftY = (d - dLeftX*dCosTheta)/dSinTheta;
        // Left edge intersection is above the image
        if (dLeftY < -dHalfRows)
          {
            dLeftY = -dHalfRows;
            dLeftX = (d - dLeftY*dSinTheta)/dCosTheta;
          }
        // Left edge intersection is below the image
        else if (dLeftY > dHalfRows - 1)
          {
            dLeftY = dHalfRows - 1;
            dLeftX = (d - dLeftY*dSinTheta)/dCosTheta;
          }

        dRightX = dHalfCols - 1;
        dRightY = (d - dRightX*dCosTheta)/dSinTheta;
        // Right edge intersection is above the image
        if (dRightY < -dHalfRows)
          {
            dRightY = -dHalfRows;
            dRightX = (d - dRightY*dSinTheta)/dCosTheta;
          }
        // Right edge intersection is below the image
        else if (dRightY > dHalfRows - 1)
          {
            dRightY = dHalfRows - 1;
            dRightX = (d - dRightY*dSinTheta)/dCosTheta;
          }
        dLeftX += dHalfCols;
        dRightX += dHalfCols;
        dLeftY += dHalfRows;
        dRightY += dHalfRows;
      }
    //qDebug("(%lf %lf) -> (%lf %lf)", dLeftX, dLeftY, dRightX, dRightY);
    return PiiMatrix<double>(1,4, dLeftX, dLeftY, dRightX, dRightY);
  }

  PeakList findPeaks(PiiMatrix<int>& mat, int cnt)
  {
    PeakList heap(cnt, PiiMatrixValue<int>(Pii::Numeric<int>::minValue()), true);
    if (mat.isEmpty())
      return heap;
    
    const int iRows = mat.rows(), iCols = mat.columns();
    const int *pPrevRow = 0, *pRow = 0, *pNextRow = mat[0];
    for (int r=0; r<iRows; ++r)
      {
        pRow = pNextRow;
        if (r < iRows-1)
          pNextRow = mat[r+1];
        for (int c=0; c<iCols; ++c)
          {
            // Accept only local maxima (4-connected)
            if ((c == 0 || pRow[c] >= pRow[c-1]) &&
                (c == iCols-1 || pRow[c] >= pRow[c+1]) &&
                (r == 0 || pRow[c] >= pPrevRow[c]) &&
                (r == iRows-1 || pRow[c] >= pNextRow[c]))
              heap.put(PiiMatrixValue<int>(pRow[c], r, c));
          }
        pPrevRow = pRow;
      }
    heap.sort();
    return heap;
  }
}
