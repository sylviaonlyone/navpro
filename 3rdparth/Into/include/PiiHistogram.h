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

#ifndef _PIIHISTOGRAM_H
#define _PIIHISTOGRAM_H

#include "PiiQuantizer.h"
#include <PiiRoi.h>
#include <PiiMath.h>

/**
 * @file
 * Histogram handling functions.
 */

namespace PiiImage
{
  /**
   * Calculate the histogram of a one-channel image. The result will
   * be a row matrix containing the frequencies of all values in the
   * input image. The return type is determined by the @p T template
   * parameter.
   *
   * @param image the input image. All integer types are supported. If
   * another type is used, each element is casted to an int in
   * processing. The minimum value of the image must not be negative.
   *
   * @param roi region-of-interest. See PiiImage.
   *
   * @param levels the number of distinct levels in the image. If zero
   * is given, the maximum value of the image will be found. For 8 bit
   * gray-scale images, use 256.
   *
   * @return the histogram as a PiiMatrix<T>
   */
  template <class T, class U, class Roi> PiiMatrix<T> histogram(const PiiMatrix<U>& image, const Roi& roi, unsigned int levels);

  /**
   * Calculate the histogram of a one-channel image. This is a
   * shorthand for <tt>histogram<int>(image, roi, levels)</tt>.
   */
  template <class T, class Roi> inline PiiMatrix<int> histogram(const PiiMatrix<T>& image, const Roi& roi, unsigned int levels)
  {
    return histogram<int,T,Roi>(image, roi, levels);
  }

  /**
   * Calculate the histogram of a one-channel image. This is a
   * shorthand for <tt>histogram<int>(image, PiiImage::DefaultRoi(),
   * levels)</tt>.
   */
  template <class T> inline PiiMatrix<int> histogram(const PiiMatrix<T>& image, unsigned int levels=0)
  {
    return histogram<int,T>(image, PiiImage::DefaultRoi(), levels);
  }


  /**
   * Calculate the histogram of a one-channel image. The result will
   * be a row matrix containing the frequencies of all values in the
   * input image. The return type is determined by the @p T template
   * parameter.
   *
   * @param image the input image. All integer types are supported. If
   * another type is used, each element is casted to an int in
   * processing. The minimum value of the image must not be negative.
   *
   * @param roi region-of-interest. See PiiImage.
   *
   * @param quantizer a quantizer that converts image pixels into
   * quantized values.
   */
  template <class T, class U, class Roi> PiiMatrix<T> histogram(const PiiMatrix<U>& image, const Roi& roi, const PiiQuantizer<U>& quantizer);

  /**
   * Calculate the histogram of a one-channel image. This is a
   * shorthand for <tt>histogram<int>(image, quantizer)</tt>.
   */
  template <class T, class Roi> inline PiiMatrix<int> histogram(const PiiMatrix<T>& image, const Roi& roi, const PiiQuantizer<T>& quantizer)
  {
    return histogram<int>(image, roi, quantizer);
  }

  /**
   * Calculate the histogram of a one-channel image. This is a
   * shorthand for <tt>histogram<int>(image, quantizer,
   * PiiImage::DefaultRoi())</tt>.
   */
  template <class T> inline PiiMatrix<int> histogram(const PiiMatrix<T>& image, const PiiQuantizer<T>& quantizer)
  {
    return histogram<int>(image, PiiImage::DefaultRoi(), quantizer);
  }

  /**
   * Calculate the cumulative frequency distribution of the given
   * frequency distribution (histogram). The histogram must be
   * represented as a row vector. If the input matrix has many rows,
   * the cumulative histogram for each row is calculated.
   *
   * @code
   * PiiMatrix<int> histogram(1,5, 1,2,3,4,5);
   * PiiMatrix<int> cum(PiiImage::cumulative(histogram)); // sic!
   * // cum = (1,3,6,10,15)
   * @endcode
   *
   * @see Pii::cumulativeSum()
   */
  template <class T> inline PiiMatrix<T> cumulative(const PiiMatrix<T>& histogram)
  {
    return Pii::cumulativeSum<T,PiiMatrix<T> >(histogram, Pii::Horizontally);
  }

