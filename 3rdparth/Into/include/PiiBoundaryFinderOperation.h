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

#ifndef _PIIBOUNDARYFINDEROPERATION_H
#define _PIIBOUNDARYFINDEROPERATION_H

#include <PiiDefaultOperation.h>

/**
 * Extracts boundaries of connected objects in images.
 *
 * @inputs
 *
 * @in image - input image. This is usually a binary image or a
 * labeled image, but any gray-level image works. To avoid a separate
 * thresholding step one can set the #threshold property to a non-zero
 * value.
 *
 * @outputs
 *
 * @out boundary - 0-N polygons that contain the boundaries of the
 * detected objects. Each polygon is represented as a M-by-2
 * PiiMatrix<int> in which each row stores stores the (x,y)
 * coordinates of a boundary point in the order the algorithm
 * traversed the boundary. The flow level of this output is one higher
 * than the @p image input.
 *
 * @out boundaries - all boundaries extracted from the input image
 * concatenated to one matrix.
 *
 * @out limits - the indices of boundary polygon ends
 * (PiiMatrix<int>). The first vector is always at index 0, and the
 * first limit is the index of the start row of the second one. If the
 * first polygon has 120 points and the second one 90 points, @p
 * limits will be the 1-by-2 matrix (120, 210).
 *
 * @out mask - boundary mask. A gray-level image in which the detected
 * edges are marked according to their type. See PiiBoundaryFinder for
 * an explanation.
 *
 * @ingroup PiiImagePlugin
 */
class PiiBoundaryFinderOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * A static threshold for binarizing a gray-level input image. Every
   * pixel in the image whose gray level is higher than @p threshold
   * is taken to be part of an object. The default value is zero,
   * which assumes a pre-binarized or labeled input image. Setting
   * this value to a non-zero value may be useful in avoiding a
   * separate thresholding step.
   */
  Q_PROPERTY(double threshold READ threshold WRITE setThreshold);

  /**
   * The minimum number of connected pixels required to form a
   * boundary. If a detected boundary has less pixels, it will be
   * discarded.
   */
  Q_PROPERTY(int minLength READ minLength WRITE setMinLength);

  /**
   * The maximum number of connected pixels allowed to form a
   * boundary. If a detected boundary has more pixels, it will be
   * discarded.
   */
  Q_PROPERTY(int maxLength READ maxLength WRITE setMaxLength);
  
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  PiiBoundaryFinderOperation();

protected:
  void process();

  void setThreshold(double threshold);
  double threshold() const;
  void setMinLength(int minLength);
  int minLength() const;
  void setMaxLength(int maxLength);
  int maxLength() const;

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dThreshold;
    PiiOutputSocket *pBoundaryOutput, *pBoundariesOutput, *pLimitsOutput, *pMaskOutput;
    int iMinLength;
    int iMaxLength;
  };
  PII_D_FUNC;

  template <class T> void findBoundaries(const PiiVariant& obj);
};

#endif //_PIIBOUNDARYFINDEROPERATION_H
