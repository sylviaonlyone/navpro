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

#ifndef _PIIROI_H
#define _PIIROI_H

#include <PiiMatrix.h>
#include <PiiExecutionException.h>
#include <PiiVariant.h>
#include <PiiRectangle.h>
#include <PiiInputSocket.h>
#include <PiiOutputSocket.h>
#include <PiiYdinTypes.h>
#include "PiiImageGlobal.h"


/**
 * @file
 * Utilities for ROI (region-of-interest) processing
 */

namespace PiiImage
{
  /**
   * Default region-of-interest function object for feature extraction
   * and image analysis methods that support ROI processing. This
   * function returns @p true for all pixels, and reduces to a no-op
   * with compiler optimizations turned on.
   */
  struct DefaultRoi
  {
    /**
     * Check if a pixel is within a region-of-interest. Each ROI
     * functor must implement a function with this signature
     * (inline/const are optional). This function returns @p true.
     *
     * @param r the row coordinate of a pixel
     *
     * @param c the column coordinate of a pixel
     */
    inline bool operator() (int r, int c) const { Q_UNUSED(r); Q_UNUSED(c); return true; }
  };

  /**
   * A region-of-interest function object that returns @p true if the
   * alpha channel has a non-zero value at (r,c) and @p false
   * otherwise. If there is no alpha channel in the color type @p T,
   * returns always @p true.
   */
  template <class T> struct AlphaRoi : DefaultRoi
  {
    AlphaRoi(const PiiMatrix<T>&) {}
  };

  template <class T> struct AlphaRoi<PiiColor4<T> >
  {
    AlphaRoi(const PiiMatrix<PiiColor4<T> >& img) : _image(img) {}
    
    inline bool operator() (int r, int c) const { return _image(r,c).rgbaA != 0; }

  private:
    const PiiMatrix<PiiColor4<T> > _image;
  };

  /**
   * Converts any integer matrix to a boolean matrix. If @p obj is an
   * invalid variant, an empty matrix will be returned.
   *
   * @exception PiiExecutionException& if the type of the object is
   * not suitable for a ROI.
   */
  PII_IMAGE_EXPORT PiiMatrix<bool> toRoiMask(const PiiVariant& obj);

  /**
   * Creates a binary ROI mask out of a set of rectangular regions.
   *
   * @param rows the number of rows in the result image
   *
   * @param columns the number of columns in the result image
   *
   * @param rectangles a N-by-4 matrix in which each row represents a
   * rectangle (x, y, width, height). If a rectangle exceeds the
   * boundaries of the result image, it will be ignored.
   *
   * @return a @a rows -by- @a columns binary image in which all
   * pixels that are in any of the @a rectangles will be set to @p
   * true.
   */
  PII_IMAGE_EXPORT PiiMatrix<bool> createRoiMask(int rows, int columns,
                                                 const PiiMatrix<int>& rectangles);

  /**
   * Returns @p true if any two of the given @a rectangles overlap
   * each other and @p false otherwise.
   */
  PII_IMAGE_EXPORT bool overlapping(const PiiMatrix<int>& rectangles);

  /// @internal
  PII_IMAGE_EXPORT extern const char* roiRectangleSizeError;
  /// @internal
  PII_IMAGE_EXPORT extern const char* roiMaskSizeError;

  /**
   * Returns the alpha channel of @p image as a boolean mask. Non-zero
   * entries in the alpha channel will be @p true in the returned
   * mask.
   */
  template <class T> PiiMatrix<bool> alphaToMask(const PiiMatrix<PiiColor4<T> >& image);
  
  /**
   * @internal
   *
   * Reads a ROI object from @a input and handles @a image based on it
   * and @a roiType. Uses @a process to actually perform the image
   * processing operation.
   */
  template <class T, class Processor> void handleRoiInput(PiiInputSocket* input,
                                                          RoiType roiType,
                                                          const PiiMatrix<T>& image,
                                                          Processor& process);

  /// @cond null
  struct HistogramHandler
  {
    HistogramHandler() :
      iPixelCount(0),
      iLevels(256),
      bNormalized(false)
    {}
  
    virtual ~HistogramHandler() {}

    static inline bool acceptsManyRegions() { return true; }

    static void addToVariant(PiiVariant& variant, const PiiMatrix<int>& histogram)
    {
      if (!variant.isValid())
        variant = PiiVariant(histogram);
      else
        variant.valueAs<PiiMatrix<int> >() += histogram;
    }
    
    void initialize(int levels, bool normalized)
    {
      iPixelCount = 0;
      iLevels = levels;
      bNormalized = normalized;
    }

    int iPixelCount;
    int iLevels;
    int bNormalized;
  };

  template <class T> struct GrayHistogramHandler : HistogramHandler
  {
    void initialize(int levels, bool normalized);
    void operator() (const PiiMatrix<T>& image);
    template <class Roi> void operator() (const PiiMatrix<T>& image, const Roi& roi);
    void normalize();
  
    PiiVariant varHistogram;
  };

  template <class Clr> struct ColorHistogramHandler : HistogramHandler
  {
    ColorHistogramHandler()
    {
      baCalculate[0] = baCalculate[1] = baCalculate[2] = true;
    }
    
    void initialize(int levels, bool normalized);
    void operator() (const PiiMatrix<Clr>& image);
    template <class Roi> void operator() (const PiiMatrix<Clr>& image, const Roi& roi);
    void normalize();
    
    PiiVariant varHistograms[3];
    PiiMatrix<typename Clr::Type> channelImages[3];
    bool baCalculate[3];
  };
  /// @endcond
}

#include "PiiRoi-templates.h"

#endif //_PIIROI_H
