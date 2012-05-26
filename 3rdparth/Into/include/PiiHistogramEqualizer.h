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

#ifndef _PIIHISTOGRAMEQUALIZER_H
#define _PIIHISTOGRAMEQUALIZER_H

#include <PiiDefaultOperation.h>

/**
 * Histogram equalizer. Enhances the contrast of input images by
 * making their gray-level distribution as uniform as possible.
 *
 * @inputs
 *
 * @in image - The input image. Any gray-level image.
 * 
 * @outputs
 *
 * @out image - Equalized image. Type equals that of the input.
 *
 * @ingroup PiiImagePlugin
 */
class PiiHistogramEqualizer : public PiiDefaultOperation
{
  Q_OBJECT
  
  /**
   * The number of discrete gray levels in the output image. The
   * default value is 256. Setting this value to zero makes the
   * operation use the maximum value of the input image.
   */
  Q_PROPERTY(int levels READ levels WRITE setLevels);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiHistogramEqualizer();

  void setLevels(int levels);
  int levels() const;

protected:
  void process();

private:
  template <class T> void equalize(const PiiVariant& obj);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    int iLevels;};
  PII_D_FUNC;

};


#endif //_PIIHISTOGRAMEQUALIZER_H
