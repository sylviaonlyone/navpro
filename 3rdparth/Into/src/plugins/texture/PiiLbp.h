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

#ifndef _PIILBP_H
#define _PIILBP_H

#include <Pii.h>
#include <PiiMatrix.h>
#include <PiiRoi.h>
#include <PiiFunctional.h>
#include <cmath>
#include "PiiTextureGlobal.h"

/**
 * An implementation of the Local Binary Patterns (LBP) texture
 * operator. The LBP works by comparing N pairs of pixels in a local
 * neighborhood. The results of these comparisons build up a N-bit
 * binary number. The process is repeated for each pixel in an image,
 * and the codes are collected into a histogram. Alternatively, each
 * pixel in the input image can be converted to a LBP code. The number
 * of different LBP codes and hence the number of bins in the output
 * histogram depends on LBP parameters. See #Mode for details.
 *
 * Usually, the local neighborhood in LBP is circular, parameterized
 * by the number of samples and the neighborhood radius as shown in
 * the picture below. (There are other alternatives, such as @ref
 * PiiRandomLbp "random neighborhood".) Each pixel in the neighborhood
 * is compared to another pixel in the neighborhood. Depending on the
 * result of the comparison, one bit of a binary number is set to
 * either zero or one. Usually, the neighbors are compared to the
 * pixel at the center of the circle. In symmetric LBP, opposing
 * neighbors are compared to each other. In @ref PiiRandomLbp "random
 * LBP", the compared pixels are chosen at random.
 *
 * @image html lbp_neighborhoods.png
 *
 * @ingroup PiiTexturePlugin
 */
class PII_TEXTURE_EXPORT PiiLbp
{
public:
  class Histogram;
  class Image;
  
  /**
   * The operation mode of the LBP. Possible values are:
   *
   * @lip Standard - no modifications. All binary codes build up their
   * own histogram bins. The total number of bins is @f$2^N@f$. For
   * example, an 8-bit LBP produces a 256-bin histogram.
   *
   * @lip Uniform - only "uniform" binary codes are accepted. Others
   * build up a single "miscellaneous" bin. This reduces the number of
   * bins significantly and increases robustness in some applications. 
   * Uniform binary codes are binary numbers that have at most two
   * 1-to-0 or 0-to-1 transitions in their circularly interpreted
   * binary representation. For example, the 8-bit binary number
   * 10000011 is uniform whereas 10000101 is not.
   *
   * @lip RotationInvariant - each code is rotated to its minimum
   * value so that rotation of the image does not (ideally) change the
   * result. This reduces the number of codes. For example, the 8-bit
   * codes 11100000, 01110000, and 00111000 would all become 00000111. 
   *
   * @lip UniformRotationInvariant - like @p RotationInvariant, but
   * only uniform codes are accepted. This results in a short,
   * rotation invariant feature vector.
   *
   * @lip Symmetric - An LBP operator that compares opposing pairs of
   * pixels in a circular neighborhood and ignores the center. The
   * advantage is that only N/2 comparisons per pixel are needed
   * compared to N comparisons in the original LBP operator. 
   * Furthermore, the length of the (standard) feature vector will be
   * @f$2^{N/2}@f$ instead of @f$2^N@f$, which makes classification
   * faster. Unfortunately, there is no free lunch. The symmetric
   * version cannot be made rotation invariant as easily as the basic
   * LBP. Its classification accuracy may also be somewhat worse. 
   * Uniform patterns and rotation invariance loose their meaning with
   * the symmetric LBP.
   */
  enum Mode
    {
      Standard,
      Uniform,
      RotationInvariant,
      UniformRotationInvariant,
      Symmetric
    };

  /**
   * Creates a new LBP operator.
   *
   * @param samples the number of neighborhood samples
   *
   * @param radius the radius of the neighborhood
   *
   * @param mode the operation mode
   *
   * @param interpolation interpolation mode
   */
  PiiLbp(int samples = 8, double radius = 1, Mode mode = Standard,
         Pii::Interpolation interpolation = Pii::NearestNeighborInterpolation);

  /**
   * Destroy the LBP operator.
   */
  ~PiiLbp();
  
  /**
   * Sets LBP parameters. Calling this function will update mapping
   * tables and interpolation coefficients.
   *
   * @param samples the number of neighborhood samples. The basic LBP
   * uses eight neighbors. 16 is a common choice, and even 24 can be
   * used, but with 24 samples, a 32MB look-up table is needed.
   *
   * @param radius the neighborhood radius. The radius can, in
   * principle, take any value, but typically only discrete values in
   * the range 1 ... 5 are used. The basic LBP uses 1 as the radius.
   *
   * @param mode the operation mode.
   *
   * @param interpolation the type of interpolation. Due to the
   * circular neighborhood, not all samples fall exactly on pixels. 
   * For such samples, interpolation is needed. Supported
   * interpolation types are @p NearestNeighborInterpolation and @p
   * LinearInterpolation. The default is nearest neighbor.
   */
  void setParameters(int samples, double radius, Mode mode, Pii::Interpolation interpolation);

