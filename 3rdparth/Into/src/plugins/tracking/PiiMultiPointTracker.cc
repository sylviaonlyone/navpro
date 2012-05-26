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

#include "PiiMultiPointTracker.h"

#include <PiiYdinTypes.h>
#include <QPainter>
#include <QList>
#include <PiiQImage.h>
#include <PiiMatrix.h>
#include <cstdlib> // for abs()
#include <PiiMath.h> //for INFINITY values.
#include <PiiUtil.h> //for (polygon,point) contains
#include <QPolygon>
#include <QPoint>

PiiMultiPointTracker::Data::Data(PiiMultiPointTracker *parent) :
  tracker(parent),
  uiPreviousEmissionTime(0),
  iMinimumTrajectoryLength(5),
  iFrameCount(0),
  matMeasurementCounts(1,10),
  bCumulativeStatistics(false),
  iEmissionInterval(570),
  bAllowMerging(false)
{
}

PiiMultiPointTracker::Data::~Data()
{
}

PiiMultiPointTracker::PiiMultiPointTracker() :
  PiiDefaultOperation(new Data(this), NonThreaded)
{
  PII_D;

  //init previousEmissionTime to the currentTime
  d->uiPreviousEmissionTime = QDateTime::currentDateTime().toTime_t();
  
  //add inputs
  addSocket(new PiiInputSocket("coordinates"));
  
  PiiInputSocket *imageInput = new PiiInputSocket("image");
  imageInput->setOptional(true);
  addSocket(imageInput);

  //add outputs
  addSocket(d->pAreaIdOutput = new PiiOutputSocket("area id"));
  addSocket(d->pDwellHistogramOutput = new PiiOutputSocket("dwell histogram"));
  addSocket(d->pAverageDwellOutput = new PiiOutputSocket("average dwell"));
  addSocket(d->pObjectsOutput = new PiiOutputSocket("objects"));
  addSocket(d->pVisitorsOutput = new PiiOutputSocket("visitors"));
  addSocket(d->pAreaStartTimeOutput = new PiiOutputSocket("area start time"));
  addSocket(d->pAreaEndTimeOutput = new PiiOutputSocket("area end time"));
  addSocket(d->pLineIdOutput = new PiiOutputSocket("line id"));
  addSocket(d->pObjectsInOutput = new PiiOutputSocket("objects in"));
  addSocket(d->pObjectsOutOutput = new PiiOutputSocket("objects out"));
  addSocket(d->pLineStartTimeOutput = new PiiOutputSocket("line start time"));
  addSocket(d->pLineEndTimeOutput = new PiiOutputSocket("line end time"));
  addSocket(d->pImageOutput = new PiiOutputSocket("image"));
  
  d->colorList.append(QColor(Qt::black));
  d->colorList.append(QColor(Qt::red));
  d->colorList.append(QColor(Qt::cyan));
  d->colorList.append(QColor(Qt::green));
  d->colorList.append(QColor(Qt::blue));
}

void PiiMultiPointTracker::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  d->trackerTime.start();
  setFrameCount(0);
}

void PiiMultiPointTracker::setFrameCount(int frameCount)
{
  PII_D;
  d->iFrameCount = frameCount;
  if (frameCount == 0)
    {
      // Reset statistics
      QList<int> lineKeys = d->hashLines.keys();
      for ( int i=0; i<lineKeys.size(); i++ )
        {
          int key = lineKeys[i];
          d->hashLines[key].objectsIn = 0;
          d->hashLines[key].objectsOut = 0;
        }
      QList<int> areaKeys = d->hashAreas.keys();
      for ( int i=0; i<areaKeys.size(); i++ )
        {
          int key = areaKeys[i];
          d->hashAreas[key].totalStayTime = 0;
          d->hashAreas[key].totalObjectCount = 0;
          d->hashAreas[key].dwellHistogram = 0;
          d->hashAreas[key].visitors = 0;
        }


      d->matMeasurementCounts = 0;

      // Reset tracker
      d->tracker.resetTracker();
      d->trackerTime.start();
    }
}

