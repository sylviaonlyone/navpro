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

#ifndef _PIIABSDIFFDISTANCE_H
#define _PIIABSDIFFDISTANCE_H

#include "PiiDistanceMeasure.h"

/**
 * Calculates the sum of absolute differences between corresponding
 * elements in two feature vectors. @f$d = \sum |S_i - M_i|@f$, where
 * @e S and @e M represent the sample and model distributions,
 * respectively.
 *
 * @ingroup PiiClassificationPlugin
 */
PII_DEFAULT_DISTANCE_MEASURE_DEF(PiiAbsDiffDistance)
{
  double distance = 0;
	for (int i=0; i<length; ++i)
    distance += Pii::abs(sample[i] - model[i]);
  return distance;
}

#endif //_PIIABSDIFFDISTANCE_H
