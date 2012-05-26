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

#ifndef _PIIMULTIPOINTTRACKER_H
#define _PIIMULTIPOINTTRACKER_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>
#include <QTime>
#include <QPoint>
#include <PiiExtendedCoordinateTracker.h>

/**
 * PiiMultiPointTracker description
 *
 * @inputs
 *
 * @in coordinates - PiiMatrix<int>
 * @in image       - image (for test)
 * 
 * @outputs
 *
 * @out area id         - Description
 * @out dwell histogram - Description
 * @out average dwell   - Description
 * @out objects         - Description
 * @out visitors        - Description
 * @out area start time - Description
 * @out area end time   - Description
 * @out line id         - Description
 * @out objects in      - Description
 * @out objects out     - Description
 * @out line start time - Description
 * @out line end time   - Description
 * @out image           - Description
 *
 * @ingroup OtosTrackerPlugin
 */
class PiiMultiPointTracker : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * initialThreshold description
   */
  Q_PROPERTY(double initialThreshold READ initialThreshold WRITE setInitialThreshold);

  /**
   * predictionThreshold description
   */
  Q_PROPERTY(double predictionThreshold READ predictionThreshold WRITE setPredictionThreshold);
  
  /**
   * maximumStopTime description
   */
  Q_PROPERTY(int maximumStopTime READ maximumStopTime WRITE setMaximumStopTime);

  /**
   * maximumPredictionLength description
   */
  Q_PROPERTY(int maximumPredictionLength READ maximumPredictionLength WRITE setMaximumPredictionLength);

  /**
   * minimumTrajectoryLength description
   */
  Q_PROPERTY(int minimumTrajectoryLength READ minimumTrajectoryLength WRITE setMinimumTrajectoryLength);

  /**
   * lineStartPoint description
   */
  Q_PROPERTY(QPoint lineStartPoint READ lineStartPoint WRITE setLineStartPoint);
  /**
   * lineEndPoint description
   */
  Q_PROPERTY(QPoint lineEndPoint READ lineEndPoint WRITE setLineEndPoint);

  /**
   * cumulativeStatistics description
   */
  Q_PROPERTY(bool cumulativeStatistics READ cumulativeStatistics WRITE setCumulativeStatistics);
  
  /**
   * frameCount description
   */
  Q_PROPERTY(int frameCount READ frameCount WRITE setFrameCount);

  /**
   * trackingArea description
   */
  Q_PROPERTY(QRect trackingArea READ trackingArea WRITE setTrackingArea);


  /**
   * areaIdentifications description
   */
  Q_PROPERTY(QVariantList areaIdentifications READ areaIdentifications WRITE setAreaIdentifications);

  /**
   * lineIdentifications description
   */
  Q_PROPERTY(QVariantList lineIdentifications READ lineIdentifications WRITE setLineIdentifications);

  /**
   * areas description
   */
  Q_PROPERTY(QVariantList areas READ areas WRITE setAreas);

  /**
   * lines description
   */
  Q_PROPERTY(QVariantList lines READ lines WRITE setLines);
  
  /**
   * emissionInterval description
   */
  Q_PROPERTY(int emissionInterval READ emissionInterval WRITE setEmissionInterval);

  /**
   * Allow merging in the trajectory. If this property is true, we will
   * allow merging of the trajectories. If this property is false, we
   * will allow only the first trajectory of the group where is the
   * same end point.
   */
  Q_PROPERTY(bool allowMerging READ allowMerging WRITE setAllowMerging);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiMultiPointTracker();

  void setInitialThreshold(int initialThreshold);
  int initialThreshold() const;

  void setPredictionThreshold(int predictionThreshold);
  int predictionThreshold() const;

  void setMaximumStopTime(int maximumStopTime);
  int maximumStopTime() const;

  void setMaximumPredictionLength(int maximumPredictionLength);
  int maximumPredictionLength() const;

  void setMinimumTrajectoryLength(int minimumTrajectoryLength);
  int minimumTrajectoryLength() const;

  void setLineStartPoint(const QPoint& lineStartPoint);
  QPoint lineStartPoint() const;

  void setLineEndPoint(const QPoint& lineEndPoint);
  QPoint lineEndPoint() const;

  void setCumulativeStatistics(bool cumulativeStatistics);
  bool cumulativeStatistics() const;

  void setFrameCount(int frameCount);
  int frameCount() const;

  void setTrackingArea(const QRect& trackingArea);
  QRect trackingArea() const;
  
  void setAreaIdentifications(const QVariantList& areaIdentifications);
  QVariantList areaIdentifications() const;
  
  void setLineIdentifications(const QVariantList& lineIdentifications);
  QVariantList lineIdentifications() const;

  void setAreas(const QVariantList& areas);
  QVariantList areas() const;

  void setLines(const QVariantList& lines);
  QVariantList lines() const;

  void setEmissionInterval(int emissionInterval);
  int emissionInterval() const;

  void setAllowMerging(bool allowMerging);
  bool allowMerging() const;

  struct AreaStatistics
  {
    PiiMatrix<int> dwellHistogram;
    int totalObjectCount;
    int totalStayTime;
    int visitors;
  };
  
  struct LineStatistics
  {
    int objectsIn;
    int objectsOut;
  };
  
