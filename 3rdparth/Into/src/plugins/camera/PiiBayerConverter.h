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

#ifndef _PIIBAYERCONVERTER_H
#define _PIIBAYERCONVERTER_H

#include <PiiMatrix.h>
#include <PiiColor.h>
#include <PiiCameraGlobal.h>

namespace PiiCamera
{
  /**
   * A structure used for handling different Bayer encoding schemes
   * with bayerToRgb().
   *
   * Normally, the Bayer pattern is like this (RGGB):
   *
@verbatim
RGRGRGRG
GBGBGBGB
RGRGRGRG
GBGBGBGB
@endverbatim
   *
   * The name of the pattern is composed of the names of color
   * channels forming a 2-by-2 square of pixels. For example, GRBG
   * means that the pixel at (0,0) is the green color channel and the
   * pixel to its right red. The first pixel on the second row is
   * blue, and the pixel to its right is green again.
   *
   * Let us take the second pixel on the second row (B) as an example.
   * The value for the red color channel is obtained by calculating
   * the average of the diagonal neighbor pixels. The value for the
   * green channel is the average of the other four neighbors, and B
   * needs no interpolation. That's what (d,s,c) in the following
   * comes from.
   *
   * The sequence of (r,g,b) triplets goes like this:
   *
   * (c,s,d),(h,c,v),(c,s,d),(h,c,v) ...
   * (v,c,h),(d,s,c),(v,c,h),(d,s,c) ...
   * (c,s,d),(h,c,v),(c,s,d),(h,c,v) ...
   *
   * @li c = no interpolation, take the center of the neighborhood (CenterInterpolator)
   * @li s = interpolate points at four main directions (StraightInterpolator)
   * @li d = interpolate points at four diagonals (DiagonalInterpolator)
   * @li v = interpolate two points vertically (VerticalInterpolator)
   * @li h = interpolate two points horizontally (HorizontalInterpolator)
   *
   * The encoder structure is composed of interpolators for three
   * color channels in a 2-by-2 neighborhood. Thus, there must be
   * 3*4=12 interpolators in total. Each interpolator handles one of
   * the possible interpolation options (no interpolation, diagonal
   * neighbors, straight neighbors, vertical neighbors, or horizontal
   * neighbors).
   */
  template <class I0, class I1, class I2, class I3, class I4, class I5, class I6, class I7, class I8, class I9, class I10, class I11> struct BayerDecoder
  {
    /**
     * Interpolator for the red channel of the upper left pixel in a
     * 2-by-2 neighborhood.
     */
    I0 interpolatorR00;
    /**
     * Interpolator for the green channel of the upper left pixel in a
     * 2-by-2 neighborhood.
     */
    I1 interpolatorG00;
    /**
     * Interpolator for the blue channel of the upper left pixel in a
     * 2-by-2 neighborhood.
     */
    I2 interpolatorB00;

    /**
     * Interpolator for the red channel of the upper right pixel in a
     * 2-by-2 neighborhood.
     */
    I3 interpolatorR01;
    /**
     * Interpolator for the green channel of the upper right pixel in
     * a 2-by-2 neighborhood.
     */
    I4 interpolatorG01;
    /**
     * Interpolator for the blue channel of the upper right pixel in a
     * 2-by-2 neighborhood.
     */
    I5 interpolatorB01;

    /**
     * Interpolator for the red channel of the lower left pixel in a
     * 2-by-2 neighborhood.
     */
    I6 interpolatorR10;
    /**
     * Interpolator for the green channel of the lower left pixel in a
     * 2-by-2 neighborhood.
     */
    I7 interpolatorG10;
    /**
     * Interpolator for the blue channel of the lower left pixel in a
     * 2-by-2 neighborhood.
     */
    I8 interpolatorB10;

    /**
     * Interpolator for the red channel of the lower right pixel in a
     * 2-by-2 neighborhood.
     */
    I9 interpolatorR11;
    /**
     * Interpolator for the green channel of the lower right pixel in a
     * 2-by-2 neighborhood.
     */
    I10 interpolatorG11;
    /**
     * Interpolator for the blue channel of the lower right pixel in a
     * 2-by-2 neighborhood.
     */
    I11 interpolatorB11;
  };
  