void PiiMultiPointTracker::process()
{
  PII_D;
  QDateTime time = QDateTime::currentDateTime();
  unsigned int currentTime = time.toTime_t();

  PiiVariant obj = readInput();

  if ( obj.type() == PiiYdin::IntMatrixType )
    {
      PiiMatrix<int> coordinates = obj.valueAs<PiiMatrix<int> >();
      operate(coordinates);
    }
  else
    PII_THROW_UNKNOWN_TYPE(inputAt(0));
  
  if (inputAt(1)->isConnected() && d->pImageOutput->isConnected())
    {
      //if image-input is connected, draw all routes to the image
      PiiVariant imageObj = inputAt(1)->firstObject();
      switch (imageObj.type())
        {
        case PiiYdin::UnsignedCharColorMatrixType:
          operateImage<PiiColor<unsigned char> >(imageObj);
          break;        
        case PiiYdin::UnsignedCharColor4MatrixType:
          operateImage<PiiColor4<unsigned char> >(imageObj);
          break;
          PII_NUMERIC_MATRIX_CASES(operateImage, imageObj);
        default:
          PII_THROW_UNKNOWN_TYPE(inputAt(0));
        }
    }

  d->iFrameCount++;

  // We need to emit the analysis results
  if ( (currentTime - d->uiPreviousEmissionTime) > (unsigned int)d->iEmissionInterval )
    {
      // Area statistics
      QList<int> areaKeys = d->hashAreas.keys();
      for ( int i=0; i<areaKeys.size(); i++ )
        {
          int key = areaKeys[i];
          AreaStatistics areaStruct = d->hashAreas[key];
          int visitorCount = areaStruct.visitors;
          
          d->pAreaIdOutput->emitObject(key);
          d->pDwellHistogramOutput->emitObject(areaStruct.dwellHistogram);
          d->pAverageDwellOutput->emitObject(visitorCount ? double(areaStruct.totalStayTime) / visitorCount : 0);
          d->pObjectsOutput->emitObject(double(areaStruct.totalObjectCount) / (double)d->iFrameCount);
          d->pVisitorsOutput->emitObject(visitorCount);
          d->pAreaStartTimeOutput->emitObject(static_cast<int>(d->uiPreviousEmissionTime));
          d->pAreaEndTimeOutput->emitObject(static_cast<int>(currentTime));
        }
      
      // Line statistics
      QList<int> lineKeys = d->hashLines.keys();
      for ( int i=0; i<lineKeys.size(); i++ )
        {
          int key = lineKeys[i];
          d->pLineIdOutput->emitObject(key);
          d->pObjectsInOutput->emitObject(d->hashLines[key].objectsIn);
          d->pObjectsOutOutput->emitObject(d->hashLines[key].objectsOut);      
          d->pLineStartTimeOutput->emitObject(static_cast<int>(d->uiPreviousEmissionTime));
          d->pLineEndTimeOutput->emitObject(static_cast<int>(currentTime));
        }

      if (!d->bCumulativeStatistics)
        {
          // Reset all statistics
          for ( int i=0; i<lineKeys.size(); i++ )
            {
              int key = lineKeys[i];
              d->hashLines[key].objectsIn = 0;
              d->hashLines[key].objectsOut = 0;
            }

          for ( int i=0; i<areaKeys.size(); i++ )
            {
              int key = areaKeys[i];
              d->hashAreas[key].totalStayTime = 0;
              d->hashAreas[key].totalObjectCount = 0;
              d->hashAreas[key].dwellHistogram = 0;
              d->hashAreas[key].visitors = 0;
            }

          d->iFrameCount = 0;
        }
      d->uiPreviousEmissionTime = currentTime;
    }
}

int PiiMultiPointTracker::mapTime(int time)
{
  static const int limits[8] = { 10, 20, 30, 60, 120, 180, 240, 300 };
  for (int i=0; i<8; i++)
    if (time < limits[i])
      return i;
  return 8;
}

