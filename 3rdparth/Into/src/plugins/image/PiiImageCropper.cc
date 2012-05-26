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

#include "PiiImageCropper.h"
#include <PiiYdinTypes.h>
#include <PiiColor.h>
#include <PiiMath.h>
#include "PiiImage.h"
#include <complex>

PiiImageCropper::Data::Data() :
  area(0,0,1,1),
  mode(OneToOne),
  bImageReceived(false),
  iLeftX(0),
  iTopY(0)
{
}

PiiImageCropper::PiiImageCropper() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  addSocket(d->pImageInput = new PiiInputSocket("image"));
  addSocket(d->pLocationInput = new PiiInputSocket("location"));
  d->pLocationInput->setOptional(true);
  addSocket(d->pAreaInput = new PiiInputSocket("area"));
  d->pAreaInput->setOptional(true);
  addSocket(d->pTransformInput = new PiiInputSocket("transform"));
  d->pTransformInput->setOptional(true);
  addSocket(d->pImageOutput = new PiiOutputSocket("image"));
  addSocket(d->pLocationOutput = new PiiOutputSocket("location"));
}

void PiiImageCropper::check(bool reset)
{
  PII_D;

  if (reset)
    {
      if (d->mode == ManyToMany)
        {
          d->pAreaInput->setGroupId(1);
          d->pTransformInput->setGroupId(1);
          if (!d->pAreaInput->isConnected() && !d->pTransformInput->isConnected())
            PII_THROW(PiiExecutionException, tr("Either area or transform must be connected."));
        }
      else
        {
          d->pAreaInput->setGroupId(0);
          d->pTransformInput->setGroupId(0);
        }
      d->lstBuffer.clear();
      d->bImageReceived = false;
      d->iLeftX = d->iTopY = 0;
    }
  
  switch (d->varTransform.type())
    {
    case PiiVariant::InvalidType:
      d->matTransform.resize(0,0);
      break;
      PII_PRIMITIVE_MATRIX_CASES(d->matTransform = PiiYdin::matrixAs, d->varTransform);
    default:
      PII_THROW(PiiExecutionException, tr("Transform is not a matrix."));
    }
  if (!d->matTransform.isEmpty() &&
      (d->matTransform.rows() != 3 || d->matTransform.columns() != 3))
    PII_THROW(PiiExecutionException, tr("Transform matrix must be 3-by-3."));

  PiiDefaultOperation::check(reset);
}

void PiiImageCropper::syncEvent(SyncEvent* event)
{
  if (event->type() == SyncEvent::StartInput)
    {
      startMany();
      _d()->bImageReceived = false;
    }
  else
    {
      endMany();
      _d()->varImage = PiiVariant(); // release memory
    }
}

void PiiImageCropper::process()
{
  PII_D;

  if (d->mode != ManyToMany)
    {
      if (d->mode == OneToMany)
        startMany();

      if (d->pLocationInput->isConnected())
        readLocation();
      
      crop(d->pImageInput->firstObject(),
           d->pAreaInput->firstObject(),
           d->pTransformInput->firstObject());

      if (d->mode == OneToMany)
        endMany();
    }
  else
    {
      // Received area and/or transform
      if (activeInputGroup() == 1)
        {
          // If the image hasn't arrived yet, buffer data
          if (!d->bImageReceived)
            d->lstBuffer << qMakePair(d->pAreaInput->firstObject(),
                                      d->pTransformInput->firstObject());
          // Otherwise crop directly
          else
            crop(d->varImage,
                 d->pAreaInput->firstObject(),
                 d->pTransformInput->firstObject());
        }
      else // Image arrives
        {
          d->bImageReceived = true;
          d->varImage = d->pImageInput->firstObject();
          if (d->pLocationInput->isConnected())
            readLocation();
          // Clear buffered data
          for (int i=0; i<d->lstBuffer.size(); ++i)
            crop(d->varImage,
                 d->lstBuffer[i].first,
                 d->lstBuffer[i].second);
          d->lstBuffer.clear();
        }
    }
}

