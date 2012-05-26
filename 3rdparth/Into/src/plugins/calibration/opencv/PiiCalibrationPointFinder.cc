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

#include "PiiCalibrationPointFinder.h"
#include <PiiClassification.h>
#include <PiiSquaredGeometricDistance.h>
#include <PiiMatrixUtil.h>
#include <PiiMath.h>
#include <QPair>
#include <QDebug>
#include <iostream>

PiiCalibrationPointFinder::Data::Data(double minDistance, double maxDistance) :
  matCurrentPoints(0,2),
  dMinDistance(minDistance*minDistance), // square here for speed
  dMaxDistance(maxDistance*maxDistance), // ditto
  iPointCount(0),
  dMinError(INFINITY)
{
}

PiiCalibrationPointFinder::PiiCalibrationPointFinder(double minDistance, double maxDistance) :
  d(new Data(minDistance, maxDistance))
{
}

PiiCalibrationPointFinder::~PiiCalibrationPointFinder()
{
  delete d;
}

PiiMatrix<double> PiiCalibrationPointFinder::selectedPoints() const
{
  PiiMatrix<double> points(0,2);
  if (d->lstMinIndices.size() < d->iPointCount)
    return points;

  for (int i=0; i<d->iPointCount; ++i)
    points.appendRow(d->matImagePoints[d->lstMinIndices[i]]);

  return points;
}


// Each possible combination of point indices ends up here.
void PiiCalibrationPointFinder::operator()(int* combination)
{
  // Check if any two points are too close or too distant.
  for (int r=d->matCombinations.rows(); r--; )
    {
      double distance = d->matDistances(combination[d->matCombinations(r,0)],
                                      combination[d->matCombinations(r,1)]);
      // Too large/small structure -> this can't be the right combination
      if (distance > d->dMaxDistance || distance < d->dMinDistance)
        return;
    }
       
  // Insert selected rows into the current point matrix.
  d->matCurrentPoints.resize(0,2);
  for (int i=0; i<d->iPointCount; ++i)
    d->matCurrentPoints.appendRow(d->matImagePoints[combination[i]]);

  // Order the points counter-clockwise wrt to the center of mass.
  PiiMatrix<double> avg = Pii::mean<double>(d->matCurrentPoints, Pii::Vertically);
  // Stores the angle to each point and the corresponding row
  // index in the original image point matrix.
  QVector<QPair<float,int> > angles(d->iPointCount);
  for (int i=d->iPointCount; i--; )
    // In pixel coordinates, angle grows clockwise because the y axis
    // points down.
    angles[i] = qMakePair(Pii::fastAtan2(float(avg(0,1)-d->matCurrentPoints(i,1)),
                                         float(d->matCurrentPoints(i,0)-avg(0,0))),
                          combination[i]);
  qSort(angles);

  // Collects ordered point indices
  QVector<int> indices(d->iPointCount);

  // Now we have an ordered list of points, but we still don't
  // know which of them corresponds to the first point in
  // worldPoints. Let's try all.
  for (int firstPoint=0; firstPoint<d->iPointCount; ++firstPoint)
    {
      for (int i=0; i<d->iPointCount; ++i)
        // Circular index...
        indices[i] = angles[(i+firstPoint)%d->iPointCount].second;

      calculateProjectionError(indices);
    }
}