bool PiiMultiPointTracker::collectLineStatistics(PiiCoordinateTrackerNode<double,2>* trajectory)
{
  PII_D;
  bool bAmountChanged = false;
  
  QPoint previousPoint = QPoint(int(trajectory->measurement()[0]),
                                int(trajectory->measurement()[1]));
  trajectory = trajectory->next();

  QList<int> keys = d->hashLines.keys();
  PiiMatrix<int> directionSums(1,keys.size());
  
  while (trajectory)
    {
      QPoint currentPoint = QPoint(int(trajectory->measurement()[0]),
                                   int(trajectory->measurement()[1]));
      for ( int i=0; i<keys.size(); i++ )
        {
          QPoint lineStartPoint = d->lstLines[i].value<QPolygon>().point(0);
          QPoint lineEndPoint = d->lstLines[i].value<QPolygon>().point(1);
          
          directionSums(0,i) += checkCalculationLine(lineStartPoint, lineEndPoint, currentPoint, previousPoint);
        }

      trajectory = trajectory->next();
      previousPoint = currentPoint;
    }

  for ( int i=0; i<keys.size(); i++)
    {
      if (directionSums(0,i) > 0)
        {
          bAmountChanged = true;
          d->hashLines[keys[i]].objectsIn++;
        }
      else if (directionSums(0,i) < 0)
        {
          bAmountChanged = true;
          d->hashLines[keys[i]].objectsOut++;
        }
    }
  return bAmountChanged;
}

bool PiiMultiPointTracker::collectAreaStatistics(PiiCoordinateTrackerNode<double,2>* trajectory)
{
  PII_D;
  bool bAmountChanged = false;
  
  PiiVector<double,2> previousPoint = trajectory->measurement();
  int size = d->lstAreas.size();

  bool *previousIn = new bool[size];
  bool *currentIn = new bool[size];
  bool *someIn = new bool[size];
  int *exitTime = new int[size];
  for ( int i=0; i<size; i++ )
    {
      previousIn[i] = false;
      currentIn[i] = false;
      someIn[i] = false;
      exitTime[i] = 0;
    }
  
  int previousTime = trajectory->time();
  
  while (trajectory)
    {
      PiiVector<double,2> currentPoint = trajectory->measurement();
      QPoint point((int)currentPoint[0],(int)currentPoint[1]);
      for ( int i=0; i<size; i++ )
        {
          int key = d->lstAreaIdentifications[i].toInt();
          if ( d->hashAreas.contains(key) )
            {
              QPolygon polygon = d->lstAreas[i].value<QPolygon>();
              currentIn[i] = polygon.contains(point); //Pii::contains(polygon,point);
              someIn[i] = someIn[i] || currentIn[i];
              
              // If the point is within the area, we increase total object
              // count.
              if (currentIn[i])
                {
                  d->hashAreas[key].totalObjectCount++;
                  // Object exited area
                  if (!previousIn[i])
                    exitTime[i] = previousTime;
                }
              // Object entered area
              else if (previousIn[i])
                {
                  int stayTime = int(double(exitTime[i] - previousTime)/1000.0 + 0.5);
                  int k=mapTime(stayTime); 
                  d->hashAreas[key].dwellHistogram(k)++;
                  d->hashAreas[key].totalStayTime += stayTime;
                }
              previousIn[i] = currentIn[i];
              
            }
        }
      previousPoint = currentPoint;
      previousTime = trajectory->time();
      trajectory = trajectory->next();
    }
  // If the start point of the trajectory is still within the area,
  // and the object has exited it later, we need to count
  for ( int i=0; i<size; i++ )
    {
      int key = d->lstAreaIdentifications[i].toInt();
      if (previousIn[i] && exitTime[i])
        {
          if ( d->hashAreas.contains(key) )
            {
              int stayTime = int(double(exitTime[i] - previousTime)/1000.0 + 0.5);
              int k=mapTime(stayTime);
              d->hashAreas[key].dwellHistogram(k)++;
              d->hashAreas[key].totalStayTime += stayTime;
              bAmountChanged = true;
            }
        }

      // Increase visitor counting if necessary
      if (someIn[i])
        d->hashAreas[key].visitors++;
    }
  
  delete[] previousIn;
  delete[] currentIn;
  delete[] someIn;
  delete[] exitTime;

  return bAmountChanged;
}

