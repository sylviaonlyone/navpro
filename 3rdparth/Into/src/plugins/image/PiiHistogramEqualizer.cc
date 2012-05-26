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

#include "PiiHistogramEqualizer.h"
#include <PiiYdinTypes.h>
#include "PiiHistogram.h"

PiiHistogramEqualizer::Data::Data() :
  iLevels(256)
{
}

PiiHistogramEqualizer::PiiHistogramEqualizer() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("image"));
}

void PiiHistogramEqualizer::setLevels(int levels)
{
  if (levels >= 0 && levels < 65536)
    _d()->iLevels = levels;
}

void PiiHistogramEqualizer::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_GRAY_IMAGE_CASES(equalize, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiHistogramEqualizer::equalize(const PiiVariant& obj)
{
  const PiiMatrix<T> img = obj.valueAs<PiiMatrix<T> >();
  emitObject(PiiImage::equalize(img, (unsigned)_d()->iLevels));
}

int PiiHistogramEqualizer::levels() const
{
  return _d()->iLevels;
}
