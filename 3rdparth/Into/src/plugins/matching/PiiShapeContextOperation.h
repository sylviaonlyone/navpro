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

#ifndef _PIISHAPECONTEXTOPERATION_H
#define _PIISHAPECONTEXTOPERATION_H

#include <PiiDefaultOperation.h>
#include "PiiMatching.h"

/**
 * An operation that calculates shape descriptors using the Shape
 * Context technique. Feature points are selected on the boundary
 * curve of an object, and the shape context feature vector is
 * calculated for each.
 *
 * The shape context is 2-dimensional histogram that stores the
 * occurrences of boundary pixels with respect to a selected key point
 * in polar coordinates. The discretization of the sampling grid can
 * be controlled by the #angles and #distances properties.
 *
 * @inputs
 *
 * @in boundaries - boundaries of detected objects. Boundaries are
 * represented by a PiiMatrix<int> in which each row stores the (x,y)
 * coordinates of a point on the boundary of an object. The limits of
 * separate boundaries are read from the @p limits input. See
 * PiiBoundaryFinderOperation.
 *
 * @in limits - limits of separate boundaries in the matrix read from
 * @p boundaries.
 *
 * @outputs
 *
 * @out points - the key points to which shape contexts are
 * calculated. An N-by-2 PiiMatrix<int>.
 *
 * @out features - an N-by-M PiiMatrix<float> that stores the shape
 * context descriptors for each key point. The length of each vector
 * (M) is #angles * #distances.
 *
 * @out boundaries - boundaries of calculated objects.
 *
 * @out limits - limits of separate boundaries.
 *
 * @ingroup PiiMatchingPlugin
 */
class PiiShapeContextOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Sample spacing in @p SelectEveryNthPoint mode. The default value
   * is one, which means that every vertex of the incoming boundary
   * polygon is considered a key point.
   */
  Q_PROPERTY(int samplingStep READ samplingStep WRITE setSamplingStep);
  
  /**
   * Tolerance in @p SelectImportantPoints mode. The maximum allowed
   * deviation between an original boundary point and a reduced edge. 
   * The tolerance is used in @p SelectImportantPoints mode to get rid
   * of approximately co-linear boundary points. For example, a value
   * of 5 will allow straightening an edge as far as the maximum
   * deviation from a boundary pixel to the straightened edge is less
   * than 5 pixels. The default value is 3.
   */
  Q_PROPERTY(double tolerance READ tolerance WRITE setTolerance);

  /**
   * The number of quantization bins for angle in the polar coordinate
   * system. The default value is 8, which means 45 degree wide
   * quantization bins.
   */
  Q_PROPERTY(int angles READ angles WRITE setAngles);

  /**
   * The number of quantization bins for distance in the polar
   * coordinate system. The default value is five.
   */
  Q_PROPERTY(int distances READ distances WRITE setDistances);

  /**
   * The size of the first quantization bin for distance. Distance is
   * quantized into growing bins. Everything closer than @p
   * minDistance will be put into the first distance bin. The default
   * value is five.
   */
  Q_PROPERTY(double minDistance READ minDistance WRITE setMinDistance);

  /**
   * A scaling factor for the growing distance bins. The first
   * distance bin will be from zero to #minDistance. The second one
   * will be from @p minDistance to @p minDistance * @p distanceScale
   * and so on. The last bin will be from @p minDistance * @p
   * distanceScale ^ (@p distances - 2) to @p minDistance * @p
   * distanceScale ^ (@p distances - 1). The default value is 2.2.
   */
  Q_PROPERTY(double distanceScale READ distanceScale WRITE setDistanceScale);

  /**
   * Controls the effect of points that are far away from the key
   * point. If the distance between the key point and another point is
   * greater than #minDistance * #distanceScale ^ (#distances - 1), it
   * is "far away". If this flag is @p true, all these points will be
   * collected to the last distance bin. If the value is @p false, the
   * points will be discarded. This makes it possible to switch
   * between a local and global descriptor. The default value is @p
   * false.
   */
  Q_PROPERTY(bool collectDistantPoints READ collectDistantPoints WRITE setCollectDistantPoints);
  
  /**
   * Invariance properties of the descriptor. The shape context
   * descriptor can be made invariant agains rotations (@p
   * RotationInvariant) and scale (@p ScaleInvariant). Other
   * invariance type will be ignored. Rotation invariance is achieved
   * by normalizing the descriptors according to local boundary
   * orientation. Scale invariance is achieved by dividing distances
   * between points by their mean value. In scale invariant mode,
   * #minDistance must be set relative to the mean (squared) distance. 
   * The default value is @p RotationInvariant.
   */
  Q_PROPERTY(PiiMatching::InvarianceFlags invariance READ invariance WRITE setInvariance);
  
  /**
   * Controls how key points are selected on the boundary curves. The
   * default value is @p SelectEveryNthPoint.
   */
  Q_PROPERTY(KeyPointSelectionMode keyPointSelectionMode READ keyPointSelectionMode WRITE setKeyPointSelectionMode);
  Q_ENUMS(KeyPointSelectionMode);

  /**
   * Controls how incoming shapes are joined to form objects. The
   * default is @p JoinAllShapes.
   */
  Q_PROPERTY(ShapeJoiningMode shapeJoiningMode READ shapeJoiningMode WRITE setShapeJoiningMode);
  Q_ENUMS(ShapeJoiningMode);

  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  /**
   * Key point selection modes.
   *
   * @lip SelectEveryNthPoint - every Nth point of the boundary
   * polygon is selected as a key point. The #samplingStep property
   * can be used to change the spacing.
   *
   * @lip SelectImportantPoints - select points that are important in
   * preserving the shape of the boundary. In this mode, the operation
   * will first simplify the polygon representing a boundary so that
   * only the end points of approximately co-linear point sequences
   * are retained. These end points are then selected as key points.
   */
  enum KeyPointSelectionMode { SelectEveryNthPoint, SelectImportantPoints };

  /**
   * Different ways of joining shapes into object descriptors.
   *
   * @lip DoNotJoinShapes - every incoming shape is treated as a
   * separate object. The operation will output one shape context
   * descriptor for each incoming boundary, even for nested ones.
   *
   * @lip JoinNestedShapes - nested shapes are treated as a single
   * object. In this mode, both inner and outer boundaries of an
   * object will be used in building the shape context descriptor. The
   * number of descriptors varies.
   *
   * @lip JoinAllShapes - all incoming shapes are assumed to belong to
   * a single object. The @p limits input is not used in this mode.
   */
  enum ShapeJoiningMode { DoNotJoinShapes, JoinNestedShapes, JoinAllShapes };
  
  PiiShapeContextOperation();

  void check(bool reset);

  void setSamplingStep(int samplingStep);
  int samplingStep() const;
  void setAngles(int angles);
  int angles() const;
  void setDistances(int distances);
  int distances() const;
  void setMinDistance(double minDistance);
  double minDistance() const;
  void setDistanceScale(double distanceScale);
  double distanceScale() const;
  void setTolerance(double tolerance);
  double tolerance() const;
  void setCollectDistantPoints(bool collectDistantPoints);
  bool collectDistantPoints() const;
  void setKeyPointSelectionMode(KeyPointSelectionMode keyPointSelectionMode);
  KeyPointSelectionMode keyPointSelectionMode() const;
  void setShapeJoiningMode(ShapeJoiningMode shapeJoiningMode);
  ShapeJoiningMode shapeJoiningMode() const;
  void setInvariance(PiiMatching::InvarianceFlags invariance);
  PiiMatching::InvarianceFlags invariance() const;

protected:
  void process();

private:
  /**
   * Process the given boundary.
   */
  void processBoundary(const PiiMatrix<int>& boundary, const PiiMatrix<int>& limits);

  /**
   * Reduce points from the given boundary depends on
   * KeyPointSelectionMode.
   */
  PiiMatrix<int> reducePoints(const PiiMatrix<int>& boundary, bool addLastPoint);

  /**
   * Join nested shapes.
   */
  QList<PiiMatrix<int> > joinNestedShapes(const PiiMatrix<int>& boundaries,
                                          const PiiMatrix<int>& limits,
                                          PiiMatrix<int>& joinedBoundaries);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    
    int iSamplingStep;
    double dTolerance;
    int iAngles;
    int iDistances;
    double dMinDistance;
    double dDistanceScale;
    bool bCollectDistantPoints;
    PiiMatching::InvarianceFlags invariance;
    KeyPointSelectionMode keyPointSelectionMode;
    ShapeJoiningMode shapeJoiningMode;
    QVector<double> vecDistances;
    PiiInputSocket *pBoundariesInput, *pLimitsInput;
    PiiOutputSocket *pPointsOutput, *pFeaturesOutput, *pBoundariesOutput, *pLimitsOutput;
  };
  PII_D_FUNC;
};

#endif //_PIISHAPECONTEXTOPERATION_H
