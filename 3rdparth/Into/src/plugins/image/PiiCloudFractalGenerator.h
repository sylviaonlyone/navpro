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

#ifndef _PIICLOUDFRACTALGENERATOR_H
#define _PIICLOUDFRACTALGENERATOR_H

#include <PiiMath.h>
#include <cstdlib>

// Generate a randon number between -roughness and roughness.
#define PII_FRAC_RAND(roughness)      ((rand() - RAND_MAX/2) % int(roughness))
// Truncate a value to allowed range
#define PII_FRAC_TRUNC(a)             ((a) < _iMinimum ? _iMinimum : (a) > _iMaximum ? _iMaximum : T(a))
// Given three points a,b,c so that b is midway between a and c,
// interpolate between a->b or b->c depending on which end is closer
// to x. Dir is either Row or Col and determines the interpolation
// direction.
#define PII_FRAC_INTP(x,dir,a,b,c)    (1.0 - half##dir##Weight) * b + half##dir##Weight * (x < half##dir##s ?  a : c)
// Adjust val towards mean if requested
#define PII_FRAC_ADJUST_MEAN(val) do { if (_iTargetMean >= _iMinimum) val = (val + _iTargetMean3) >> 2; } while(0)

/**
 * A class that generates two-dimensional cloud-like fractal pictures. 
 * This class works with integer-valued matrices (int, char, etc.) and
 * produces gray-scale clouds that can also be used as height maps.
 *
 * @ingroup PiiImagePlugin
 */
class PiiCloudFractalGenerator
{
public:
  /**
   * Create a new fractal generator.
   *
   * @param rougnessScale controls how quickly the initial roughness
   * of a fractal dies out.
   *
   * @param minimum the minimum allowed value for the generated
   * fractal.
   *
   * @param maximum the maximum allowed value for the generated
   * fractal.
   */
  PiiCloudFractalGenerator(float roughnessScale = 0.5,
                           int minimum = 0, int maximum = 255) :
    _fRoughnessScale(roughnessScale),
    _iMinimum(minimum), _iMaximum(maximum),
    _iTargetMean(-1), _iTargetMean3(0)
  {}

  /**
   * Generate a square piece of fractal into the given buffer. It the
   * buffer already has data in it, the fractal will be seamlessly
   * connected to it from its left and top side.
   *
   * @param buffer generate the fractal to this matrix
   *
   * @param row the starting row
   *
   * @param col the starting column
   *
   * @param size the size of the fractal. This value must be a power
   * of two. You may experiment with other values, but the result will
   * not look like a cloud. The fractal image will always be square.
   *
   * @param c0 initial value for the upper left corner
   *
   * @param c1 initial value for the upper right corner
   *
   * @param c2 initial value for the lower left corner
   *
   * @param c3 initial value for the lower right corner
   *
   * @param roughness the scale of random variations in the fractal. 
   * This value determines how large changes are allowed on the large
   * scale. Usually, a value between minimum and maximum is used. Use
   * #setRoughnessScale to control how rapidly the roughness dies out.
   */
  template <class T> void generateSquareFractal(PiiMatrix<T>& buffer,
                                                int row, int col, int size,
                                                int c0, int c1, int c2, int c3,
                                                int roughness);

  /**
   * Generate a size-by-size cloud fractal with the given initial
   * corner values and rougness. Return a new image.
   */
  template <class T> PiiMatrix<T> generateSquareFractal(int size,
                                                        int c0, int c1, int c2, int c3,
                                                        int roughness);

  /**
   * Generate a non-square fractal. This function splits the area into
   * square blocks and recursively calls #generateSquareFractal(). It
   * is possible to give a roughness value for x and y direction
   * separately, but it is usually wise to let the program decide the
   * correct value for @p roughnessY.
   */
  template <class T> void generateFractal(PiiMatrix<T>& buffer,
                                          int row, int col, int rows, int cols,
                                          int c0, int c1, int c2, int c3,
                                          double roughnessX, double roughnessY = 0);


  /**
   * Set the rougness scaling factor. This value controls how quickly
   * the initial roughness of the fractal dies out. 0 means immediately
   * and 1 means not at all.
   */
  void setRoughnessScale(float roughnessScale) { _fRoughnessScale = roughnessScale; }
  /**
   * Get the rouhgness scaling factor.
   */
  float roughnessScale() const { return _fRoughnessScale; }
  /**
   * Set the minimum possible value for the generated fractals.
   */
  void setMinimum(int minimum) { _iMinimum = minimum; }
  /**
   * Get the minimum possible value.
   */
  int minimum() const { return _iMinimum; }
  /**
   * Set the maximum possible value for the generated fractals.
   */
  void setMaximum(int maximum) { _iMaximum = maximum; }
  /**
   * Get the maximum possible value.
   */
  int maximum() const { return _iMaximum; }

