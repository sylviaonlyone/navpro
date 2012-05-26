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

#include <PiiYdinTypes.h>
#include "PiiCornerDetector.h"
#include "PiiImage.h"

PiiCornerDetector::Data::Data() :
  dThreshold(25)
{
}

PiiCornerDetector::PiiCornerDetector() : PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("corners"));
}

void PiiCornerDetector::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_GRAY_IMAGE_CASES(detectCorners, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiCornerDetector::detectCorners(const PiiVariant& obj)
{
  emitObject(PiiImage::detectFastCorners(obj.valueAs<PiiMatrix<T> >(),
                                         T(_d()->dThreshold)));
}

void PiiCornerDetector::setThreshold(double threshold) { _d()->dThreshold = threshold; }
double PiiCornerDetector::threshold() const { return _d()->dThreshold; }
