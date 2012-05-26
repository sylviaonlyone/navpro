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

#include "PiiImageSplitter.h"
#include <PiiYdinTypes.h>
#include <PiiColor.h>
#include <complex>

using namespace Pii;

PiiImageSplitter::Data::Data() :
  iRows(0), iColumns(0), iWidth(32), iHeight(32),
  iXOffset(-1), iYOffset(-1), iXSpacing(0), iYSpacing(0),
  bRandomize(false),
  iCurrentIndex(0), iSubimageCount(0)
{
}

PiiImageSplitter::PiiImageSplitter() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;

  addSocket(d->pImageInput = new PiiInputSocket("image"));
  addSocket(d->pLocationInput = new PiiInputSocket("location"));
  d->pLocationInput->setOptional(true);
  addSocket(d->pImageOutput = new PiiOutputSocket("image"));
  addSocket(d->pSubImageOutput = new PiiOutputSocket("subimage"));
  addSocket(d->pLocationOutput = new PiiOutputSocket("location"));
}

void PiiImageSplitter::check(bool reset)
{
  PII_D;
  
  PiiDefaultOperation::check(reset);

  if (d->iWidth <= 0 && d->iHeight <= 0 && d->iColumns <= 0 && d->iRows <= 0)
    PII_THROW(PiiExecutionException, tr("At least one of the properties (width, height, rows, columns) must be specified."));
}

