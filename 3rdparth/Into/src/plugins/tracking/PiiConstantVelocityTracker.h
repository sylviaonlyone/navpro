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

#ifndef _PIICONSTANTVELOCITYTRACKER_H
#define _PIICONSTANTVELOCITYTRACKER_H

#include "PiiCoordinateTracker.h"
#include "PiiTracking.h"

/**
 * A simple tracker that assumes movement has constant velocity. This
 * tracker is mainly for illustration purposes as it does no more than
 * implements the measurement mode by calling
 * PiiTracking::predictConstantVelocity().
 *
 * @ingroup PiiTrackingPlugin
 */
template <class T, int D> class PiiConstantVelocityTracker : public PiiCoordinateTracker<T,D>
{
protected:
  typename PiiCoordinateTracker<T,D>::MeasurementType* predict(typename PiiCoordinateTracker<T,D>::TrajectoryType* trajectory, int t);
};

template <class T, int D> typename PiiCoordinateTracker<T,D>::MeasurementType*
PiiConstantVelocityTracker<T,D>::predict(typename PiiCoordinateTracker<T,D>::TrajectoryType* trajectory, int t)
{
  return ::PiiTracking::predictConstantVelocity(trajectory, t);
}

#endif //_PIICONSTANTVELOCITYTRACKER_H
