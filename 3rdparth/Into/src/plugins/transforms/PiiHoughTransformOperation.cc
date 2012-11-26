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

#include "PiiHoughTransformOperation.h"
#include "PiiTransforms.h"
#include <PiiGeometricObjects.h>
#include <PiiHeap.h>
#include <PiiMath.h>


PiiHoughTransformOperation::Data::Data() :
  type(Linear),
  dAngleResolution(1.0),
  dDistanceResolution(1.0),
  iPeakCount(1),
  bPeaksConnected(false),
  iRows(0),
  iCols(0),
  iStartAngle(0),
  iEndAngle(180),
  iStartDistance(Pii::Numeric<int>::minValue()),
  iEndDistance(Pii::Numeric<int>::maxValue())
{
}

PiiHoughTransformOperation::PiiHoughTransformOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("accumulator"));
  addSocket(new PiiOutputSocket("peaks"));
  addSocket(new PiiOutputSocket("coordinates"));
}

PiiHoughTransformOperation::~PiiHoughTransformOperation()
{}

void PiiHoughTransformOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if(d->type != Linear)
    PII_THROW(PiiExecutionException, tr("Transformation type is not yet implemented"));
  
  if (d->iStartAngle >= d->iEndAngle)
    PII_THROW(PiiExecutionException, tr("Start angle must be smaller than end angle."));
  if (d->iStartDistance >= d->iEndDistance)
    PII_THROW(PiiExecutionException, tr("Start distance must be smaller than end distance."));

  d->bPeaksConnected = outputAt(1)->isConnected() || outputAt(2)->isConnected();
}

void PiiHoughTransformOperation::process()
{
  PiiVariant obj = readInput();
   
   switch (obj.type())
     {
       // For gray images do hough transform directly
       PII_GRAY_IMAGE_CASES(doHough, obj);
       // For color images throw unknow type
      default:
       PII_THROW_UNKNOWN_TYPE(inputAt(0));
     }
}


template <class T> void PiiHoughTransformOperation::doHough(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> image = obj.valueAs<PiiMatrix<T> >();
  typedef typename TransformTraits<T>::Type ResultType;
  PiiMatrix<ResultType> accumulator;
   
  switch(d->type)
    {
    case Linear:
      accumulator = PiiTransforms::linearHough<ResultType>(image,
                                                           std::bind2nd(std::greater<int>(), 0),
                                                           d->dAngleResolution,
                                                           d->dDistanceResolution,
                                                           d->iStartAngle, d->iEndAngle,
                                                           d->iStartDistance, d->iEndDistance);
      
      if(d->bPeaksConnected)
        {
          d->iRows = image.rows();
          d->iCols = image.columns();
          findPeaks(accumulator);
        }
      break;

    case Circle:
      break;
    }
 
  emitObject(accumulator);
}
  
template <class T> void PiiHoughTransformOperation::findPeaks(const PiiMatrix<T>& accumulator)

{
  PII_D;
  //Coordinates matrix contains now two points -- line begin and end points in
  //image coordinate systems. and they are written so that (x,y,x,y)
  //in matrix
  
  PiiMatrix<double> matPeaks(0,2), matCoordinates(0, 4);
   
  // Inverse heap keeps smallest value at top
  PiiHeap<LineObject<T>, 16> lstPeaks(d->iPeakCount, true);

  for (int r = 0; r < accumulator.rows(); ++r)
    {
      const T* pRow = accumulator[r];
      for (int c = 0; c < accumulator.columns(); ++c)
        lstPeaks.put(LineObject<T>(r, c, pRow[c]));
    }

  lstPeaks.sort();

  for (int i=0; i<d->iPeakCount; ++i)
    {
      //No more high lines --> break;
      if (lstPeaks[i].value == 0)
        break;
      else
        savePeak(matPeaks, matCoordinates, lstPeaks[i].row, lstPeaks[i].col);
    }

  outputAt(1)->emitObject(matPeaks);
  outputAt(2)->emitObject(matCoordinates);
}