void PiiMultiPointTracker::operate(const PiiMatrix<int>& coordinates)
{
  PII_D;
  d->tracker.addMeasurements(coordinates, d->trackerTime.elapsed());
}


template <class T> void PiiMultiPointTracker::operateImage(const PiiVariant& obj)
{
  PII_D;
  QList<PiiCoordinateTrackerNode<double,2>* > trajectories = d->tracker;
  QList<QPolygonF> trajects;
  for ( int i=0; i<trajectories.count(); i++ )
    {
      PiiCoordinateTrackerNode<double,2> *node = trajectories[i];
      QPolygonF polygon;
      while ( node )
        {
          QPointF point(node->measurement()[0],node->measurement()[1]);
          polygon.append(point);
          
          node = node->next();
        }
      trajects.append(polygon);
    }

  const PiiMatrix<T>& matrix = obj.valueAs<PiiMatrix<T> >();
  PiiColorQImage* qmatrix = PiiColorQImage::create(matrix);
  
  QPainter painter(qmatrix);
  
  QPen pen(Qt::DashLine);
  pen.setWidth(2);
  pen.setCosmetic(true);
  pen.setColor(QColor(Qt::blue));
  painter.setPen(pen);
  painter.setBrush(QBrush(Qt::NoBrush));

  // Draw calculating lines
  for ( int i=0; i < d->lstLines.size(); i++ )
    painter.drawPolygon(d->lstLines[i].value<QPolygon>());

  // Draw calculating areas
  pen.setColor(QColor(Qt::green));
  painter.setPen(pen);
  for ( int i=0; i < d->lstAreas.size(); i++ )
    painter.drawPolygon(d->lstAreas[i].value<QPolygon>());

  // Draw tracking area
  pen.setColor(QColor(Qt::gray));
  painter.setPen(pen);
  painter.drawRect(d->trackingArea);
  
  pen.setStyle(Qt::SolidLine);
  painter.setBrush(QBrush(Qt::NoBrush));

  // Draw trajectories
  for ( int i=0; i<trajects.count(); i++ )
    {
      pen.setColor(d->colorList[i%d->colorList.size()]);
      painter.setPen(pen);
      painter.drawPolyline(trajects[i]);
    }

  painter.end();
  d->pImageOutput->emitObject(qmatrix->toMatrix());
}


/* 
 * This function checks, if the path from the point @p prev to the
 * point @p curr intersects the calculation line from the point
 * @p calcLineStart to @p calcLineEnd. If there is now intersection, 0 is
 * returned. If the intersection occurs from right to left relative to
 * the calculation line (vector) direction, +1 is returned. In the
 * opposite case (from left to right), -1 is returned.
 */
int PiiMultiPointTracker::checkCalculationLine(const QPoint& calcLineStart, const QPoint& calcLineEnd, const QPoint& prev, const QPoint& curr )
{
  if (hasIntersection(calcLineStart, calcLineEnd, prev, curr))
    {
      return pathDirection(calcLineStart, calcLineEnd, prev, curr);
    }
  else
    {
      return 0;
    }
}

/*
 * Returns true, if the line from @p prev to @p curr intersects the line
 * from @p calcLineStart and @p calcLineEnd
 */