  /**
   * An interpolation functor that calculates the value of a color
   * channel by averaging the four straight neighbors of a pixel.
   * Each of the funtions performs the interpolation at some special
   * point of the image (corners, borders). The #center() function
   * is used elsewhere.
   */
  template <class T> struct StraightInterpolator
  {
    int topLeft(const T* ptr1, const T* ptr2) const { return (ptr1[2] + ptr2[1]) >> 1; }
    int topRight(const T* ptr1, const T* ptr2) const { return (ptr1[0] + ptr2[1]) >> 1; }
    int bottomLeft(const T* ptr0, const T* ptr1) const { return (ptr0[1] + ptr1[2]) >> 1; }
    int bottomRight(const T* ptr0, const T* ptr1) const { return (ptr0[1] + ptr1[0]) >> 1; }
    int top(const T* ptr1, const T* ptr2) const { return (ptr1[0] + ptr1[2] + ptr2[1]) / 3; }
    int left(const T* ptr0, const T* ptr1, const T* ptr2) const { return (ptr0[1] + ptr1[2] + ptr2[1]) / 3; }
    int bottom(const T* ptr0, const T* ptr1) const { return (ptr0[1] + ptr1[0] + ptr1[2]) / 3; }
    int right(const T* ptr0, const T* ptr1, const T* ptr2) const { return (ptr0[1] + ptr1[0] + ptr2[1]) / 3; }
    int center(const T* ptr0, const T* ptr1, const T* ptr2) const { return (ptr0[1] + ptr1[0] + ptr1[2] + ptr2[1]) >> 2; }
  };

  /**
   * An interpolation functor that calculates the value of a color
   * channel by averaging the four diagonal neighbors of a pixel. Each
   * of the funtions performs the interpolation at some special point
   * of the image (corners, borders). The #center() function is used
   * elsewhere.
   */
  template <class T> struct DiagonalInterpolator
  {
    int topLeft(const T* /*ptr1*/, const T* ptr2) const { return ptr2[2]; }
    int topRight(const T* /*ptr1*/, const T* ptr2) const { return ptr2[0]; }
    int bottomLeft(const T* ptr0, const T* /*ptr1*/) const { return ptr0[2]; }
    int bottomRight(const T* ptr0, const T* /*ptr1*/) const { return ptr0[0]; }
    int top(const T* /*ptr1*/, const T* ptr2) const { return (ptr2[0] + ptr2[2]) >> 1; }
    int left(const T* ptr0, const T* /*ptr1*/, const T* ptr2) const { return (ptr0[2] + ptr2[2]) >> 1; }
    int bottom(const T* ptr0, const T* /*ptr1*/) const { return (ptr0[0] + ptr0[2]) >> 1; }
    int right(const T* ptr0, const T* /*ptr1*/, const T* ptr2) const { return (ptr0[0] + ptr2[0]) >> 1; }
    int center(const T* ptr0, const T* /*ptr1*/, const T* ptr2) const { return (ptr0[0] + ptr0[2] + ptr2[0] + ptr2[2]) >> 2; }
  };

  /**
   * An interpolation functor that calculates the value of a color
   * channel by averaging the two vertical neighbors of a pixel.
   * Each of the funtions performs the interpolation at some special
   * point of the image (corners, borders). The #center() function
   * is used elsewhere.
   */
  template <class T> struct VerticalInterpolator
  {
    int topLeft(const T* /*ptr1*/, const T* ptr2) const { return ptr2[1]; }
    int topRight(const T* /*ptr1*/, const T* ptr2) const { return ptr2[1]; }
    int bottomLeft(const T* ptr0, const T* /*ptr1*/) const { return ptr0[1]; }
    int bottomRight(const T* ptr0, const T* /*ptr1*/) const { return ptr0[1]; }
    int top(const T* /*ptr1*/, const T* ptr2) const { return ptr2[1]; }
    int left(const T* ptr0, const T* /*ptr1*/, const T* ptr2) const { return (ptr0[1] + ptr2[1]) >> 1; }
    int bottom(const T* ptr0, const T* /*ptr1*/) const { return ptr0[1]; }
    int right(const T* ptr0, const T* /*ptr1*/, const T* ptr2) const { return (ptr0[1] + ptr2[1]) >> 1; }
    int center(const T* ptr0, const T* /*ptr1*/, const T* ptr2) const { return (ptr0[1] + ptr2[1]) >> 1; }
  };

