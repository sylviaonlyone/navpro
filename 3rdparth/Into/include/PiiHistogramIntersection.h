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

#ifndef _PIIHISTOGRAMINTERSECTION_H
#define _PIIHISTOGRAMINTERSECTION_H

#include "PiiDistanceMeasure.h"

/**
 * Histogram intersection. Measures difference between two
 * distributions in a straightforward way: @f$ d = - \sum
 * \min(S_i,M_i) @f$ , where @e S and @e M represent the sample and
 * model distributions, respectively. Histogram intersection assumes
 * that the input samples are distributions. In classification, the
 * sum of the values in each should be the same, preferably one.
 *
 * The histogram intersection is optimized so that it produces a
 * negative distance. The minimum possible value (@e S and @e M are
 * equal) is the negation of the sum of input features. The maximum is
 * zero.
 *
 * @ingroup PiiClassificationPlugin
 */
PII_DEFAULT_DISTANCE_MEASURE_DEF(PiiHistogramIntersection)
{
  double diffSum = 0.0;
  for (int i=0; i<length; ++i)
    diffSum += double(qMin(sample[i], model[i]));

  return -diffSum;
}

#endif //_PIIHISTOGRAMINTERSECTION_H
