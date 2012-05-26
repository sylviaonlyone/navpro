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

#include "PiiMatching.h"
#include <PiiMath.h>
#include <PiiMatrixUtil.h>
#include <PiiPoint.h>

#include <iostream>

PiiMatrix<float> PiiMatching::shapeContextDescriptor(const PiiMatrix<int>& boundaryPoints,
                                                     const PiiMatrix<int>& keyPoints,
                                                     int angles,
                                                     const QVector<double>& distances,
                                                     const QVector<double>& directions,
                                                     InvarianceFlags invariance)
{
  const int iColumns = angles * distances.size();
  const int iKeyPoints = keyPoints.rows(), iDistances = distances.size();

  int iBoundaryPoints = boundaryPoints.rows()-1;

  PiiMatrix<float> matFeatures(iKeyPoints, iColumns);

  if (iKeyPoints < 1 || iBoundaryPoints < 1)
    return matFeatures;
  
  // If the first and last point on the boundary are not the same,
  // handle the last point too.
  if (boundaryPoints.rowAs<PiiPoint<int> >(0) != boundaryPoints.rowAs<PiiPoint<int> >(iBoundaryPoints))
    ++iBoundaryPoints;

  if (iBoundaryPoints < 2)
    return matFeatures;
  
  // Calculate max distance and angle step
  double dMaxDistance = distances.last();
  double dAngleStep = 2*M_PI / qMax(1,angles);

  double* pNewDistances = 0;
  const double* pDistances = 0;
  if (invariance & ScaleInvariant)
    {
      double dMeanDistance = 0;
      int iPairCount = iKeyPoints * (iKeyPoints - 1) / 2;
      int iStep = 1;
      if (iPairCount > 10000)
        iStep = iPairCount / 10000;
      iPairCount = 0;
      
      for (int i=0; i<iKeyPoints-1; ++i)
        for (int j=i+1; j<iKeyPoints; j += iStep)
          {
            int dx = keyPoints(i,0) - keyPoints(j,0);
            int dy = keyPoints(i,1) - keyPoints(j,1);
            double dDistance = dx*dx + dy*dy;
            ++iPairCount;
            // Calculate mean iteratively
            double dWeight = 1.0 / iPairCount;
            dMeanDistance = (1.0 - dWeight) * dMeanDistance + dWeight * dDistance;
          }
      // Scale distance limits (same as dividing each distance by the
      // mean)
      pDistances = pNewDistances = new double[iDistances];
      for (int i=0; i<iDistances; ++i)
        pNewDistances[i] = distances[i] * dMeanDistance;
    }
  else
    pDistances = distances.data();

  // Calculate features for selected points
  for (int i=0; i<iKeyPoints; ++i)
    {
      float* pCurrentRow = matFeatures[i];
      
      // Get the main point
      int x = keyPoints(i,0);
      int y = keyPoints(i,1);

      // Loop all points
      for (int j=0; j<iBoundaryPoints; ++j)
        {
          // Calculate distance
          int dx = x-boundaryPoints(j,0);
          int dy = y-boundaryPoints(j,1);
          double dDistance = dx*dx + dy*dy;

          if (dDistance < dMaxDistance && dDistance != 0)
            {
              // Find distance bin index
              int iDistanceIndex = 0;
              for (int c=0; c<iDistances; ++c)
                {
                  if (dDistance < pDistances[c])
                    iDistanceIndex = c;
                  else
                    break;
                }

              // Calculate angle between feature point and current boundary point
              float dAngle = Pii::atan2((float)dy,(float)dx) + M_PI;

              // Rotate along boundary direction
              if (directions.size() > 0)
                {
                  dAngle -= directions[i];
                  if (dAngle < 0)
                    dAngle += 2*M_PI;
                  else if (dAngle > 2*M_PI)
                    dAngle -= 2*M_PI;
                }

              int iBinIndex = iDistances * int(dAngle / dAngleStep) + iDistanceIndex;
              // Special case: dAngle == 2*M_PI
              if (iBinIndex >= iColumns)
                iBinIndex = 0;
              ++pCurrentRow[iBinIndex];
            }
        }
      // Normalize histogram and return it
      float fSum = Pii::accumulateN(pCurrentRow, iColumns, std::plus<float>(), 0.0f);
      if (fSum != 0)
        Pii::mapN(pCurrentRow, iColumns, std::bind2nd(std::multiplies<float>(), 1.0f/fSum));
    }
  
  delete[] pNewDistances;
  
  return matFeatures;
}


QVector<double> PiiMatching::boundaryDirections(const PiiMatrix<int>& boundaryPoints)
{
  int iPoints = boundaryPoints.rows();
  if (iPoints == 0)
    return QVector<double>();
  // Last and first point is the same
  if (boundaryPoints.rowAs<PiiPoint<int> >(0) == boundaryPoints.rowAs<PiiPoint<int> >(iPoints-1))
    --iPoints;
  QVector<double> angles(iPoints);
  if (iPoints < 3)
    return angles;

  const PiiPoint<int>& vecFirst = boundaryPoints.rowAs<PiiPoint<int> >(0);
  const PiiPoint<int>& vecLast = boundaryPoints.rowAs<PiiPoint<int> >(iPoints-1);
  angles[0] = Pii::atan2(float(boundaryPoints(1,1) - vecLast.y),
                         float(boundaryPoints(1,0) - vecLast.x));
  angles[iPoints-1] = Pii::atan2(float(vecFirst.y - boundaryPoints(iPoints-2,1)),
                                 float(vecFirst.x - boundaryPoints(iPoints-2,0)));
  
  for (int i=1; i<iPoints-1; ++i)
    {
      const PiiPoint<int>& prevPt(boundaryPoints.rowAs<PiiPoint<int> >(i-1));
      const PiiPoint<int>& nextPt(boundaryPoints.rowAs<PiiPoint<int> >(i+1));
      angles[i] = Pii::atan2(float(nextPt.y - prevPt.y), float(nextPt.x - prevPt.x));
    }
    
  return angles;
}
