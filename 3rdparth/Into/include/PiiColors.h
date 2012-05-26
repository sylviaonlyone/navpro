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

#ifndef _PIICOLORS_H
#define _PIICOLORS_H

#include <PiiColor.h>
#include <PiiMatrix.h>
#include <PiiMath.h>
#include <PiiFunctional.h>
#include <PiiImageTraits.h>

#include "PiiColorsGlobal.h"

/**
 * Functions for transforming colors.
 *
 * @note Unless stated otherwise, color transform functions assume
 * <em>linear RGB</em>. Almost all digitally stored image data are
 * however gamma-adjusted and therefore non-linear. For example, JPEG
 * and MPEG store colors in a non-linear form, and almost all consumer
 * digital cameras apply gamma adjustment to the colors. Therefore,
 * @ref correctGamma() "gamma correction" needs to be applied to the
 * input images. Otherwise, color transformation results will not be
 * theoretically correct. It is up to the user if this matters in
 * practice, but you have been warned. It can be assumed that digital
 * cameras typically apply a gamma of 1/2.2. To linearize the colors,
 * one needs to invert the gamma compression:
 *
 * @code
 * using namespace PiiColors;
 * PiiColor<> nonlinearRgbColor;
 * PiiColor<> linearRgbColor = correctGamma(nonlinearRgbColor, 2.2);
 * @endcode
 *
 * @ingroup PiiColorsPlugin
 */
namespace PiiColors
{
  /**
   * Convert an RGB color image to normalized RGB and separate two of
   * the normalized color layers. Normalized RGB is obtained by
   * dividing each channel by the sum of all channels. (@f$ r = R/(R +
   * G + B)@f$ etc.) Note that only two channels are extracted because
   * the third one is reduntant (@f$ r+g+b = 1@f$).
   *
   * @param image The input image. Any color will do as the content
   * type (PiiColor or PiiColor4).
   *
   * @param ch1 store the first extracted channel to this matrix
   *
   * @param ch2 store the second extracted channel to this matrix
   *
   * @param multiplier multiply each normalized channel by this value
   * before storing to the output matrix. This is useful as the result
   * often needs to be quantized.
   *
   * @param ch1Index the BGR index of the first color channel to
   * extract. 0 = r, 1 = g, 2 = b. With PiiColor4, channel index may,
   * in principle, be 3, but the fourth channel is never normalized.
   *
   * @param ch2Index same for the second channel
   *
   * @code
   * PiiMatrix<PiiColor<> > img(100,100);
   * PiiMatrix<unsigned char> red, green, blue;
   * // Extract normalized RG with 8 bit resolution
   * PiiColors::normalizedRgb(img, red, green);
   *
   * // Extract normalized RB with 5 bit resolution
   * PiiColors::normalizedRgb(img, red, blue, 32, 0, 2);
   * @endcode
   */
  template <class T> void normalizedRgb(const PiiMatrix<T>& image,
                                        PiiMatrix<typename T::Type>& ch1,
                                        PiiMatrix<typename T::Type>& ch2,
                                        float multiplier = 255,
                                        int ch1Index = 0, int ch2Index = 1);

  /** 
   * Measure the distribution of colors in an image. This function
   * uses PCA to find the main axes of a three-dimensional color
   * distribution. As a result, it gives the centroid of the
   * distribution and three orthogonal vectors forming a new base. The
   * first of these vectors points to the direction of the largest
   * variance in the color distribution. The length of each vector is
   * inversely proportional to the variance of the color distribution
   * in that direction.
   *
   * Information about the color distribution can be used to normalize
   * colors and to measure the likelihood that a color belongs to a
   * model distribution. Once you have measured the distribution of
   * colors you can project the colors to the new base and achieve an
   * approximately zero-mean and unit variance color distribution.
   * The second norm of any color coordinates in the normalized
   * coordinate system is inversely proportional to the likelihood of
   * the color belongin to the model distribution.
   *
   * @param baseVectors a 3-by-3 matrix return-value matrix in which
   * columns represent the three main axes of the color distribution. 
   * If the three vectors are used as a new base for the color
   * distribution, it'll have a unit variance in x, y, and z
   * directions in the new base.
   *
   * @param center the 1-by-3 return-value matrix that will store the
   * center of mass of the color distribution.
   *
   * @param maxPixels the maximum number of pixels to consider. If the
   * input image is large, a subset of its pixels is usually enough to
   * derive a reliable estimate of distribution. Use this paramater to
   * limit the number of pixels used for estimation. -1 means that
   * all pixels will be used.
   *
   * @see normalizeColorDistribution()
   * @see matchColors()
   *
   * @note This function is temporarily broken.
   */
  template <class ColorType> void measureColorDistribution(const PiiMatrix<ColorType>& clrImage,
                                                           PiiMatrix<double>& baseVectors,
                                                           PiiMatrix<double>& center,
                                                           int maxPixels = -1);

