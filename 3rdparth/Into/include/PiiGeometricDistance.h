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

#ifndef _PIIGEOMETRICDISTANCE_H
#define _PIIGEOMETRICDISTANCE_H

#include "PiiDistanceMeasure.h"

/**
 * Geometric distance. The geometric distance is calculated as the
 * square root of the sum of squared differences of the vectors: @f$ d
 * = \sqrt{(S-M)(S-M)^T} @f$, where @e S and @e M represent the sample
 * and model feature vectors, respectively.
 *
 * @ingroup PiiClassificationPlugin
 */
PII_DEFAULT_DISTANCE_MEASURE_DEF(PiiGeometricDistance)
{
  double sum = 0.0, tmp;
  for (int i=0; i<length; ++i)
    {
      tmp = double(sample[i] - model[i]);
      sum += tmp*tmp;
    }
  return sqrt(sum);
}	

#endif //_PIIGEOMETRICDISTANCE_H