  /**
   * An interpolation functor that calculates the value of a color
   * channel by averaging the two horizontal neighbors of a pixel. 
   * Each of the funtions performs the interpolation at some special
   * point of the image (corners, borders). The #center() function is
   * used elsewhere.
   */
  template <class T> struct HorizontalInterpolator
  {
    int topLeft(const T* ptr1, const T* /*ptr2*/) const { return ptr1[2]; }
    int topRight(const T* ptr1, const T* /*ptr2*/) const { return ptr1[0]; }
    int bottomLeft(const T* /*ptr0*/, const T* ptr1) const { return ptr1[2]; }
    int bottomRight(const T* /*ptr0*/, const T* ptr1) const { return ptr1[0]; }
    int top(const T* ptr1, const T* /*ptr2*/) const { return (ptr1[0] + ptr1[2]) >> 1; }
    int left(const T* /*ptr0*/, const T* ptr1, const T* /*ptr2*/) const { return ptr1[2]; }
    int bottom(const T* /*ptr0*/, const T* ptr1) const { return (ptr1[0] + ptr1[2]) >> 1; }
    int right(const T* /*ptr0*/, const T* ptr1, const T* /*ptr2*/) const { return ptr1[0]; }
    int center(const T* /*ptr0*/, const T* ptr1, const T* /*ptr2*/) const { return (ptr1[0] + ptr1[2]) >> 1; }
  };

  /**
   * An interpolation functor that always returns the value of the
   * center of a neighborhood without interpolation.
   */
  template <class T> struct CenterInterpolator
  {
    int topLeft(const T* ptr1, const T* /*ptr2*/) const { return ptr1[1]; }
    int topRight(const T* ptr1, const T* /*ptr2*/) const { return ptr1[1]; }
    int bottomLeft(const T* /*ptr0*/, const T* ptr1) const { return ptr1[1]; }
    int bottomRight(const T* /*ptr0*/, const T* ptr1) const { return ptr1[1]; }
    int top(const T* ptr1, const T* /*ptr2*/) const { return ptr1[1]; }
    int left(const T* /*ptr0*/, const T* ptr1, const T* /*ptr2*/) const { return ptr1[1]; }
    int bottom(const T* /*ptr0*/, const T* ptr1) const { return ptr1[1]; }
    int right(const T* /*ptr0*/, const T* ptr1, const T* /*ptr2*/) const { return ptr1[1]; }
    int center(const T* /*ptr0*/, const T* ptr1, const T* /*ptr2*/) const { return ptr1[1]; }
  };

  /**
   * An interpolation functor that does nothing. Useful when
   * subclassing BayerDecoder if not all color channels are needed.
   * All functions return 0.
   */
  struct NoInterpolator
  {
    int topLeft(const void*, const void*) const { return 0; }
    int topRight(const void*, const void*) const { return 0; }
    int bottomLeft(const void*, const void*) const { return 0; }
    int bottomRight(const void*, const void*) const { return 0; }
    int top(const void*, const void*) const { return 0; }
    int left(const void*, const void*, const void*) const { return 0; }
    int bottom(const void*, const void*) const { return 0; }
    int right(const void*, const void*, const void*) const { return 0; }
    int center(const void*, const void*, const void*) const { return 0; }
  };

  /**
   * Bayer decoding structure for RGGB color ordering.
   */
  template <class T = unsigned char> struct RggbDecoder :
    public BayerDecoder<CenterInterpolator<T>,StraightInterpolator<T>,DiagonalInterpolator<T>,
                        HorizontalInterpolator<T>,CenterInterpolator<T>,VerticalInterpolator<T>,
                        VerticalInterpolator<T>,CenterInterpolator<T>,HorizontalInterpolator<T>,
                        DiagonalInterpolator<T>,StraightInterpolator<T>,CenterInterpolator<T> >
  {};

  /**
   * Bayer decoding structure for GRBG color ordering.
   */
  template <class T = unsigned char> struct GrbgDecoder :
    public BayerDecoder<HorizontalInterpolator<T>,CenterInterpolator<T>,VerticalInterpolator<T>,
                        CenterInterpolator<T>,StraightInterpolator<T>,DiagonalInterpolator<T>,
                        DiagonalInterpolator<T>,StraightInterpolator<T>,CenterInterpolator<T>,
                        VerticalInterpolator<T>,CenterInterpolator<T>,HorizontalInterpolator<T> >
  {};


  /**
   * Bayer decoding structure for BGGR color ordering.
   */
  template <class T = unsigned char> struct BggrDecoder :
    public BayerDecoder<DiagonalInterpolator<T>,StraightInterpolator<T>,CenterInterpolator<T>,
                        VerticalInterpolator<T>,CenterInterpolator<T>,HorizontalInterpolator<T>,
                        HorizontalInterpolator<T>,CenterInterpolator<T>,VerticalInterpolator<T>,
                        CenterInterpolator<T>,StraightInterpolator<T>,DiagonalInterpolator<T> >
  {};

