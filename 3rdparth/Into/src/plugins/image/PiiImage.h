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

#ifndef _PIIIMAGE_H
#define _PIIIMAGE_H

#include "PiiImageGlobal.h"
#include <PiiMath.h>
#include <PiiMatrixUtil.h>
#include <PiiDsp.h>
#include <PiiColor.h>
#include <PiiPoint.h>

/**
 * @file
 *
 * Generic image handling operations.
 *
 * @ingroup PiiImagePlugin
 */

namespace PiiImage
{
  typedef PiiMatrix<PiiPoint<int> > IntCoordinateMap;
  typedef PiiMatrix<PiiPoint<double> > DoubleCoordinateMap;

  template <class T> struct GrayToGray
  {
    typedef T Type;
  };

  template <class Clr> struct ColorToGray
  {
    typedef typename Clr::Type Type;
  };

  template <class T> struct ToGray :
    Pii::IfClass<Pii::IsColor<T>, ColorToGray<T>, GrayToGray<T> >::Type
  {};

  template <class T> inline const PiiMatrix<T>& toGray(const PiiMatrix<T>& image)
  {
    return image;
  }

  template <class T> inline PiiMatrix<T> toGray(const PiiMatrix<PiiColor<T> >& image)
  {
    return PiiMatrix<T>(image);
  }

  template <class T> inline PiiMatrix<T> toGray(const PiiMatrix<PiiColor4<T> >& image)
  {
    return PiiMatrix<T>(image);
  }

  /**
   * Convert the gray values of an image to floating point. This
   * function divides 8-bit gray levels by 255.
   */
  template <class T> inline PiiMatrix<float> toFloat(const PiiMatrix<T>& image)
  {
    PiiMatrix<float> result(image);
    result /= 255.0f;
    return result;
  }

  /**
   * An explicit specialization of the toFloat() function. Returns the
   * argument without modifications.
   */
  inline PiiMatrix<float> toFloat(const PiiMatrix<float>& image)
  {
    return image;
  }

  /**
   * Convert the gray values of an image to 8-bit gray. The default
   * implementation just casts the type.
   */
  template <class T> inline PiiMatrix<unsigned char> to8Bit(const PiiMatrix<T>& image)
  {
    return PiiMatrix<unsigned char>(image);
  }

  /// @overload
  inline PiiMatrix<unsigned char> to8Bit(const PiiMatrix<unsigned char>& image)
  {
    return image;
  }

  /**
   * Convert the gray values of an image to 8-bit gray. This function
   * multiplies floating-point gray values by 255.
   */
  inline PiiMatrix<unsigned char> to8Bit(const PiiMatrix<float>& image)
  {
    return PiiMatrix<unsigned char>(image * 255.0);
  }
  
  /**
   * Extract a channel from a color image. This is a generic template
   * function that works with any color type.
   *
   * @code
   * PiiMatrix<PiiColor4<> > image(5,5);
   * PiiMatrix<unsigned char> ch1(PiiImage::colorChannel(image, 1)); // Green channel
   *
   * PiiMatrix<PiiColor<unsigned short> > image2(5,5);
   * PiiMatrix<unsigned short> ch2(PiiImage::colorChannel(image2, 2)); // Blue channel
   * @endcode
   *
   * @param image the input image
   *
   * @param channel the zero-based index of the color channel
   *
   * @return the channel as a matrix whose dimensions equal to the
   * input image.
   */
  template <class ColorType> PiiMatrix<typename ColorType::Type> colorChannel(const PiiMatrix<ColorType>& image,
                                                                              int channel);

  /**
   * Set a color channel. This is a generic template that works with
   * any color type. If the sizes of @p image and @p value do not
   * match, the function does nothing.
   *
   * @code
   * PiiMatrix<PiiColor4<> > image(5,5);
   * PiiMatrix<unsigned char> red(5,5);
   * red = 255;
   * PiiImage::setColorChannel(image, 0, red);
   * @endcode
   *
   * @param image the color image
   *
   * @param channel index of the color channel to modify
   *
   * @param values new values for the channel
   */
  template <class ColorType> void setColorChannel(PiiMatrix<ColorType>& image,
                                                  int channel,
                                                  const PiiMatrix<typename ColorType::Type>& values);

