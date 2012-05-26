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

#ifndef _PIICORNERDETECTOR_H
#define _PIICORNERDETECTOR_H

#include <PiiDefaultOperation.h>

/**
 * Detects corners in gray-level images using the FAST corner
 * detector.
 *
 * @inputs
 *
 * @in image - a gray-level input image
 *
 * @outputs
 *
 * @out corners - corner coordinates, a N-by-2 PiiMatrix<int> in which
 * each row stores the (x,y) coordinates of a detected corner.
 *
 * @ingroup PiiImagePlugin
 */
class PiiCornerDetector : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Corner detection threshold. See @ref
   * PiiImage::detectFastCorners().
   */
  Q_PROPERTY(double threshold READ threshold WRITE setThreshold);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiCornerDetector();

  void setThreshold(double threshold);
  double threshold() const;
  
protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dThreshold;
  };
  PII_D_FUNC;

  template <class T> void detectCorners(const PiiVariant& obj);
};

#endif //_PIICORNERDETECTOR_H