bool PiiMultiPointTracker::hasIntersection(const QPoint& calcLineStart, const QPoint& calcLineEnd, const QPoint& prev, const QPoint& curr )
{
  // Slopes (k) of the calculation line and path
  double kCalcLine, kPath;
  // Intersection (b) between the y-axis and the extension of the
  // calculation line.
  double bCalcLine = 0;
  // Intersection (b) between the y-axis and the extension
  // of the path line.
  double bPath = 0;
  
  // Intersection between the (extension of the) calculation line and the
  // (extension of the) path.
  double x = 0;

  kCalcLine = slope(calcLineStart, calcLineEnd);
  kPath = slope(prev, curr);

  if (kCalcLine == INFINITY)
    {
      // The calculation line has an infinite slope value. If also the
      // slope of the path line is infinite, the slopes are same, and
      // they (or their extensions) don't intersect. In that case false is
      // returned. Else, the x-coordinate of the intersection is the
      // x-coordinate of the calculation line.
      if (kPath == INFINITY)
        {
          return false;
        }
      x = (double)calcLineStart.x();
    }
  else
    {
      bCalcLine = yAxisIntersection(calcLineStart, kCalcLine);
    }
  
  if (kPath == INFINITY)
    {
      // The path has an infinite slope value. The x-coordinate of the
      // intersection is the x-coordinate of the path line.
      // This case recuires a special treatment below. Also
      // y-coordinate needs to be checked.
      x = (double)prev.x();
    }
  else
    {
      bPath = yAxisIntersection(prev, kPath);

      if (kCalcLine != INFINITY)
      {
        // The case, where neither of the slopes has infinity
        // values. X-coordinate of the intersection can be calculated
        // from the line equation.
        
        // Calulate the intersection of the extensions of the
        // calculation line and the path line.
        x=(bPath-bCalcLine)/(kCalcLine-kPath);
        
      }      

    }



  
  // Check if the slopes of the calculation line and the path line are
  // the same. If the slopes are same, the lines don't intersect.
  if (kCalcLine == kPath)
    {
      // Slopes are the same, no intersection.
      return false;
    }
  else
    {
      // Check, if the intersection of the extensions of the both
      // lines is in the actual line (not in either of the extenstions)
      double minXCalcLine = qMin(calcLineStart.x(), calcLineEnd.x());
      double maxXCalcLine = qMax(calcLineStart.x(), calcLineEnd.x());
      double minXPath = qMin(prev.x(), curr.x());
      double maxXPath = qMax(prev.x(), curr.x());
      if (x >= minXCalcLine && x<= maxXCalcLine && x>=minXPath && x<=maxXPath)
        {
          if (kPath == INFINITY)
            {
              // The slope of the path line is infinite. We have to
              // also calculate the y-coordinate of the intersection,
              // and check, if it fits into the required areas.
              double y = kCalcLine*x + bCalcLine;
              double minYCalcLine = qMin(calcLineStart.y(), calcLineEnd.y());
              double maxYCalcLine = qMax(calcLineStart.y(), calcLineEnd.y());
              double minYPath = qMin(prev.y(), curr.y());
              double maxYPath = qMax(prev.y(), curr.y());
              if (y >= minYCalcLine && y<= maxYCalcLine && y>=minYPath && y<=maxYPath)
                {
                  return true;
                }
              else
                {
                  return false;
                }
            }
          else if (kCalcLine == INFINITY)
            {
              // The slope of the calculation line is infinite. We have to
              // also calculate the y-coordinate of the
              // intersection, and check, if it fit into the required
              // areas.
              double y = kPath*x + bPath;
              double minYCalcLine = qMin(calcLineStart.y(), calcLineEnd.y());
              double maxYCalcLine = qMax(calcLineStart.y(), calcLineEnd.y());
              double minYPath = qMin(prev.y(), curr.y());
              double maxYPath = qMax(prev.y(), curr.y());
              if (y >= minYCalcLine && y<= maxYCalcLine && y>=minYPath && y<=maxYPath)
                {
                  return true;
                }
              else
                {
                  return false;
                }
            }
          else
            // normal case
            return true;
        }
      else
        {
          return false;
        }
    }
  
  return false;
}

/*
 * Calculates the slope for the line from @p start to @p end. If the
 * slope has an infinity value, value INFINITY is returned.
 */
double PiiMultiPointTracker::slope(const QPoint& start, const QPoint& end)
{
  if (end.x() == start.x())
    return INFINITY;
  return ((double)end.y()-(double)start.y())/((double)end.x()-(double)start.x());
}

/*
 * Calculates the y coordinate of the intersection of a line and the
 * y-axis. A point in the line and the slope of the line is given as a
 * parameter.
 */
double PiiMultiPointTracker::yAxisIntersection(const QPoint& point, double slope )
{
  return ((double)point.y()-slope*(double)point.x());
}

/*
 * Calculates the path direction relative to the calculation line. If the
 * path goes to leftwards compared to the calculation line (vector), +1 is
 * returned. In the opposite case -1 is returned. If the path goes parallel
 * compared to the calculation line, 0 is returned.
 */