  /**
   * Project colors into a new base and translate to a new center. In
   * the result image, the color distribution will have approximately
   * zero mean and unit variance. Each color @e c in the input image
   * will be treated as a row vector. The normalized color is given by
   * @f$c_n = (c-t)A@f$, where @e t is the translation vector (@p
   * center) @e A is the matrix representing the new base (@p
   * baseVectors).
   *
   * @param clrImage the input image
   *
   * @param baseVectors a 3-by-3 matrix in which rows represent a new
   * base for the color system.
   *
   * @param center a 1-by-3 translation vector
   *
   * @return a new normalized color image
   *
   * @see measureColorDistribution()
   */
  template <class ColorType> PiiMatrix<PiiColor<float> > normalizeColorDistribution(const PiiMatrix<ColorType>& clrImage,
                                                                                    const PiiMatrix<double>& baseVectors,
                                                                                    const PiiMatrix<double>& center);

  /**
   * Match colors in an image to a precalculated model. This function
   * compares colors in an image to a color model represented by @p
   * baseVectors and @p center. The function tries to find the
   * likelihood of a color belonging to the model. A value of one is
   * given to a pixel whose color that is exactly at @p center. The
   * value decreases by distance according to the formula @f$f(d^)@f$,
   * where @e d is the geometric distance of the color to the center
   * of the distribution in normalized coordinates and @e f() is a
   * user-specified function.
   *
   * @param clrImage the input image
   *
   * @param baseVectors a 3-by-3 matrix in which rows represent a
   * normalized base for the color system.
   *
   * @param center a 1-by-3 translation vector
   *
   * @param func an adaptable unary function that converts the
   * distance @e d (a @p float) to the output value. See
   * LikelihoodFunction for an example.
   *
   * @return an image in which each value represents the "likelihood"
   * of that pixel belonging to the given model.
   *
   * @see measureColorDistribution()
   * @see normalizeColorDistribution()
   * @see PiiImage::backProject()
   *
   * @relates LikelihoodFunction
   */
  template <class ColorType, class UnaryFunction>
  PiiMatrix<typename UnaryFunction::result_type> matchColors(const PiiMatrix<ColorType>& clrImage,
                                                             const PiiMatrix<double>& baseVectors,
                                                             const PiiMatrix<double>& center,
                                                             UnaryFunction func);

  /**
   * Convert a color image into indexed colors. This function
   * quantizes each color channel to the specified number of levels.
   * The color index is calculated as @f$I = R*l_r*l_g + G*l_b + B@f$,
   * where R, G, and B represent the quantized color channel values,
   * and @f$l_X@f$ is the number of quantization levels for channel X.
   *
   * @param clrImage the input image. @p ColorType must be either
   * PiiColor or PiiColor4. If color channels are floats, the maximum
   * value is assumed to be 1.0. Otherwise, 255 will be used as max.
   *
   * @param redLevels the number of quantization levels for the red
   * color channel.
   *
   * @param greenLevels the number of quantization levels for the red
   * color channel. 0 means same as @p redLevels.
   *
   * @param blueLevels the number of quantization levels for the blue
   * color channel. 0 means same as @p redLevels.
   *
   * @note Despite the color channel names used here, the function
   * also works with color spaces other than RGB.
   */
  template <class ColorType> PiiMatrix<int> toIndexed(const PiiMatrix<ColorType>& clrImage,
                                                      int redLevels, int greenLevels = 0, int blueLevels = 0);
  
