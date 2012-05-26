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

#ifndef _PIIMORPHOLOGY_H
#define _PIIMORPHOLOGY_H

#include <PiiMatrix.h>
#include <iostream>
#include "PiiImageGlobal.h"
#include <PiiTemplateExport.h>

/**
 * @file
 * @ingroup PiiImagePlugin
 *
 * Morphological operations such as erosion, dilation, opening,
 * closing, top-hat and bottom-hat.
 */

namespace PiiImage
{ 
  /**
   * Create a morphological mask. A template implementation that can
   * be used to create binary masks with any content type.
   *
   * @param type a predefined mask type
   *
   * @param rows the number of rows in the mask
   *
   * @param columns the number of columns in the mask (0 means equal to rows)
   *
   * @return a mask suitable for morphological operations
   */
  template <class T> PiiMatrix<T> createMask(MaskType type, int rows, int columns = 0);

  /**
   * Writes a morphological structuring element to @a mask.
   *
   * @param mask a matrix to be filled with the morphological mask.
   *
   * @note The implementation writes only non-zero values to @a mask. 
   * One usually needs to make sure the mask is initially zeros.
   */
  template <class T> void createMask(MaskType type, PiiMatrix<T>& mask);
  
  /**
   * Create a morphological mask. This is the default version of this
   * function with no template arguments that returns a
   * PiiMatrix<int>.
   */
  PII_IMAGE_EXPORT PiiMatrix<int> createMask(MaskType type, int rows, int columns = 0);

  /**
   * Create a morphological mask. This version returns a
   * 8-bit matrix.
   */
  PII_DECLARE_EXPORTED_FUNCTION_TEMPLATE(PiiMatrix<unsigned char>,
                                         createMask<unsigned char>,
                                         (MaskType type, int rows, int columns),
                                         PII_BUILDING_IMAGE);

  /**
   * Masks for detecting border pixels in binary objects. This array
   * consists of eight masks suitable for use with the hitAndMiss()
   * function. Each mask detects borders in one direction. The masks
   * are numbered as follows:
   *
   * @li 0 - north
   * @li 1 - north-east
   * @li 2 - east
   * @li 3 - south-east
   * @li 4 - south
   * @li 5 - south-west
   * @li 6 - west
   * @li 7 - north-west
   *
   * <tt>borderMasks[X][0]</tt> is the detector and
   * <tt>borderMasks[X][1]</tt> the corresponding significance mask.
   */
  PII_IMAGE_EXPORT extern PiiMatrix<int> borderMasks[8][2];
  
  /**
   * Perform a morphological operation on an image.
   *
   * @param image is original binary image.
   *
   * @param mask structuring element
   *
   * @param type type of moprhological operation
   */
  template <class T, class U> PiiMatrix<T> morphology(const PiiMatrix<T>& image, const PiiMatrix<U>& mask,
                                                      MorphologyOperation type, bool handleBorders = false);
  