  /**
   * Set a color channel to a constant value. Works analogously to the
   * above function but uses the same value for each pixel.
   *
   * @code
   * PiiMatrix<PiiColor4<> > image(5,5);
   * PiiImage::setColorChannel(image, 0, unsigned char(255));
   * @endcode
   */
  template <class ColorType> void setColorChannel(PiiMatrix<ColorType>& image,
                                                  int channel,
                                                  typename ColorType::Type value);

  /**
   * Split a color image into channels. This function is equivalent to
   * but faster than calling colorChannel() @a channels times in
   * sequence.
   *
   * @param image the color image to be split into channels
   *
   * @param channelImages an array of at least @a channels images. The
   * images will be automatically resized to the size of the input
   * image.
   *
   * @param channels the number of color channels to extract. Either
   * three or four.
   */
  template <class ColorType> void separateChannels(const PiiMatrix<ColorType>& image,
                                                   PiiMatrix<typename ColorType::Type>* channelImages,
                                                   int channels = 3);
  
  /**
   * Predefined filter masks for the x and y components of the Sobel
   * edge finder.
   */
  PII_IMAGE_EXPORT extern PiiMatrix<int> sobelX, sobelY;
  /**
   * Predefined filter masks for the x and y components of the Roberts
   * edge finder.
   */
  PII_IMAGE_EXPORT extern PiiMatrix<int> robertsX, robertsY;
  /**
   * Predefined filter masks for the x and y components of the Prewitt
   * edge finder.
   */
  PII_IMAGE_EXPORT extern PiiMatrix<int> prewittX, prewittY;
  
  /**
   * Create a size-by-size Gaussian low-pass filter. Size must be odd. 
   * The filter is cut at approximately @f$3\sigma@f$ and normalized
   * so that it sums up to unity.
   */
  PII_IMAGE_EXPORT PiiMatrix<double> makeGaussian(unsigned int size);

  /**
   * Create a size-by-size Laplacian-of-Gaussian filter. Size must be
   * odd. The filter is cut at approximately @f$3\sigma@f$ and
   * normalized so that it sums up to zero.
   */
  PII_IMAGE_EXPORT PiiMatrix<double> makeLoGaussian(unsigned int size);

  /**
   * Prebuilt filter types.
   *
   * @lip SobelXFilter - Sobel's edge detection filter in x direction
   *
   * @lip SobelYFilter - Sobel's edge detection filter in y direction
   *
   * @lip PrewittXFilter - Prewitt's edge detection filter in x
   * direction
   *
   * @lip PrewittYFilter - Prewitt's edge detection filter in y
   * direction
   *
   * @lip RobertsXFilter - Robert's edge detection filter in x
   * direction
   *
   * @lip RobertsYFilter - Robert's edge detection filter in y
   * direction
   *
   * @lip UniformFilter - a moving average filter. Every element in
   * the filter matrix is equal.
   *
   * @lip GaussianFilter - Gaussian low-pass filter.
   *
   * @lip LoGFilter - Laplacian-of-Gaussian edge detection filter
   */
  enum PrebuiltFilterType
    {
      SobelXFilter,
      SobelYFilter,
      PrewittXFilter,
      PrewittYFilter,
      RobertsXFilter,
      RobertsYFilter,
      UniformFilter,
      GaussianFilter,
      LoGFilter
    };
  
  /**
   * Create an image filter. This function can be used to create
   * well-known filters for many image processing tasks.
   *
   * @param type the type of the filter
   *
   * @param size the size of the filter. Some filters (Sobel, Roberts,
   * Prewitt) are fixed in size, and this value will be ignored.
   *
   * @return a square-shaped image filter matrix
   *
   * @note @p UniformFilter, @p GaussianFilter, and @p LoGFilter
   * cannot be represented as integers. Use @p float or @p double as
   * the data type.
   *
   * @code
   * // Create a 5-by-5 gaussian filter
   * PiiMatrix<double> filter = PiiImage::makeFilter<double>(PiiImage::GaussianFilter, 5);
   * @endcode
   */
  template <class T> PiiMatrix<T> makeFilter(PrebuiltFilterType type, unsigned int size=3);