	/**
	 * Calculate the autocorrelogram of an indexed color image. 
	 * Autocorrelogram describes the spatial distribution of colors in
	 * image. Each entry (d,c) in the autocorrelogram represents the
	 * probability that a pair of pixels, whose distance is d, both have
	 * color c. In practice, the correlogram is built as a histogram and
	 * must be normalized to obtain propabilities. The technique is
	 * described in detail in <i>Jing Huang; Kumar, S.R.; Mitra, M.;
	 * Wei-Jing Zhu; Zabih, R.: "Image indexing using color
	 * correlograms", Computer Vision and Pattern Recognition, 1997. 
	 * Proceedings., 1997 IEEE Computer Society Conference on, 1997,
	 * Pages: 762 -768</i>. This implementation does not use the bogus
	 * "optimization" technique reported in the paper.
   *
   * @param image an indexed color image
   *
   * @param maxDistance measure correlation between colors separated
   * up to this many pixels. The autocorrelogram will be calculated
   * for distances {1, ..., @p maxDistance}.
   *
   * @param levels the number of indexed colors in @p image. A
   * non-positive number means auto-detect.
   *
   * @return a 1 by @p levels * @p maxDistance matrix representing the
   * autocorrelogram (folded into a row matrix). Note that if the
   * number of quantization levels is auto-detected, the size of the
   * output may change in successive calls.
	 */
  PII_COLORS_EXPORT PiiMatrix<float> autocorrelogram(const PiiMatrix<int>& image,
                                                     int maxDistance = 5,
                                                     int levels = 0);

  /**
   * Calculate the autocorrelogram of an indexed olor image. This
   * version of the function makes it possible to manually specify the
   * distances for which the autocorrelogram will be calculated.
   *
   * @param image an indexed color image
   *
   * @param distances a list of distances for which the
   * autocorrelogram will be calculated.
   *
   * @param levels the number of indexed colors in @p image. A
   * non-positive number means auto-detect.
   *
   * @return a 1 by @p levels * @p maxDistance matrix representing the
   * autocorrelogram (folded into a row matrix). Note that if the
   * number of quantization levels is auto-detected, the size of the
   * output may change in successive calls.
   */
  PII_COLORS_EXPORT PiiMatrix<float> autocorrelogram(const PiiMatrix<int>& image,
                                                     const QList<int>& distances,
                                                     int levels = 0);
  
  /**
   * Apply gamma correction to a color channel. Gamma correction is
   * defined as @f$v_o = v_i^\gamma@f$, where @p o and @p i stand for
   * output and input. Contemporary display devices are usually
   * assumed to have a gamma of about 2.2. (This is in fact a rough
   * approximation: LG flat panels, for example, measure closer to
   * 1.97.) Linear colors must thus be compressed with a gamma of
   * @f$1/2.2 \approx 0.45@f$ to get linear @e intensity response on
   * screen. Linear intensity response does not however mean linear @p
   * lightness (perceptual luminance) response; the human vision is
   * more sensitive to lower luminance.
   *
   * @note This function can be used only with floating-point color
   * channels.
   *
   * @param value the value of a color channel, in [0, 1]. Usually,
   * only positive values are used.
   *
   * @param gamma the value of gamma, must be positive
   *
   * @return gamma-adjusted value of the color channel
   */
  template <class T> inline T correctGamma(T value, double gamma,
                                           typename Pii::OnlyFloatingPoint<T>::Type dummy = 0)
  {
    Q_UNUSED(dummy);
    return Pii::pow(value, T(gamma));
  }
  
  /**
   * Apply gamma correction to a color channel. This version of the
   * function compresses the color channel to [0, 1] before applying
   * the correction and then expands it back to original scale.
   *
   * @param value the value of a color channel, in [0, maximum].
   *
   * @param gamma the value of gamma, must be positive
   *
   * @param maximum the maximum value of a color channel
   *
   * @return gamma-adjusted value of the color channel, rescaled to
   * original range. If @p T is an integer type, the result will be
   * rounded to closest integer.
   */
  template <class T> inline T correctGamma(T value, double gamma, double maximum)
  {
    typename Pii::IfClass<Pii::IsInteger<T>, Pii::Round<double,T>, Pii::Cast<double,T> >::Type rounder;
    return rounder(maximum * Pii::pow(double(value)/maximum, gamma));
  }

  /**
   * Apply gamma correction to all color channels of @p clr. Color
   * channels are assumed to be in [0, 1].
   */
  template <class T> inline PiiColor<T> correctGamma(const PiiColor<T>& clr, double gamma)
  {
    return PiiColor<T>(correctGamma(clr.c0, gamma),
                       correctGamma(clr.c1, gamma),
                       correctGamma(clr.c2, gamma));
  }