  /**
   * Set the mean level the fractal generator will try to achieve.
   * This value is useful in large non-rectangular fractals. The
   * generator will always drive the random variations slightly
   * towards the target mean. If the value is less than @ref
   * setMinimum() "minimum", it will not be used.
   */
  void setTargetMean(int targetMean)
  {
    _iTargetMean = targetMean;
    _iTargetMean3 = 3 * targetMean;
  }
  /**
   * Get the target mean value.
   */
  int targetMean() const { return _iTargetMean; }
  
  /**
   * The magic equation of roughness scaling. Halving the size of
   * generated fractal in the recursive algorithm scales roughness
   * down by _fRoughnessScale. Here's what you get if you scale to an
   * arbitrary factor a/b. Assume your initial roughness for a
   * 100-by-100 fractal is 100. Comparable roughness for a 200-by-200
   * fractal is obtained with scaleRoughness(200,100)*100.
   */
  double scaleRoughness(int a, int b)
  {
    return ::pow(_fRoughnessScale, float(::log(double(b)/(a))/M_LN2));
  }

private:
  float _fRoughnessScale;
  int _iMinimum, _iMaximum;
  int _iTargetMean, _iTargetMean3;
};

template <class T>
void PiiCloudFractalGenerator::generateSquareFractal(PiiMatrix<T>& buffer,
                                                     int row, int col, int size,
                                                     int c0, int c1, int c2, int c3,
                                                     int roughness)
{
  /* Original corner points are denoted as cX. New corners are ncX. 
   * New corners for the recursive call are generated in between the
   * original ones as shown below.
   *
   *  c0 nc0 c1
   * nc1 nc2 nc3
   *  c2 nc4 c3
   */

  if (roughness == 0) roughness = 1;

  // Center point (new corner #2)
  int nc2 = ((c0 + c1 + c2 + c3) >> 2) + PII_FRAC_RAND(roughness);

  // Does not happen in recursion, but someone might do this
  // directly...
  if (size == 1)
    {
      buffer(row,col) = PII_FRAC_TRUNC(nc2);
      return;
    }

  int halfSize = size >> 1;
  
  // Other new corners (in addition to the center)
  int nc0 = row > 0 ? buffer(row-1,col+halfSize) : ((c0 + c1) >> 1) + PII_FRAC_RAND(roughness),
    nc1 = col > 0 ? buffer(row+halfSize,col-1) : ((c0 + c2) >> 1) + PII_FRAC_RAND(roughness);

  // Stop recursion
  if (size == 2)
    {
      buffer(row,col) = PII_FRAC_TRUNC(c0);
      buffer(row,col+1) = PII_FRAC_TRUNC(nc0);
      buffer(row+1,col) = PII_FRAC_TRUNC(nc1);
      buffer(row+1,col+1) = PII_FRAC_TRUNC(nc2);
      return;
    }
  
  int nc3 = ((c1 + c3) >> 1) + PII_FRAC_RAND(roughness),
    nc4 = ((c2 + c3) >> 1) + PII_FRAC_RAND(roughness);


  // New roughness
  int newRoughness = int(_fRoughnessScale * roughness);

  // Recursively generate quadrants
  generateSquareFractal(buffer,
                        row, col, halfSize,
                        c0, nc0, nc1, nc2,
                        newRoughness);
  generateSquareFractal(buffer,
                        row, col+halfSize, halfSize,
                        nc0, c1, nc2, nc3,
                        newRoughness);
  generateSquareFractal(buffer,
                        row+halfSize, col, halfSize,
                        nc1, nc2, c2, nc4,
                        newRoughness);
  generateSquareFractal(buffer,
                        row+halfSize, col+halfSize, halfSize,
                        nc2, nc3, nc4, c3,
                        newRoughness);
}

template <class T>
PiiMatrix<T> PiiCloudFractalGenerator::generateSquareFractal(int size,
                                                             int c0, int c1, int c2, int c3,
                                                             int roughness)
{
  PiiMatrix<T> result(PiiMatrix<T>::uninitialized(size, size));
  
  generateSquareFractal(result,
                        0, 0, size,
                        c0, c1, c2, c3,
                        roughness);

  return result;
}

