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

#include "PiiImageReaderOperation.h"
#include <PiiYdinTypes.h>
#include <PiiQImage.h>
#include <PiiColor.h>

using namespace PiiYdin;
using namespace Pii;

PiiImageReaderOperation::Data::Data() :
  imageType(Original), iMaxImages(-1), iCurrentIndex(0)
{
}

PiiImageReaderOperation::PiiImageReaderOperation(Data* dat) :
  PiiDefaultOperation(dat, Threaded)
{
  PII_D;
  d->pTriggerInput = new PiiInputSocket("trigger");
  d->pTriggerInput->setOptional(true);
  addSocket(d->pTriggerInput);
  
  d->pImageOutput = new PiiOutputSocket("image");
  addSocket(d->pImageOutput);
}

void PiiImageReaderOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);
  if (reset)
    d->iCurrentIndex = 0;
}

int PiiImageReaderOperation::maxImages() const { return _d()->iMaxImages; }
void PiiImageReaderOperation::setMaxImages(int cnt) { _d()->iMaxImages = cnt; }
PiiImageReaderOperation::ImageType PiiImageReaderOperation::imageType() const { return _d()->imageType; }
int PiiImageReaderOperation::currentImageIndex() const { return _d()->iCurrentIndex; }

void PiiImageReaderOperation::emitGrayImage(QImage& img)
{
  Pii::convertToGray(img);
  _d()->pImageOutput->emitObject(PiiGrayQImage::create(img)->toMatrix());
}

void PiiImageReaderOperation::emitColorImage(QImage& img)
{
  convertToRgba(img);
  _d()->pImageOutput->emitObject(PiiColorQImage::create(img)->toMatrix());
}

void PiiImageReaderOperation::emitImage(QImage& img)
{
  if (img.depth() == 32)
    emitColorImage(img);
  else
    emitGrayImage(img);
}

void PiiImageReaderOperation::setImageType(ImageType type) { _d()->imageType = type; }