  /**
   * Decompose a two-dimensional filter into two one-dimensional
   * filters. This function decomposes a matrix A into a row vector H
   * and column vector V so that A = VH.
   *
   * Decomposition is useful in speeding up convolution because
   * one-dimensional convolution is much faster than two-dimensional,
   * but not all filters are separable. Prominent examples of
   * separable filters are the two-dimensional Gaussian function (the
   * only separable, circularly symmetric filter), the Sobel filters,
   * and the moving average filter. These can be created with the
   * #makeFilter() function using @p GaussianFilter, @p SobelXFilter,
   * @p SobelYFilter, or @p UniformFilter as the filter type,
   * respectively.
   *
   * @param filter a two-dimensional filter mask
   *
   * @param horizontalFilter the horizontal component of the
   * decomposition, a row vector. Output-value parameter.
   *
   * @param verticalFilter the vertical component of the
   * decomposition, a column vector. Output-value parameter.
   *
   * @return @p true if the decomposition was successful, @p false
   * otherwise.
   *
   * @code
   * PiiMatrix<int> filter = PiiImage::makeFilter<int>(PiiImage::SobelXFilter);
   * PiiMatrix<int> h, v;
   * PiiImage::separateFilter(filter, h, v);
   * QVERIFY(filter.equals(v*h));
   * @endcode
   *
   * @note A filter with floating-point coefficients may not be
   * separable if it is not stored accurately enough. This applies
   * especially to the Gaussian filter.
   *
   * @code
   * PiiMatrix<float> h, v;
   * // Returns false, must use double as the data type
   * PiiImage::separateFilter(PiiImage::makeFilter<float>(PiiImage::GaussianFilter), h, v);
   * @endcode
   */
  template <class T> bool separateFilter(const PiiMatrix<T>& filter,
                                         PiiMatrix<T>& horizontalFilter,
                                         PiiMatrix<T>& verticalFilter);
  
  /**
   * Calculate gradient angle from gradient magnitude components.
   *
   * @param gradX horizontal gradient magnitude
   *
   * @param gradY vertical gradient magnitude
   *
   * @return gradient angle for each pixel (@f$[-\pi,\pi]@f$), or an
   * empty matrix if @p gradX and @p gradY are of different size.
   *
   * @code
   * using namespace PiiImage;
   * PiiMatrix<float> directions = gradientDirection(filter(image, SobelXFilter),
   *                                                 filter(image, SobelYFilter));
   * @endcode
   */
  template <class T> PiiMatrix<float> gradientDirection(const PiiMatrix<T>& gradX, const PiiMatrix<T>& gradY)
  {
    return gradY.mapped(Pii::FastAtan2<T>(), gradX);
  }

  /**
   * Calculate total gradient magnitude from gradient magnitude
   * components.
   *
   * @param gradX horizontal gradient magnitude
   *
   * @param gradY vertical gradient magnitude
   *
   * @param fast a flag that determines whether a fast approximation
   * is used (default is true). The real magnitude is
   * @f$\sqrt{x^2+y^2}@f$ (@p fast == @p false). In @p fast mode, the
   * magnitude is approximated with @f$|x|+|y|@f$.
   *
   * @return gradient magnitude for each pixel, or an
   * empty matrix if @p gradX and @p gradY are of different size.
   *
   * @code
   * using namespace PiiImage;
   * PiiMatrix<int> image; // construct somewhere
   * // Use built-in filter masks directly
   * PiiMatrix<int> magnitude = gradientMagnitude(filter<int>(image, sobelX), filter<int>(image, sobelY));
   * @endcode
   */
  template <class T> PiiMatrix<T> gradientMagnitude(const PiiMatrix<T>& gradX, const PiiMatrix<T>& gradY, bool fast = true)
  {
    if (fast)
      return gradX.mapped(Pii::AbsSum<T>(), gradY);
    return gradX.mapped(Pii::Hypotenuse<T>(), gradY);
  }

