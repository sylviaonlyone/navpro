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

#ifndef _PIIIMAGECROPPER_H
#define _PIIIMAGECROPPER_H

#include <PiiDefaultOperation.h>
#include <QRect>
#include <PiiMatrix.h>
#include <QList>
#include <QPair>

/**
 * An operation that takes a piece of an image or any other matrix. If
 * the configured coordinates exceed image boundaries, a smaller area
 * will be extracted.
 *
 * @inputs
 *
 * @in image - the input image
 *
 * @in location - an optional input specifying the location of the
 * input image within a larger image. If this input is connected,
 * emitted locations will be added to the upper left corner of the
 * input location. The location is expected to be a PiiMatrix<int>
 * with one row and at least two columns. The first two elements of
 * the matrix are treated as the (x,y) coordinates of the upper left
 * corner. This input is always coupled with the @a image input.
 *
 * @in area - an optional input framing the rectangle to extract. If
 * this input is connected, the #area property will be ignored. The
 * input must be a PiiMatrix<int> with four columns (x, y, width,
 * height). If #mode is @p OneToOne, the matrix must have at least one
 * row. In @p OneToMany and @p ManyToMany modes the matrix may have
 * zero to N rows. In @p ManyToMany mode any number of @p
 * area inputs may be read for each @p image input.
 *
 * @in transform - an optional input that specifies a transform
 * between area and image coordinates. If this input is connected, the
 * crop area given either as a property or in the @p area input is
 * first transformed to image coordinates using the given
 * transformation matrix for homogeneous image coordinates. The matrix
 * must be 3-by-3. This makes it possible to crop non-rectangular
 * areas out of the image. If the area to be extracted out of the
 * input image exceeds input boundaries, there will be black areas in
 * the output image. This input is always coupled with the @p area
 * input. If it is connected, the #transform property will be ignored.
 *
 * In @p ManyToMany mode either @p area or @p transform must be
 * connected.
 *
 * @outputs
 *
 * @out image - a smaller portion of the input image. In @p OneToMany
 * and @p ManyToMany modes, an image will be emitted for each row in
 * the matrix read from the @p location input.
 *
 * @out location - the location of the sub-image. A 1-by-4
 * PiiMatrix<int> that stores the location (x, y, width, height) of
 * the sub-image. If the @p transform input is connected, this matrix
 * will store the bounding box of the transformed rectangle.
 *
 * @ingroup PiiImagePlugin
 */
class PiiImageCropper : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The crop area.
   */
  Q_PROPERTY(QRect area READ area WRITE setArea);

  /**
   * Controls how cropped images are emitted. In @p OneToOne mode (the
   * default), one cropped image is always emitted per each input
   * image. In the other modes, 0 to N images are emitted per each
   * incoming image. Note that the flow level of the output is higher
   * than that of the input in these cases.
   */
  Q_PROPERTY(Mode mode READ mode WRITE setMode);
  Q_ENUMS(Mode);

  /**
   * The width of the cropped image. If a negative value is given, the
   * right edge of the cropped image will be always positioned @p
   * width pixels from that of the input image.
   */
  Q_PROPERTY(int width READ width WRITE setWidth STORED false);
  
  /**
   * The height of the cropped image. If a negative value is given,
   * the bottom edge of the cropped image will be always positioned @p
   * width pixels from that of the input image.
   */
  Q_PROPERTY(int height READ height WRITE setHeight STORED false);

  /**
   * The number of pixels the left edge of the crop area is
   * horizontally offset with respect to that of the input image. The
   * default value is 0. Negative value counts from the right edge.
   */
  Q_PROPERTY(int xOffset READ xOffset WRITE setXOffset STORED false);
  
  /**
   * The number of pixels the top edge of the crop area is vertically
   * offset with respect to that of the input image. The default value
   * is 0. Negative value means counts from the bottom edge.
   */
  Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset STORED false);

  /**
   * A 3-by-3 PiiMatrix<float> that specifies the transformation
   * between area and image coordinates. If this matrix is empty, or
   * the variant is invalid, no transformation will be made. The @p
   * transform input overrides this value.
   */
  Q_PROPERTY(PiiVariant transform READ transform WRITE setTransform);

  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  /**
   * Modes of operation.
   *
   * @lip OneToOne - one image will be emitted for each incoming image
   *
   * @lip OneToMany - 0-N images will be emitted for each
   * incoming image, determined by the number of rows in the matrix
   * read from the @p area input.
   *  
   * @lip ManyToMany - 0-N images will be emitted for each incoming
   * image. Any number of crop areas may be received for each incoming
   * image. The @p area and @p transform inputs are assumed to receive
   * objects at a higher flow level.
   */
  enum Mode { OneToOne, OneToMany, ManyToMany };
  
  PiiImageCropper();

  void setMode(Mode mode);
  Mode mode() const;
  int width() const;
  void setWidth(int width);
  int height() const;
  void setHeight(int height);
  int xOffset() const;
  void setXOffset(int offset);
  int yOffset() const;
  void setYOffset(int offset);
  QRect area() const;
  void setArea(const QRect& area);
  void setTransform(const PiiVariant& transform);
  PiiVariant transform() const;

  void check(bool reset);
  
protected:
  void process();
  void syncEvent(SyncEvent* event);

private:
  void readLocation();
  void startMany();
  void endMany();
    
  void crop(const PiiVariant& imageObj,
            const PiiVariant& areaObj,
            const PiiVariant& transformObj);
  template <class T> void cropTemplate(const PiiVariant& imageObj,
                                       const PiiVariant& areaObj,
                                       const PiiVariant& transformObj);
  template <class T> void crop(const PiiMatrix<T>& image, int left, int top, int width, int height);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    QRect area;
    Mode mode;  
    
    PiiInputSocket *pImageInput, *pAreaInput, *pLocationInput, *pTransformInput;
    PiiOutputSocket *pImageOutput, *pLocationOutput;

    PiiMatrix<float> matTransform;
    PiiVariant varTransform;
    QList<QPair<PiiVariant,PiiVariant> > lstBuffer;
    PiiVariant varImage;
    bool bImageReceived;

    int iLeftX, iTopY;
  };
  PII_D_FUNC;
};

#endif //_PIIIMAGECROPPER_H