void PiiImageSplitter::process()
{
  PII_D;
  PiiVariant obj = d->pImageInput->firstObject();

  d->pImageOutput->emitObject(obj);

  switch (obj.type())
    {
      PII_ALL_MATRIX_CASES(split, obj);
      PII_COLOR_IMAGE_CASES(split, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }
}


template <class T> void PiiImageSplitter::split(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> image = obj.valueAs<PiiMatrix<T> >();
  int rows, cols, width, height;
  int baseX = 0, baseY = 0;
  bool bUseXOffset = d->iXOffset >= 0, bUseYOffset = d->iYOffset >= 0;
  int iXOffset = bUseXOffset ? d->iXOffset : 0;
  int iYOffset = bUseYOffset ? d->iYOffset : 0;
  
  d->iCurrentIndex = 0;
  
  if (d->pLocationInput->isConnected())
    {
      PiiVariant obj = d->pLocationInput->firstObject();
      if (obj.type() != PiiYdin::IntMatrixType)
        PII_THROW_UNKNOWN_TYPE(d->pLocationInput);
      const PiiMatrix<int> mat = obj.valueAs<PiiMatrix<int> >();
      if (mat.rows() < 1 || mat.columns() < 2)
        PII_THROW_WRONG_SIZE(d->pLocationInput, mat, 1, 2);
      baseX = mat(0,0);
      baseY = mat(0,1);
    }

  d->pSubImageOutput->startMany();
  d->pLocationOutput->startMany();

  //width not specified -> calculate from columns
  if (d->iWidth <= 0)
    {
      cols = qMax(d->iColumns,1);
      width = (image.columns() - iXOffset - (cols-1)*d->iXSpacing) / cols;
      checkWidth(width, image.columns(), iXOffset);
    }
  else //width specified -> calculate columns from width
    {
      width = d->iWidth;
      checkWidth(width, image.columns(), iXOffset);
      cols = (image.columns() - iXOffset - width) / (width + d->iXSpacing) + 1;
    }

  //height not specified -> calculate from rows
  if (d->iHeight <= 0)
    {
      rows = qMax(d->iRows,1);
      height = (image.rows() - iYOffset - (rows-1)*d->iYSpacing) / rows;
      checkHeight(height, image.rows(), iYOffset);
    }
  else //height specified -> calculate columns from width
    {
      height = d->iHeight;
      checkHeight(height, image.rows(), iYOffset);
      rows = (image.rows() - iYOffset - height) / (height + d->iYSpacing) + 1;
    }

  d->iSubimageCount = rows*cols;

  // Calculate new offsets if necessary
  if (!bUseXOffset)
    iXOffset = (image.columns() - cols * (width + d->iXSpacing) + d->iXSpacing) / 2;
  if (!bUseYOffset)
    iYOffset = (image.rows() - rows * (height + d->iYSpacing) + d->iYSpacing) / 2;

  if (!d->bRandomize)
    {
      for (int r=0; r<rows; r++)
        for (int c=0; c<cols; c++)
          {
            int x = c * (width + d->iXSpacing) + iXOffset, y = r * (height + d->iYSpacing) + iYOffset;
            ++d->iCurrentIndex;
            d->pSubImageOutput->emitObject(image(y, x, height, width));
            d->pLocationOutput->emitObject(PiiMatrix<int>(1, 4, x + baseX, y + baseY, width, height));
          }
    }
  else
    {
      QVector<int> vecIndices(d->iSubimageCount);
      for (int i=0; i<d->iSubimageCount; ++i)
        vecIndices[i] = i;
      Pii::shuffle(vecIndices);
      for (int i=0; i<vecIndices.size(); ++i)
        {
          int x = vecIndices[i] % cols * (width + d->iXSpacing) + iXOffset,
            y = vecIndices[i] / cols * (height + d->iYSpacing) + iYOffset;
          ++d->iCurrentIndex;
          d->pSubImageOutput->emitObject(image(y, x, height, width));
          d->pLocationOutput->emitObject(PiiMatrix<int>(1, 4, x + baseX, y + baseY, width, height));
        }
    }

  d->pSubImageOutput->endMany();
  d->pLocationOutput->endMany();
}

void PiiImageSplitter::checkHeight(int height, int imageHeight, int yOffset)
{
  PII_D;
  
  if (height < 1 || height <= -d->iYSpacing)
    PII_THROW(PiiExecutionException,
              tr("The height of a sub-image would be less than one pixel "
                 "or too few compared to the Y spacing."));

  if (height > imageHeight - yOffset)
    PII_THROW(PiiExecutionException,
              tr("The input image does not have a sufficient number of rows."));
}

void PiiImageSplitter::checkWidth(int width, int imageWidth, int xOffset)
{
  PII_D;
  
  if (width < 1 || width <= -d->iXSpacing)
    PII_THROW(PiiExecutionException,
              tr("The width of a sub-image would be less than one pixel "
                 "or too few compared to the X spacing."));

  if (width > imageWidth - xOffset)
    PII_THROW(PiiExecutionException,
              tr("The input image does not have a sufficient number of columns."));
  

}

int PiiImageSplitter::rows() const { return _d()->iRows; }
void PiiImageSplitter::setRows(int rows) { _d()->iRows = rows; }
int PiiImageSplitter::columns() const { return _d()->iColumns; }
void PiiImageSplitter::setColumns(int columns) { _d()->iColumns = columns; }
int PiiImageSplitter::width() const { return _d()->iWidth; }
void PiiImageSplitter::setWidth(int width) { _d()->iWidth = width; }
int PiiImageSplitter::height() const { return _d()->iHeight; }
void PiiImageSplitter::setHeight(int height) { _d()->iHeight = height; }
int PiiImageSplitter::xOffset() const { return _d()->iXOffset; }
void PiiImageSplitter::setXOffset(int offset) { _d()->iXOffset = offset; }
int PiiImageSplitter::yOffset() const { return _d()->iYOffset; }
void PiiImageSplitter::setYOffset(int offset) { _d()->iYOffset = offset; }
int PiiImageSplitter::xSpacing() const { return _d()->iXSpacing; }
void PiiImageSplitter::setXSpacing(int spacing) { _d()->iXSpacing = spacing; }
int PiiImageSplitter::ySpacing() const { return _d()->iYSpacing; }
void PiiImageSplitter::setYSpacing(int spacing) { _d()->iYSpacing = spacing; }
void PiiImageSplitter::setRandomize(bool randomize) { _d()->bRandomize = randomize; }
bool PiiImageSplitter::randomize() const { return _d()->bRandomize; }
int PiiImageSplitter::currentIndex() const { return _d()->iCurrentIndex; }
int PiiImageSplitter::subimageCount() const { return _d()->iSubimageCount; }