  /**
   * Perform a morphological erosion operation on a image. 
   *
   * @param image is orginal binary image.
   *
   * @param mask is structuring element.
   *
   * @param handleBorders is flag that determines whether image
   * borders are handled with a padding technique. If this flag is @p
   * false (the default), zeros are assumed outside of the image.
   *
   * @return the binary image which is result of erosion 
   *
   * @code
   *
   * PiiMatrix<int> source(8,8,
   *                        0,0,0,0,0,0,0,0,
   *                        1,1,1,1,1,1,1,1,
   *                        0,0,0,1,1,1,1,1,
   *                        0,0,0,1,1,1,1,1,
   *                        0,0,1,1,1,1,1,1,
   *                        0,0,0,1,1,1,1,1,
   *                        0,0,1,1,0,0,0,0,
   *                        0,0,0,0,0,0,0,0);
   *
   * PiiMatrix<int> mask(3,4,
   *                     1,1,1,1,
   *                     1,1,1,1,
   *                     1,1,1,1); // mask origin now (1,2)
   *
   * PiiMatrix<int> result = PiiImage::erode(source, mask,false);
   * Pii::matlabPrint(std::cout, result);
   *
   * //Output
   * //0,0,0,0,0,0,0,0,
   * //0,0,0,0,0,0,0,0,
   * //0,0,0,0,0,1,1,0,
   * //0,0,0,0,0,1,1,0,
   * //0,0,0,0,0,1,1,0,
   * //0,0,0,0,0,1,1,0,
   * //0,0,0,0,0,0,0,0,
   * //0,0,0,0,0,0,0,0
   *
   * //Padding example:
   * PiiMatrix<int> source(8,8,
   *                       1,1,0,0,0,0,0,1,
   *                       1,1,1,1,1,1,1,0,
   *                       1,1,0,1,1,1,1,0,
   *                       0,0,0,1,1,1,1,0,
   *                       0,0,1,1,1,1,1,0,
   *                       0,0,0,1,1,1,1,0,
   *                       0,0,1,1,0,0,0,0,
   *                       1,0,0,0,0,0,0,0);
   *
   * PiiMatrix<int> mask(3,3,
   *                     1,1,1,
   *                     1,1,1,
   *                     1,1,1);
   * 
   * PiiMatrix<int> result = PiiImage::erode(source, mask, true);
   * Pii::matlabPrint(std::cout, result);
   * //Output
   * //1,0,0,0,0,0,0,0,
   * //1,0,0,0,0,0,0,0,
   * //0,0,0,0,1,1,0,0,
   * //0,0,0,0,1,1,0,0,
   * //0,0,0,0,1,1,0,0,
   * //0,0,0,0,0,0,0,0,
   * //0,0,0,0,0,0,0,0,
   * //0,0,0,0,0,0,0,0
   *
   * @endcode
   */
  template <class T, class U> PiiMatrix<T> erode(const PiiMatrix<T>& image, const PiiMatrix<U>& mask,
                                                 bool handleBorders = false );
  /**
   * Dilation.
   *
   * @param image is orginal binary image.
   *
   * @param mask is structuring element.
   *
   * @return the binary image which is result of dilation.
   * 
   * @code
   *    
   * PiiMatrix<int> source(8,8,
   *                       0,0,0,0,0,0,0,0,
   *                       1,1,1,1,1,1,1,0,
   *                       0,0,0,1,1,1,1,0,
   *                       0,0,0,1,1,1,1,0,
   *                       0,0,1,1,1,1,1,0,
   *                       0,0,0,1,1,1,1,0,
   *                       0,0,1,1,0,0,0,0,
   *                       0,0,0,0,0,0,0,0);
   *
   * PiiMatrix<int> mask(3,3,
   *                     1,1,1,
   *                     1,1,1,
   *                     1,1,1);
   * 
   * PiiMatrix<int> result = PiiImage::dilate(source, mask);
   * Pii::matlabPrint(std::cout,result);
   * //Output
   * //1,1,1,1,1,1,1,1,
   * //1,1,1,1,1,1,1,1,
   * //1,1,1,1,1,1,1,1,
   * //0,1,1,1,1,1,1,1,
   * //0,1,1,1,1,1,1,1,
   * //0,1,1,1,1,1,1,1,
   * //0,1,1,1,1,1,1,1,
   * //0,1,1,1,1,0,0,0   
   *
   * @endcode
   */
  template <class T, class U> PiiMatrix<T> dilate(const PiiMatrix<T>& image, const PiiMatrix<U>& mask );
  /**
   * Morphological opening.
   */
  template <class T, class U> inline PiiMatrix<T> open(const PiiMatrix<T>& image, const PiiMatrix<U>& mask )
  { return dilate(erode(image,mask),mask); }
  /**
   * Morphological closing.
   */
  template <class T, class U> inline PiiMatrix<T> close(const PiiMatrix<T>& image, const PiiMatrix<U>& mask )
  { return erode(dilate(image,mask), mask); }

