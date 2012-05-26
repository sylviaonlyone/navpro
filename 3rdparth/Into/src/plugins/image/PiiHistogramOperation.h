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

#ifndef _PIIHISTOGRAMOPERATION_H
#define _PIIHISTOGRAMOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>
#include <PiiImage.h>

/**
 * An operation that calculates the histogram of a gray-scale or a
 * color image.
 *
 * @inputs
 *
 * @in image - the input image, any gray-scale or color image.
 *
 * @in roi - region-of-interest. See @ref PiiImagePlugin for details. 
 * Optional.
 *
 * @outputs
 *
 * @out red - the red channel histogram
 *
 * @out green - the green channel histogram
 *
 * @out blue - the blue channel histogram
 *
 * If a gray-scale image is read, the histogram will be sent to all
 * three outputs.
 *
 * @ingroup PiiImagePlugin
 */
class PiiHistogramOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The number of distinct levels in the output histogram. If a
   * non-positive number is given, the maximum value of the
   * image/color channel is found and used (after adding one). For 8
   * bit gray-scale images, use 256. The default value is 256.
   */
  Q_PROPERTY(int levels READ levels WRITE setLevels);

  /**
   * Output normalization. If set to @p false (the default), the
   * output will be a PiiMatrix<int> in which each column represents
   * the number of times the specific color was encountered. If @p
   * true, output histograms will be normalized so that they always
   * sum up to unity and represented by PiiMatrix<float>.
   */
  Q_PROPERTY(bool normalized READ normalized WRITE setNormalized);

  /**
   * The type or the @p roi input, if connected. The default value is
   * @p AutoRoi.
   */
  Q_PROPERTY(PiiImage::RoiType roiType READ roiType WRITE setRoiType);

  PII_OPERATION_SERIALIZATION_FUNCTION;
public:
  PiiHistogramOperation();

protected:
  void process();
  
  void aboutToChangeState(State state);

  void setNormalized(bool normalize);
  bool normalized() const;

  void setRoiType(PiiImage::RoiType roiType);
  PiiImage::RoiType roiType() const;

  int levels() const;
  void setLevels(int levels);

private:
  template <class T> struct GrayHistogram;
  template <class T> struct ColorHistogram;
  template <class T> struct Histogram;

  template <class T> void histogram(const PiiVariant& obj);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    ~Data();
    
    int iLevels;
    PiiInputSocket* pImageInput, *pRoiInput;
    
    bool bNormalized;
    PiiImage::RoiType roiType;
    PiiImage::HistogramHandler* pHistogram;
    unsigned int uiPreviousType;
  };
  PII_D_FUNC;
};

#endif //_PIIHISTOGRAMOPERATION_H
