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

#ifndef _PIITRANSFORMS_H
#define _PIITRANSFORMS_H

#include <PiiMatrix.h>
#include <PiiMath.h>
#include <PiiMatrixValue.h>
#include <PiiHeap.h>

#include "PiiTransformsGlobal.h"

/**
 * A namespace for functions that transform images into different
 * types of domains.
 *
 * @ingroup PiiTransformsPlugin
 */
namespace PiiTransforms
{
  /**
   * Linear Hough transform. The linear Hough transform is used in
   * detecting lines in images. Lines are paramerized by their slope
   * and distance to the origin, resulting in a two-dimensional
   * transformation domain. In the transformation domain, columns
   * represent slopes and rows distances to the origin. The origin of
   * the domain is at the center of the image. The quantization of the
   * transformation domain is user-specifiable.
   *
   * The number of rows in the result will always be odd. The row at
   * the middle always represents lines that intersect the origin
   * (i.e. distance zero). If @p distanceResolution is 1.0 (the
   * default), moving up or down from the middle row increases
   * distance to the origin by one pixel. The first column represents
   * lines with a zero slope. If @p angleResolution is 1.0 (the
   * default), the next column represents lines with a 1 degree slope
   * and so on. Zero angle points downwards, and the angle grows
   * clockwise. This peculiar choice was made to make it easy to
   * calculate the resulting lines in image coordinates (x is columns
   * and y is rows, positive y axis downwards).
   *
   * Formally, the detected lines are parametrized with the following
   * equation:
   *
   * @f[
   * d = x\cos \theta + y\sin \theta
   * @f]
   *
   * where @f$\theta@f$ stands for the angle, and d is the "distance"
   * from origin (d can be negative). If you pick a value from the
   * transform domain, its row and column coordinates tell the values
   * of d and @f$\theta@f$, respectively. Once these are known, it
   * is straightforward to solve x with respect to y or vice versa.
   *
   * Note that the transform adds the value of a transformed pixel to
   * the transformation domain (a.k.a. the accumulator). If the input
   * image is binary, each pixel will have an equal effect. Higher
   * values can be used in giving higher significance to certain
   * pixels.
   *
   * @param img the input image. The image will be scanned, and each
   * pixel that makes @p evaluator evaluate @p true will be added to
   * the parameter space. Typically, the input image is binary.
   *
   * @param rule the is unary operator which is used to determine if a
   * pixel may be part of a line.
   *
   * @param angleResolution the number of degrees each column
   * represents. The default value is 1.0 which produces 180 columns
   * in the result matrix.
   *
   * @param distanceResolution the distance (in pixels) each row
   * represents.
   *
   * @param angleStart the start angle of the transformation domain.
   * The default value is zero, which places zero angle to the
   * leftmost column. This value can be negative.
   *
   * @param angleEnd the end angle of the transformation domain. This
   * value with @p angleStart can be used to limit the line search to
   * a certain range of angles. Note that the last angle will not be
   * present in the transformation. That is, if @p angleEnd is 180
   * (the default) and @p angleResolution is 1, the last angle will be
   * 179.
   *
   * @param distanceStart the smallest (signed) distance to the origin
   * considered in the transformation. If the distance is smaller than
   * the minimum possible distance, the theoretical minimum will be
   * used.
   *
   * @param distanceEnd the largest (signed) distance to the origin
   * considered in the transformation. If the distance is larger than
   * the maximum possible distance, the theoretical maximum will be
   * used.
   *
   * @return the accumulator array. The size of the returned matrix
   * depends on angle and distance limits and their resolution.
   *
   * @code
   * PiiMatrix<int> img; // fill somehow...
   * // Make the Hough transform. The result will be a PiiMatrix<int>,
   * // and each pixel with a value higher than or equal to three will
   * // be transformed. The angles will be quantized to 90 discrete
   * // levels (2 degrees each)
   *    
   * double distanceResolution = 1.0;
   * double angleResolution = 2.0;
   * PiiMatrix<int> result(PiiTransforms::linearHough<int>(img,
   *                                                       std::bind2nd(std::greater<int>(), 3),
   *                                                       angleResolution,
   *                                                       distanceResolution));
   * @endcode
   */
  template <class T, class U, class UnaryOp>
  PiiMatrix<T> linearHough(const PiiMatrix<U>& img,
                           UnaryOp rule,
                           double angleResolution = 1.0,
                           double distanceResolution = 1.0,
                           int angleStart = 0, int angleEnd = 180,
                           int distanceStart = Pii::Numeric<int>::minValue(),
                           int distanceEnd = Pii::Numeric<int>::maxValue());

