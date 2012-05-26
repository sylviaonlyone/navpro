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

#ifndef _PIIIMAGESPLITTER_H
#define _PIIIMAGESPLITTER_H

#include <PiiDefaultOperation.h>

/**
 * An operation for splitting images into pieces. The operation reads
 * a large image and extracts a number of sub-images from it. It may
 * be configured to extract a fixed number of images or images with a
 * fixed size. (These options are equal if the input images are of
 * equal size.) Unless the #randomize flag is set to @p true, the
 * output is ordered in a raster-scan order: the first sub-image is
 * extracted on the upper left corner of the large image (but may be
 * offset by a predefined amount). The next sub-image is extracted on
 * the right side of the first one, until the whole large image has
 * been covered. The process is repeated for each row of sub-images. 
 * Each sub-image is accompanied with a point that determines the
 * location of the upper left corner of the sub-image with respect to
 * that of the large image.
 *
 * @inputs
 *
 * @in image - the large image to be splitted into pieces. Accepts
 * all matrix and image types.
 *
 * @in location - an optional input specifying the location of the
 * input image within a larger image. If this input is connected,
 * emitted locations will be added to the upper left corner of the
 * input location. This input is useful if splitters are chained and
 * the results need to be placed in the context of the original image.
 *
 * @outputs
 *
 * @out image - the large image. @p %PiiImageSplitter will pass the
 * input image to this output before it sends the pieces.
 *
 * @out subimage - pieces of the large image. The type of the
 * subimages is the same as that of the input images.
 *
 * @out location - the location of the corresponding sub-image as a
 * rectangle (1-by-4 PiiMatrix<int> containing x, y, width, and height
 * in this order).
 *
 * @ingroup PiiImagePlugin
 */
class PiiImageSplitter : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The number of rows of equally sized image pieces. If the height
   * (in pixels) of the large image is not a multiple of @p rows,
   * empty pixels are left at the bottom of the large image. For
   * example, if the height of the large image is 10 pixels and @p
   * rows equals to 3, one pixel will be left over.
   *
   * If a fixed number of rows or columns is used, the size of the
   * small images may vary, if #width is set to a non-positive value. 
   * If this value is zero or negative, #width determines the number
   * of sub-image rows. The default value is 0.
   */
  Q_PROPERTY(int rows READ rows WRITE setRows);

  /**
   * The number of columns of equally sized image pieces.
   *
   * @see #rows
   */
  Q_PROPERTY(int columns READ columns WRITE setColumns);

  /**
   * The width of the extracted sub-images in pixels. If the size of
   * the input images varies, the number of extracted sub-images may
   * also vary. If this value is zero or negative, #columns is used to
   * determine the width. The default value is 32.
   */
  Q_PROPERTY(int width READ width WRITE setWidth);
  
  /**
   * The height of the extracted sub-images in pixels.
   *
   * @see #width
   */
  Q_PROPERTY(int height READ height WRITE setHeight);

  /**
   * The number of pixels the upper left corner of the first extracted
   * sub-image is horizontally offset with respect to that of the
   * input image. The default value is -1, which causes the sub-images
   * to be horizontally centered into the image.
   */
  Q_PROPERTY(int xOffset READ xOffset WRITE setXOffset);
  
  /**
   * The number of pixels the upper left corner of the first extracted
   * sub-image is vertically offset with respect to that of the input
   * image. The default value is -1, which causes the sub-images to be
   * vertically centered into the image.
   */
  Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset);

  /**
   * The number of pixels each sub-image is separated from the
   * previous one in horizontal direction. Negative values are OK, and
   * cause overlapping images to be extracted. The default value is 0.
   */
  Q_PROPERTY(int xSpacing READ xSpacing WRITE setXSpacing);

  /**
   * The number of pixels each sub-image is separated from the
   * previous one in vertical direction. Default is 0.
   */
  Q_PROPERTY(int ySpacing READ ySpacing WRITE setYSpacing);

  /**
   * Randomization flag. If this flag is set to @p true, the order of
   * emitted sub-images will be random. Otherwise, the upper left
   * corner will always become first. The default value is @p false.
   */
  Q_PROPERTY(bool randomize READ randomize WRITE setRandomize);

  /**
   * The index of the next-to-be-sent sub-image within the last
   * received image.
   */
  Q_PROPERTY(int currentIndex READ currentIndex);

  /**
   * The number of sub-images in the last received image.
   */
  Q_PROPERTY(int subimageCount READ subimageCount);

  PII_OPERATION_SERIALIZATION_FUNCTION;
public:
  PiiImageSplitter();

  int rows() const;
  void setRows(int rows);

  int columns() const;
  void setColumns(int columns);

  int width() const;
  void setWidth(int width);

  int height() const;
  void setHeight(int height);

  int xOffset() const;
  void setXOffset(int offset);

  int yOffset() const;
  void setYOffset(int offset);

  int xSpacing() const;
  void setXSpacing(int spacing);

  int ySpacing() const;
  void setYSpacing(int spacing);
  
  void setRandomize(bool randomize);
  bool randomize() const;

  int currentIndex() const;
  int subimageCount() const;

  void check(bool reset);

protected:
  void process();

private:
  template <class T> void split(const PiiVariant& obj);
  void checkHeight(int height, int imageHeight, int yOffset);
  void checkWidth(int width, int imageWidth, int xOffset);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    int iRows, iColumns, iWidth, iHeight, iXOffset, iYOffset, iXSpacing, iYSpacing;
    bool bRandomize;
    int iCurrentIndex;
    int iSubimageCount;
    
    PiiInputSocket *pImageInput, *pLocationInput;
    PiiOutputSocket *pImageOutput, *pSubImageOutput, *pLocationOutput;
  };
  PII_D_FUNC;
};

#endif //_PIIIMAGESPLITTER_H