  /**
   * A unary function that converts radians to points of the compass. 
   * Angles in @f$[-\pi/8, \pi/8)@f$ turned to 0, angles in @f$[\pi/8,
   * \pi/4)@f$pi to 1 and so on.
   */
  template <class T> struct RadiansToPoints : public Pii::UnaryFunction<T,int>
  {
    // Quantize angle to 0-7. Binary AND adds 8 to negative numbers.
    int operator()(T radians) const { return int(Pii::round(4.0/M_PI*radians - 0.25)) & 7; }
  };

  /**
   * Thin edge detection result. This function checks each pixel in @p
   * magnitude to see if it is a local maximum. It searches up to @p
   * radius pixels to the direction of the gradient. Only the maximum
   * pixels are retained in the result image; others are set to zero.
   *
   * @param magnitude gradient magnitude image. (see
   * gradientMagnitude())
   *
   * @param direction gradient direction image (see
   * gradientDirection()).
   *
   * @param quantizer a unary function that converts the angles stored
   * in @p direction into points of the compass (0-7, 0 = east, 1 =
   * north-east, ..., 7 = south-east).
   *
   * @code
   * // Typical case: thin edges using a gradient direction image (in radians)
   * PiiMatrix<int> gradientX, gradientY, edges;
   * edges =
   * PiiImage::suppressNonMaxima(PiiImage::gradientMagnitude(gradientX, gradienY),
   *                             PiiImage::gradientDirection(gradientX, gradientY),
   *                             PiiImage::RadiansToPoints<float>());
   *
   * // If you have already quantized the gradient direction image into 8 levels:
   * PiiMatrix<int> gradientDirection;
   * edges =
   * PiiImage::suppressNonMaxima(PiiImage::gradientMagnitude(gradientX, gradienY),
   *                             gradientDirection,
   *                             Pii::Identity<int>());
   * @endcode
   *
   * @relates RadiansToPoints
   */
  template <class T, class U, class Quantizer> PiiMatrix<T> suppressNonMaxima(const PiiMatrix<T>& magnitude,
                                                                              const PiiMatrix<U>& direction,
                                                                              Quantizer quantizer);

  /**
   * Detect edges in a gray-level image. This function implements the
   * Canny edge detector.
   *
   * @param image a gray-level image in which edges are to be found
   *
   * @param smoothWidth the width of a Gaussian smoothing window
   *
   * @param lowThreshold the low threshold value for hysteresis
   * thresholding. If zero, 0.4 * highThreshold will be used.
   *
   * @param highThreshold the high threshold value for hysteresis
   * thresholding. If zero, mean+2*std of the gradient magnitude will
   * be used.
   *
   * @return a binary image in which detected edges are ones and other
   * pixels zeros.
   */
  template <class T> PiiMatrix<int> detectEdges(const PiiMatrix<T>& image,
                                                int smoothWidth = 0,
                                                T lowThreshold = 0, T highThreshold = 0);
  
  /**
   * Filter an image with the given filter. This is equivalent to
   * PiiDsp::filter(), except for the @p mode parameter.
   *
   * @param image the image to be filtered
   *
   * @param filter the filter
   *
   * @param mode "extension" mode, i.e. the way of handling border
   * effects.
   *
   * @code
   * // Gaussian low-pass filtering assuming zeros outside of the image.
   * PiiMatrix<float> filtered = PiiImage::filter<float>(image,
   *                                                     PiiImage::makeFilter<float>(PiiImage::GaussianFilter),
   *                                                     Pii::ExtendZeros);
   * @endcode
   *
   * @note It is not a good idea to use @p unsigned @p char as the
   * result type. If the filters are @p double, use @p double as the
   * output type.
   */
  template <class ResultType, class T, class U>
  PiiMatrix<ResultType> filter(const PiiMatrix<T>& image,
                               const PiiMatrix<U>& filter,
                               Pii::ExtendMode mode = Pii::ExtendReplicate)
  {
    if (mode == Pii::ExtendZeros)
      return PiiDsp::filter<ResultType>(image, filter, PiiDsp::FilterOriginalSize);
    const int rows = filter.rows() >> 1, cols = filter.columns() >> 1;
    return PiiDsp::filter<ResultType>(Pii::extend(image, rows, rows, cols, cols, mode), filter, PiiDsp::FilterValidPart);
  }