int PiiMultiPointTracker::pathDirection(const QPoint& calcLineStart, const QPoint& calcLineEnd, const QPoint& prev, const QPoint& curr)
{
  // Calculate the dot product of the calculation line vector and the
  // vector perpendicular (90 degrees rotated clockwise) to the path
  // vector. If the result is positive, the path goes leftwards
  // compared to the calculte line, and the path goes in rightwards in
  // the opposite case.
  QPoint calcLineVector = calcLineEnd-calcLineStart;
  QPoint pathVector = curr-prev;
  int dotProduct = calcLineVector.x()*(-pathVector.y()) + calcLineVector.y()*pathVector.x();
  return dotProduct/abs(dotProduct);
}

void PiiMultiPointTracker::setAreas(const QVariantList& areas)
{
  PII_D;
  d->lstAreas.clear();
  d->lstAreas = areas;
  if ( d->lstAreaIdentifications.empty() )
    for ( int i=0; i<d->lstAreas.size(); i++ )
      d->lstAreaIdentifications << i;

  //update area hashtable
  //add new calculation areas
  for ( int i=0; i<d->lstAreaIdentifications.size(); i++ )
    {
      int id = d->lstAreaIdentifications[i].toInt();
      if ( !d->hashAreas.contains(id) )
        {
          AreaStatistics newStruct;
          newStruct.dwellHistogram = PiiMatrix<int>(1,9);
          newStruct.totalStayTime = 0;
          newStruct.totalObjectCount = 0;
          newStruct.visitors = 0;
          d->hashAreas.insert(id, newStruct);
        }
    }
  
  //remove unnecessary areas
  QHashIterator<int, AreaStatistics> k(d->hashAreas);
  while (k.hasNext())
    {
      k.next();
      if ( !d->lstAreaIdentifications.contains(k.key()) )
        d->hashAreas.remove(k.key());
    }

}

void PiiMultiPointTracker::setLines(const QVariantList& lines)
{
  PII_D;
  d->lstLines.clear();
  d->lstLines = lines;
  if ( d->lstLineIdentifications.empty() )
    for ( int i=0; i<d->lstLines.size(); i++ )
      d->lstLineIdentifications << i;

  //update line hashtable
  //add new calculation lines
  for ( int i=0; i<d->lstLineIdentifications.size(); i++ )
    {
      int id = d->lstLineIdentifications[i].toInt();
      if ( !d->hashLines.contains(id) )
        {
          LineStatistics newStruct;
          newStruct.objectsIn = 0;
          newStruct.objectsOut = 0;
          d->hashLines.insert(id, newStruct);
        }
    }
  
  //remove unnecessary lines
  QHashIterator<int, LineStatistics> k(d->hashLines);
  while (k.hasNext())
    {
      k.next();
      if ( !d->lstLineIdentifications.contains(k.key()) )
        d->hashLines.remove(k.key());
    }
}