void PiiImageCropper::startMany()
{
  PII_D;
  d->pImageOutput->startMany();
  d->pLocationOutput->startMany();
}

void PiiImageCropper::endMany()
{
  PII_D;
  d->pImageOutput->endMany();
  d->pLocationOutput->endMany();
}

void PiiImageCropper::readLocation()
{
  PII_D;
  PiiVariant locationObj = d->pLocationInput->firstObject();
  if (locationObj.type() != PiiYdin::IntMatrixType)
    PII_THROW_UNKNOWN_TYPE(d->pLocationInput);
  const PiiMatrix<int> mat = locationObj.valueAs<PiiMatrix<int> >();
  if (mat.columns() < 2 && mat.rows() != 1)
    PII_THROW_WRONG_SIZE(d->pLocationInput, mat, 1, "2-N");
  d->iLeftX = mat(0,0);
  d->iTopY = mat(0,1);
}

void PiiImageCropper::crop(const PiiVariant& imageObj,
                           const PiiVariant& areaObj,
                           const PiiVariant& transformObj)
{
  switch (imageObj.type())
    {
      PII_ALL_MATRIX_CASES_M(cropTemplate, (imageObj, areaObj, transformObj));
      PII_COLOR_IMAGE_CASES_M(cropTemplate, (imageObj, areaObj, transformObj));
    default:
      PII_THROW_UNKNOWN_TYPE(_d()->pImageInput);
    }
}

template <class T> void PiiImageCropper::cropTemplate(const PiiVariant& imageObj,
                                                      const PiiVariant& areaObj,
                                                      const PiiVariant& transformObj)
{
  PII_D;
  const PiiMatrix<T> image = imageObj.valueAs<PiiMatrix<T> >();

  // Upper left corner of incoming image
  int iLeftX = d->iLeftX, iTopY = d->iTopY;

  if (transformObj.isValid())
    {
      d->matTransform = PiiYdin::convertMatrixTo<float>(d->pTransformInput);
      if (d->matTransform.columns() != 3 || d->matTransform.rows() != 3)
        PII_THROW_WRONG_SIZE(d->pTransformInput, d->matTransform, 3, 3);
    }
 
  if (areaObj.isValid())
    {
      if (areaObj.type() != PiiYdin::IntMatrixType)
        PII_THROW_UNKNOWN_TYPE(d->pAreaInput);
      const PiiMatrix<int> mat = areaObj.valueAs<PiiMatrix<int> >();
      if (d->mode == OneToOne)
        {
          if (mat.columns() != 4 ||
              mat.rows() < 1)
            PII_THROW_WRONG_SIZE(d->pAreaInput, mat, "1-N", 4);
          crop(image, mat(0,0) + iLeftX, mat(0,1) + iTopY, mat(0,2), mat(0,3));
        }
      else
        {
          if (mat.columns() != 4)
            PII_THROW_WRONG_SIZE(d->pAreaInput, mat, "N", 4);
          for (int r=0; r<mat.rows(); r++)
            crop(image, mat(r,0) + iLeftX, mat(r,1) + iTopY, mat(r,2), mat(r,3));
        }
    }
  else
    crop(image, d->area.left() + iLeftX, d->area.top() + iTopY, d->area.width(), d->area.height());
}