  /**
   * Filter an image with two one-dimensional filters. If a
   * two-dimensional filter can be decomposed into two one-dimensional
   * ones, the filtering operation is much faster. Use
   * #separateFilter() to find the decomposition.
   *
   * @param image the image to be filtered
   *
   * @param horizontalFilter a row matrix representing the horizontal
   * component of a decomposed filter
   *
   * @param verticalFilter a column matrix representing the vertical
   * component of a decomposed filter
   *
   * @param mode "extension" mode, i.e. the way of handling border
   * effects.
   *
   * @return filtered image. If @p horizontalFilter is not a row
   * vector or @p verticalFilter is not a column vector, a clone of
   * the input image will be returned.
   *
   * @code
   * // Convolution with a 23-by-23 filter is (informally) O(N * 23^2)
   * // Convolution with two 23-by-1 filters is O(N * 23*2)
   * // In theory, the processing time can go down to one 11th
   * PiiMatrix<int> filter = PiiImage::makeFilter(PiiImage::UniformFilter, 23);
   * PiiMatrix<int> hFilter, vFilter;
   * PiiImage::separateFilter(filter, hFilter, vFilter);
   * PiiMatrix<int> filtered = PiiImage::filter<int>(image, hFilter, vFilter);
   * @endcode
   *
   * @note It is not a good idea to use @p unsigned @p char as the
   * result type. If the filters are @p double, use @p double as the
   * output type.
   */
  template <class ResultType, class T, class U>
  PiiMatrix<ResultType> filter(const PiiMatrix<T>& image,
                               const PiiMatrix<U>& horizontalFilter,
                               const PiiMatrix<U>& verticalFilter,
                               Pii::ExtendMode mode = Pii::ExtendReplicate);

  /**
   * Same as above, but filters the image with a named filter. See
   * #makeFilter() for information about filter names. This function
   * determines suitable data types for the filtering operation based
   * on the filter type and decomposes the filter if possible.
   *
   * @param image input image
   *
   * @param type filter type
   *
   * @param mode how to handle image borders
   *
   * @param filterSize the size of the filter, ignored by some filter
   * types
   *
   * @see makeFilter()
   *
   * @code
   * // First template parameter is the type of the result
   * PiiMatrix<int> smoothed = PiiImage::filter<int>(image,
   *                                                 PiiImage::GaussianFilter,
   *                                                 Pii::ExtendSymmetric);
   * @endcode
   */
  template <class ResultType, class ImageType>
  PiiMatrix<ResultType> filter(const PiiMatrix<ImageType>& image,
                               PrebuiltFilterType type,
                               Pii::ExtendMode mode = Pii::ExtendReplicate,
                               int filterSize = 3);

  /**
   * Filters an integer image by a double-valued filter. The filter is
   * first scaled and rounded to integers. The image is then filtered
   * with the integer-valued filter, and the result is rescaled. This
   * is more efficient (and less accurate) than performing the
   * convolution with doubles.
   *
   * @param image input image
   *
   * @param doubleFilter the filter
   *
   * @param mode how to deal with borders
   *
   * @param scale a scaling factor for the filter. If @p scale is
   * zero, the function uses 256/max(abs(filter)).
   *
   * @code
   * PiiMatrix<int> filtered = PiiImage::intFilter(image, PiiImage::makeGaussian(5));
   * @endcode
   *
   * @note Beware of overflows! If the input image contains large
   * entries and @p scale is not set carefully, the operation may
   * easily overflow @p int.
   */
  PII_IMAGE_EXPORT PiiMatrix<int> intFilter(const PiiMatrix<int>& image,
                                            const PiiMatrix<double>& doubleFilter,
                                            Pii::ExtendMode mode = Pii::ExtendReplicate,
                                            double scale = 0);

