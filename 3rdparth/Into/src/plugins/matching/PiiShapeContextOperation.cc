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

#include "PiiShapeContextOperation.h"

#include <PiiYdinTypes.h>
#include <PiiGeometry.h>
#include <PiiMatching.h>

#include <PiiMatrixUtil.h>
#include <QtDebug>


PiiShapeContextOperation::Data::Data() :
  iSamplingStep(5),
  dTolerance(1),
  iAngles(16),
  iDistances(5),
  dMinDistance(5),
  dDistanceScale(2.2),
  bCollectDistantPoints(false),
  invariance(PiiMatching::RotationInvariant),
  keyPointSelectionMode(SelectEveryNthPoint),
  shapeJoiningMode(JoinNestedShapes)
{
}

PiiShapeContextOperation::PiiShapeContextOperation() : PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  addSocket(d->pBoundariesInput = new PiiInputSocket("boundaries"));
  addSocket(d->pLimitsInput = new PiiInputSocket("limits"));
  
  addSocket(d->pPointsOutput = new PiiOutputSocket("points"));
  addSocket(d->pFeaturesOutput = new PiiOutputSocket("features"));
  addSocket(d->pBoundariesOutput = new PiiOutputSocket("boundaries"));
  addSocket(d->pLimitsOutput = new PiiOutputSocket("limits"));
}

void PiiShapeContextOperation::check(bool reset)
{
  PII_D;

  // Create vecDistances
  d->vecDistances = QVector<double>(d->iDistances);
  d->vecDistances[0] = d->dMinDistance;
  for (int i=1; i<d->vecDistances.count(); i++)
    d->vecDistances[i] = d->vecDistances[i-1] * d->dDistanceScale;
  if (d->bCollectDistantPoints)
    d->vecDistances[d->vecDistances.count()-1] = INFINITY;

  // Use squared distances
  for (int i=0; i<d->vecDistances.count(); i++)
    d->vecDistances[i] *= d->vecDistances[i];
  
  PiiDefaultOperation::check(reset);
}

void PiiShapeContextOperation::process()
{
  PII_D;
  PiiVariant boundariesObject = d->pBoundariesInput->firstObject();
  if (boundariesObject.type() != PiiYdin::IntMatrixType)
    PII_THROW_UNKNOWN_TYPE(d->pBoundariesInput);

  PiiVariant limitsObject = d->pLimitsInput->firstObject();
  if (limitsObject.type() != PiiYdin::IntMatrixType)
    PII_THROW_UNKNOWN_TYPE(d->pLimitsInput);

  PiiMatrix<int> matBoundaries = boundariesObject.valueAs<PiiMatrix<int> >();
  PiiMatrix<int> matLimits = limitsObject.valueAs<PiiMatrix<int> >();
  
  startMany();

  switch (d->shapeJoiningMode)
    {
    case DoNotJoinShapes:
      {
        int iStart=0;
        for (int i=0; i<matLimits.columns(); i++)
          {
            processBoundary(matBoundaries(iStart,0,matLimits(0,i)-iStart,-1), PiiMatrix<int>(1,1,matLimits(0,i)-iStart));
            iStart = matLimits(0,i);
          }
        break;
      }
    case JoinNestedShapes:
      {
        PiiMatrix<int> matJoinedBoundaries(matBoundaries.rows(), matBoundaries.columns());
        QList<PiiMatrix<int> > lstJoinedLimits = joinNestedShapes(matBoundaries, matLimits, matJoinedBoundaries);

        int iStart=0, iRows = 0;
        for (int i=0; i<lstJoinedLimits.size(); i++)
          {
            // Get the last limit
            iRows = lstJoinedLimits[i](0,lstJoinedLimits[i].columns()-1);

            // Process boundary
            processBoundary(matJoinedBoundaries(iStart,0,iRows,-1), lstJoinedLimits[i]);

            // Save the next start point
            iStart += iRows;
          }
        break;
      }
    case JoinAllShapes:
      processBoundary(matBoundaries, PiiMatrix<int>(1,1,matBoundaries.rows()));
      break;
    }

  endMany();
}