  /**
   * Rescale the distance (row) coordinate in a Hough transform
   * accumulator.
   *
   * @param row a row index in the result of a Hough transform.
   *
   * @param rows the number of rows in the source image. Needed if
   * @p distanceStart is not specified.
   *
   * @param columns the number of columns in the source image. Needed
   * if @p distanceStart is not specified.
   *
   * @param distanceResolution the value of the @p distanceResolution
   * parameter used in the Hough transform
   *
   * @param distanceStart the value of the @p distanceStart parameter
   * used in the Hough transform. This value must be specified if you
   * leave @p rows and @p columns to zero.
   *
   * @return the distance ot the origin of the image domain
   * corresponding to @p row in a transformation result, in pixels
   */
  inline double rescaleHoughDistance(int row,
                                     int rows = 0, int columns = 0,
                                     double distanceResolution = 1.0,
                                     double distanceStart = Pii::Numeric<int>::minValue())

  {
    if (rows != 0)
      {
        double maxDistance = Pii::hypotenuse(rows/2, columns/2);
        if (distanceStart < -maxDistance)
          distanceStart = -Pii::round<int>(maxDistance);
      }
    return distanceResolution * row + distanceStart;
  }
  
  /**
   * Rescale the angle (column) coordinate in a Hough transform
   * accumulator.
   *
   * @param column a column index in the result of a Hough transform.
   *
   * @param angleStart the value of the @p angleStart parameter used
   * in the Hough transform.
   *
   * @param angleResolution the value of the @p angleResolution
   * parameter used in the Hough transform
   *
   * @return @p angleResolution * @p column + @p angleStart
   */
  inline double rescaleHoughAngle(int column,
                                  int angleStart = 0,
                                  double angleResolution = 1.0)
  {
    return angleResolution * column + angleStart;
  }


  /**
   * Convert the rescaled peak coordinates @f$d, \theta@f$ into points
   * on the boundary of an image with the given number of rows and
   * columns.
   *
   * @param d the distance of the line to the transform's origin
   *
   * @param theta the angle of the line, in degrees.
   *
   * @return a 1-by-4 matrix storing the start and end points of a
   * line segment (x1, y1, x2, y2). The returned value is suitable for
   * use with PiiImageAnnotator's @p property input.
   *
   * @code
   * using namespace PiiTransforms;
   * // Transform an image
   * PiiMatrix<int> matTransformed(linearHough<int>(img, std::bind2nd(std::greater<int>(), 0)));
   * // Find 10 highest peaks in the transformation domain
   * PiiHeap<PiiMatrixValue<int>,16> maxima = Pii::findMaxima(matTransformed, 10);
   * PiiMatrix<double> matPoints(0,4);
   * for (int i=0; i<maxima.size(); ++i)
   *   matPoints.insertRow(peakToPoints(rescaleHoughDistance(maxima[i].row, img.rows(), img.columns()),
   *                                    rescaleHoughAngle(maxima[i].column), // not necessary
   *                                    img.rows(), img.columns()));
   * @endcode
   *
   * @see Pii::findMaxima()
   */
  PII_TRANSFORMS_EXPORT PiiMatrix<double> peakToPoints(double d, double theta,
                                                       int rows, int columns);

  /**
   * Stores information about detected peaks in the Hough transform.
   *
   * @see Pii::findMaxima()
   */
  typedef PiiHeap<PiiMatrixValue<int>, 16> PeakList;

  /**
   * Find peaks in the Hough transform domain. This function differs
   * from Pii::findMaxima() in that it considers only local maxima
   * and never returns two peaks that are (4-connected) neighbors to
   * each other in the transformation domain.
   *
   * @param transformation the result of a Hough transform
   *
   * @param cnt the number of highest peaks to return
   *
   * @return a list of detected peaks
   */
  PII_TRANSFORMS_EXPORT PeakList findPeaks(PiiMatrix<int>& transformation, int cnt);

