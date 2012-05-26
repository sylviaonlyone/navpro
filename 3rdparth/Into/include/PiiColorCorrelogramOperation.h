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

#ifndef _PIICOLORCORRELOGRAMOPERATION_H
#define _PIICOLORCORRELOGRAMOPERATION_H

#include <PiiDefaultOperation.h>

/**
 * Calculate the color autocorrelogram of an image. See
 * @ref PiiColors::autoCorrelogram() for details.
 *
 * @inputs
 *
 * @in image - any color image or an indexed image. If a color image
 * is received, it will be converted to an indexed image. See #levels.
 *
 * @outputs
 *
 * @out correlogram - the autocorrelogram. A PiiMatrix<float>.
 *
 * @ingroup PiiColorsPlugin
 */
class PiiColorCorrelogramOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * A flag that controls quantization of color channels input. If
   * this flag is @p true (the default), color channels will be
   * quantized to the specified number of discrete #levels. If the
   * flag is @p false, color channels are assumed to be quantized
   * beforehand to the specified number of #levels.
   */
  Q_PROPERTY(bool quantize READ quantize WRITE setQuantize);

  /**
   * The number of quantization levels per each color channel for
   * color indexing. Color indices for RGB images are calculated as
   * specified in @ref PiiColors::toIndexed(). The default value is 4. 
   * If the #quantize flag is @p true, color channels are quantized to
   * this many levels before calculating the correlogram. If the
   * #quantize flag is @p false, color channel values greater than or
   * equal to @p levels will be ignored.
   */
  Q_PROPERTY(int levels READ levels WRITE setLevels);

  /**
   * The distances for which the color correlogram will be calculated.
   * The default is 1,3,5,7.
   */
  Q_PROPERTY(QVariantList distances READ distances WRITE setDistances);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiColorCorrelogramOperation();
  ~PiiColorCorrelogramOperation();

  void setLevels(int levels);
  int levels() const;
  void setDistances(const QVariantList& distances);
  QVariantList distances() const;
  void setQuantize(bool quantize);
  bool quantize() const;

  void check(bool reset);
  
protected:
  void process();
  
private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiInputSocket* pInput;
    PiiOutputSocket* pOutput;
    int iLevels;
    QList<int> lstDistances;
    bool bQuantize;
  };
  PII_D_FUNC;

  template <class Clr> void processColor(const PiiVariant& obj);
  template <class T> void processGray(const PiiVariant& obj);
};


#endif //_PIICOLORCORRELOGRAMOPERATION_H