void PiiCalibrationPointFinder::calculateProjectionError(const QVector<int>& indices)
{
  // Rearrange the rows to the current point matrix according to
  // the given indices to the all points list
  d->matCurrentPoints.resize(0,2);
  for (int i=0; i<d->iPointCount; ++i)
    d->matCurrentPoints.appendRow(d->matImagePoints[indices[i]]);
      
  // Assume that the currently selected points are the calibration
  // points. Find the camera's extrinsic parameters based on this
  // assumption.
  PiiCalibration::RelativePosition pos = PiiCalibration::calculateCameraPosition(d->matWorldPoints,
                                                                                 d->matCurrentPoints,
                                                                                 d->intrinsic);
  // Project the world coordinates to normalized camera
  // coordinates.
  PiiMatrix<double> normalized = PiiCalibration::perspectiveProjection(PiiCalibration::worldToCameraCoordinates(d->matWorldPoints,pos));

  // Compare to undistorted points precalculated in
  // initialization.
  double error = 0.0;
  for (int r=d->iPointCount; r--; )
    error += Pii::squaredDistanceN(normalized[r], 2, d->matUndistorted[indices[r]], 0.0);

  // This is the best match so far
  if (error < d->dMinError)
    {
      d->dMinError = error;
      d->lstMinIndices = indices;
      d->minPosition = pos;
    }
}

    
void PiiCalibrationPointFinder::createDistanceMatrix()
{
  d->matDistances = PiiClassification::calculateDistanceMatrix(d->matImagePoints,
                                                               PiiSquaredGeometricDistance<const double*>());
 
  // Find the closest neighbor for each point
  PiiMatrix<double> minDistance = Pii::min<double>(d->matDistances, Pii::Vertically);
  // Get rid of outlier points whose closest neighbor is already
  // too far away.
  for (int c=minDistance.columns(); c--; )
    if (minDistance(c) > d->dMaxDistance)
      {
        d->matDistances.removeRow(c);
        d->matDistances.removeColumn(c);
        d->matImagePoints.removeRow(c);
      }
}

void PiiCalibrationPointFinder::createCombinationMatrix()
{
  // Find all possible combinations of two points out of the
  // number of calibration points. This is the total number of
  // possible pairings within the calibration points.
  d->matCombinations.resize(0,2);
  d->matCombinations.reserve(d->iPointCount * (d->iPointCount-1));
  Pii::MatrixRowAdder<int> adder(d->matCombinations);
  Pii::combinations(d->iPointCount,2,adder);
}
    
  
PiiCalibration::RelativePosition PiiCalibrationPointFinder::calculateCameraPosition(const PiiMatrix<double>& worldPoints,
                                                                                    const PiiMatrix<double>& imagePoints,
                                                                                    const PiiCalibration::CameraParameters& intrinsic)

{
  if (d->matImagePoints.rows() < d->matWorldPoints.rows())
    PII_THROW(PiiCalibrationException,
              QCoreApplication::translate("PiiCalibrationPointFinder",
                                          "The number of valid calibration points is less than the number of reference points."));
  d->matWorldPoints = worldPoints;
  d->matImagePoints = imagePoints;
  d->matCurrentPoints.resize(0,2);
  d->intrinsic = intrinsic;
  d->iPointCount = worldPoints.rows();
  d->matCurrentPoints.reserve(d->iPointCount);
  d->lstMinIndices.clear();
  createDistanceMatrix();
  createCombinationMatrix();

  d->matUndistorted = PiiCalibration::undistort(d->matImagePoints, intrinsic);
  d->dMinError = INFINITY;
  
  // Take all possible combinations of N calibration points out of a
  // total of M detections. M >= N
  Pii::combinations<PiiCalibrationPointFinder&>(d->matImagePoints.rows(), d->matWorldPoints.rows(), *this);

  if (d->lstMinIndices.size() == 0)
    PII_THROW(PiiCalibrationException,
              QCoreApplication::translate("PiiCalibrationPointFinder",
                                          "No combination of calibration point candidates satisfies the given restrictions."));
  
  return d->minPosition;
}

double PiiCalibrationPointFinder::minError() const { return d->dMinError / d->iPointCount; }
void PiiCalibrationPointFinder::setMinDistance(double minDistance) { d->dMinDistance = minDistance*minDistance; }
double PiiCalibrationPointFinder::minDistance() const { return sqrt(d->dMinDistance); }
void PiiCalibrationPointFinder::setMaxDistance(double maxDistance) { d->dMaxDistance = maxDistance*maxDistance; }
double PiiCalibrationPointFinder::maxDistance() const { return sqrt(d->dMaxDistance); }

