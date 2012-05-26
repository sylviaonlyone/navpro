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

#include "PiiRigidPlaneMatcher.h"

#include <PiiYdinTypes.h>
#include <PiiRigidPlaneRansac.h>
#include <PiiMath.h>


PiiRigidPlaneMatcher::Data::Data() :
  PiiRansacPointMatcher::Data(2, new PiiRigidPlaneRansac<float>),
  dScaleTolerance(0.1),
  dAngleTolerance(M_PI/36),
  dTranslationTolerance(10)
{
}

PiiRigidPlaneMatcher::PiiRigidPlaneMatcher() :
  PiiRansacPointMatcher(new Data)
{
}

PiiMatching::MatchList PiiRigidPlaneMatcher::match(Matcher& matcher,
                                                   const PiiMatrix<float>& points,
                                                   const PiiMatrix<float>& features)
{
  return matcher.findMatchingModels(points, features, ransac());
}

PiiMatrix<double> PiiRigidPlaneMatcher::toTransformMatrix(const PiiMatrix<double>& transformParams)
{
  return PiiRigidPlaneRansac<float>::toTransformMatrix(transformParams);
}

bool PiiRigidPlaneMatcher::operator() (const PiiMatching::Match& match1, PiiMatching::Match& match2)
{
  PII_D;
  const double* p1 = match1.transformParams().constRowBegin(0);
  const double* p2 = match2.transformParams().constRowBegin(0);
  if (1 - qMin(p1[0], p2[0]) / qMax(p1[0], p2[0]) <= d->dScaleTolerance &&
      Pii::abs(Pii::angleDiff(p1[1], p2[1])) <= d->dAngleTolerance &&
      Pii::square(p1[2] - p2[2]) + Pii::square(p1[3] - p2[3]) <= Pii::square(d->dTranslationTolerance))
    {
      if (match2.matchedPointCount() < match1.matchedPointCount())
        match2 = match1;
      return true;
    }
  return false;
}

void PiiRigidPlaneMatcher::removeDuplicates(PiiMatching::MatchList& matchedModels)
{
  PiiMatching::removeDuplicates(matchedModels, *this);
}

PiiRigidPlaneRansac<float>& PiiRigidPlaneMatcher::ransac()
{
  return static_cast<PiiRigidPlaneRansac<float>&>(PiiRansacPointMatcher::ransac());
}
const PiiRigidPlaneRansac<float>& PiiRigidPlaneMatcher::ransac() const
{
  return static_cast<const PiiRigidPlaneRansac<float>&>(PiiRansacPointMatcher::ransac());
}

void PiiRigidPlaneMatcher::setMinScale(double minScale) { ransac().setMinScale(minScale); }
double PiiRigidPlaneMatcher::minScale() const { return ransac().minScale(); }
void PiiRigidPlaneMatcher::setMaxScale(double maxScale) { ransac().setMaxScale(maxScale); }
double PiiRigidPlaneMatcher::maxScale() const { return ransac().maxScale(); }
void PiiRigidPlaneMatcher::setMaxRotationAngle(double maxRotationAngle) { ransac().setMaxRotationAngle(maxRotationAngle); }
double PiiRigidPlaneMatcher::maxRotationAngle() const { return ransac().maxRotationAngle(); }
void PiiRigidPlaneMatcher::setMaxRotationAngleDeg(double maxRotationAngle) { setMaxRotationAngle(Pii::degToRad(maxRotationAngle)); }
double PiiRigidPlaneMatcher::maxRotationAngleDeg() const { return Pii::radToDeg(maxRotationAngle()); }
void PiiRigidPlaneMatcher::setAutoRefine(bool autoRefine) { ransac().setAutoRefine(autoRefine); }
bool PiiRigidPlaneMatcher::autoRefine() const { return ransac().autoRefine(); }

void PiiRigidPlaneMatcher::setScaleTolerance(double scaleTolerance) { _d()->dScaleTolerance = scaleTolerance; }
double PiiRigidPlaneMatcher::scaleTolerance() const { return _d()->dScaleTolerance; }
void PiiRigidPlaneMatcher::setAngleTolerance(double angleTolerance) { _d()->dAngleTolerance = angleTolerance; }
double PiiRigidPlaneMatcher::angleTolerance() const { return _d()->dAngleTolerance; }
void PiiRigidPlaneMatcher::setAngleToleranceDeg(double angleTolerance) { setAngleTolerance(Pii::degToRad(angleTolerance)); }
double PiiRigidPlaneMatcher::angleToleranceDeg() const { return Pii::radToDeg(angleTolerance()); }
void PiiRigidPlaneMatcher::setTranslationTolerance(double translationTolerance) { _d()->dTranslationTolerance = translationTolerance; }
double PiiRigidPlaneMatcher::translationTolerance() const { return _d()->dTranslationTolerance; }