template <class T>
void PiiCloudFractalGenerator::generateFractal(PiiMatrix<T>& buffer,
                                               int row, int col, int rows, int columns,
                                               int c0, int c1, int c2, int c3,
                                               double roughnessX, double roughnessY)
{
  /* Splits non-square area into a square and two rectangles. The two
   * rectangles will be recursively split with the same algorithm.
   *
   * +---------+-----+
   * |         |     |
   * |    1    |  2  |
   * |         |     |
   * |         |     |
   * +---------+-----+
   * |       3       |
   * +---------------+
   *
   * Points marked with + are corners. See the illustration below for
   * explanation. Number denote generation order.
   */

  // Find largest possible square with power of two sides that fits
  // into the area.
  int size = 1;
  while (rows >= size && columns >= size)
    size <<= 1;
  size >>= 1;
  
  // Terminate recursion if we ended into a 2^N square
  if (rows == columns &&
      rows == size)
    {
      generateSquareFractal(buffer,
                            row, col, rows,
                            c0, c1, c2, c3,
                            int(roughnessX));
      return;
    }

  /* Corner values. c0->nc0->nc2->nc1 forms a square with power-of-two
   * sides.
   *
   *  c0 nc0 c1
   * nc1 nc2 nc3
   *  c2     c3
   *
   * The algorithm handles the square first, then the rectangle framed
   * by nc0->c1->nc3->nc2 (if it exists) and finally nc1->nc3->c3->c2.
   */

  // We first calculate the real center values and linearly scale the
  // slope.
  int halfRows = rows >> 1, halfCols = columns >> 1;

  // Since the new center points (ncX) are not generally in the middle
  // of the current tile, we need to interpolate. These are the
  // interpolation factors for the random middle points.
  int halfRowWeight = Pii::abs(halfRows-size) / halfRows;
  int halfColWeight = Pii::abs(halfCols-size) / halfCols;

  // If roughness in Y direction is not given, calculate it
  // automatically.
  if (roughnessY == 0)
    roughnessY = int(scaleRoughness(rows,columns) * roughnessX);

  // Trap divzeros
  if (roughnessX < 1) roughnessX = 1;
  if (roughnessY < 1) roughnessY = 1;

  // Calculate the "heights" at the centers of each side of the tile
  // and at the center of the tile (nc2).
  int nc0, nc1, nc2, nc3;

  // Smoothly join to existing data if possible.
  if (row > 0)
    nc0 = buffer(row-1,col+size-1);
  else
    {
      nc0 = ((c0 + c1) >> 1) + PII_FRAC_RAND(roughnessX);
      // Interpolate from c0 to nc0 or nc0 to c1
      nc0 = int(PII_FRAC_INTP(size, Col, c0, nc0, c1));
      PII_FRAC_ADJUST_MEAN(nc0);
    }
      
  if (col > 0)
    nc1 = buffer(row+size-1,col-1);
  else
    {
      nc1 = ((c0 + c2) >> 1) + PII_FRAC_RAND(roughnessY);
      nc1 = int(PII_FRAC_INTP(size, Row, c0, nc1, c2));
      PII_FRAC_ADJUST_MEAN(nc1);
    }

  nc2 = ((c0 + c1 + c2 + c3) >> 2) + PII_FRAC_RAND(int(roughnessX + roughnessY + 0.5) >> 1);
  nc2 = int(PII_FRAC_INTP(size, Row, c0, nc2, c3));

  // If target mean value is provided, drive the fractal towards it
  PII_FRAC_ADJUST_MEAN(nc2);

  // Scale roughness value for the square portion
  int squareRoughness = int(scaleRoughness(size,columns) * roughnessX);

  // Generate the large square fractal.
  generateSquareFractal(buffer, row, col, size,
                        c0, nc0,
                        nc1, nc2,
                        squareRoughness);

  int rightSpace = columns-size, bottomSpace = rows-size;

  // Generate the area right to the square's right side
  if (rightSpace > 0)
    {
      nc3 = ((c1 + c3) >> 1) + PII_FRAC_RAND(roughnessY);
      nc3 = int(PII_FRAC_INTP(size, Col, c1, nc3, c3));
      PII_FRAC_ADJUST_MEAN(nc3);
      
      generateFractal(buffer, row, col+size, size, rightSpace,
                      nc0, c1,
                      nc2, nc3,
                      scaleRoughness(rightSpace,columns) * roughnessX,
                      squareRoughness);
    }
  // Otherwise, nc3 coincides with nc2
  else
    nc3 = nc2;

  // Generate possible empty area below the square and its right-hand
  // side.
  if (bottomSpace > 0)
    {
      generateFractal(buffer, row+size, col, bottomSpace, columns,
                      nc1, nc3,
                      c2, c3,
                      roughnessX,
                      scaleRoughness(bottomSpace,rows) * roughnessY);
    }

}

#undef PII_FRAC_RAND
#undef PII_FRAC_TRUNC
#undef PII_FRAC_INTP

#endif //_PIICLOUDFRACTALGENERATOR_H
