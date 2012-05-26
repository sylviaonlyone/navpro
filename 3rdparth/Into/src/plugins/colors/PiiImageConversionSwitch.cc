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

#include "PiiImageConversionSwitch.h"

#include <PiiYdinTypes.h>
#include <PiiMatrix.h>
#include <PiiColor.h>

PiiImageConversionSwitch::Data::Data() :
  bGrayImageOutputConnected(false),
  bColorImageOutputConnected(false),
  colorConversion(RgbToGrayMean)
  
{
}

PiiImageConversionSwitch::PiiImageConversionSwitch() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  addSocket(d->pImageInput = new PiiInputSocket("image"));
  addSocket(d->pGrayImageOutput = new PiiOutputSocket("gray image"));
  addSocket(d->pColorImageOutput = new PiiOutputSocket("color image"));
}

void PiiImageConversionSwitch::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  d->bGrayImageOutputConnected = d->pGrayImageOutput->isConnected();
  d->bColorImageOutputConnected = d->pColorImageOutput->isConnected();
}

void PiiImageConversionSwitch::process()
{
  PiiVariant obj = readInput();
  
  switch (obj.type())
    {
    case PiiYdin::UnsignedCharMatrixType:
      operateGrayImage(obj);
      break;
    case PiiYdin::UnsignedCharColor4MatrixType:
      operateColorImage(obj);
      break;
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}


void PiiImageConversionSwitch::operateGrayImage(const PiiVariant& obj)
{
  PII_D;
  if (d->bGrayImageOutputConnected)
    d->pGrayImageOutput->emitObject(obj);

  //convert to color image and emit it
  if (d->bColorImageOutputConnected)
    {
      const PiiMatrix<unsigned char> image = obj.valueAs<PiiMatrix<unsigned char> >();
      d->pColorImageOutput->emitObject(PiiMatrix<PiiColor4<unsigned char> >(image));
    }
}

void PiiImageConversionSwitch::operateColorImage(const PiiVariant& obj)
{
  PII_D;
  if (d->bGrayImageOutputConnected)
    {
      const PiiMatrix<PiiColor4<unsigned char> > image = obj.valueAs<PiiMatrix<PiiColor4<unsigned char> > >();

      switch(d->colorConversion)
        {
        case RgbToGrayMean:
          d->pGrayImageOutput->emitObject(PiiMatrix<unsigned char>(image));
          break;
        }
    }

  if (d->bColorImageOutputConnected)
    d->pColorImageOutput->emitObject(obj);
}

void PiiImageConversionSwitch::setColorConversion(ColorConversion colorConversion) { _d()->colorConversion = colorConversion; }
PiiImageConversionSwitch::ColorConversion PiiImageConversionSwitch::colorConversion() const { return _d()->colorConversion; }