  /**
   * Returns the number of neighborhood samples.
   */
  int sampleCount() const;

  /**
   * Returns the neighborhood radius.
   */
  double neighborhoodRadius() const;

  /**
   * Returns the operation mode.
   */
  Mode mode() const;

  /**
   * Returns the type of interpolation.
   */
  Pii::Interpolation interpolation() const;

  /**
   * A template function that extracts the LBP texture features from
   * images. The function works with all primitive types. This
   * function is an entry point that selects the appropriate optimized
   * LBP implementation based on the current mode.
   *
   * The template parameter @p T is for the primitive type. @p
   * MatrixClass is a special class derived from PiiMatrix<int> that
   * has a known structure. By changing this class one can use the LBP
   * operator to create either histograms or feature images. See
   * PiiLbp::Histogram and PiiLbp::Image for details.
   *
   * @param image the input image
   *
   * @param roi region-of-interest. See PiiRoi.
   *
   * @param centerFunc the "center function" is applied to each pixel
   * before comparing it to the neighbors. See #basicLbp().
   *
   * @code
   * PiiMatrix<int> image;
   * PiiLbp lbp;
   * lbp.genericLbp<PiiLbp::Histogram>(image, PiiRoi::DefaultRoi(), Pii::Identity<int>()); // Outputs histogram
   * lbp.genericLbp<PiiLbp::Image>(image, PiiRoi::DefaultRoi(),
   *                               std::bind2nd(std::plus<int>(), 3)); // Outputs feature image
   * @endcode
   */
  template <class MatrixClass, class T, class Roi, class UnaryFunction>
  PiiMatrix<int> genericLbp(const PiiMatrix<T>& image, Roi roi, UnaryFunction centerFunc);

  /**
   * A convenience function that uses Pii::Identity as the center
   * function.
   */
  template <class MatrixClass, class T, class Roi> inline PiiMatrix<int> genericLbp(const PiiMatrix<T>& image, Roi roi)
  {
    return genericLbp<MatrixClass>(image, roi, Pii::Identity<T>());
  }

  /**
   * A convenience function that uses PiiRoi::DefaultRoi and
   * Pii::Identity as the region-of-interest and center function.
   */
  template <class MatrixClass, class T> inline PiiMatrix<int> genericLbp(const PiiMatrix<T>& image)
  {
    return genericLbp<MatrixClass>(image, PiiImage::DefaultRoi(), Pii::Identity<T>());
  }

  /**
   * A highly optimized template implementation of the LBP 8,1
   * operator with nearest neighbor "interpolation". The function
   * works with all primitive types.
   *
   * @param image the input image
   *
   * @param roi region-of-interest. See PiiRoi.
   *
   * @param centerFunc the "center function" is applied to each pixel
   * before comparing it to the neighbors. For example, a useful trick
   * to increase LBP's noise tolerance is to add a constant value to
   * the center pixel before comparing it to neighbors:
   *
   * @code
   * PiiMatrix<unsigned char> image;
   * PiiLbp::basicLbp<PiiLbp::Histogram>(image, PiiRoi::DefaultRoi(),
   *                                     std::bind2nd(std::plus<unsigned>(), 4));
   * @endcode
   *
   * Please ensure that the result type of @p centerFunc can store the
   * calculation result without overflows or underflows. For example,
   * using @p unsigned @p char is not a good idea because 255 + 4 = 3.
   */
  template <class MatrixClass, class T, class Roi, class UnaryFunction>
  static PiiMatrix<int> basicLbp(const PiiMatrix<T>& image, Roi roi, UnaryFunction centerFunc);

  /**
   * A highly optimized template implementation of the LBP 8,1
   * operator with nearest neighbor "interpolation". The function
   * works with all primitive types. This version uses Pii::Identity
   * as the center function.
   */
  template <class MatrixClass, class T, class Roi> static inline PiiMatrix<int> basicLbp(const PiiMatrix<T>& image, Roi roi)
  {
    return basicLbp<MatrixClass>(image, roi, Pii::Identity<T>());
  }

  /**
   * A convenience function that uses PiiRoi::DefaultRoi as the
   * region-of-interest and Pii::Identity as the center function.
   */
  template <class MatrixClass, class T> static inline PiiMatrix<int> basicLbp(const PiiMatrix<T>& image)
  {
    return basicLbp<MatrixClass>(image, PiiImage::DefaultRoi(), Pii::Identity<T>());
  }