  /**
   * Apply gamma correction to the first three color channels of @p
   * clr. Note that the fourth color channel (e.g. alpha) will not be
   * touched. Color channels are assumed to be in [0, 1].
   */
  template <class T> inline PiiColor4<T> correctGamma(const PiiColor4<T>& clr, double gamma)
  {
    return PiiColor4<T>(correctGamma(clr.c0, gamma),
                        correctGamma(clr.c1, gamma),
                        correctGamma(clr.c2, gamma));
  }

  /**
   * Apply gamma correction to all channels of @p clr. Color
   * channels are assumed to be in [0, @p maximum].
   */
  template <class T> inline PiiColor<T> correctGamma(const PiiColor<T>& clr, double gamma, double maximum)
  {
    return PiiColor<T>(correctGamma(clr.c0, gamma, maximum),
                       correctGamma(clr.c1, gamma, maximum),
                       correctGamma(clr.c2, gamma, maximum));
  }

  /**
   * Apply gamma correction to the first three color channels of @p
   * clr. Note that the fourth color channel (e.g. alpha) will not be
   * touched. Color channels are assumed to be in [0, @p maximum].
   */
  template <class T> PiiColor4<T> correctGamma(const PiiColor4<T>& clr, double gamma, double maximum)
  {
    return PiiColor4<T>(correctGamma(clr.c0, gamma, maximum),
                        correctGamma(clr.c1, gamma, maximum),
                        correctGamma(clr.c2, gamma, maximum));
  }

  /**
   * An adaptable binary function that performs gamma correction.
   */
  template <class T> class CorrectGamma : Pii::BinaryFunction<T>
  {
  public:
    CorrectGamma(double gamma) : _gamma(gamma) {}
    T operator() (const T& value) const { return T(correctGamma(value, _gamma)); }
  private:
    double _gamma;
  };

  /**
   * An adaptable binary function that performs gamma correction on
   * scaled image channels. The maximum value of an image channel is
   * given in constructor.
   */
  template <class T> class CorrectGammaScaled : public Pii::BinaryFunction<T>
  {
  public:
    CorrectGammaScaled(double gamma, double maximum=255) : _gamma(gamma), _max(maximum) {}
    T operator() (const T& value) const { return correctGamma(value, _gamma, _max); }
  private:
    double _gamma, _max;
  };

  /**
   * Apply gamma correction to all pixels in @p image. The function
   * works with both gray-level and color images. Color channels are
   * assumed to be in [0, 1].
   *
   * @see correctGamma(T, double)
   */
  template <class T> inline PiiMatrix<T> correctGamma(const PiiMatrix<T>& image, double gamma)
  {
    return image.mapped(CorrectGamma<T>(gamma));
  }
  
  /**
   * Apply gamma correction to all pixels in @p image. The function
   * works with both gray-level and color images. Color channels are
   * assumed to be in [0, @p maximum].
   *
   * @see correctGamma(T, double, double)
   */
  template <class T> inline PiiMatrix<T> correctGamma(const PiiMatrix<T>& image, double gamma, double maximum)
  {
    return image.mapped(CorrectGammaScaled<T>(gamma, maximum));
  }

  /**
   * A unary function for converting color distances to "likelihoods". 
   * The function is defined as @f$f(x) = e^{-x}@f$. As a result, zero
   * distance to the center of a color model maps to one, and infinity
   * distance to zero.
   */
  struct LikelihoodFunction : Pii::UnaryFunction<float>
  {
    float operator() (float value) const { return Pii::exp(-value); }
  };