void PiiHoughTransformOperation::savePeak(PiiMatrix<double>& peaks, PiiMatrix<double>& coordinates, int dd, int angle)
{
  PII_D;
  // Here we search two points which are intersection points of line
  // and image border

  double dAngle = angle*d->dAngleResolution + d->iStartAngle;
  int iMaxDistance = Pii::round<int>(Pii::hypotenuse(double(d->iRows)/2, double(d->iCols)/2));
  double dR = d->iStartDistance < -iMaxDistance ?
    dd * d->dDistanceResolution - iMaxDistance:
    dd * d->dDistanceResolution + d->iStartDistance;

  peaks.insertRow(-1, dR, dAngle);
  qDebug("dAngle = %lf, dR = %lf", dAngle, dR);
  
  double* pRow = coordinates.appendRow();

  //PENDING This is quite stupid way to do this, i just copy-paste it
  //from old code...
  
  //Line x cos theta + y sin theta = r
  // -> y = r/sin theta - x cos theta/sin theta
  // -> y = -kx+b
  
  
  double angleInRad = dAngle*M_PI/180.0;
  double sinTheta = sin(angleInRad);
  double cosTheta = cos(angleInRad);

  
  double dYintercept = 0.0, dSlope = 0.0;
  if(sinTheta != 0.0)
    {
      dYintercept = dR/sinTheta;
      dSlope = -(cosTheta/sinTheta);
    }
  else
    {
      //Now case of straight line is dealed so that :
      // x = \frac{ r - y sin theta }{ cos theta } (now sin theta  == 0)
      // x = \frac{r}{cos theta}
      dYintercept = dR;
      dSlope = INFINITY;
    }
  
  double yDiff = d->iRows/2;
  double xDiff = d->iCols/2;

  
  if (Pii::isInf(dSlope))
    {
      pRow[0] = pRow[2] = Pii::round<int>(dYintercept) + xDiff;
      pRow[1] = 0;
      pRow[3] = d->iRows-1;
      
    }
  else if (Pii::almostEqualRel(dSlope,0.0))
    {
      pRow[0] = 0;
      pRow[1] = pRow[3] = Pii::round<int>(dYintercept) + yDiff;
      pRow[2] = d->iCols-1;
    }
  else
    {  
      QList<PiiPoint<int> > cutPoints;
      PiiPoint<int> point;

      int y = Pii::round<int>(dSlope*(-xDiff)+dYintercept) + yDiff;

      if (y >= 0 && y <= d->iRows)
        {
          point.x = 0;
          point.y = y;
          cutPoints.append(point);
        }
        
      int x = Pii::round<int>((-yDiff-dYintercept)/dSlope + xDiff);

      if (x > 0 && x < d->iCols)
        {
          point.x = x;
          point.y = 0;
          cutPoints.append(point);
        }

      y = Pii::round<int>(dSlope*(d->iCols- xDiff -1)+dYintercept)+ yDiff;

      if ( y >= 0 && y < d->iRows)
        {
          point.x = d->iCols - 1;
          point.y = y;
          cutPoints.append(point);
        }
        
      x = Pii::round<int>((d->iRows - yDiff - dYintercept - 1)/dSlope + xDiff);
      if ( x > 0 && x < d->iCols)
        {
          point.x = x;
          point.y = d->iRows - 1;
          cutPoints.append(point);
        }

      if (cutPoints.size() != 2)
        {
          pRow[0] = pRow[1] = pRow[2] = pRow[3] = 0;
        }
      else
        {
          pRow[0] = cutPoints[0].x;
          pRow[1] = cutPoints[0].y;
          pRow[2] = cutPoints[1].x;
          pRow[3] = cutPoints[1].y;
        }
    }
}

void PiiHoughTransformOperation::setTransform(TransformType type) { _d()->type = type; }
PiiHoughTransformOperation::TransformType PiiHoughTransformOperation::transform() const { return _d()->type; }
void PiiHoughTransformOperation::setAngleResolution(double resolution) { _d()->dAngleResolution = resolution; }
double PiiHoughTransformOperation::angleResolution() const { return _d()->dAngleResolution; }
void PiiHoughTransformOperation::setDistanceResolution(double resolution) { _d()->dDistanceResolution = resolution; }
double PiiHoughTransformOperation::distanceResolution() const { return _d()->dDistanceResolution; }
void PiiHoughTransformOperation::setPeakCount(int value) { _d()->iPeakCount = value; }
int PiiHoughTransformOperation::peakCount() const { return _d()->iPeakCount; }
void PiiHoughTransformOperation::setStartAngle(int startAngle) { _d()->iStartAngle = startAngle; }
int PiiHoughTransformOperation::startAngle() const { return _d()->iStartAngle; }
void PiiHoughTransformOperation::setEndAngle(int endAngle) { _d()->iEndAngle = endAngle; }
int PiiHoughTransformOperation::endAngle() const { return _d()->iEndAngle; }
void PiiHoughTransformOperation::setStartDistance(int startDistance) { _d()->iStartDistance = startDistance; }
int PiiHoughTransformOperation::startDistance() const { return _d()->iStartDistance; }
void PiiHoughTransformOperation::setEndDistance(int endDistance) { _d()->iEndDistance = endDistance; }
int PiiHoughTransformOperation::endDistance() const { return _d()->iEndDistance; }