  /**
   * Filters an integer image by a double-valued separable filter. See
   * #intFilter() and #filter() for details.
   *
   * @param image input image
   *
   * @param horizontalFilter horizontal component of a decomposed
   * filter
   * 
   * @param verticalFilter vertical component of a decomposed filter
   *
   * @param mode how to deal with borders
   *
   * @param scale a scaling factor for the filters. If @p scale is
   * zero, the function uses 64/max(abs(filter)) for both.
   *
   * @note Beware of overflows! If the input image contains large
   * entries and @p scale is not set carefully, the operation may
   * easily overflow @p int.
   */
  PII_IMAGE_EXPORT PiiMatrix<int> intFilter(const PiiMatrix<int>& image,
                                            const PiiMatrix<double>& horizontalFilter,
                                            const PiiMatrix<double>& verticalFilter,
                                            Pii::ExtendMode mode = Pii::ExtendReplicate,
                                            double scale = 0);

  /**
   * Filters an image with a median filter.
   *
   * @param image the input image
   *
   * @param filterRows filter size in vertical direction
   *
   * @param filterColumns filter size in horizontal direction. If this
   * value is less than one, @p filterRows will be used instead.
   *
   * @param mode the method of handling image borders
   */
  template <class T> PiiMatrix<T> medianFilter(const PiiMatrix<T>& image,
                                               int filterRows = 3, int filterColumns = 0,
                                               Pii::ExtendMode mode = Pii::ExtendZeros);


  /**
   * Scales image to a specified size.
   *
   * @param image input image
   *
   * @param rows the number of pixel rows in the result image
   *
   * @param columns the number of pixel columns in the result image
   *
   * @param interpolation interpolation mode
   */
  template <class T> PiiMatrix<T> scale(const PiiMatrix<T>& image, int rows, int columns,
                                        Pii::Interpolation interpolation = Pii::LinearInterpolation);

  /**
   * Scales image according to a scale ratio.
   *
   * @param image input image
   *
   * @param scaleRatio scale ratio, which must be larger than zero.
   *
   * @param interpolation interpolation mode
   */
  template <class T> PiiMatrix<T> scale(const PiiMatrix<T>& image, double scaleRatio, Pii::Interpolation interpolation = Pii::LinearInterpolation)
  {
    return scale(image, int(scaleRatio*image.rows()), int(scaleRatio*image.columns()), interpolation);
  }

  /**
   * Transforms a 2D point using @a transform. The source point is
   * represented in homogeneous coordinates; it is assumed that the
   * third coordinate is one.
   *
   * @param transform a 3-by-3 transformation matrix
   *
   * @param sourceX the x coordinate of the source point
   *
   * @param sourceY the y coordinate of the source point
   *
   * @param transformedX an output value argument that will store the
   * transformed x coordinate.
   *
   * @param transformedY an output value argument that will store the
   * transformed y coordinate.
   */
  template <class T> void transformHomogeneousPoint(const PiiMatrix<T>& transform,
                                                    T sourceX, T sourceY,
                                                    T* transformedX, T* transformedY);

  /**
   * Transforms 2D point coordinates using @a transform. This function
   * applies transformHomogeneousPoint() to all rows in @a points and
   * returns the transformed points in a new matrix. @a points must
   * have two columns.
   */
  template <class T, class U> PiiMatrix<U> transformHomogeneousPoints(const PiiMatrix<T>& transform,
                                                                      const PiiMatrix<U>& points);

  /**
   * Creates a transform that rotates a coordinate system @p theta
   * radians around its origin.
   *
   * @note In the image coordinate system, angles grow clockwise.
   *
   * @see transform()
   */
  PII_IMAGE_EXPORT PiiMatrix<float> createRotationTransform(float theta);
  