  /**
   * Rgb color pixel type functor for Bayer decoding. Uses PiiColor<T>
   * as output type. Use this structure as a model for new pixel types.
   */
  template <class T = unsigned char> struct RgbPixel
  {
    /**
     * The output type. Each pixel type must have a typedef for @p
     * Type. This functor uses PiiColor<T>.
     */
    typedef PiiColor<T> Type;
    /**
     * How are the interpolated color channel values converted to the
     * output type? This operator must be present in all pixel types. 
     * This function returns PiiColor<T>(r,g,b).
     */
    Type operator()(int r, int g, int b) const { return Type(r,g,b); }
  };

  /**
   * Four-channel rgb color pixel type functor for Bayer decoding. 
   * Uses PiiColor4 as output type.
   */
  template <class T = unsigned char> struct Rgb4Pixel
  {
    typedef PiiColor4<T> Type;
    Type operator()(int r, int g, int b) const { return Type(r,g,b); }
  };

  /**
   * A Bayer decoding functor that extracts the red channel only. T is
   * the output type.
   */
  template <class T> struct RedPixel
  {
    typedef T Type;
    Type operator()(int r, int, int) const { return Type(r); }
  };
  
  /**
   * A Bayer decoding functor that extracts the green channel only. T
   * is the output type.
   */
  template <class T> struct GreenPixel
  {
    typedef T Type;
    Type operator()(int, int g, int) const { return Type(g); }
  };
 
  /**
   * A Bayer decoding functor that extracts the blue channel only. T
   * is the output type.
   */
  template <class T> struct BluePixel
  {
    typedef T Type;
    Type operator()(int, int, int b) const { return Type(b); }
  };

  /**
   * A Bayer decoding functor that directly converts the image to gray
   * levels by averaging RGB channels. T is the output type.
   *
   * @note Using %GrayPixel as the pixel type in Bayer decoding is
   * faster than first converting to RGB and then to gray, but it is
   * still far from optimal. If you really need fast direct conversion
   * to gray, you need to a) create four custom interpolators that
   * calculate the gray value of the four pixels in the pattern (See,
   * for example, StraightInterpolator.) b) subclass BayerDecoder with
   * your custom interpolators set to the red color channel c) use the
   * RedPixel functor to extract the outcome.
   */
  template <class T> struct GrayPixel
  {
    typedef T Type;
    Type operator()(int r, int g, int b) const { return Type((r+g+b)/3); }
  };

