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

#ifndef _PIIIMAGEGLOBAL_H
#define _PIIIMAGEGLOBAL_H

#include <PiiGlobal.h>
#include <QObject>

#ifdef PII_BUILD_IMAGE
#  define PII_IMAGE_EXPORT PII_DECL_EXPORT
#  define PII_BUILDING_IMAGE 1
#else
#  define PII_IMAGE_EXPORT PII_DECL_IMPORT
#  define PII_BUILDING_IMAGE 0
#endif

#ifdef Q_MOC_RUN
class PiiImage
#else
namespace PiiImage
#endif
{
#ifdef Q_MOC_RUN
  Q_GADGET
    
  Q_ENUMS(TransformedSize);
  Q_ENUMS(Connectivity);
  Q_ENUMS(MorphologyOperation);
  Q_ENUMS(MaskType);
  Q_ENUMS(RoiType);
public:
#endif
  /// @internal
  extern PII_IMAGE_EXPORT const QMetaObject staticMetaObject;

  /**
   * Border handling methods for geometric image transforms.
   *
   * @lip RetainOriginalSize - crop the transformed image to the size
   * of the source image. Any border areas visible in the final image
   * will be left black. If the image is translated, the whole result
   * may be left black. (Figure A)
   *
   * @lip ExpandAsNecessary - create a smaller/larger image that
   * frames all transformed pixels. Translation won't affect the
   * result as it will always be centered. (Figure B)
   *
   * @code
   *                ______ 
   *    /\         |  /\  |
   * +-/--\-+      | /  \ |
   * |/    \|      |/    \|
   * |\    /|      |\    /|
   * +-\--/-+      | \  / |
   *    \/         |__\/__|
   *
   *    A             B
   * @endcode
   *
   * In the figures, the diamond shape denotes the transformed image. 
   * The boxed area shows the size of the result image.
   */
  enum TransformedSize { RetainOriginalSize, ExpandAsNecessary };
  
  /**
   * Pixel connectivitity modes for operations such as labeling and
   * boundary finding.
   *
   * @lip Connect4 - the four horizontal and vertical neighbors of a
   * pixel are treated as connected pixels.
   *
   * @lip Connect8 - all neighbors (horizontal, vertical and diagonal)
   * of a pixel are treated as connected pixels.
   * @code
   *  4-connected     8-connected
   * +---+---+---+   +---+---+---+
   * |   | X |   |   | X | X | X |
   * +---+---+---+   +---+---+---+
   * | X | o | X |   | X | o | X |
   * +---+---+---+   +---+---+---+
   * |   | X |   |   | X | X | X |
   * +---+---+---+   +---+---+---+
   * @endcode
   */
  enum Connectivity { Connect4, Connect8 };

  /**
	 * Binary morphology operations:
   *
	 * @lip Erode - erosion
	 * @lip Dilate - dilation
   * @lip Open - dilation after erosion
   * @lip Close - erosion after dilation
   * @lip Tophat - original minus opened
   * @lip Bottomhat - closed minus original
	 */
  enum MorphologyOperation { Erode, Dilate, Open, Close, TopHat, BottomHat };
  
  /**
   * Predefined types for structuring elements.
   *
   * @lip RectangularMask - the whole mask is filled with ones.
   *
   * @lip EllipticalMask - circle or ellipse, depending on the size of
   * the mask.
   *
   * @lip DiamondMask - a diamond
   *
   * Examples of 5-by-5 masks:
   *
   * @code
   * Rectangular  Elliptical   Diamond
   *  1 1 1 1 1   0 1 1 1 0   0 0 1 0 0
   *  1 1 1 1 1   1 1 1 1 1   0 1 1 1 0
   *  1 1 1 1 1   1 1 1 1 1   1 1 1 1 1
   *  1 1 1 1 1   1 1 1 1 1   0 1 1 1 0
   *  1 1 1 1 1   0 1 1 1 0   0 0 1 0 0
   * @endcode
   */
  enum MaskType { RectangularMask, EllipticalMask, DiamondMask };

  /**
   * Region-of-interest types.
   *
   * @lip AutoRoi - the type of the ROI is automatically determined. 
   * If the ROI object is a PiiMatrix<int>, @p RectangleRoi will be
   * used. Otherwise, @p MaskRoi will be used.
   *
   * @lip MaskRoi - the ROI is represented as a binary matrix
   * (PiiMatrix<bool>) in which non-zero (@p true) entries represent
   * interesting pixels. The size of the ROI mask must be equal to
   * that of the processed image.
   *
   * @lip RectangleRoi - the ROI is a set of rectangles, each
   * represented by a four-dimensional vector (x, y, width, height). 
   * The rectangles are stored in an N-by-4 PiiMatrix<int>.
   *
   * @lip NoRoi - ROI is disabled. Even if there is a ROI present, the
   * whole image will be processed.
   */
  enum RoiType { AutoRoi, MaskRoi, RectangleRoi, NoRoi };
};

#endif //_PIIIMAGEGLOBAL_H