  /**
   * Generic implementation of the symmetric LBP.
   */
  template <class MatrixClass, class T, class Roi> PiiMatrix<int> genericSymmetricLbp(const PiiMatrix<T>& image, Roi roi);

  /**
   * A convenience function that uses PiiRoi::DefaultRoi.
   */
  template <class MatrixClass, class T> inline PiiMatrix<int> genericSymmetricLbp(const PiiMatrix<T>& image)
  {
    return genericSymmetricLbp<MatrixClass>(image, PiiImage::DefaultRoi());
  }

  /**
   * Same as above, but optimized for the 8-neighbors.
   */
  template <class MatrixClass, class T, class Roi> static PiiMatrix<int> basicSymmetricLbp(const PiiMatrix<T>& image, Roi roi);

  /**
   * A convenience function that uses PiiRoi::DefaultRoi.
   */
  template <class MatrixClass, class T> static inline PiiMatrix<int> basicSymmetricLbp(const PiiMatrix<T>& image)
  {
    return basicSymmetricLbp<MatrixClass>(image, PiiImage::DefaultRoi());
  }

  /**
   * Returns the length of the feature vector an LBP operation with the
   * given parameters will produce.
   *
   * @param samples the number of neighborhood samples
   *
   * @param mode the mode of the operator
   *
   * @return the length of the feature vector. The result for
   * "Standard" mode is 2^samples. Other modes produce less features.
   */
  static int featureCount(int samples, Mode mode);

  /**
   * Returns a look-up table for converting standard LBP codes to, for
   * example, rotation invariant ones. The size of the table will be
   * 2^samples. Thus, a look-up table for LBP24 eats up 32MB of
   * memory. For LBP16, the size is 128kB, which is a practical upper
   * limit.
   *
   * @param sample the number of neighborhood samples in the LBP
   * operator
   *
   * @param mode the operator type the codes are mapped to
   */
  static unsigned short* createLookupTable(int samples, Mode mode);

private:
  struct InterpolationPoint
  {
    int x,y;
    int nearestX,nearestY;
    float coeffs[4];
  };

  class Data
  {
  public:
    Data(int samples, double radius, Mode mode, Pii::Interpolation interpolation);
    ~Data();
    
    int iSamples;
    double dRadius;
    Mode mode;
    Pii::Interpolation interpolation;
    unsigned short *pLookup;
    InterpolationPoint *pPoints;
    
    void update();
  } *d;
};

/**
 * A matrix class that creates a histogram. Each matrix class must
 * follow the structure of this class so that inline functions can be
 * bound at compile time (static polymorphism).
 */
class PiiLbp::Histogram : public PiiMatrix<int>
{
public:
  /**
   * Every matrix template must have a constructor that takes four
   * integers as parameters.
   *
   * @param rows the number of rows in the source image the LBP
   * operator is applied to.
   *
   * @param columns the number of columns
   *
   * @param margin the margin of pixels left unhandled by the LBP on
   * each side of the source image.
   *
   * @param features the number of different features the LBP operator
   * will produce with the current parameters.
   */
  Histogram(int rows, int columns, int margin, int features) :
    PiiMatrix<int>(1, features), _pCurrentRow(this->row(0))
  {
    Q_UNUSED(rows); Q_UNUSED(columns); Q_UNUSED(margin); Q_UNUSED(features);
  }
  /**
   * Inform the matrix class that the LBP operator starts scanning a
   * new row.
   *
   * @param row the current row
   */
  inline void changeRow(int row) { Q_UNUSED(row); }
  /**
   * Modify the matrix template.
   *
   * @param column the current column. This tells the location of the
   * calculated code on the current row.
   *
   * @param value the calculated LBP value.
   */
  inline void modify(int column, unsigned int value) { Q_UNUSED(column); _pCurrentRow[value]++; }
private:
  int* _pCurrentRow;
};

/**
 * A matrix class that creates a feature image. See PiiLbp::Histogram
 * for details.
 */
class PiiLbp::Image : public PiiMatrix<int>
{
public:
  Image(int rows, int columns, int margin, int features) :
    PiiMatrix<int>(PiiMatrix<int>::uninitialized(rows-margin*2, columns-margin*2)),
    _iMargin(margin)
  {
    Q_UNUSED(features);
  }
  inline void changeRow(int row) { _pCurrentRow = this->row(row-_iMargin)-_iMargin; }
  inline void modify(int column, unsigned int value) { _pCurrentRow[column] = value; }

private:
  int _iMargin;
  int* _pCurrentRow;
};

#include "PiiLbp-templates.h"


#endif //_PIILBP_H