  /**
   * Convert a Bayer-encoded color image to RGB colors. Use the
   * BayerConversionTraits structure to control the way colors are
   * interpolated.
   *
   * @param encoded a Bayer-encoded color image
   *
   * @param decoder a decoder object used for interpolating color
   * channels. The template type of the decoder must match that of the
   * input matrix.
   *
   * @param pixel a functor that converts the interpolated values into
   * output values. By changing the pixel functor one can change the
   * output type to virtually anything. Implementations for RgbPixel,
   * Rgb4Pixel, RedPixel, GreenPixel, and BluePixel are provided.
   *
   * @return decoded color image. The size of this image equals that
   * of @p encoded. If the input matrix is smaller than 2x2, no
   * conversion will be done.
   *
   * @code
   * PiiMatrix<unsigned char> encoded;
   *
   * // Convert to four-channel RGB
   * PiiMatrix<PiiColor4<> > rgbImage =
   *   PiiCamera::bayerToRgb(encoded,
   *                         PiiCamera::RggbDecoder<>(),
   *                         PiiCamera::RgbPixel<>());
   *
   * // Take only blue channel, as an integer
   * PiiMatrix<int> blueChannel =
   *   PiiCamera::bayerToRgb(encoded,
   *                         PiiCamera::RggbDecoder<>(),
   *                         PiiCamera::BluePixel<int>());
   *
   * // Convert directly to gray levels
   * PiiMatrix<int> gray =
   *   PiiCamera::bayerToRgb(encoded,
   *                         PiiCamera::RggbDecoder<>(),
   *                         PiiCamera::GrayPixel<int>());
   * @endcode
   *
   * See the documentation of RgbPixel on how to create your own pixel
   * types.
   */
  template <class T, class Decoder, class Pixel> PiiMatrix<typename Pixel::Type>
  bayerToRgb(const PiiMatrix<T>& encoded,
             Decoder decoder,
             Pixel pixel)
  {
    typedef typename Pixel::Type U;
    
    // Cannot handle too small input
    if (encoded.rows() < 2 || encoded.columns() < 2)
      return PiiMatrix<U>(encoded.rows(), encoded.columns());

    // PENDING all loops could be optimized to step two pixels at a
    // time. The c & 1 or r & 1 comparison could be avoided.
    
    PiiMatrix<U> result(PiiMatrix<T>::uninitialized(encoded.rows(), encoded.columns()));
    U* resultRow = result.row(0);

    // Pointers to the previous, current, and next row
    const T* row0, *row1, *row2;

    int c;
    row1 = encoded.row(0)-1;
    row2 = encoded.row(1)-1;

    // Top left corner
    resultRow[0] = pixel(decoder.interpolatorR00.topLeft(row1, row2),
                         decoder.interpolatorG00.topLeft(row1, row2),
                         decoder.interpolatorB00.topLeft(row1, row2));

    ++row1; ++row2;

    // Top Row
    for (c = 1; c<encoded.columns()-1; ++c, ++row1, ++row2)
      {
        if (c & 1)
          resultRow[c] = pixel(decoder.interpolatorR01.top(row1, row2),
                               decoder.interpolatorG01.top(row1, row2),
                               decoder.interpolatorB01.top(row1, row2));
        else
          resultRow[c] = pixel(decoder.interpolatorR00.top(row1, row2),
                               decoder.interpolatorG00.top(row1, row2),
                               decoder.interpolatorB00.top(row1, row2));
      }
    // Top right corner
    if (c & 1)
      resultRow[c] = pixel(decoder.interpolatorR01.topRight(row1, row2),
                           decoder.interpolatorG01.topRight(row1, row2),
                           decoder.interpolatorB01.topRight(row1, row2));
    else
      resultRow[c] = pixel(decoder.interpolatorR00.topRight(row1, row2),
                           decoder.interpolatorG00.topRight(row1, row2),
                           decoder.interpolatorB00.topRight(row1, row2));
      
    
    // Handle all but the top and bottom rows
    int r = 1;
    for (;r<encoded.rows()-1; ++r)
      {
        row0 = encoded.row(r-1)-1;
        row1 = encoded.row(r)-1;
        row2 = encoded.row(r+1)-1;
        resultRow = result.row(r);

        // Odd rows
        if (r & 1)
          {
            // Handle the lefmost pixel
            resultRow[0] = pixel(decoder.interpolatorR10.left(row0, row1, row2),
                                 decoder.interpolatorG10.left(row0, row1, row2),
                                 decoder.interpolatorB10.left(row0, row1, row2));

            ++row0; ++row1; ++row2;
            for (c = 1; c<encoded.columns()-1; ++c, ++row0, ++row1, ++row2)
              {
                if (c & 1)
                  resultRow[c] = pixel(decoder.interpolatorR11.center(row0, row1, row2),
                                       decoder.interpolatorG11.center(row0, row1, row2),
                                       decoder.interpolatorB11.center(row0, row1, row2));
                else
                  resultRow[c] = pixel(decoder.interpolatorR10.center(row0, row1, row2),
                                       decoder.interpolatorG10.center(row0, row1, row2),
                                       decoder.interpolatorB10.center(row0, row1, row2));
              }
            // Handle the rightmost pixel
            if (c & 1)
              resultRow[c] = pixel(decoder.interpolatorR11.right(row0, row1, row2),
                                   decoder.interpolatorG11.right(row0, row1, row2),
                                   decoder.interpolatorB11.right(row0, row1, row2));
            else
              resultRow[c] = pixel(decoder.interpolatorR10.right(row0, row1, row2),
                                   decoder.interpolatorG10.right(row0, row1, row2),
                                   decoder.interpolatorB10.right(row0, row1, row2));
          }
        // Even rows
        else
          {
            resultRow[0] = pixel(decoder.interpolatorR00.left(row0, row1, row2),
                                 decoder.interpolatorG00.left(row0, row1, row2),
                                 decoder.interpolatorB00.left(row0, row1, row2));

            ++row0; ++row1; ++row2;
            for (c = 1; c<encoded.columns()-1; ++c, ++row0, ++row1, ++row2)
              {
                if (c & 1)
                  resultRow[c] = pixel(decoder.interpolatorR01.center(row0, row1, row2),
                                       decoder.interpolatorG01.center(row0, row1, row2),
                                       decoder.interpolatorB01.center(row0, row1, row2));
                else
                  resultRow[c] = pixel(decoder.interpolatorR00.center(row0, row1, row2),
                                       decoder.interpolatorG00.center(row0, row1, row2),
                                       decoder.interpolatorB00.center(row0, row1, row2));
              }
            if (c & 1)
              resultRow[c] = pixel(decoder.interpolatorR01.right(row0, row1, row2),
                                   decoder.interpolatorG01.right(row0, row1, row2),
                                   decoder.interpolatorB01.right(row0, row1, row2));
            else
              resultRow[c] = pixel(decoder.interpolatorR00.right(row0, row1, row2),
                                   decoder.interpolatorG00.right(row0, row1, row2),
                                   decoder.interpolatorB00.right(row0, row1, row2));
          }
      }

    resultRow = result.row(r);
    row0 = encoded.row(r-1)-1;
    row1 = encoded.row(r)-1;

    // Last row is odd
    if (r & 1)
      {
        // Bottom left corner
        resultRow[0] = pixel(decoder.interpolatorR10.bottomLeft(row0, row1),
                             decoder.interpolatorG10.bottomLeft(row0, row1),
                             decoder.interpolatorB10.bottomLeft(row0, row1));

        ++row0; ++row1;
        // Bottom Row
        for (c = 1; c<encoded.columns()-1; ++c, ++row0, ++row1)
          {
            if (c & 1)
              resultRow[c] = pixel(decoder.interpolatorR11.bottom(row0, row1),
                                   decoder.interpolatorG11.bottom(row0, row1),
                                   decoder.interpolatorB11.bottom(row0, row1));
            else
              resultRow[c] = pixel(decoder.interpolatorR10.bottom(row0, row1),
                                   decoder.interpolatorG10.bottom(row0, row1),
                                   decoder.interpolatorB10.bottom(row0, row1));
          }

        // Bottom right corner
        if (c & 1)
          resultRow[c] = pixel(decoder.interpolatorR11.bottomRight(row0, row1),
                               decoder.interpolatorG11.bottomRight(row0, row1),
                               decoder.interpolatorB11.bottomRight(row0, row1));
        else
          resultRow[c] = pixel(decoder.interpolatorR10.bottomRight(row0, row1),
                               decoder.interpolatorG10.bottomRight(row0, row1),
                               decoder.interpolatorB10.bottomRight(row0, row1));
      }
    // Last row is even
    else
      {
        resultRow[0] = pixel(decoder.interpolatorR00.bottomLeft(row0, row1),
                             decoder.interpolatorG00.bottomLeft(row0, row1),
                             decoder.interpolatorB00.bottomLeft(row0, row1));
        
        // Bottom Row
        for (c = 1; c<encoded.columns()-1; ++c, ++row0, ++row1)
          {
            if (c & 1)
              resultRow[c] = pixel(decoder.interpolatorR01.bottom(row0, row1),
                                   decoder.interpolatorG01.bottom(row0, row1),
                                   decoder.interpolatorB01.bottom(row0, row1));
            else
              resultRow[c] = pixel(decoder.interpolatorR00.bottom(row0, row1),
                                   decoder.interpolatorG00.bottom(row0, row1),
                                   decoder.interpolatorB00.bottom(row0, row1));
          }

        // Bottom right corner
        if (c & 1)
          resultRow[c] = pixel(decoder.interpolatorR01.bottomRight(row0, row1),
                               decoder.interpolatorG01.bottomRight(row0, row1),
                               decoder.interpolatorB01.bottomRight(row0, row1));
        else
          resultRow[c] = pixel(decoder.interpolatorR00.bottomRight(row0, row1),
                               decoder.interpolatorG00.bottomRight(row0, row1),
                               decoder.interpolatorB00.bottomRight(row0, row1));
      }

    return result;
  }

  /**
   * A convenience function that decodes an RGGB-encoded 8-bit image
   * into a 32-bit RGB color image.
   */
  PII_CAMERA_EXPORT PiiMatrix<PiiColor4<> > rggbToRgb(const PiiMatrix<unsigned char>& encoded);
  /**
   * A convenience function that decodes a GRBG-encoded 8-bit image
   * into a 32-bit RGB color image.
   */
  PII_CAMERA_EXPORT PiiMatrix<PiiColor4<> > grbgToRgb(const PiiMatrix<unsigned char>& encoded);
}

#endif //_PIIBAYERCONVERTER_H