  /**
   * Bottom-hat transform. Also known as the black top-hat transform. 
   * The bottom-hat transform extracts small dark elements from
   * images. It is defined as the difference between the closing of an
   * imput image and the input image itself.
   *
   * @param image a binary image.
   *
   * @param mask structuring element.
   *
   * @return bottom-hat transform
   * 
   * @code
   * PiiMatrix<int> source(8,8,
   *                       0,0,0,0,0,0,0,0,
   *                       1,1,1,1,1,1,1,0,
   *                       0,0,0,1,1,1,1,0,
   *                       0,0,0,1,1,1,1,0,
   *                       0,0,1,1,1,1,1,0,
   *                       0,0,0,1,1,1,1,0,
   *                       0,0,1,1,0,0,0,0,
   *                       0,0,0,0,0,0,0,0);
   * 
   * PiiMatrix<int> mask(3,3,
   *                     1,1,1,
   *                     1,1,1,
   *                     1,1,1);
   *
   * PiiMatrix<int> result(PiiImage::bottomHat(source,mask));
   * //Output
   * // 0,0,0,0,0,0,0,0,
   * // 0,0,0,0,0,0,0,0,
   * // 0,0,1,0,0,0,0,0,
   * // 0,0,1,0,0,0,0,0,
   * // 0,0,0,0,0,0,0,0,
   * // 0,0,1,0,0,0,0,0,
   * // 0,0,0,0,0,0,0,0,
   * // 0,0,0,0,0,0,0,0
   * @endcode
   */
  template <class T, class U> PiiMatrix<T> bottomHat(const PiiMatrix<T>& image, const PiiMatrix<U>& mask );

  /**
   * Top-hat transform. Top-hat transform is an operation that
   * extracts small bright elements from images. The top-hat transform
   * is defined as the difference between the input image and its
   * opening by a structuring element.
   */
  template <class T, class U> PiiMatrix<T> topHat(const PiiMatrix<T>& image, const PiiMatrix<U>& mask );

  /**
   * Hit-and-miss transform. The transform slides @p mask over all
   * pixels in the input image. The mask is compared to image data
   * only on the pixels that have the corresponding @p significance
   * mask entry set to a non-zero value. The result is one if all such
   * pixels match, and zero otherwise.
   *
   * @param image input image
   *
   * @param mask structuring element
   *
   * @param significance "significance" mask for the structuring
   * element. Non-zero values in this mask mark the pixels in @p
   * structure that we should care about.
   *
   * @return transformed image
   *
   * Locate north-east pointing corners in the image:
   *
   * @code
   * PiiMatrix<int> image(5,5,
   *                      0,0,0,0,0,
   *                      0,1,1,1,0,
   *                      0,1,1,1,0,
   *                      0,1,1,1,0,
   *                      0,0,0,0,0);
   *
   * PiiMatrix<int> m(3,3
   *                  0,0,0,
   *                  1,1,0,
   *                  1,1,0),
   * PiiMatrix<int> s(3,3
   *                  1,1,0,
   *                  1,1,1,
   *                  1,1,1)
   *
   * PiiMatrix<int> result = PiiImage::hitAndMiss(image, m, s);
   * // Result equals to:
   * // PiiMatrix<int> result(5,5,
   * //                       0,0,0,0,0,
   * //                       0,0,0,1,0,
   * //                       0,0,0,0,0,
   * //                       0,0,0,0,0,
   * //                       0,0,0,0,0);
   * @endcode
   */
  template <class T, class U> PiiMatrix<T> hitAndMiss(const PiiMatrix<T>& image,
                                                      const PiiMatrix<U>& mask,
                                                      const PiiMatrix<U>& significance);

  /**
   * Thin binary objects towards a skeleton.
   *
   * @param image input image
   *
   * @param amount the number of iterations. On each iteration, one
   * border pixel will be removed. If amount < 0, the operation loops
   * until the binary objects have converged to one-pixel wide
   * skeletons.
   *
   * @return thinned image
   */
  template <class T> PiiMatrix<T> thin(const PiiMatrix<T>& image, int amount = 1);

  /**
   * Extract the border of binary objects.
   *
   * @param image input image
   *
   * @return an image in which only the borders of binary objects are
   * retained.
   */
  template <class T> PiiMatrix<T> border(const PiiMatrix<T>& image);

  /**
   * Remove object borders. This operation is equivalent to
   * subtracting border() from the original image.
   *
   * @param image input image
   *
   * @param amount the number of iterations.
   *
   * @return an image in which the borders of binary objects have been
   * removed.
   */
  template <class T> PiiMatrix<T> shrink(const PiiMatrix<T>& image, int amount = 1);
}

#include <PiiMorphology-templates.h>

#endif //_PIIMORPHOLOGY_H
