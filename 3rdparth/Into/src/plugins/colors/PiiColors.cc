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

#include "PiiColors.h"

namespace PiiColors
{
	PiiMatrix<float> ohtaKanadeMatrix(3,3,
                                    1.0/3.0, 1.0/3.0, 1.0/3.0,
                                    1.0,         0.0,    -1.0,
                                    -1.0/2.0,    1.0, -1.0/2.0);

	PiiMatrix<float> d65_709_XyzMatrix(3,3,
                                     0.412453, 0.357580, 0.180423,
                                     0.212671, 0.715160, 0.072169,
                                     0.019334, 0.119193, 0.950227);


  PiiMatrix<float> autocorrelogram(const PiiMatrix<int>& image,
                                   int maxDistance,
                                   int levels)
  {
    QList<int> lstDistances;
    for (int i=1; i<=maxDistance; ++i)
      lstDistances << i;
    return autocorrelogram(image, lstDistances, levels);
  }
  
  PiiMatrix<float> autocorrelogram(const PiiMatrix<int>& image,
                                   const QList<int>& distances,
                                   int levels)
  {
    if (levels <= 0)
      levels = Pii::maxAll(image) + 1;
    PiiMatrix<float> matCorrelogram(1, levels * distances.size());
    float* pCorrelogram = matCorrelogram[0];

    const int iRows = image.rows(), iCols = image.columns();
    for (int d=0; d<distances.size(); ++d)
      {
        const int iDist = distances[d];
        const int iWindowSize = iDist * 8;
        if (iRows <= iDist && iCols <= iDist)
          continue;

        const int iLastRow = iRows - iDist, iLastCol = iCols - iDist;

        int r = 0;
        // Top boundary
        for (; r<qMin(iDist,iRows); ++r)
          {
            int iWindowVEnd = r+iDist;
            const int* pRow = image[r], *pRow1 = 0;
            if (iWindowVEnd < iRows)
              pRow1 = image[iWindowVEnd];
            else
              iWindowVEnd = iRows-1;
            
            for (int c=0; c<iCols; ++c)
              {
                int iSum = 0, iCnt = 0;
                const int iCenter = pRow[c];
                if (iCenter >= levels)
                  continue;

                int iWindowHStart = c-iDist, iWindowHEnd = c+iDist;

                // Verticals
                if (iWindowHStart >= 0)
                  {
                    for (int v=0; v<=iWindowVEnd; ++v)
                      if (image(v, iWindowHStart) == iCenter)
                        ++iSum;
                    iCnt += iWindowVEnd + 1;
                  }
                else
                  iWindowHStart = -1;

                if (iWindowHEnd < iCols)
                  {
                    for (int v=0; v<=iWindowVEnd; ++v)
                      if (image(v, iWindowHEnd) == iCenter)
                        ++iSum;
                    iCnt += iWindowVEnd + 1;
                  }
                else
                  iWindowHEnd = iCols;

                // Horizontal
                if (pRow1 != 0)
                  {
                    for (int h=iWindowHStart+1; h<iWindowHEnd; ++h)
                      if (pRow1[h] == iCenter)
                        ++iSum;
                    iCnt += iWindowHEnd - iWindowHStart - 1;
                  }
                if (iCnt > 0)
                  pCorrelogram[iCenter] += float(iSum) / iCnt;
              }
          }
        
        // Center
        for (; r<iLastRow; ++r)
          {
            const int iWindowVStart = r-iDist, iWindowVEnd = r+iDist;
            const int* pRow1 = image[iWindowVStart], *pRow = image[r], *pRow2 = image[iWindowVEnd];
            int c = 0, iSum = 0, iCnt = 0;
            // Left center
            for (; c<qMin(iCols, iDist); ++c)
              {
                iSum = 0;
                const int iCenter = pRow[c], iWindowHEnd = qMin(iCols-1, c+iDist);
                if (iCenter >= levels)
                  continue;
                for (int h=0; h<=iWindowHEnd; ++h)
                  {
                    if (pRow1[h] == iCenter)
                      ++iSum;
                    if (pRow2[h] == iCenter)
                      ++iSum;
                  }
                iCnt = (iWindowHEnd+1)*2;
                if (c+iDist < iCols)
                  {
                    for (int v=iWindowVStart+1; v<iWindowVEnd; ++v)
                      if (image(v, iWindowHEnd) == iCenter)
                        ++iSum;
                    iCnt += iWindowVEnd - iWindowVStart - 1;
                  }
                pCorrelogram[iCenter] += float(iSum) / iCnt;
              }
            // Image center
            for (; c<iLastCol; ++c)
              {
                iSum = 0;
                const int iCenter = pRow[c], iWindowHStart = c-iDist, iWindowHEnd = c+iDist;
                if (iCenter >= levels)
                  continue;
                for (int h=iWindowHStart; h<=iWindowHEnd; ++h)
                  {
                    if (pRow1[h] == iCenter)
                      ++iSum;
                    if (pRow2[h] == iCenter)
                      ++iSum;
                  }
                for (int v=iWindowVStart+1; v<iWindowVEnd; ++v)
                  {
                    if (image(v, iWindowHStart) == iCenter)
                      ++iSum;
                    if (image(v, iWindowHEnd) == iCenter)
                      ++iSum;
                  }
                pCorrelogram[iCenter] += float(iSum) / iWindowSize;
              }
            // Right center
            for (; c<iCols; ++c)
              {
                iSum = 0, iCnt = 0;
                const int iCenter = pRow[c], iWindowHStart = qMax(0, c-iDist), iWindowHEnd = iCols-1;
                if (iCenter >= levels)
                  continue;
                for (int h=iWindowHStart; h<=iWindowHEnd; ++h, iCnt += 2)
                  {
                    if (pRow1[h] == iCenter)
                      ++iSum;
                    if (pRow2[h] == iCenter)
                      ++iSum;
                  }
                if (c-iDist >= 0)
                  for (int v=iWindowVStart+1; v<iWindowVEnd; ++v, ++iCnt)
                    if (image(v, iWindowHStart) == iCenter)
                      ++iSum;
                pCorrelogram[iCenter] += float(iSum) / iCnt;
              }
          }

        // Bottom boundary
        for (; r<iRows; ++r)
          {
            const int iWindowVEnd = iRows-1, iWindowVStart = r-iDist;
            const int* pRow = image[r], *pRow1 = image[iWindowVStart];

            for (int c=0; c<iCols; ++c)
              {
                int iSum = 0, iCnt = 0;
                const int iCenter = pRow[c];
                if (iCenter >= levels)
                  continue;

                int iWindowHStart = c-iDist, iWindowHEnd = c+iDist;

                // Verticals
                if (iWindowHStart >= 0)
                  {
                    for (int v=iWindowVStart; v<=iWindowVEnd; ++v)
                      if (image(v, iWindowHStart) == iCenter)
                        ++iSum;
                    iCnt += iWindowVEnd - iWindowVStart + 1;
                  }
                else
                  iWindowHStart = -1;
                
                if (iWindowHEnd < iCols)
                  {
                    for (int v=iWindowVStart; v<=iWindowVEnd; ++v)
                      if (image(v, iWindowHEnd) == iCenter)
                        ++iSum;
                    iCnt += iWindowVEnd - iWindowVStart + 1;
                  }
                else
                  iWindowHEnd = iCols;

                // Horizontal
                for (int h=iWindowHStart+1; h<iWindowHEnd; ++h)
                  if (pRow1[h] == iCenter)
                    ++iSum;
                iCnt += iWindowHEnd - iWindowHStart - 1;
                pCorrelogram[iCenter] += float(iSum) / iCnt;
              }
          }

        pCorrelogram += levels;
      }
    return matCorrelogram;
  }
}
