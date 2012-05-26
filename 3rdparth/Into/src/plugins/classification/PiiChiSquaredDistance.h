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

#ifndef _PIICHISQUAREDDISTANCE_H
#define _PIICHISQUAREDDISTANCE_H

#include "PiiDistanceMeasure.h"

/**
 * Chi squared distance. The chi squared distance between two vectors
 * S and M is defined as @f$ d = \frac{1}{2} \sum (S_i - M_i)^2 / (S_i
 * + M_i) @f$. This distance measure doesn't however divide the sum by
 * two.
 *
 * @ingroup PiiClassificationPlugin
 */
PII_DEFAULT_DISTANCE_MEASURE_DEF(PiiChiSquaredDistance)
{
  double sum = 0.0, tmp;
  for (int i=0; i<length; ++i)
    {
      tmp = double(sample[i] - model[i]);
      sum += tmp*tmp / (sample[i] + model[i]);
    }
  return sum;
}	

#endif //_PIICHISQUAREDDISTANCE_H
