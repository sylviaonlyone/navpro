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

#include "PiiRgbNormalizer.h"

#include <PiiYdinTypes.h>
#include <PiiColor.h>
#include "PiiColors.h"

PiiRgbNormalizer::Data::Data() :
  dMaxValue(255), channels(RedGreen)
{
}

PiiRgbNormalizer::PiiRgbNormalizer() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("channel0"));
  addSocket(new PiiOutputSocket("channel1"));
}

void PiiRgbNormalizer::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_COLOR_IMAGE_CASES(normalizedRgb, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiRgbNormalizer::normalizedRgb(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> clrImg(obj.valueAs<PiiMatrix<T> >());
  // Create output matrices in heap (exception-safe)
  PiiMatrix<typename T::Type> ch1(PiiMatrix<typename T::Type>::uninitialized(clrImg.rows(), clrImg.columns()));
  PiiMatrix<typename T::Type> ch2(PiiMatrix<typename T::Type>::uninitialized(clrImg.rows(), clrImg.columns()));
  
  int iCh1 = 0, iCh2 = 1;
  if (d->channels == RedBlue)
    iCh2 = 2;
  else if (d->channels == GreenBlue)
    {
      iCh1 = 1;
      iCh2 = 2;
    }
  
  PiiColors::normalizedRgb(clrImg, ch1, ch2, (float)d->dMaxValue, iCh1, iCh2);

  emitObject(ch1, 0);
  emitObject(ch2, 1);
}

void PiiRgbNormalizer::setMaxValue(double maxValue) { _d()->dMaxValue = maxValue; }
double PiiRgbNormalizer::maxValue() const { return _d()->dMaxValue; }
void PiiRgbNormalizer::setChannels(const ChannelPair& channels) { _d()->channels = channels; }
PiiRgbNormalizer::ChannelPair PiiRgbNormalizer::channels() const { return _d()->channels; }