  /**
   * Convert a color in an RGB color space into the HSV color space. 
   * HSV (hue, saturation, value) is a color space in which a
   * piecewise linear function is used as an approximation to the
   * (non-linear) hue channel value. See @ref QColor for a detailed
   * description of the HSV space.
   *
   * This implementation deviates from the conventional way of scaling
   * color channels. Typically, hue is represented with values in
   * [0,359] as it is an angular variable. Furthermore, the value
   * channel is often scaled into [0,100]. This convention has a
   * couple of problems:
   *
   * @li @p unsigned @p char can only store values up to 255.
   *
   * @li Scaling @e value down from 255 to 100 degrades accuracy.
   *
   * @li With floating-point types, angles are conventionally
   * represented as radians, not degrees.
   *
   * For these reasons, this function scales the color channels
   * depending on the input type as follows:
   *
   * @li @p unsigned @p char - H, S, and V are all in [0,255]
   *
   * @li Other integer types - H is [0,359], S and V in [0,255]
   *
   * @li Floating-point types - H is in [0,2*pi), S and V in [0,1].
   *
   * @note Due to numerical inaccuracies, converting from RGB to HSV
   * and back doesn't necessarily result into the color you started
   * with. The back-and-forth conversion of floating-point types is
   * accurate to at least 1e-6. The conversion error with @p unsigned
   * @p char is at most three levels on any color channel. The
   * conversion error with other integer types it at most two levels.
   *
   * @param rgbColor a color in an RGB color space. Floating-point
   * color channels are assumed to be in [0,1]. Integer types are
   * assumed to be in [0,255].
   *
   * @return the color in an HSV color space. The type of the
   * returned color matches that of the input color.
   *
   * @code
   * PiiColor<> rgb(1,2,3);
   * PiiColor<> hsv = PiiColors::rgbToHsv(rgb);
   * @endcode
   */
  template <class Clr> Clr rgbToHsv(const Clr& rgbColor);

  /**
   * Convert a color in an HSV color space into an RGB color space. 
   * This function works inversely to #rgbToHsv() and assumes the same
   * conventions on scaling color channels. The output color channels
   * will be in [0,1] with floating-point types and in [0,255] with
   * integer types.
   *
   * @param hsvColor a color in an HSV color space.
   *
   * @return the color in an RGB color space. The type of the
   * returned color matches that of the input color.
   */
	template <class Clr> Clr hsvToRgb(const Clr& hsvColor);

  /**
   * An adaptable unary function that converts an RGB color to an HSV
   * color.
   */
  template <class Clr> struct RgbToHsv : Pii::UnaryFunction<Clr>
  {
    Clr operator() (const Clr& clr) const { return rgbToHsv(clr); }
  };

  /**
   * An adaptable unary function that converts an HSV color to an RGB
   * color.
   */
  template <class Clr> struct HsvToRgb : Pii::UnaryFunction<Clr>
  {
    Clr operator() (const Clr& clr) { return hsvToRgb(clr); }
  };

  /**
   * Convert an RGB color image into an HSV color image.
   *
   * @see rgbToHsv(Clr)
   */
  template <class Clr> inline PiiMatrix<Clr> rgbToHsv(const PiiMatrix<Clr>& rgbColorImage)
  {
    return rgbColorImage.mapped(RgbToHsv<Clr>());
  }

  /**
   * Convert an HSV color image into an RGB color image.
   *
   * @see hsvToRgb(Clr)
   */
  template <class Clr> inline PiiMatrix<Clr> hsvToRgb(const PiiMatrix<Clr>& hsvColorImage)
  {
    return hsvColorImage.mapped(HsvToRgb<Clr>());
  }

  /**
   * Reverse the order of color channels. This function can be used to
   * convert RGB to BGR or RGBA to BGRA and vice versa. It just swaps
   * the first and third color channels.
   */
  template <class Clr> inline Clr reverseColor(const Clr& color)
  {
    return Clr(color.c2, color.c1, color.c0);
  }

  /**
   * An adaptable unary function that inverses the order of color
   * channels.
   */
  template <class Clr> struct ReverseColors : Pii::UnaryFunction<Clr>
  {
    Clr operator() (const Clr& clr) const { return reverseColor(clr); }
  };

  /**
   * Reverse the order of color channels in a color image.
   */
  template <class Clr> inline PiiMatrix<Clr> reverseColors(const PiiMatrix<Clr>& clrImage)
  {
    return clrImage.mapped(ReverseColors<Clr>());
  }

  /**
   * Convert a color in the CIE XYZ color space into the CIE 1976
   * L*a*b* color space.
   *
   * @param xyzColor the XYZ color to be converted to L*a*b*
   *
   * @param whitePoint the XYZ coordinates of the white point of the
   * imaging equipment. Make sure the white point has no zero color
   * channels.
   *
   * @note The conversion only works with floating-point color
   * values.
   */
  template <class Clr> Clr xyzToLab(const Clr& xyzColor,
                                    const Clr& whitePoint);

