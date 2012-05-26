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

#include "PiiRansacPointMatcher.h"
#include <PiiRansac.h>

PiiRansacPointMatcher::Data::Data(int pointDimensions, PiiRansac* ransac) :
  PiiPointMatchingOperation::Data(pointDimensions),
  pRansac(ransac)
{}

PiiRansacPointMatcher::Data::~Data()
{
  delete pRansac;
}

PiiRansacPointMatcher::PiiRansacPointMatcher(Data* data) :
  PiiPointMatchingOperation(data)
{}

PiiRansacPointMatcher::PiiRansacPointMatcher(int pointDimensions, PiiRansac* ransac) :
  PiiPointMatchingOperation(new Data(pointDimensions, ransac))
{}

void PiiRansacPointMatcher::setMaxIterations(int maxIterations) { _d()->pRansac->setMaxIterations(maxIterations); }
int PiiRansacPointMatcher::maxIterations() const { return _d()->pRansac->maxIterations(); }
void PiiRansacPointMatcher::setMaxSamplings(int maxSamplings) { _d()->pRansac->setMaxSamplings(maxSamplings); }
int PiiRansacPointMatcher::maxSamplings() const { return _d()->pRansac->maxSamplings(); }
void PiiRansacPointMatcher::setMinInliers(int minInliers) { _d()->pRansac->setMinInliers(minInliers); }
int PiiRansacPointMatcher::minInliers() const { return _d()->pRansac->minInliers(); }
void PiiRansacPointMatcher::setFittingThreshold(double fittingThreshold) { _d()->pRansac->setFittingThreshold(fittingThreshold); }
double PiiRansacPointMatcher::fittingThreshold() const { return _d()->pRansac->fittingThreshold(); }
void PiiRansacPointMatcher::setSelectionProbability(double selectionProbability) { _d()->pRansac->setSelectionProbability(selectionProbability); }
double PiiRansacPointMatcher::selectionProbability() const { return _d()->pRansac->selectionProbability(); }

PiiRansac& PiiRansacPointMatcher::ransac() { return *_d()->pRansac; }
const PiiRansac& PiiRansacPointMatcher::ransac() const { return *_d()->pRansac; }