  /**
   * Normalize the given histogram so that its elements sum up to one. 
   * If the matrix has many rows, each row is normalized. The return
   * type may be different from the input type. Typically, float or
   * double is used as the return type. If all values in a row equal
   * to zero, they are left as such.
   *
   * @code
   * PiiMatrix<int> histogram(1,4, 1,2,3,4);
   * PiiMatrix<double> normalized(PiiImage::normalize<double>(histogram));
   * //normalized = (0.1, 0.2, 0.3, 0.4)
   * @endcode
   */
  template <class T, class U> PiiMatrix<T> normalize(const PiiMatrix<U>& histogram);

  /**
   * Find the index of the first entry in a cumulative frequency
   * distribution that exceeds or equals to the given value. For
   * normalized cumulative distributions, @p value should be between 0
   * and 1.
   *
   * @code
   * PiiMatrix<double> cumulative(1,4, 0.1, 0.3, 0.6, 1.0);
   * int p = percentile(cumulative, 0.5);
   * //p = 2
   * @endcode
   *
   * @param cumulative a cumulative frequency distribution. A row
   * matrix with monotonically increasing values.
   *
   * @param value the percentile value, which should be smaller than
   * or equal to the maximum value in @p cumulative.
   *
   * @return the index of the first element exceeding or equal to @p
   * value, or -1 if no such element was found
   */
  template <class T> int percentile(const PiiMatrix<T>& cumulative, T value);

  /**
   * Histogram backprojection. In backprojection, each pixel in @p img
   * is replaced by the corresponding value in @p histogram. Despite
   * histogram backprojection this function can be used to convert
   * indexed images to color images.
   *
   * @param img input image
   *
   * @param histogram the histogram. A 1-by-N matrix.
   *
   * The function makes no boundary checks for performance reasons. If
   * you aren't sure about your data, you must check that @p
   * histogram.columns() is larger than the maximum value in @p img
   * and that there are no negative values in @p img.
   *
   * Since pixels in @p img are used as indices in @p histogram, they
   * will be converted to integers. Using floating-point types in the
   * @p img parameter is not suggested.
   *
   * @code
   * // Normal backprojection
   * PiiMatrix<int> histogram(1,256);
   * PiiMatrix<unsigned char> img(100,100);
   * PiiMatrix<int> backProjected(PiiImage::backProject(img,histogram));
   *
   * // Color mapping
   * PiiMatrix<PiiColor<> > colorMap(1,256);
   * PiiMatrix<unsigned char> indexedImg(100,100);
   * PiiMatrix<PiiColor<> > colorImg(PiiImage::backProject(indexedImg, colorMap);
   * @endcode
   */
  template <class T, class U> PiiMatrix<U> backProject(const PiiMatrix<T>& img, const PiiMatrix<U>& histogram);

  /**
   * Two-dimensional histogram backprojection. This function is
   * analogous to the previous one, but uses a two-dimensional
   * histogram for backprojection. It is provided just for convenience
   * as two-dimensional distributions can be converted to one
   * dimension.
   *
   * @param ch1 first channel (indexes rows in @p histogram, maximum
   * value N-1)
   *
   * @param ch2 second channel (indexes columns in @p histogram,
   * maximum value M-1)
   *
   * @param histogram two-dimensional histogram (N-by-M)
   *
   * The sizes of @p ch1 and @p ch2 must be equal.
   *
   * @code
   * // Backproject a two-dimensional RG histogram
   * PiiMatrix<int> histogram(256,256);
   * PiiMatrix<unsigned char> redChannel(100,100);
   * PiiMatrix<unsigned char> greenChannel(100,100);
   * PiiMatrix<int> backProjected(PiiImage::backProject(redChannel, greenChannel, histogram));
   * @endcode
   */
  template <class T, class U> PiiMatrix<U> backProject(const PiiMatrix<T>& ch1, const PiiMatrix<T>& ch2,
                                                       const PiiMatrix<U>& histogram);

  /**
   * Histogram equalization. Enhances the contrast of @p img by making
   * its gray levels as uniformly distributed as possible.
   *
   * @param img the input image
   *
   * @param levels the number of quantization levels. If this value is
   * omitted, the maximum value found in @p image will be used. If @p
   * levels is smaller than the maximum value, the latter will be used.
   *
   * @return an image with enhanced contrast
   */
	template <class T> PiiMatrix<T> equalize(const PiiMatrix<T>& img, unsigned int levels = 0);
};

#include "PiiHistogram-templates.h"

#endif //_PIIHISTOGRAM_H