void PiiMultiPointTracker::setMinimumTrajectoryLength(int minimumTrajectoryLength) { _d()->iMinimumTrajectoryLength = minimumTrajectoryLength; }
int PiiMultiPointTracker::minimumTrajectoryLength() const { return _d()->iMinimumTrajectoryLength; }
void PiiMultiPointTracker::setLineStartPoint(const QPoint& lineStartPoint) { _d()->lineStartPoint = lineStartPoint; }
QPoint PiiMultiPointTracker::lineStartPoint() const { return _d()->lineStartPoint; }
void PiiMultiPointTracker::setLineEndPoint(const QPoint& lineEndPoint) { _d()->lineEndPoint = lineEndPoint; }
QPoint PiiMultiPointTracker::lineEndPoint() const { return _d()->lineEndPoint; }
void PiiMultiPointTracker::setCumulativeStatistics(bool cumulativeStatistics) { _d()->bCumulativeStatistics = cumulativeStatistics; }
bool PiiMultiPointTracker::cumulativeStatistics() const { return _d()->bCumulativeStatistics; }
int PiiMultiPointTracker::frameCount() const { return _d()->iFrameCount; }
void PiiMultiPointTracker::setTrackingArea(const QRect& trackingArea) { _d()->trackingArea = trackingArea; }
QRect PiiMultiPointTracker::trackingArea() const { return _d()->trackingArea; }
void PiiMultiPointTracker::setAreaIdentifications(const QVariantList& areaIdentifications) { _d()->lstAreaIdentifications = areaIdentifications; }
QVariantList PiiMultiPointTracker::areaIdentifications() const { return _d()->lstAreaIdentifications; }
void PiiMultiPointTracker::setLineIdentifications(const QVariantList& lineIdentifications) { _d()->lstLineIdentifications = lineIdentifications; }
QVariantList PiiMultiPointTracker::lineIdentifications() const { return _d()->lstLineIdentifications; }
QVariantList PiiMultiPointTracker::areas() const { return _d()->lstAreas; }
QVariantList PiiMultiPointTracker::lines() const { return _d()->lstLines; }
void PiiMultiPointTracker::setEmissionInterval(int emissionInterval) { _d()->iEmissionInterval = emissionInterval; }
int PiiMultiPointTracker::emissionInterval() const { return _d()->iEmissionInterval; }
void PiiMultiPointTracker::setAllowMerging(bool allowMerging) { _d()->bAllowMerging = allowMerging; }
bool PiiMultiPointTracker::allowMerging() const { return _d()->bAllowMerging; }


void PiiMultiPointTracker::setInitialThreshold(int initialThreshold)
{
  _d()->tracker.setInitialThreshold(initialThreshold);
}

int PiiMultiPointTracker::initialThreshold() const
{
  return _d()->tracker.initialThreshold();
}

void PiiMultiPointTracker::setPredictionThreshold(int predictionThreshold)
{
  _d()->tracker.setPredictionThreshold(predictionThreshold);
}

int PiiMultiPointTracker::predictionThreshold() const
{
  return _d()->tracker.predictionThreshold();
}

void PiiMultiPointTracker::setMaximumStopTime(int maximumStopTime)
{
  _d()->tracker.setMaximumStopTime(maximumStopTime);
}
int PiiMultiPointTracker::maximumStopTime() const
{
  return _d()->tracker.maximumStopTime();
}

void PiiMultiPointTracker::setMaximumPredictionLength(int maximumPredictionLength)
{
  _d()->tracker.setMaximumPredictionLength(maximumPredictionLength);
}
int PiiMultiPointTracker::maximumPredictionLength() const
{
  return _d()->tracker.maximumPredictionLength();
}


// PiiMultiPointTracker::Tracker functions
PiiMultiPointTracker::Tracker::Tracker(PiiMultiPointTracker *parent)
  : _pParent(parent)
{
}

PiiMultiPointTracker::Tracker::~Tracker()
{
}

void PiiMultiPointTracker::Tracker::resetTracker()
{
  qDeleteAll(*this);
  clear();
}

double PiiMultiPointTracker::Tracker::evaluateTrajectory(PiiCoordinateTrackerNode<double,2>* trajectory)
{
  return trajectory->length();
}


void PiiMultiPointTracker::Tracker::predict(int /*t*/)
{
  // Do nothing because we manually invoke the superclass' prediction
  // function at the start of addMeasurements().
}

void PiiMultiPointTracker::Tracker::addMeasurements(const PiiMatrix<int>& coordinates, int t)
{
  PiiCoordinateTracker<double,2>::addMeasurements(PiiMatrix<double>(coordinates), t);
}