template <class T> void PiiImageCropper::crop(const PiiMatrix<T>& image, int left, int top, int width, int height)
{  
  PII_D;

  if (d->matTransform.isEmpty())
    {
      // Calculate area limits
      int firstRow = top >= 0 ? top : image.rows() + top;
      int firstCol = left >= 0 ? left : image.columns() + left;
  
      int lastRow = height >= 0 ? firstRow + height - 1 : image.rows() + height;
      int lastCol = width >= 0 ? firstCol + width - 1 : image.columns() + width;

      // Ensure the limits don't exceed image dimensions
      if (firstRow < 0) firstRow = 0;
      else if (firstRow >= image.rows()) firstRow = image.rows()-1;
      if (firstCol < 0) firstCol = 0;
      else if (firstCol >= image.columns()) firstCol = image.columns()-1;

      if (lastRow < firstRow) lastRow = firstRow;
      else if (lastRow >= image.rows()) lastRow = image.rows()-1;
      if (lastCol < firstCol) lastCol = firstCol;
      else if (lastCol >= image.columns()) lastCol = image.columns()-1;

      //qDebug("%s::crop(): (%d, %d, %d, %d) of (%d x %d)", qPrintable(objectName()), firstRow, firstCol, lastRow, lastCol, image.rows(), image.columns());

      // Create a cropped piece of the image (share matrix buffer)
      if (d->pImageOutput->isConnected())
        d->pImageOutput->emitObject(image(firstRow, firstCol, lastRow-firstRow+1, lastCol-firstCol+1));
      d->pLocationOutput->emitObject(PiiMatrix<int>(1,4, firstCol, firstRow, lastCol-firstCol+1, lastRow-firstRow+1));
    }
  else
    {
      //qDebug("%s::crop(): (%d, %d, %d, %d) of (%d x %d)", qPrintable(objectName()), left, top, width, height, image.rows(), image.columns());
      if (d->pImageOutput->isConnected())
        d->pImageOutput->emitObject(PiiImage::crop(image, left, top, width, height, d->matTransform));
      if (d->pLocationOutput->isConnected())
        {
          float fX1, fY1, fX2, fY2, fX3, fY3, fX4, fY4;
          PiiImage::transformHomogeneousPoint(d->matTransform, float(left), float(top), &fX1, &fY1);
          PiiImage::transformHomogeneousPoint(d->matTransform, float(left+width), float(top), &fX2, &fY2);
          PiiImage::transformHomogeneousPoint(d->matTransform, float(left+width), float(top+height), &fX3, &fY3);
          PiiImage::transformHomogeneousPoint(d->matTransform, float(left), float(top+height), &fX4, &fY4);

          int iMinX = Pii::floor(Pii::min(fX1, fX2, fX3, fX4));
          int iMaxX = Pii::floor(Pii::max(fX1, fX2, fX3, fX4));
          int iMinY = Pii::ceil(Pii::min(fY1, fY2, fY3, fY4));
          int iMaxY = Pii::ceil(Pii::max(fY1, fY2, fY3, fY4));

          d->pLocationOutput->emitObject(PiiMatrix<int>(1,4, iMinX, iMinY, iMaxX-iMinX, iMaxY-iMinY));
        }
    }
}

void PiiImageCropper::setMode(Mode mode) { _d()->mode = mode; }
PiiImageCropper::Mode PiiImageCropper::mode() const { return _d()->mode; }
int PiiImageCropper::width() const { return _d()->area.width(); }
void PiiImageCropper::setWidth(int width) { _d()->area.setWidth(width); }
int PiiImageCropper::height() const { return _d()->area.height(); }
void PiiImageCropper::setHeight(int height) { _d()->area.setHeight(height); }
int PiiImageCropper::xOffset() const { return _d()->area.left(); }
void PiiImageCropper::setXOffset(int offset) { _d()->area.moveLeft(offset); }
int PiiImageCropper::yOffset() const { return _d()->area.top(); }
void PiiImageCropper::setYOffset(int offset) { _d()->area.moveTop(offset); }
QRect PiiImageCropper::area() const { return _d()->area; }
void PiiImageCropper::setArea(const QRect& area) { _d()->area = area; }
void PiiImageCropper::setTransform(const PiiVariant& transform) { _d()->varTransform = transform; }
PiiVariant PiiImageCropper::transform() const { return _d()->varTransform; }
