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

#include "PiiColorChannelSplitter.h"
#include <PiiYdinTypes.h>
#include <PiiColor.h>

using namespace Pii;
using namespace PiiYdin;

PiiColorChannelSplitter::Data::Data() :
  pImageInput(0)
{
}

PiiColorChannelSplitter::PiiColorChannelSplitter() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  
  d->pImageInput = new PiiInputSocket("image");
  addSocket(d->pImageInput);

  setNumberedOutputs(4,0,"channel");
}

PiiColorChannelSplitter::~PiiColorChannelSplitter()
{
}

void PiiColorChannelSplitter::process()
{
  PII_D;
  
  PiiVariant obj = d->pImageInput->firstObject();

  switch (obj.type())
    {
    case UnsignedCharColorMatrixType:
      splitChannels<PiiColor<unsigned char>, 3>(obj);
      break;
    case UnsignedShortColorMatrixType:
      splitChannels<PiiColor<unsigned short>, 3>(obj);
      break;
    case FloatColorMatrixType:
      splitChannels<PiiColor<float>, 3>(obj);
      break;
    case UnsignedCharColor4MatrixType:
      splitChannels<PiiColor4<unsigned char>, 4>(obj);
      break;
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }
}

template <class Color, int channels> void PiiColorChannelSplitter::splitChannels(const PiiVariant& obj)
{
  typedef typename Color::Type T;
  const PiiMatrix<Color> image = obj.valueAs<PiiMatrix<Color> >();
  PiiMatrix<T> channelImages[channels];
  
  for (int i=0; i<channels; ++i)
    channelImages[i] = PiiMatrix<T>(PiiMatrix<T>::uninitialized(image.rows(), image.columns()));
  
  const int iRows = image.rows(), iCols = image.columns();
  for (int r=0; r<iRows; ++r)
    {
      const Color* row = image.row(r);
      T *row0 = channelImages[0][r],
        *row1 = channelImages[1][r],
        *row2 = channelImages[2][r],
        *row3;

      if (channels == 4)
        row3 = channelImages[3][r];
      
      for (int c=0; c<iCols; ++c)
        {
          row0[c] = row[c].c0;
          row1[c] = row[c].c1;
          row2[c] = row[c].c2;
          if (channels == 4)
            row3[c] = row[c].channels[3];
        }
    }

  for (int i=0; i<channels; ++i)
    outputAt(i)->emitObject(channelImages[i]);
}
