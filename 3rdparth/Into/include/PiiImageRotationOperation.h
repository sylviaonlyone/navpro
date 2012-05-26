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

#ifndef _PIIIMAGEROTATIONOPERATION_H
#define _PIIIMAGEROTATIONOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiMath.h>
#include "PiiImageGlobal.h"

/**
 * Rotate images to arbitrary angles in two dimension.
 *
 * @inputs
 *
 * @in image - the input image. Any primitive type matrix.
 *
 * @in angle - rotation angle (radians, clockwise). This input is
 * optional. If it is not connected, the @p angle property will be
 * used.
 * 
 * @outputs
 *
 * @out image - rotated image
 *
 * @ingroup PiiImagePlugin
 */
class PiiImageRotationOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Rotation angle in radians, measured clockwise. This value is used
   * only if the @p angle input is not connected. Default is 0.
   */
  Q_PROPERTY(double angle READ angle WRITE setAngle);

  /**
   * Rotation angle in degrees.
   */
  Q_PROPERTY(double angleDeg READ angleDeg WRITE setAngleDeg STORED false);

  /**
   * How to handle the size of the result. Default is @p ExpandAsNecessary.
   *
   * @see PiiImage::TransformedSize
   */
  Q_PROPERTY(PiiImage::TransformedSize transformedSize READ transformedSize WRITE setTransformedSize);

  /**
   * The color used for the parts of the output image that are outside
   * of the input image. The default color is black.
   */
  Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiImageRotationOperation();

  void setAngle(double angle);
  double angle() const;
  void setAngleDeg(double angleDeg);
  double angleDeg() const;
  void setTransformedSize(PiiImage::TransformedSize transformedSize);
  PiiImage::TransformedSize transformedSize() const;
  void setBackgroundColor(const QColor& backgroundColor);
  QColor backgroundColor() const;

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dAngle;
    PiiImage::TransformedSize transformedSize;
    QColor backgroundColor;
  };
  PII_D_FUNC;

  template <class T> void rotate(const PiiVariant& obj);
  template <class T> struct Background;
};


#endif //_PIIIMAGEROTATIONOPERATION_H