void PiiMultiPointTracker::Tracker::addMeasurements(const QList<PiiVector<double,2> >& measurements, int t)
{
  PiiMultiPointTracker::Data* const d = _pParent->_d();
  
  // First predict a new position for all measurements
  PiiCoordinateTracker<double,2>::predict(t);
  
  // Then get rid of all trajectories whose predictions are outside
  // of the tracking area.
  if (d->trackingArea.isValid())
    {
      QList<PiiCoordinateTrackerNode<double,2>*> deletedTrajectories;
      for (int i=count(); i--;)
        {
          PiiVector<double,2>* prediction = at(i)->prediction();
          if (prediction)
            {
              // The prediction is outside of image boundaries ->
              // finalize it
              if (prediction->values[0] < d->trackingArea.left() ||
                  prediction->values[0] > d->trackingArea.right() ||
                  prediction->values[1] < d->trackingArea.top() ||
                  prediction->values[1] > d->trackingArea.bottom())
                deletedTrajectories << takeAt(i);
            }
        }
      endTrajectories(deletedTrajectories, t);
    }

  // Now run the tracker once
  PiiExtendedCoordinateTracker<double,2>::addMeasurements(measurements, t);

  // Evaluate and sort trajectories
  for (int i=count(); i--;)
    {
      PiiCoordinateTrackerNode<double,2> *node = (*this)[i];
      node->setTrajectoryFitness(evaluateTrajectory(node));
    }

  // Puts the trajectories in descending order
  sortTrajectories();

  // Store the number of measurements
  d->matMeasurementCounts(d->iFrameCount % d->matMeasurementCounts.columns()) = measurements.size();

  // The number of routes considered depens on the local maximum in
  // measurement counts.
  int maxRetainedTrajectories = Pii::maxAll(d->matMeasurementCounts) + 1;

  QList<PiiCoordinateTrackerNode<double,2>*> retainedTrajectories;
  // Retain at least one route (the best one) for each measurement
  // independent of its fitness.
  for (int m = measurements.size(); m--; )
    {
      for (int t = count(); t--;)
        {
          // This trajectory ends at this point
          if (at(t)->measurement() == measurements[m])
            {
              retainedTrajectories << takeAt(t);
              break;
            }
        }
    }
  // If we still have room for candidates and something left, let's
  // add them in fitness order
  while (count() > 0 && retainedTrajectories.size() < maxRetainedTrajectories)
    retainedTrajectories << takeLast();

  // Get rid of any remaining ones
  resetTracker();
  
  // We are done
  append(retainedTrajectories);
}

void PiiMultiPointTracker::Tracker::endTrajectories(QList<PiiCoordinateTrackerNode<double,2>*> trajectories, int t)
{
  PiiMultiPointTracker::Data* const d = _pParent->_d();

  bool bLineStatChanged = false;
  bool bAreaStatChanged = false;
  
  if (trajectories.size() > 1)
    qDebug("*** Deleting more than one trajectory");
  for (int i=0; i<trajectories.size(); i++)
    {
      PiiCoordinateTrackerNode<double,2>* trajectory = trajectories[i];
      // Is the trajectory long enough? This should remove spurious
      // branches.
      if (trajectory->lengthToBranch() > d->iMinimumTrajectoryLength)
        {
          qDebug("Trajectory %i was finished at time step %i. Length: %i", i, t, trajectory->lengthToBranch());
          // Collect statistics
          if (_pParent->collectAreaStatistics(trajectory))
            bAreaStatChanged = true;

          if (_pParent->collectLineStatistics(trajectory))
            bLineStatChanged = true;


          if (!d->bAllowMerging)
            {
              // Since this route is stored, get rid of all other alternatives
              // ending at this point.
              for (int j=trajectories.size()-1; j>i; j--)
                if (*trajectories[j] == *trajectories[i])
                  {
                    qDebug("  Also deleted trajectory %i because there is the same end point.", j);
                    delete trajectories.takeAt(j);
                  }
            }
        }
      delete trajectories[i];
    }

  if (bLineStatChanged)
    {
      //debug in and outs
      QList<int> keys = d->hashLines.keys();
      for ( int i=0; i<keys.size(); i++)
        {
          qDebug("Line %i:", keys[i]);
          qDebug(" in \t%i", d->hashLines[keys[i]].objectsIn);
          qDebug(" out\t%i", d->hashLines[keys[i]].objectsOut);
        }
    }
  if (bAreaStatChanged)
    {
      //debug area visitors
      QList<int> areaKeys = d->hashAreas.keys();
      for ( int i=0; i<areaKeys.size(); i++ )
        {
          int key = areaKeys[i];
          qDebug("Area %i:", key);
          qDebug(" visitors %i", d->hashAreas[key].visitors);
        }
    }
}

