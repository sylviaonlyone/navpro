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

#ifndef _PIILABELINGOPERATION_H
#define _PIILABELINGOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>
#include "PiiImageGlobal.h"

/**
 * Basic labeling operations.
 *
 * @inputs
 *
 * @in image - the input image. Binary image. If the image is not
 * binary, it will be automatically thresholded. (Any gray-level image
 * type.)
 *
 * @outputs
 *
 * @out image - the labeled image. A PiiMatrix<int> type in which
 * objects are marked with increasing ordinal numbers 1,2,...,N. The
 * background will be zero.
 *
 * @out labels - the number of distinct objects in the input image. 
 * (int)
 *
 * @ingroup PiiImagePlugin
 */
class PiiLabelingOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Connectivity type for labeling. The default is @p Connect4.
   */
  Q_PROPERTY(PiiImage::Connectivity connectivity READ connectivity WRITE setConnectivity);

  /**
   * A static threshold value for automatic thresholding. Every pixel
   * whose gray level is higher than this value will be considered an
   * object. The default value is zero.
   */
  Q_PROPERTY(double threshold READ threshold WRITE setThreshold);

  /**
   * A hysteresis for the thresholding. If this value is non-zero, @p
   * threshold will decide the minimum value for a gray level that can
   * be a "seed" for a connected object. A all pixels surrounding the
   * seed that are brighter than @p threshold - @p hysteresis are
   * joined to the connected component.
   */
  Q_PROPERTY(double hysteresis READ hysteresis WRITE setHysteresis);

  /**
   * A flag that can be used to invert the automatic thresholding. If
   * this value is set to @p true, dark objects will be considered
   * targets. The default value is @p false.
   */
  Q_PROPERTY(bool inverse READ inverse WRITE setInverse);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiLabelingOperation();

  void setConnectivity(PiiImage::Connectivity connectivity);
  PiiImage::Connectivity connectivity() const;
  void setThreshold(double threshold);
  double threshold() const;
  void setHysteresis(double hysteresis);
  double hysteresis() const;
  void setInverse(bool inverse);
  bool inverse() const;

protected:
  void process();
  
private:
  template <class T> void operate(const PiiVariant& obj);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiImage::Connectivity connectivity;
    
    PiiInputSocket* pBinaryImageInput;
    PiiOutputSocket* pLabeledImageOutput;
    PiiOutputSocket* pLabelsOutput;
    double dThreshold;
    double dHysteresis;
    bool bInverse;
  };
  PII_D_FUNC;
};


#endif //_PIILABELINGOPERATION_H
