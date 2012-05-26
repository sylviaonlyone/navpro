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

#ifndef _PIIIMAGEFILTEROPERATION_H
#define _PIIIMAGEFILTEROPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiMatrixUtil.h>

/**
 * An operation that implements basic image filtering operations. It
 * supports the most commonly used edge detection and noise reduction
 * filters and also allows one to specify a custom filter.
 *
 * @inputs
 *
 * @in image - the image to be filtered. Any image type. For color
 * images, the filter will be applied channel-wise.
 * 
 * @outputs
 *
 * @out image - the filtered image. The type of the output image
 * equals that of the input.
 *
 * @ingroup PiiImagePlugin
 */
class PiiImageFilterOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The name of the image filter. This is an easy way to set the
   * filter. For valid values see PiiImage::PrebuiltFilterType. Use
   * "sobelx" for @p SobelXFilter, "gaussian" for @p GaussianFilter
   * etc. There are two special values not supported by makeFilter():
   *
   * @lip median - a median filter. Median filter is non-linear and
   * cannot be implemented with ordinary correlation masks.
   *
   * @lip custom - #filter will be used as the filter mask.
   *
   * The default value is "uniform".
   */
  Q_PROPERTY(QString filterName READ filterName WRITE setFilterName);

  /**
   * The size of the filter mask. This property has no effect if
   * #filterName is set to @p custom. It is also ignored by some
   * filters as described in PiiImage::makeFilter(). Automatically
   * created filters are always square. The default value is 3.
   */
  Q_PROPERTY(int filterSize READ filterSize WRITE setFilterSize);
    
  /**
   * The filter as a matrix. This value is used only if #filterName is
   * set to "custom".
   *
   * @code
   * PiiOperation* op = engine.createOperation("PiiImageFilter");
   * op->setProperty("filter",
   *                 PiiVariant(new PiiMatrix<int>(3, 3,
   *                                                       1, 2, 1,
   *                                                       2, 4, 2,
   *                                                       1, 2, 1));
   * @endcode
   */
  Q_PROPERTY(PiiVariant filter READ filter WRITE setFilter);

  /**
   * The method of handling image borders. The default value is @p
   * ExtendZeros.
   */
  Q_PROPERTY(ExtendMode borderHandling READ borderHandling WRITE setBorderHandling);
  Q_ENUMS(ExtendMode);

  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  /**
   * Different ways of extending images in filtering. Copied from
   * PiiMatrixUtil to avoid extensive @p moc hacking.
   *
   * @lip ExtendZeros - pad with zeros
   * @lip ExtendReplicate - replicate the value on the border
   * @lip ExtendSymmetric - symmetrically mirror boundary values
   * @lip ExtendPeriodic - take values from the opposite border
   * @lip ExtendNot - do not extend
   */
  enum ExtendMode { ExtendZeros, ExtendReplicate, ExtendSymmetric, ExtendPeriodic, ExtendNot };

  PiiImageFilterOperation();

  void setFilterName(const QString& filterName);
  QString filterName() const;
  void setFilter(const PiiVariant& filter);
  PiiVariant filter() const;
  void setFilterSize(int filterSize);
  int filterSize() const;
  void setBorderHandling(ExtendMode borderHandling);
  ExtendMode borderHandling() const;

  void check(bool reset);

protected:
  void process();

private:
  enum FilterType { Prebuilt, Median, Custom };

  template <class T> void intGrayFilter(const PiiVariant& obj);
  template <class T> void floatGrayFilter(const PiiVariant& obj);
  template <class T> void intColorFilter(const PiiVariant& obj);
  template <class T> void floatColorFilter(const PiiVariant& obj);
  template <class T> void setCustomFilter(const PiiVariant& obj);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    FilterType filterType;
    QString strFilterName;
    int iFilterSize;
    Pii::ExtendMode borderHandling;
    
    PiiVariant pCustomFilter;
    PiiMatrix<double> matPrebuiltFilter;
    PiiMatrix<double> matCustomFilter;
    // Active filter and its decomposition (is available)
    bool bSeparableFilter;
    PiiMatrix<double> matActiveFilter, matHorzFilter, matVertFilter;
  };
  PII_D_FUNC;

};


#endif //_PIIIMAGEFILTEROPERATION_H
