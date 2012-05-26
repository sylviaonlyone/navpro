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

#include "PiiColorModelMatcher.h"

#include <PiiYdinTypes.h>
#include <PiiThresholding.h>
#include "PiiColors.h"

PiiColorModelMatcher::Data::Data() :
  matBaseVectors(3,3), matCenter(1,3), dMatchingThreshold(0)
{
}

PiiColorModelMatcher::PiiColorModelMatcher() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiInputSocket("model"));
  addSocket(new PiiOutputSocket("image"));
}

void PiiColorModelMatcher::process()
{
  PiiVariant modelObj = inputAt(1)->firstObject();
  switch (modelObj.type())
    {
      PII_COLOR_IMAGE_CASES(calculateModel, modelObj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(1));
    }
  
  PiiVariant imageObj = readInput();

  switch (imageObj.type())
    {
      PII_COLOR_IMAGE_CASES(matchImageToModel, imageObj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiColorModelMatcher::calculateModel(const PiiVariant& obj)
{
  PII_D;
  PiiColors::measureColorDistribution(obj.valueAs<PiiMatrix<T> >(),
                                      d->matBaseVectors,
                                      d->matCenter);
}

template <class T> void PiiColorModelMatcher::matchImageToModel(const PiiVariant& obj)
{
  PII_D;
  if (d->dMatchingThreshold > 0)
    emitObject(PiiColors::matchColors(obj.valueAs<PiiMatrix<T> >(),
                                                      d->matBaseVectors,
                                                      d->matCenter,
                                                      std::bind2nd(PiiImage::InverseThresholdFunction<float,unsigned char>(),
                                                                   d->dMatchingThreshold)));
  else
    emitObject(PiiColors::matchColors(obj.valueAs<PiiMatrix<T> >(),
                                                      d->matBaseVectors,
                                                      d->matCenter,
                                                      PiiColors::LikelihoodFunction()));
}

void PiiColorModelMatcher::setMatchingThreshold(double matchingThreshold) { _d()->dMatchingThreshold = matchingThreshold; }
double PiiColorModelMatcher::matchingThreshold() const { return _d()->dMatchingThreshold; }