protected:
  void process();
  void check(bool reset);

private:
  friend class Tracker;
  
  double evaluateTrajectory(PiiCoordinateTrackerNode<double,2>* trajectory);

  int mapTime(int time);
  bool collectLineStatistics(PiiCoordinateTrackerNode<double,2>* trajectory);
  bool collectAreaStatistics(PiiCoordinateTrackerNode<double,2>* trajectory);

  static int checkCalculationLine(const QPoint& calcLineStart, const QPoint& calcLineEnd, const QPoint& prev, const QPoint& curr );
  static bool hasIntersection(const QPoint& calcLineStart, const QPoint& calcLineEnd, const QPoint& prev, const QPoint& curr );
  static double slope(const QPoint& start, const QPoint& end);
  static double yAxisIntersection(const QPoint& point, double slope );
  static int pathDirection(const QPoint& calcLineStart, const QPoint& calcLineEnd, const QPoint& prev, const QPoint& curr);
  
  void operate( const PiiMatrix<int>& coordinates );
  template <class T> void operateImage(const PiiVariant& obj);

  class Tracker : public PiiExtendedCoordinateTracker<double,2>
  {
  public:
    Tracker(PiiMultiPointTracker *parent);
    ~Tracker();
    
    void resetTracker();
    double evaluateTrajectory(PiiCoordinateTrackerNode<double,2>* trajectory);
    void addMeasurements(const PiiMatrix<int>& coordinates, int t);
    void addMeasurements(const QList<PiiVector<double,2> >& measurements, int t);
    void predict(int t);
    void endTrajectories(QList<PiiCoordinateTrackerNode<double,2>*> trajectories, int t);
    
  private:
    PiiMultiPointTracker* _pParent;
  };
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data(PiiMultiPointTracker *parent);
    ~Data();
    
    PiiOutputSocket *pAreaIdOutput,*pDwellHistogramOutput,*pAverageDwellOutput,
      *pObjectsOutput,*pVisitorsOutput,*pAreaStartTimeOutput,
      *pAreaEndTimeOutput,*pLineIdOutput,*pObjectsInOutput,
      *pObjectsOutOutput,*pLineStartTimeOutput,*pLineEndTimeOutput,
      *pImageOutput;

    Tracker tracker;
    QTime trackerTime;
    unsigned int uiPreviousEmissionTime;
    int iMinimumTrajectoryLength;
    QPoint lineStartPoint;
    QPoint lineEndPoint;
    int iFrameCount;
    QList<QColor> colorList;
    PiiMatrix<int> matMeasurementCounts;
    bool bCumulativeStatistics;
    QRect trackingArea;
    QVariantList lstAreaIdentifications;
    QVariantList lstLineIdentifications;
    QVariantList lstAreas;
    QVariantList lstLines;
    int iEmissionInterval;
    QHash<int, AreaStatistics> hashAreas;
    QHash<int, LineStatistics> hashLines;
    bool bAllowMerging;
  };
  PII_D_FUNC;
};

#endif //_PIIMULTIPOINTTRACKER_H
