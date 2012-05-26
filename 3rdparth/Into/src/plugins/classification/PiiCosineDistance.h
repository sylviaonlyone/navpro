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

#ifndef _PIICOSINEDISTANCE_H
#define _PIICOSINEDISTANCE_H

#include "PiiDistanceMeasure.h"

/**
 * Cosine distance calculates the cosine of the angle between feature
 * vectors.
 *
 * @ingroup PiiClassificationPlugin
 */
PII_DEFAULT_DISTANCE_MEASURE_DEF(PiiCosineDistance)
{
  double sum = 0.0, len1 = 0.0, len2 = 0.0, tmp1, tmp2;
  for (int i=0; i<length; ++i)
    {
      tmp1 = double(sample[i]);
      tmp2 = double(model[i]);
      sum += tmp1*tmp2;
      len1 += tmp1*tmp1;
      len2 += tmp2*tmp2;
    }
	len1 *= len2;
	if (len1 != 0)
		return -sum/::sqrt(len1);
	return 0;
}

#endif //_PIICOSINEDISTANCE_H