  /**
   * Convert a color in the CIE 1976 L*a*b* color space into the CIE
   * XYZ color space.
   *
   * @param labColor the L*a*b* color to be converted to XYZ
   *
   * @param whitePoint the XYZ coordinates of the white point of the
   * imaging equipment. Make sure the white point has no zero color
   * channels.
   *
   * @note The conversion only works with floating-point color
   * values.
   */
  template <class Clr> Clr labToXyz(const Clr& labColor,
                                    const Clr& whitePoint);

  /**
   * An adaptable binary function that converts an XYZ color to an
   * L*a*b* color, given the white point of the imaging equipment in
   * XYZ.
   */
  template <class Clr> struct XyzToLab : Pii::BinaryFunction<Clr>
  {
    Clr operator() (const Clr& clr, const Clr& whitePoint) const { return xyzToLab(clr, whitePoint); }
  };

  /**
   * An adaptable binary function that converts an L*a*b* color to an
   * XYZ color, given the white point of the imaging equipment in
   * XYZ.
   */
  template <class Clr> struct LabToXyz : Pii::BinaryFunction<Clr>
  {
    Clr operator() (const Clr& clr, const Clr& whitePoint) const { return labToXyz(clr, whitePoint); }
  };

  /**
   * Convert an XYZ color image into an L*a*b* color image.
   *
   * @see xyzToLab(Clr, Clr)
   */
  template <class Clr> inline PiiMatrix<Clr> xyzToLab(const PiiMatrix<Clr>& xyzColorImage, const Clr& whitePoint)
  {
    return xyzColorImage.mapped(XyzToLab<Clr>(), whitePoint);
  }

  /**
   * Convert an L*a*b* color image into an XYZ color image.
   *
   * @see labToXyz(Clr, Clr)
   */
  template <class Clr> inline PiiMatrix<Clr> labToXyz(const PiiMatrix<Clr>& labColorImage, const Clr& whitePoint)
  {
    return labColorImage.mapped(LabToXyz<Clr>(), whitePoint);
  }

  /**
   * Convert a color in a @e non-linear RGB color space to luminance
   * as defined in ITU-R BT.709: @f$Y_{709} = 0.2126R' + 0.7152G' +
   * 0.0722B'@f$.
   */
  template <class Clr> inline float rgbToY709(const Clr& clr)
  {
    return float(clr.rgbR * 0.2126 + clr.rgbG * 0.7152 + clr.rgbB * 0.0722);
  }
  
  /**
   * An adaptable binary function that converts a @e non-linear RGB
   * color to Y709 luminance.
   *
   * @see rgbToY709(Clr)
   */
  template <class Clr> struct RgbToY709 : Pii::BinaryFunction<Clr,float>
  {
    float operator() (const Clr& clr) const { return rgbToY709(clr); }
  };

  /**
   * Convert a color image from a @e non-linear RGB color space to
   * Y709 luminance.
   */
  template <class Clr> inline PiiMatrix<float> rgbToY709(const PiiMatrix<Clr>& clrImage)
  {
    return clrImage.mapped(RgbToY709<Clr>());
  }

  /**
   * Convert a @e non-linear (gamma-adjusted) RGB color into Y'PbPr. 
   * Y'PbPr is the analog counterpart of Y'CbCr. The color channels in
   * Y'PbPr are not quantized and thus not suitable for digital
   * representation. Different encodings use different offsets and
   * scaling factors for the color channels.
   *
   * @note This function does not use the conventional BT.601
   * primaries. Instead, the more contemporary BT.709 (HDTV) primaries
   * are used.
   *
   * @param clr a gamma-adjusted RGB color, channels in [0, X]
   *
   * @return a Y'PbPr color. Y' (luma) is in [0, X], Pb (difference
   * between blue and luma) and Pr (difference between red and luma)
   * in [-0.5X, 0.5X]
   */
  template <class Clr> inline Clr rgbToYpbpr(const Clr& clr)
  {
    // Scaling constants for ITU-R BT.709 primaries:
    // Kb = 0.0722, Kr = 0.2126;
    double dY = rgbToY709(clr);
    double dPb = 0.53890924768269023496 * (clr.rgbB - dY);
    double dPr = 0.63500127000254000508 * (clr.rgbR - dY);
    return Clr(dY, dPb, dPr);
  }

