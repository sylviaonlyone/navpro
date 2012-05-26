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

#ifndef _PIIIMAGECONVERSIONSWITCH_H
#define _PIIIMAGECONVERSIONSWITCH_H

#include <PiiDefaultOperation.h>

/**
 * PiiImageConversionSwitch is an operation which convert a receiving
 * image from the color/gray to the gray/color type if necessary. 
 *
 * @inputs
 *
 * @in image - color or gray scale image
 *
 * @outputs
 *
 * @out gray image - gray scale image
 *
 * @out color image - color image
 *
 * @see PiiColorConverter
 *
 * @ingroup PiiColorsPlugin
 */
class PiiImageConversionSwitch : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The type of color conversion. The default value is @p
   * RgbToGrayMean.
   */
  Q_PROPERTY(ColorConversion colorConversion READ colorConversion WRITE setColorConversion);
  Q_ENUMS(ColorConversion);


  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Supported color conversion schemes.
   *
   * @lip RgbToGrayMean - calculate the mean of three color channels. 
   * Retains the type of color channels, but outputs a gray-level
   * image.
   */
  enum ColorConversion
  {
    RgbToGrayMean
  };

  PiiImageConversionSwitch();

  void setColorConversion(ColorConversion colorConversion);
  ColorConversion colorConversion() const;

protected:
  void check(bool reset);
  void process();

private:
  /**
   * Emit the gray image forward to gray image output and convert it
   * to PiiUnsignedCharColor4MatrixType for color output if it is connected.
   */
  void operateGrayImage(const PiiVariant& obj);
  /**
   * Emit the color image forward to color image output and convert it
   * to PiiUnsignedCharMatrixType for gray output if it is connected.
   */
  void operateColorImage(const PiiVariant& obj);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiInputSocket *pImageInput;
    PiiOutputSocket *pGrayImageOutput, *pColorImageOutput;
    bool bGrayImageOutputConnected, bColorImageOutputConnected;
    
    ColorConversion colorConversion;
  };
  PII_D_FUNC;
};


#endif //_PIIIMAGECONVERSIONSWITCH_H
