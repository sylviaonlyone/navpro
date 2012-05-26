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

#ifndef _PIIMASKGENERATOR_H
#define _PIIMASKGENERATOR_H

#include <PiiDefaultOperation.h>
#include <QPolygon>
#include <PiiQImage.h>
#include <PiiColor.h>
#include "PiiImageGlobal.h"

/**
 * An operation that creates binary mask images. The masks can be
 * used, for example, for ROI processing.
 *
 * @inputs
 *
 * @in trigger - mask matrix is emitted whenever any object is
 * received in this input. The input value is ignored. Either this or
 * @p size needs to be connected.
 *
 * @in size - the size of the mask matrix as a PiiMatrix<int>. If the
 * size of the matrix is 1-by-4, the input is treated as a rectangle
 * (x,y,w,h) whose width and height are used as the mask size. If the
 * size is 1-by-2, the input is treated as a size (w,h). Either this
 * or @p trigger needs to be connected.
 * 
 * @outputs
 *
 * @out mask - a binary mask (PiiMatrix<unsigned char>)
 *
 * @ingroup PiiImagePlugin
 */
class PiiMaskGenerator : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The type of mask to generate. Default is @p EllipticalMask.
   */
  Q_PROPERTY(PiiImage::MaskType maskType READ maskType WRITE setMaskType);

  /**
   * The size of the mask to be generated. If the @p size input is
   * connected, this value will be ignored. If the size is set to
   * (0,0) (the default), the size of the mask image will be
   * automatically determined. With polygons, a mask will be generated
   * that just encloses all the polygons. With other mask types, a
   * 1-by-1 mask with a single one will be created.
   */
  Q_PROPERTY(QSize maskSize READ maskSize WRITE setMaskSize);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiMaskGenerator();

  void setMaskType(PiiImage::MaskType maskType);
  PiiImage::MaskType maskType() const;
  void setMaskSize(QSize maskSize);
  QSize maskSize() const;

  void check(bool reset);
  
protected:
  void process();

private:
  PiiMatrix<unsigned char> createMask(int rows, int columns);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiMatrix<unsigned char> maskMatrix;
    PiiImage::MaskType maskType;
    QSize maskSize;
  };
  PII_D_FUNC;
};


#endif //_PIIMASKGENERATOR_H
