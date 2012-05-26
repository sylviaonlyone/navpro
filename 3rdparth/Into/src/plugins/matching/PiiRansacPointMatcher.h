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

#ifndef _PIIRANSACPOINTMATCHER_H
#define _PIIRANSACPOINTMATCHER_H

#include "PiiPointMatchingOperation.h"

class PiiRansac;

/**
 * A superclass for operations that use @ref PiiRansac "RANSAC" for
 * matching points. This class performs no useful action of its own,
 * but merely reflects the configuration values of the RANSAC
 * algorithm as properties. See PiiRansac for documentation on the
 * properties.
 *
 * @ingroup PiiMatchingPlugin
 */
class PII_MATCHING_EXPORT PiiRansacPointMatcher : public PiiPointMatchingOperation
{
  Q_OBJECT
  
  Q_PROPERTY(int maxIterations READ maxIterations WRITE setMaxIterations);
  Q_PROPERTY(int maxSamplings READ maxSamplings WRITE setMaxSamplings);
  Q_PROPERTY(int minInliers READ minInliers WRITE setMinInliers);
  Q_PROPERTY(double fittingThreshold READ fittingThreshold WRITE setFittingThreshold);
  Q_PROPERTY(double selectionProbability READ selectionProbability WRITE setSelectionProbability);
  
public:
  void setMaxIterations(int maxIterations);
  int maxIterations() const;
  void setMaxSamplings(int maxSamplings);
  int maxSamplings() const;
  void setMinInliers(int minInliers);
  int minInliers() const;
  void setFittingThreshold(double fittingThreshold);
  double fittingThreshold() const;
  void setSelectionProbability(double selectionProbability);
  double selectionProbability() const;
  
protected:
  /// @internal
  class Data : public PiiPointMatchingOperation::Data
  {
  public:
    Data(int pointDimensions, PiiRansac* ransac);
    ~Data();
    
    PiiRansac* pRansac;
  };
  PII_D_FUNC;

  /// @internal
  PiiRansacPointMatcher(Data* data);

  /**
   * Constructs a new %PiiRansacPointMatcher with the given number of
   * dimensions for the point space and a pointer to a newly allocated
   * RANSAC estimator. This class takes the ownership of the pointer.
   */
  PiiRansacPointMatcher(int pointDimensions, PiiRansac* ransac);

  /**
   * Returns a reference to the internal RANSAC estimator.
   */
  PiiRansac& ransac();
  /**
   * @overload
   */
  const PiiRansac& ransac() const;
};

#endif //_PIIRANSACPOINTMATCHER_H