  /**
   * Creates a transform that rotates a coordinate system @p theta
   * radians around the specified center point.
   *
   * @code
   * // Rotate 45 degrees around image center
   * PiiMatrix<int> img(100,100);
   * PiiMatrix<float> matRotation = createRotationTransform(M_PI/4,
   *                                                        img.columns()/2.0,
   *                                                        img.rows()/2.0);
   * img = transform(img, matRotation);
   * @endcode
   *
   * @note In the image coordinate system, angles grow clockwise.
   *
   * @see transform()
   */
  PII_IMAGE_EXPORT PiiMatrix<float> createRotationTransform(float theta, float centerX, float centerY);

  /**
   * Creates a transform that shears a coordinate system by @p
   * shearX horizontally and by @p shearY vertically.
   *
   * @see transform()
   */
  PII_IMAGE_EXPORT PiiMatrix<float> createShearingTransform(float shearX, float shearY);

  /**
   * Creates a transform that translates a coordinate system by @p x
   * horizontally and by @p y vertically.
   *
   * @see transform()
   */
  PII_IMAGE_EXPORT PiiMatrix<float> createTranslationTransform(float x, float y);

  /**
   * Creates a transform that scales a coordinate system by @p scaleX
   * horizontally and by @p scaleY vertically. If you just want to
   * scale an image, it is a good idea to use the @ref scale()
   * function instead. It is faster than @ref transform() and produces
   * better results.
   *
   * @see transform()
   * @see scale()
   */
  PII_IMAGE_EXPORT PiiMatrix<float> createScalingTransform(float scaleX, float scaleY);

  /**
   * Applies an arbitrary geometric transform to @p image.
   *
   * @param image the image to be transformed
   *
   * @param transform a 3-by-3 matrix specifying a geometric transform
   * of original image coordinates to a new domain.
   *
   * @param handling clipping style.
   *
   * @param backgroundColor the color used for the parts of the output
   * image that are outside of the input image.
   *
   * @return the transformed image
   *
   * @exception PiiMathException& if the transform matrix is invalid
   * (singular) and cannot be inverted.
   *
   * Homogeneous coordinates are used in calculations to allow
   * simultaneous translations. Image coordinates are represented as a
   * column vector @f$p = [x\ y\ 1]^T@f$. The transformed coordinates
   * are obtained by @f$p' = Ap@f$, where @e A is the transformation
   * matrix. For example, a transformation that rotates the image
   * @f$\theta@f$ radians, is specified as follows:
   *
   * @f[
   * \left( \begin{array}{c} x' \\ y' \\ 1 \end{array} \right) =
   * \left( \begin{array}{ccc}
   * \cos \theta & -\sin\theta & 0 \\
   * \sin \theta & \cos \theta & 0 \\
   * 0 & 0 & 1
   * \end{array} \right)
   * \left( \begin{array}{c} x \\ y \\ 1 \end{array} \right)
   * @f]
   *
   * Transformation matrices are most easily created by @ref
   * createRotationTransform(), @ref createScalingTransform(), @ref
   * createTranslationTransform, and @ref createShearingTransform(). 
   * Transformations can be chained by simply multiplying the
   * transformation matrices. Assume @e R is a rotation transform and
   * @e S is a shear transform. Shear after rotate transform is
   * obtained with @f$T = SR@f$.
   */
  template <class T> PiiMatrix<T> transform(const PiiMatrix<T>& image,
                                            const PiiMatrix<float>& transform,
                                            TransformedSize handling = ExpandAsNecessary,
                                            T backgroundColor = T(0));
  
  /**
   * Rotates image @p theta radians around its center.
   *
   * @param image the image to be rotated
   *
   * @param theta rotation angle in radians. The angle grows
   * clockwise, zero pointing to the right.
   *
   * @param handling clipping style.
   *
   * @param backgroundColor the color used for the parts of the output
   * image that are outside of the input image.
   *
   * @return rotated image
   */
  template <class T> PiiMatrix<T> rotate(const PiiMatrix<T>& image,
                                         double theta,
                                         TransformedSize handling = ExpandAsNecessary,
                                         T backgroundColor = T(0));

