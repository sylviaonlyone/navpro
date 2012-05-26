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

#include "PiiFourierTransform.h"

PiiFourierTransform::Data::Data() :
  direction(Forward), bShift(false), bSubtractMean(false)
{
}

PiiFourierTransform::PiiFourierTransform(Data* data) :
  PiiDefaultOperation(data, Threaded)
{
  init();
}

PiiFourierTransform::PiiFourierTransform() :
  PiiDefaultOperation(new Data, Threaded)
{
  init();
}

void PiiFourierTransform::init()
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
  addSocket(new PiiOutputSocket("power spectrum"));
}


PiiFourierTransform::FftDirection PiiFourierTransform::direction() const { return _d()->direction; }
void PiiFourierTransform::setDirection(FftDirection direction) { _d()->direction = direction; }
bool PiiFourierTransform::shift() const { return _d()->bShift; }
void PiiFourierTransform::setShift(bool shift) { _d()->bShift = shift; }

void PiiFourierTransform::setSubtractMean(bool subtractMean) { _d()->bSubtractMean = subtractMean; }
bool PiiFourierTransform::subtractMean() const { return _d()->bSubtractMean; }