  /**
   * Convert a Y'PbPr color into @e non-linear RGB.
   *
   * @note This function does not use the conventional BT.601
   * primaries. Instead, the more contemporary BT.709 (HDTV) primaries
   * are used.
   *
   * @param clr a color in a Y'PbPr color space.
   *
   * @return a color in a non-linear RGB space.
   */
  template <class Clr> inline Clr ypbprToRgb(const Clr& clr)
  {
    double dB = clr.c1 * 1.8556 + clr.c0;
    double dR = clr.c2 * 1.5748 + clr.c0;
    double dG = clr.c0 - 0.46812427293064876957 * clr.c2 - 0.18732427293064876958 * clr.c1;
    return Clr(dR, dG, dB);
  }

  /**
   * An adaptable unary function for converting from non-linear RGB to
   * Y'PbPr.
   */
  template <class T> struct RgbToYpbpr : Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return rgbToYpbpr(value); }
  };

  /**
   * Convert a color image in a non-linear RGB space into Y'PbPr.
   */
  template <class T> inline PiiMatrix<T> rgbToYpbpr(const PiiMatrix<T>& image)
  {
    return image.mapped(RgbToYpbpr<T>());
  }

  /**
   * An adaptable unary function for converting from Y'PbPr to
   * non-linear RGB.
   */
  template <class T> struct YpbprToRgb : Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return ypbprToRgb(value); }
  };

  /**
   * Convert a color image in a Y'PbPr space to non-linear RGB.
   */
  template <class T> inline PiiMatrix<T> ypbprToRgb(const PiiMatrix<T>& image)
  {
    return image.mapped(YpbprToRgb<T>());
  }

  /// @internal Rounds and bounds color channels
  template <class T> inline T roundYcbcr(double d, double maximum)
  {
    return d > 0 ? (d < maximum ? Pii::round<T>(d) : maximum) : 0;
  }

  /**
   * Convert a @e non-linear (gamma-adjusted) RGB color into Y'CbCr. 
   * Y'CbCr is the same as Y'PbPr, but the chroma components are
   * offset to non-negative values.
   *
   * @note This function does not use the conventional BT.601
   * primaries. Instead, the more contemporary BT.709 (HDTV) primaries
   * are used.
   *
   * @param clr a gamma-adjusted RGB color, channels in [0, maximum]
   *
   * @param maximum maximum value of a color channel
   *
   * @return a Y'CbCr color. Y' (luma), Cb (difference between blue
   * and luma) and Cr (difference between red and luma) are all in [0,
   * @p maximum]. All color channels will be rounded to nearest
   * integers.
   */
  template <class Clr> inline Clr rgbToYcbcr(const Clr& clr,
                                             double maximum = PiiImage::Traits<Clr>::max())
  {
    typedef typename Clr::Type T;
    double dHalfMax = maximum/2;
    double dY = rgbToY709(clr);
    double dPb = 0.53890924768269023496 * (clr.rgbB - dY) + dHalfMax;
    double dPr = 0.63500127000254000508 * (clr.rgbR - dY) + dHalfMax;
    return Clr(roundYcbcr<T>(dY, maximum),
               roundYcbcr<T>(dPb, maximum),
               roundYcbcr<T>(dPr, maximum));
  }

  /**
   * Convert a Y'CbCr color into @e non-linear RGB.
   *
   * @note This function does not use the conventional BT.601
   * primaries. Instead, the more contemporary BT.709 (HDTV) primaries
   * are used.
   *
   * @param clr a color in a Y'CbCr color space.
   *
   * @param maximum the maximum value of a color channel
   *
   * @return a color in a non-linear RGB space. All color channels are
   * in [0, maximum] and rounded to nearest integers. To obtain linear
   * RGB, use the correctGamma() function.
   */
  template <class Clr> inline Clr ycbcrToRgb(const Clr& clr,
                                             double maximum = PiiImage::Traits<Clr>::max())
  {
    typedef typename Clr::Type T;
    double dHalfMax = maximum/2;
    double dPb = clr.c1 - dHalfMax, dPr = clr.c2 - dHalfMax;
    double dB = dPb * 1.8556 + clr.c0;
    double dR = dPr * 1.5748 + clr.c0;
    double dG = clr.c0 - 0.46812427293064876957 * dPr - 0.18732427293064876958 * dPb;
    return Clr(roundYcbcr<T>(dR, maximum),
               roundYcbcr<T>(dG, maximum),
               roundYcbcr<T>(dB, maximum));
  }
 
  /**
   * An adaptable unary function for converting from non-linear RGB to
   * Y'CbCr.
   */
  template <class T> class RgbToYcbcr : public Pii::UnaryFunction<T>
  {
  public:
    RgbToYcbcr(double maximum = PiiImage::Traits<T>::max()) : _max(maximum) {}
    T operator() (const T& value) const { return rgbToYcbcr(value, _max); }
  private:
    double _max;
  };

  /**
   * Convert a color image in a non-linear RGB space into Y'CbCr.
   */
  template <class T> inline PiiMatrix<T> rgbToYcbcr(const PiiMatrix<T>& image,
                                                    double maximum = PiiImage::Traits<T>::max())
  {
    return image.mapped(RgbToYcbcr<T>(maximum));
  }

  /**
   * An adaptable unary function for converting from Y'CbCr to
   * non-linear RGB.
   */
  template <class T> struct YcbcrToRgb : Pii::UnaryFunction<T>
  {
  public:
    YcbcrToRgb(double maximum = PiiImage::Traits<T>::max()) : _max(maximum) {}
    T operator() (const T& value) const { return ycbcrToRgb(value, _max); }
  private:
    double _max;
  };

  /**
   * Convert a color image in a Y'CbCr space to non-linear RGB.
   */
  template <class T> inline PiiMatrix<T> ycbcrToRgb(const PiiMatrix<T>& image,
                                                    double maximum = PiiImage::Traits<T>::max())
  {
    return image.mapped(YcbcrToRgb<T>(maximum));
  }

  /**
   * An adaptable binary function that multiplies a color with a
   * 3-by-3 matrix. The type of the color channels in the result will
   * always be @p float. The function makes no checks. The caller must
   * ensure the size of the conversion matrix.
   */
  template <class Clr> struct GenericConversion : Pii::BinaryFunction<PiiMatrix<float>, Clr, PiiColor<float> >
  {
    PiiColor<float> operator() (const PiiMatrix<float>& matrix, const Clr& clr) const
    {
      const float* row0 = matrix[0], *row1 = matrix[1], *row2 = matrix[2];
      return PiiColor<float>(row0[0] * clr.c0 + row0[1] * clr.c1 + row0[2] * clr.c2,
                             row1[0] * clr.c0 + row1[1] * clr.c1 + row1[2] * clr.c2,
                             row2[0] * clr.c0 + row2[1] * clr.c1 + row2[2] * clr.c2);
    }
  };

  /**
   * Convert all colors in an image with a generic conversion matrix. 
   * This function multiplies the three color channels by a 3-by-3
   * conversion matrix from left. The result type will always be
   * PiiColor<float>. The function makes no checks. The caller must
   * ensure the size of the conversion matrix.
   *
   * @code
   * using namespace PiiColors;
   * PiiMatrix<PiiColor4<> > clrImage;
   * PiiMatrix<PiiColor<float> > converted = genericConversion(clrImage, ohtaKanadeMatrix);
   * @endcode
   */
  template <class Clr> inline PiiMatrix<PiiColor<float> > genericConversion(const PiiMatrix<Clr>& colorImage,
                                                                            const PiiMatrix<float>& conversionMatrix)
  {
    return colorImage.mapped(std::bind1st(GenericConversion<Clr>(), conversionMatrix));
  }

  /**
   * A conversion matrix from RGB to the maximally independent color
   * space of Ohta and Kanade.
   *
   * @see genericConversion()
   */
	extern PII_COLORS_EXPORT PiiMatrix<float> ohtaKanadeMatrix;
  /**
   * A conversion matrix from RGB to XYZ, assuming that the RGB is CIE
   * RGB, and that the scene is illuminated with a D65 light source. 
   * If either of these assumptions does not hold, please do not
   * pretend that you have XYZ.
   *
   * @see genericConversion()
   */
	extern PII_COLORS_EXPORT PiiMatrix<float> d65_709_XyzMatrix;
}

#include "PiiColors-templates.h"

#endif //_PIICOLORS_H