void PiiShapeContextOperation::processBoundary(const PiiMatrix<int>& boundary, const PiiMatrix<int>& limits)
{
  PII_D;

  // Calculate vector angles
  QVector<double> vecAngles;
  PiiMatrix<int> matKeyPoints(0,boundary.columns());
  matKeyPoints.reserve(32);
  
  if (boundary.rows() <= 3)
    matKeyPoints = boundary;
  else if (d->invariance & PiiMatching::RotationInvariant)
    {
      int iStart = 0;
      for (int i=0; i<limits.columns(); i++)
        {
          int iRows = limits(0,i)-iStart;
          if (iRows > 3)
            {
              PiiMatrix<int> matBoundaryKeyPoints = reducePoints(boundary(iStart,0,iRows,-1), true);
              vecAngles << PiiMatching::boundaryDirections(matBoundaryKeyPoints);
              for (int j=0; j<matBoundaryKeyPoints.rows()-1; j++)
                matKeyPoints.appendRow(matBoundaryKeyPoints[j]);
              
            }
          iStart = limits(0,i);
        }
    }
  else
    matKeyPoints = reducePoints(boundary, false);
      
  // Create feature matrix
  PiiMatrix<float> matFeatures = PiiMatching::shapeContextDescriptor(boundary,
                                                                     matKeyPoints,
                                                                     d->iAngles,
                                                                     d->vecDistances,
                                                                     vecAngles,
                                                                     d->invariance);

  d->pPointsOutput->emitObject(matKeyPoints);
  d->pFeaturesOutput->emitObject(matFeatures);
  d->pBoundariesOutput->emitObject(boundary);
  d->pLimitsOutput->emitObject(limits);
}

PiiMatrix<int> PiiShapeContextOperation::reducePoints(const PiiMatrix<int>& boundary, bool addLastPoint)
{
  PII_D;
  
  switch(d->keyPointSelectionMode)
    {
    case SelectImportantPoints:
      {
        PiiMatrix<int> result = PiiGeometry::reduceVertices(boundary, d->dTolerance * d->dTolerance);
        if (!addLastPoint)
          result.resize(result.rows()-1, result.columns());
        
        return result;
      }
    case SelectEveryNthPoint:
      {
        // Check the sampling limits
        int iSize = boundary.rows() / d->iSamplingStep;
        
        if (boundary.rows() <= d->iSamplingStep || iSize <= 4)
          return boundary;

        // Select every Nth point
        PiiMatrix<int> matResult(0,boundary.columns());
        matResult.reserve(iSize + 1);
        for (int i=0; i<boundary.rows(); i += d->iSamplingStep)
          matResult.appendRow(boundary[i]);

        if (addLastPoint)
          {
            // Append last point if necessary
            if (boundary.rows()-1 % d->iSamplingStep != 0)
              matResult.appendRow(boundary[boundary.rows()-1]);
          }

        return matResult;
      }
    }

  return boundary;
}

