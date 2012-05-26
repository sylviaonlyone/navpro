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

#include "PiiRandomLbp.h"

#include <cstdlib>

PiiRandomLbp::Data::Data() :
  iPatterns(50),
  iPairs(11)
{}

PiiMatrix<int> PiiRandomLbp::initializeHistogram() const
{
  PiiMatrix<int> matResult(1, d->iPatterns * (1 << d->iPairs));
  matResult = 1;
  return matResult;
}

void PiiRandomLbp::setParameters(int patterns, int pairs, int rows, int columns)
{
  d->iPatterns = patterns;
  d->iPairs = pairs;

  d->vecPointPairs.clear();
  d->vecPointPairs.reserve(patterns*pairs);

  for (int i=0; i<patterns*pairs; ++i)
    d->vecPointPairs << qMakePair(PiiPoint<int>(rand() % rows, rand() % columns),
                                  PiiPoint<int>(rand() % rows, rand() % columns));
  /* PENDING
   * The pairs could be reordered to optimize cache usage.
   */
}
