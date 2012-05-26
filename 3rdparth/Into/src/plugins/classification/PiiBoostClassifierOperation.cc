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

#include "PiiBoostClassifierOperation.h"

#include <PiiYdinTypes.h>

PiiBoostClassifierOperation::Data::Data() :
  PiiClassifierOperation::Data(PiiClassification::WeightedLearner),
  algorithm(PiiClassification::RealBoost),
  iMaxClassifiers(100),
  dMinError(0)
{
}

PiiBoostClassifierOperation::PiiBoostClassifierOperation(Data* d) :
  PiiClassifierOperation(d)
{
}

PiiBoostClassifierOperation::~PiiBoostClassifierOperation()
{}

PiiClassification::BoostingAlgorithm PiiBoostClassifierOperation::algorithm() const { return _d()->algorithm; }
void PiiBoostClassifierOperation::setAlgorithm(PiiClassification::BoostingAlgorithm algorithm) { _d()->algorithm = algorithm; }
void PiiBoostClassifierOperation::setMaxClassifiers(int maxClassifiers) { _d()->iMaxClassifiers = maxClassifiers; }
int PiiBoostClassifierOperation::maxClassifiers() const { return _d()->iMaxClassifiers; }
void PiiBoostClassifierOperation::setMinError(double minError) { _d()->dMinError = minError; }
double PiiBoostClassifierOperation::minError() const { return _d()->dMinError; }
