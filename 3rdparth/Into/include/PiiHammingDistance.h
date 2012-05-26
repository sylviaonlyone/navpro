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

#ifndef _PIIHAMMINGDISTANCE_H
#define _PIIHAMMINGDISTANCE_H

#include "PiiDistanceMeasure.h"
#include <PiiBits.h>

/**
 * Calculates the Hamming distance between two integer-valued feature
 * vectors. The Hamming distance is the number of different bits in
 * two numbers.
 *
 * @ingroup PiiClassificationPlugin
 */
PII_DEFAULT_DISTANCE_MEASURE_DEF(PiiHammingDistance)
{
  double distance = 0;
	for (int i=0; i<length; ++i)
    distance += Pii::hammingDistance(sample[i], model[i],
                                     sizeof(typename std::iterator_traits<FeatureIterator>::value_type));
  return distance;
}

#endif //_PIIHAMMINGDISTANCE_H
