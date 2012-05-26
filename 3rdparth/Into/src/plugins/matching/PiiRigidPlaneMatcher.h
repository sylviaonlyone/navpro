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

#ifndef _PIIRIGIDPLANEMATCHER_H
#define _PIIRIGIDPLANEMATCHER_H

#include "PiiRansacPointMatcher.h"

template <class T> class PiiRigidPlaneRansac;

/**
 * An operation that matches a set of feature points to a database
 * assuming that only in-plane affine transforms are possible.
 *
 * @see PiiRigidPlaneRansac
 *
 * @ingroup PiiMatchingPlugin
 */
class PiiRigidPlaneMatcher : public PiiRansacPointMatcher
{
  Q_OBJECT

  Q_PROPERTY(bool autoRefine READ autoRefine WRITE setAutoRefine);
  Q_PROPERTY(double minScale READ minScale WRITE setMinScale);
  Q_PROPERTY(double maxScale READ maxScale WRITE setMaxScale);
  Q_PROPERTY(double maxRotationAngle READ maxRotationAngle WRITE setMaxRotationAngle);
  /**
   * Same as #maxRotationAngle, but in degrees.
   */
  Q_PROPERTY(double maxRotationAngleDeg READ maxRotationAngleDeg WRITE setMaxRotationAngleDeg STORED false);

  /**
   * The maximum allowed relative scale change in merging overlapping
   * detections. Setting @p scaleTolerance to a non-zero value makes
   * it possible to merge detected models even if their sizes are
   * slightly different. The default value is 0.1, which allows a 10%
   * scale change.
   */
  Q_PROPERTY(double scaleTolerance READ scaleTolerance WRITE setScaleTolerance);

  /**
   * The maximum allowed angle change in merging overlapping
   * detections. Setting @p angleTolerance to a non-zero value makes
   * it possible to merge detected models even if their angles are
   * slightly different. The default is pi/36, which allows a rotation
   * of at most 5 degrees.
   */
  Q_PROPERTY(double angleTolerance READ angleTolerance WRITE setAngleTolerance);
  /**
   * Same as #angleTolerance, but in degrees.
   */
  Q_PROPERTY(double angleToleranceDeg READ angleToleranceDeg WRITE setAngleToleranceDeg STORED false);

  /**
   * The maximum allowed position change in merging overlapping
   * detections. Setting @p translationTolerance to a non-zero value
   * makes it possible to merge detected models even if their
   * locations are slightly different. The default is 10, which allows
   * a translation of at most 10 pixels.
   */
  Q_PROPERTY(double translationTolerance READ translationTolerance WRITE setTranslationTolerance);

  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION;
  PII_PROPERTY_SERIALIZATION_FUNCTION(PiiRansacPointMatcher)
public:
  PiiRigidPlaneMatcher();

  void setMinScale(double minScale);
  double minScale() const;
  void setMaxScale(double maxScale);
  double maxScale() const;
  void setMaxRotationAngle(double maxRotationAngle);
  double maxRotationAngle() const;
  void setMaxRotationAngleDeg(double maxRotationAngle);
  double maxRotationAngleDeg() const;
  void setAutoRefine(bool autoRefine);
  bool autoRefine() const;
  void setScaleTolerance(double scaleTolerance);
  double scaleTolerance() const;
  void setAngleTolerance(double angleTolerance);
  double angleTolerance() const;
  void setAngleToleranceDeg(double angleToleranceDeg);
  double angleToleranceDeg() const;
  void setTranslationTolerance(double translationTolerance);
  double translationTolerance() const;

protected:
  PiiMatching::MatchList match(Matcher& matcher,
                               const PiiMatrix<float>& points,
                               const PiiMatrix<float>& features);

  PiiMatrix<double> toTransformMatrix(const PiiMatrix<double>& transformParams);

  /**
   * This function merges two matches if the differences in their
   * parameters are in tolerances (#scaleTolerance, #angleTolerance,
   * #translationTolerance). "Merging" is done by simply selecting the
   * match with more matched points.
   */
  void removeDuplicates(PiiMatching::MatchList& matchedModels);
  
private:
  class Data : public PiiRansacPointMatcher::Data
  {
  public:
    Data();
    double dScaleTolerance;
    double dAngleTolerance;
    double dTranslationTolerance;
  };
  PII_D_FUNC;

  template <class Merger> friend void PiiMatching::removeDuplicates(MatchList&, Merger&);
  bool operator() (const PiiMatching::Match& match1, PiiMatching::Match& match2);

  inline PiiRigidPlaneRansac<float>& ransac();
  inline const PiiRigidPlaneRansac<float>& ransac() const;
};


#endif //_PIIRIGIDPLANEMATCHER_H
