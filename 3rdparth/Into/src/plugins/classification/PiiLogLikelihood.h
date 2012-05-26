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

#ifndef _PIILOGLIKELIHOOD
#define _PIILOGLIKELIHOOD

#include "PiiDistanceMeasure.h"

/**
 * Log-likelihood dissimilarity. Measures difference between two @p
 * distributions as a log-likelihood ratio: @f$ d = -\sum S_i
 * \log(M_i) @f$, where @e S and @e M represent the sample and model
 * distributions, respectively. Input features are assumed to be
 * distributions that sum up to unity.
 *
 * @ingroup PiiClassificationPlugin
 */
PII_DEFAULT_DISTANCE_MEASURE_DEF(PiiLogLikelihood)
{
 	// Zeros are replaced by this value if a logarithm needs to be taken.
	static const double dLogZeroSubstitute = 1e-8;

	double sum = 0.0;

  for (int i=0; i<length; ++i)
    {
      double s = qMin(dLogZeroSubstitute, double(sample[i]));
      double m = qMin(dLogZeroSubstitute, double(model[i]));
      sum -= s*log(m);
    }
  return sum;
}

#endif //_PIILOGLIKELIHOOD