  /**
   * Crop a rectangular area out of a transformed image, in which the
   * target may not appear as a rectangular object. This function
   * makes it possible to cut parts rectangular pars of an image even
   * if the image was transformed with an affine transform.
   *
   * @param image the source image
   *
   * @param x the x coordinate of the original upper left corner of
   * the rectangle
   *
   * @param y the y coordinate of the original upper left corner of
   * the rectangle
   *
   * @param width the width of the rectangle to cut out of the image
   *
   * @param height the height of the rectangle to cut out of the image
   *
   * @param transform a 3-by-3 transformation matrix A that relates
   * the coordinates of the result r to the image i: Ar = i. The
   * function uses homogeneous coordinates.
   *
   * @code
   * PiiMatrix<int> image(5, 5,
   *                      0, 0, 1, 0, 0,
   *                      0, 0, 2, 0, 0,
   *                      0, 0, 3, 0, 0,
   *                      0, 0, 4, 0, 0,
   *                      0, 0, 5, 0, 0);
   *
   * // This transformation rotates 90 degrees clockwise around
   * // origin (in image coordinates, y down) and translates
   * // 3 steps to the right and one step down.
   * PiiMatrix<float> matTransform(3, 3,
   *                               0.0, -1.0, 3.0,
   *                               1.0, 0.0, 1.0,
   *                               0.0, 0.0, 1.0);
   *
   * // The original area is a 3-by-3 square the upper left corner.
   * PiiMatrix<int> matCropped = PiiImage::crop(image,
   *                                            0, 0, 3, 3,
   *                                            matTransform);
   *
   * // matCropped = 0 0 0
   * //              2 3 4
   * //              0 0 0
   * @endcode
   */
  template <class T> PiiMatrix<T> crop(const PiiMatrix<T>& image,
                                       int x, int y,
                                       int width, int height,
                                       const PiiMatrix<float>& transform);
  /**
   * Detects corners in @a image using the FAST corner detector.
   *
   * @param image the input image
   *
   * @param threshold detection threshold. This value affects both the
   * number of detections and the detection speed. A high value
   * accepts only strong corners. The lower the value, the more
   * corners are detected, which also means lower processing speed.
   *
   * @return a N-by-2 matrix in which each row stores the (x,y)
   * coordinates of a detected corner.
   */
  template <class T> PiiMatrix<int> detectFastCorners(const PiiMatrix<T>& image, T threshold=25);

  /**
   * Transforms @a image according to the given coordinate @a map. The
   * size of the resulting image will be equal to the size of the @a
   * map. Each pixel in the result image will be sampled from @a image
   * according to map. For example, if map(0,0) is (1,2), the pixel at
   * (0,0) in the result image will be taken from image(1,2). If the
   * mapping would result in accessing @a image outside of its
   * boundaries, the corresponding pixel in the result image will be
   * left black. If the map coordinates are given as @p doubles, this
   * function samples @a image using bilinear interpolation.
   */
  template <class T, class U> PiiMatrix<T> remap(const PiiMatrix<T>& image, const PiiMatrix<PiiPoint<U> >& map);

  template <class T, class Matrix, class UnaryFunction>
  PiiMatrix<T> collectCoordinates(const PiiRandomAccessMatrix& image,
                                  UnaryFunction decisionRule)
  {
    const int iRows = image.rows();
    const int iCols = image.columns();
    
    // Collect matching points
    PiiMatrix<T> matXY(0, 2);
    matXY.reserve(128);
    
    for (typename Matrix::const_iterator i=image.begin();
         i != image.end(); ++i)
      if (decisionRule(*i))
        matXY.appendRow(i.column(), i.row());
    
    return matXY;
  }
}

#include "PiiImage-templates.h"

#endif //_PIIIMAGE_H