  /**
   * A function object that selects pixels whose gradient magnitude is
   * higher than or equal to @a threshold.
   *
   * @see circularHough()
   */
  struct ThresholdSelector
  {
    ThresholdSelector(double threshold) : _dThreshold(threshold*threshold) {}

    template <class T> bool operator() (T magnitude) { return magnitude >= _dThreshold; }
    
  private:
    double _dThreshold;
  };

  /**
   * A function object that randomly selects pixels whose gradient
   * magnitude is higher than or equal to @a threshold.
   *
   * @see circularHough()
   */
  struct RandomSelector : ThresholdSelector
  {
    RandomSelector(double threshold, double selectionProbability) :
      ThresholdSelector(threshold),
      _iLimit(Pii::round<int>(selectionProbability * RAND_MAX))
    {}
    
    template <class T> bool operator() (T magnitude)
    {
      return ThresholdSelector::operator() (magnitude) &&
        rand() < _iLimit;
    }
    
  private:
    int _iLimit;
  };

  enum GradientSign { PositiveGradient = 1, NegativeGradient = 2, IgnoreGradientSign = 3 };
  
	/**
	 * Circular Hough transform. The circular Hough transform detects
	 * circles in images. Circles are parametrized by their center (x,y)
	 * and radius (r), resulting in a three-dimensional transform
	 * domain. This function calculates a slice of the three-dimensional
	 * domain specific to a known radius r.
	 *
   * @param gradientX estimated magnitude of horizontal image
   * gradient. (e.g. image filtered with the @p SobelX filter)
   *
   * @param gradientX estimated magnitude of vertical image gradient
   * (e.g. image filtered with the @p SobelX filter).
   *
   * @param selector a function that is used to accept or reject
   * pixels. Use ThresholdSelector to choose pixels based on the
   * gradient magnitude only. Use RandomSelector if every pixel is not
   * necessarily needed.
   *
   * @param radius the radius of the circles to be found. If this
   * value is a @p double, the returned matrix will also be a double,
   * and a floating-point (more accurate) algorithm will be used. If
   * this value is an @p int, a faster integer algorithm will be used.
   *
   * @param angleError an esimate of angular error in the calculated
   * gradients. Setting @p angleError to a non-zero value will
   * increase accuracy but also slow down the algorithm.
   *
   * @return the transformation domain. Each pixel in the
   * transformation domain stores the number of times it was accessed
   * during the algorithm. With the floating point version, the counts
   * are weighted with gradient magnitude. A high value means that a
   * circle with the given @a radius is centered at that pixel with a
   * high likelihood.
   *
   * @code
   * PiiMatrix<unsigned char> matInput; // input image
   * PiiMatrix<GradType> matGradX(PiiImage::filter<int>(matInput, PiiImage::SobelXFilter));
   * PiiMatrix<GradType> matGradY(PiiImage::filter<int>(matInput, PiiImage::SobelYFilter));
   * using namespace PiiTransforms;
   * PiiMatrix<int> matTransform(circularHough(matGradX, matGradY,
   *                                           ThresholdSelector(5.0),
   *                                           5,
   *                                           0.01));
   * @endcode
	 */
  template <class T, class Selector, class U>
  PiiMatrix<U> circularHough(const PiiMatrix<T>& gradientX,
                             const PiiMatrix<T>& gradientY,
                             Selector select,
                             U radius,
                             double angleError,
                             GradientSign sign = IgnoreGradientSign);
  /**
   * @overload
   *
   * This version uses the Sobel edge detector to first estimate
   * gradient in @a image. Then, it applies circularHough() to all
   * radii in [@a startRadius, @a endRadius] in @a radiusStep steps.
   *
   * @return a transformation domain for each inspected radius
   *
   * @code
   * using namespace PiiTransforms;
   * QList<PiiMatrix<int> > lstMatrices(circularHough(matInput,
   *                                                  RandomSelector(1.0, 0.1), // Selects one tenth of pixels
   *                                                  10, 20, 2)); // Scan radii from 10 to 20 in 2 pixel steps
   * @code
   */
  template <class T, class Selector, class U>
  QList<PiiMatrix<U> > circularHough(const PiiMatrix<T>& image,
                                     Selector select,
                                     U startRadius,
                                     U endRadius,
                                     U radiusStep = 1,
                                     GradientSign sign = IgnoreGradientSign);
}

#include "PiiTransforms-templates.h"

#endif //_PIITRANSFORMS_H
