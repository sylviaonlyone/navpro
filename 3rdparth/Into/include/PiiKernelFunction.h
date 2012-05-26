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

#ifndef _PIIKERNELFUNCTION_H
#define _PIIKERNELFUNCTION_H

#include "PiiDistanceMeasure.h"

/**
 * @file
 * @ingroup PiiClassificationPlugin
 */

#ifdef PII_CXX0X
template <class SampleSet> using PiiKernelFunction = PiiDistanceMeasure<SampleSet>
#else
#  define PiiKernelFunction PiiDistanceMeasure
#endif

/**
 * Type definition for a polymorphic implementation of the function
 * object @a KERNEL.
 *
 * @code
 * PiiKernelFunction<ConstFeatureIterator>* pKernel = new PII_POLYMORPHIC_KERNEL(PiiGaussianKernel);
 * @endcode
 */
#define PII_POLYMORPHIC_KERNEL(KERNEL) typename PiiKernelFunction<ConstFeatureIterator>::template Impl<KERNEL<ConstFeatureIterator> >

#endif //_PIIKERNELFUNCTION_H
