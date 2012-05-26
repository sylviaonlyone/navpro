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

#ifndef _PIIMORPHOLOGYOPERATION_H
#define _PIIMORPHOLOGYOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>
#include <QSize>
#include "PiiImageGlobal.h"

/**
 * Basic binary morphology operations.
 *
 * @inputs
 *
 * @in image - the input image. Any gray-scale image. Zero is treated
 * as "false". Any value other than zero is considered "true".
 *
 * @outputs
 *
 * @out image - the image output. Output image is of the same data
 * type as the input image, but contains only ones and zeros.
 *
 * @ingroup PiiImagePlugin
 */
class PiiMorphologyOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Type of the structuring element. The default is @p RectangularMask.
   */
  Q_PROPERTY(PiiImage::MaskType maskType READ maskType WRITE setMaskType);
  
  /**
   * The type of a moprhological operation to be applied to input
   * images. The default value is @p Erode.
   */
  Q_PROPERTY(PiiImage::MorphologyOperation operation READ operation WRITE setOperation);

  /**
   * A flag that controls the behavior of border handling with
   * erosion. See PiiImage::erode() for details. The default value is
   * @p false.
   */
  Q_PROPERTY(bool handleBorders READ handleBorders WRITE setHandleBorders);
  
  /**
   * Size of the structuring element. Any size will do as far as the
   * mask is smaller than the input image. The default is 3x3.
   */
  Q_PROPERTY(QSize maskSize READ maskSize WRITE setMaskSize);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiMorphologyOperation();

  PiiImage::MorphologyOperation operation() const;
  void setOperation(PiiImage::MorphologyOperation operation);

  void setHandleBorders(bool handleBorders);
  bool handleBorders() const;

  PiiImage::MaskType maskType() const;
  void setMaskType(PiiImage::MaskType maskType);

  QSize maskSize() const;
  void setMaskSize(QSize maskSize);
  
  PiiMatrix<int> mask() const;
  template <class T> void setMask(PiiMatrix<T> mask);
  
protected:
  void process();
  
private:
  template <class T> void morphologyOperation(const PiiVariant& obj);

  void prepareMask();
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiImage::MorphologyOperation operation;
    PiiImage::MaskType maskType;
    bool bHandleBorders;
    QSize maskSize;
    
    PiiMatrix<int> matMask;
    
    PiiInputSocket* pImageInput;
    PiiOutputSocket* pBinaryImageOutput;
  };
  PII_D_FUNC;
};

#endif //_PIIMORPHOLOGYOPERATION_H
