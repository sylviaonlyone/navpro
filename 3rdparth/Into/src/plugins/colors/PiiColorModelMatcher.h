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

#ifndef _PIICOLORMODELMATCHER_H
#define _PIICOLORMODELMATCHER_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>

/**
 * An operation that converts a color image into an intensity map. 
 * Pixel values in the intensity map are proportional to the
 * likelihood of that pixel belonging to a predefined color model.
 *
 * @inputs
 *
 * @in image - the input color image. Any color image type.
 *
 * @in model - an image out of which the color model is to be derived.
 * Any color image type.
 *
 * @outputs
 *
 * @out image - an intensity map image indicating the likelihood of
 * each pixel belonging to the model (PiiMatrix<float>) or a
 * thresholded image (PiiMatrix<unsigned char>), if #matchingThreshold
 * is non-zero.
 *
 * @ingroup PiiColorsPlugin
 */
class PiiColorModelMatcher : public PiiDefaultOperation
{
  Q_OBJECT
  /**
   * matchingThreshold description
   */
  Q_PROPERTY(double matchingThreshold READ matchingThreshold WRITE setMatchingThreshold);
 
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiColorModelMatcher();
  
  void setMatchingThreshold(double matchingThreshold);
  double matchingThreshold() const;
  
protected:
  void process();

private:
  template <class T> void calculateModel(const PiiVariant& obj);
  template <class T> void matchImageToModel(const PiiVariant& obj);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiMatrix<double> matBaseVectors, matCenter;
    double dMatchingThreshold;
  };
  PII_D_FUNC;
};



#endif //_PIICOLORMODELMATCHER_H