QList<PiiMatrix<int> > PiiShapeContextOperation::joinNestedShapes(const PiiMatrix<int>& boundaries,
                                                                  const PiiMatrix<int>& limits,
                                                                  PiiMatrix<int>& joinedBoundaries)
{

  // Initialize variables
  QList<PiiMatrix<int> > lstBoundaries;
  PiiMatrix<int> matLimitLengths(1,limits.columns());
  QVector<QList<int> > lstJoinedIndices(limits.columns());
  QList<int> lstMainShapes;

  int iPreviousLimit = 0;
  for (int i=0; i<limits.columns(); i++)
    {
      lstBoundaries << boundaries(iPreviousLimit,0,limits(0,i)-iPreviousLimit,-1);
      matLimitLengths(0,i) = limits(0,i) - iPreviousLimit;
      iPreviousLimit = limits(0,i);

      lstJoinedIndices[i].append(i);
      lstMainShapes << i;
    }

  // Check the nested shapes
  for (int i=0; i<lstBoundaries.size(); i++)
    {
      int x = lstBoundaries[i](0,0);
      int y = lstBoundaries[i](0,1);
      
      for (int j=0; j<lstBoundaries.size(); j++)
        {
          if (i != j && PiiGeometry::contains(lstBoundaries[j], x, y))
            {
              lstJoinedIndices[j].append(i);
              lstMainShapes.removeAll(i);
            }
        }
    }
  for (int i=0; i<lstJoinedIndices.size(); i++)
    if (!lstMainShapes.contains(i))
      lstJoinedIndices[i].clear();

  // Join boundaries and limits
  QList<PiiMatrix<int> > lstLimits;
  int iJoinedStartPoint = 0;
  for (int i=0; i<lstJoinedIndices.size(); i++)
    {
      if (lstMainShapes.contains(i))
        {
          QList<int> lstIndices = lstJoinedIndices[i];
          PiiMatrix<int> matLimits(1,lstIndices.size());
          int iStart = 0;
          for (int j=0; j<lstIndices.size(); j++)
            {
              int index = lstIndices[j];
              int iLength = matLimitLengths(0,index);
              
              // Calculate limits
              matLimits(0,j) = iLength + iStart;
              iStart = matLimits(0,j);
              
              // Reorganize boundaries
              joinedBoundaries(iJoinedStartPoint,0,iLength,-1) << boundaries(limits(0,index)-iLength,0,iLength,-1);
              iJoinedStartPoint += iLength;
            }

          // Append calculated limits to the list
          lstLimits << matLimits;
        }
    }

  return lstLimits;
}


void PiiShapeContextOperation::setSamplingStep(int samplingStep) { _d()->iSamplingStep = qMax(1,samplingStep); }
int PiiShapeContextOperation::samplingStep() const { return _d()->iSamplingStep; }
void PiiShapeContextOperation::setAngles(int angles) { _d()->iAngles = qMin(360,qMax(1,angles)); }
int PiiShapeContextOperation::angles() const { return _d()->iAngles; }
void PiiShapeContextOperation::setDistances(int distances) { _d()->iDistances = qMin(128,qMax(1,distances)); }
int PiiShapeContextOperation::distances() const { return _d()->iDistances; }
void PiiShapeContextOperation::setMinDistance(double minDistance) { _d()->dMinDistance = minDistance; }
double PiiShapeContextOperation::minDistance() const { return _d()->dMinDistance; }
void PiiShapeContextOperation::setDistanceScale(double distanceScale) { _d()->dDistanceScale = distanceScale; }
double PiiShapeContextOperation::distanceScale() const { return _d()->dDistanceScale; }
void PiiShapeContextOperation::setTolerance(double tolerance) { _d()->dTolerance = tolerance; }
double PiiShapeContextOperation::tolerance() const { return _d()->dTolerance; }
void PiiShapeContextOperation::setCollectDistantPoints(bool collectDistantPoints)
{ _d()->bCollectDistantPoints = collectDistantPoints; }
bool PiiShapeContextOperation::collectDistantPoints() const
{ return _d()->bCollectDistantPoints; }
void PiiShapeContextOperation::setInvariance(PiiMatching::InvarianceFlags invariance)
{ _d()->invariance = invariance; }
PiiMatching::InvarianceFlags PiiShapeContextOperation::invariance() const
{ return _d()->invariance; }

void PiiShapeContextOperation::setKeyPointSelectionMode(KeyPointSelectionMode keyPointSelectionMode)
{ _d()->keyPointSelectionMode = keyPointSelectionMode; }
PiiShapeContextOperation::KeyPointSelectionMode PiiShapeContextOperation::keyPointSelectionMode() const
{ return _d()->keyPointSelectionMode; }
void PiiShapeContextOperation::setShapeJoiningMode(ShapeJoiningMode shapeJoiningMode)
{ _d()->shapeJoiningMode = shapeJoiningMode; }
PiiShapeContextOperation::ShapeJoiningMode PiiShapeContextOperation::shapeJoiningMode() const
{ return _d()->shapeJoiningMode; }
