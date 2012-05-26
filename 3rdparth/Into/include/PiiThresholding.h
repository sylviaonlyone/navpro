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

#ifndef _PIITHRESHOLDING_H
#define _PIITHRESHOLDING_H

#include <PiiMatrix.h>
#include "PiiHistogram.h"
#include "PiiLabeling.h"

/**
 * @file
 * @ingroup PiiImagePlugin
 *
 * Image thresholding routines.
 */


namespace PiiImage
{
  /**
   * The Otsu method is used in automatically selecting an optimal
   * threshold. The Otsu method works best with histograms that have
   * two modes: dark objects on bright background or vice-versa.
   *
   * @param histogram normalized histogram
   *
   * @return optimal threshold for separating the two modes
   *
   * @code
   * using namespace PiiImage;
   * int iThreshold = otsuThreshold(normalize<float>(histogram(image)));
   * @endcode
   */
  template <class T> int otsuThreshold(const PiiMatrix<T>& histogram);

  /**
   * %ThresholdFunction compares two values and outputs either 0 or 1
   * based on the comparison result. You can use this struct for
   * example with PiiMatrix<T>::map():
   *
   * @code
   * int threshold = 5;
   * PiiMatrix<int> mat(1,9,1,2,3,4,5,6,7,8,9);
   * mat.map<PiiImage::ThresholdFunction<int> >(threshold);
   *
   * // mat = 0 0 0 0 1 1 1 1 1
   *
   * const PiiMatrix<int> mat2(1,9,1,2,3,4,5,6,7,8,9);
   * PiiMatrix<bool> binary = mat2.mapped(PiiImage::ThresholdFunction<int,bool>(), 5);
   *
   * // binary = 0 0 0 0 1 1 1 1 1
   * @endcode
   */
  template <class T, class U = T> struct ThresholdFunction : public Pii::BinaryFunction<T,T,U>
  {
    ThresholdFunction(U value=1) : _value(value) {}
    U operator() (T value, T threshold) const { return value < threshold ? U(0) : _value; }

  private:
    const U _value;
  };

  /**
   * %InverseThresholdFunction compares two values and outputs either 0
   * or 1 based on the comparison result. Works analogously to
   * ThresholdFunction, but outputs one if @p value is smaller than or
   * equal to @p threshold. You can use this struct for
   * example with PiiMatrix<T>::binaryOp():
   *
   * @code
   * int threshold = 5;
   * PiiMatrix<int> mat(1,9,1,2,3,4,5,6,7,8,9);
   * mat.binaryOp(PiiImage::InverseThresholdFunction<int>(), threshold);
   *
   * // mat = 1 1 1 1 0 0 0 0 0
   * @endcode
   */
  template <class T, class U = T> struct InverseThresholdFunction : public Pii::BinaryFunction<T,T,U>
  {
    InverseThresholdFunction(U value=1) : _value(value) {}
    U operator() (T value, T threshold) const { return value >= threshold ? U(0) : _value; }

  private:
    const U _value;
  };

  /**
   * %TwoLevelThresholdFunction compares its input argument to two
   * thresholds and returns one if the argument is in between them
   * (inclusive).
   *
   * @code
   * PiiMatrix<int> mat(1,9,1,2,3,4,5,6,7,8,9);
   * mat.unaryOp(PiiImage::TwoLevelThresholdFunction<int>(5, 7));
   *
   * // mat = 0 0 0 0 1 1 1 0 0
   * @endcode
   */
  template <class T, class U = T> class TwoLevelThresholdFunction : public Pii::BinaryFunction<T,T,U>
  {
  public:
    /**
     * Contruct a two-level threshold function that requires returns
     * one iff the input value is between the given two values
     * (inclusive).
     */
    TwoLevelThresholdFunction(T lowThreshold, T highThreshold, U value=1) :
      _lowThreshold(lowThreshold), _highThreshold(highThreshold), _value(value) {}
    U operator() (T value) const { return value >= _lowThreshold && value <= _highThreshold ? _value : U(0); }

  private:
    T _lowThreshold, _highThreshold;
    U _value;
  };

  /**
   * %InverseTwoLevelThresholdFunction works analogously to
   * TwoLevelThresholdFunction, but inverses the result.
   *
   * @code
   * PiiMatrix<int> mat(1,9,1,2,3,4,5,6,7,8,9);
   * mat.unaryOp(PiiImage::InverseTwoLevelThresholdFunction<int>(5, 7));
   *
   * // mat = 1 1 1 1 0 0 0 1 1
   * @endcode
   */
  template <class T, class U = T> class InverseTwoLevelThresholdFunction : public Pii::BinaryFunction<T,T,U>
  {
  public:
    /**
     * Contruct a two-level threshold function that requires returns
     * zero iff the input value is between the given two values
     * (inclusive).
     */
    InverseTwoLevelThresholdFunction(T lowThreshold, T highThreshold, U value=1) :
      _lowThreshold(lowThreshold), _highThreshold(highThreshold), _value(value) {}
    U operator() (T value) const { return value >= _lowThreshold && value <= _highThreshold ? U(0) : _value; }

  private:
    T _lowThreshold, _highThreshold;
    U _value;
  };

  /**
   * CutFunction compares @p value to a @p threshold and outputs either
   * @p value or @p threshold based on the comparison result. You can
   * use this struct for example with PiiMatrix<T>::binaryOp():
   *
   * @code
   * int threshold = 5;
   * PiiMatrix<int> mat(1,9,1,2,3,4,5,6,7,8,9);
   * mat.binaryOp(PiiImage::CutFunction<int>(), threshold);
   *
   * // mat = 1 2 3 4 5 5 5 5 5
   * @endcode
   */
  template <class T, class U = T> struct CutFunction : public Pii::BinaryFunction<T,T,U>
  {
    U operator() (T value, T threshold) const { return value < threshold ? U(value) : U(threshold); }
  };

  /**
   * InverseCutFunction compares @p value to a @p threshold and
   * outputs either @p value or @p threshold based on the comparison
   * result. You can use this struct for example with
   * PiiMatrix<T>::binaryOp():
   *
   * @code
   * int threshold = 5;
   * PiiMatrix<int> mat(1,9,1,2,3,4,5,6,7,8,9);
   * mat.binaryOp(PiiImage::InverseCutFunction<int>(), threshold);
   *
   * // mat = 5 5 5 5 5 6 7 8 9
   * @endcode
   */
  template <class T, class U = T> struct InverseCutFunction : public Pii::BinaryFunction<T,T,U>
  {
    U operator() (T value, T threshold) const { return value < threshold ? U(threshold) : U(value); }
  };

  /**
   * ZeroBelowFunction compares @p value to a @p threshold and outputs
   * either @p value or 0 based on the comparison result. You can use
   * this struct for example with PiiMatrix<T>::binaryOp():
   *
   * @code
   * int threshold = 5;
   * PiiMatrix<int> mat(1,9,1,2,3,4,5,6,7,8,9);
   * mat.binaryOp(PiiImage::ZeroBelowFunction<int>(), threshold);
   *
   * // mat = 0 0 0 0 5 6 7 8 9
   * @endcode
   */
  template <class T, class U = T> struct ZeroBelowFunction : public Pii::BinaryFunction<T,T,U>
  {
    U operator() (T value, T threshold) const { return value < threshold ? U(0) : U(value); }
  };

  /**
   * ZeroAboveFunction works analogously to ZeroBelowFunction, but
   * outputs zero for values that are above or equal to the threshold.
   *
   * @code
   * int threshold = 5;
   * PiiMatrix<int> mat(1,9,1,2,3,4,5,6,7,8,9);
   * mat.binaryOp(PiiImage::ZeroAboveFunction<int>(), threshold);
   *
   * // mat = 1 0 1 2 3 4 0 0 0 0 0
   * @endcode
   */
  template <class T, class U = T> struct ZeroAboveFunction : public Pii::BinaryFunction<T,T,U>
  {
    U operator() (T value, T threshold) const { return value < threshold ? U(value) : U(0); }
  };
  
  /**
   * DropFunction compares @p value to a @p threshold and outputs
   * either @p value - @p threshold or 0 based on the comparison
   * result. The function "drops" gray levels above a threshold to
   * zero. You can use this struct for example with
   * PiiMatrix<T>::binaryOp():
   *
   * @code
   * int threshold = 5;
   * PiiMatrix<int> mat(1,9,1,2,3,4,5,6,7,8,9);
   * mat.binaryOp(PiiImage::DropFunction<int>(), threshold);
   *
   * // mat = 0 0 0 0 0 1 2 3 4
   * @endcode
   */
  template <class T, class U = T> struct DropFunction : public Pii::BinaryFunction<T,T,U>
  {
    U operator() (T value, T threshold) const { return value < threshold ? U(0) : U(value-threshold); }
  };

  template <class ThresholdFunc>
  struct AdaptiveThresholdFunction :
    Pii::BinaryFunction<typename ThresholdFunc::first_argument_type,
                        float,
                        typename ThresholdFunc::result_type>
  {
    AdaptiveThresholdFunction(const ThresholdFunc& func,
                              float relativeThreshold,
                              float absoluteThreshold) :
      _func(func),
      _relativeThreshold(relativeThreshold),
      _absoluteThreshold(absoluteThreshold)
    {}

    typename ThresholdFunc::result_type operator() (typename ThresholdFunc::first_argument_type pixel, float mean) const
    {
      return _func(pixel, _relativeThreshold * mean + _absoluteThreshold);
    }
    
    ThresholdFunc _func;
    float _relativeThreshold, _absoluteThreshold;
  };

  template <class ThresholdFunc> AdaptiveThresholdFunction<ThresholdFunc>
  adaptiveThresholdFunction(const ThresholdFunc& func,
                            float relativeThreshold,
                            float absoluteThreshold)
  {
    return AdaptiveThresholdFunction<ThresholdFunc>(func, relativeThreshold, absoluteThreshold);
  }
  
  /**
   * Threshold an image.
   *
   * @param image original image.
   *
   * @param threshold gray level value for thresholding the original image
   *
   * @return thresholded binary image
   *
   * @see ThresholdFunction
   */
  template <class T> inline PiiMatrix<T> threshold(const PiiMatrix<T>& image, T threshold)
  {
    return image.mapped(ThresholdFunction<T>(), threshold);
  }

  /**
   * Threshold and invert an image.
   *
   * @param image original image.
   *
   * @param threshold gray level value for thresholding the original image
   *
   * @return thresholded binary image
   *
   * @see InverseThresholdFunction
   */
  template <class T> inline PiiMatrix<T> inverseThreshold(const PiiMatrix<T>& image, T threshold)
  {
    return image.mapped(InverseThresholdFunction<T>(), threshold);
  }

  /**
   * Cut gray levels. Every pixel above @p threshold will be set to @p
   * threshold.
   *
   * @see CutFunction
   */
  template <class T> inline PiiMatrix<T> cut(const PiiMatrix<T>& image, T threshold)
  {
    return image.mapped(CutFunction<T>(), threshold);
  }

  /**
   * Inversely cut gray levels. Every pixel below @p threshold will be
   * set to @p threshold.
   *
   * @see InverseCutFunction
   */
  template <class T> inline PiiMatrix<T> inverseCut( const PiiMatrix<T>& image, T threshold )
  {
    return image.mapped(InverseCutFunction<T>(), threshold);
  }

  /**
   * Zero values below threshold.
   *
   * @see ZeroBelowFunction
   */
  template <class T> inline PiiMatrix<T> zeroBelow(const PiiMatrix<T>& image, T threshold)
  {
    return image.mapped(ZeroBelowFunction<T>(), threshold);
  }

  /**
   * Zero values above or equal to threshold.
   *
   * @see ZeroAboveFunction
   */
  template <class T> inline PiiMatrix<T> zeroAbove(const PiiMatrix<T>& image, T threshold)
  {
    return image.mapped(ZeroAboveFunction<T>(), threshold);
  }

  /**
   * Perform hysteresis thresholding on image. This is a convenience
   * function that uses #labelImage() to find connected components in
   * which the gray level of all pixels is greater than or equal to @p
   * lowThreshold and and at least one pixel is greater than or equal
   * to @p highThreshold.
   *
   * @param image a gray-level image
   *
   * @param lowThreshold low threshold value
   *
   * @param highThreshold high threshold value
   *
   * @param connectivity the type of connected components
   *
   * @return a matrix in which all thresholded pixels are ones and
   * others are zeros.
   */
  template <class T> inline PiiMatrix<int> hysteresisThreshold(const PiiMatrix<T>& image,
                                                               T lowThreshold, T highThreshold,
                                                               Connectivity connectivity = Connect8)
  {
    return labelImage(image,
                      std::bind2nd(std::greater_equal<T>(), lowThreshold),
                      std::bind2nd(std::greater_equal<T>(), highThreshold),
                      connectivity,
                      0);
  }

  /**
   * Inverse hysteresis thresholding. Works inversely to
   * hysteresisThreshold(): connected components in which all gray
   * levels are below @p highThreshold and at least one gray level is
   * below @p lowThreshold will be set to one. Note that the result is
   * @b not the same as inverting the result of hysteresisThreshold().
   *
   * @param image a gray-level image
   *
   * @param lowThreshold low threshold value
   *
   * @param highThreshold high threshold value
   *
   * @param connectivity the type of connected components
   *
   * @return a matrix in which all thresholded pixels are ones and
   * others are zeros.
   */
  template <class T> inline PiiMatrix<int> inverseHysteresisThreshold(const PiiMatrix<T>& image,
                                                                      T lowThreshold, T highThreshold,
                                                                      Connectivity connectivity = Connect8)
  {
    return labelImage(image,
                      std::bind2nd(std::less<T>(), highThreshold),
                      std::bind2nd(std::less<T>(), lowThreshold),
                      connectivity,
                      0);
  }

  /**
   * Threshold an image adaptively. The threshold is calculated
   * separately for each pixel based on the local mean. The local mean
   * @f$\mu@f$ is calculated over an area determined by @p windowRows
   * and @p windowColumns. The final threshold @e t is obtained by
   * @f$t = \mu * r + a@f$, where @e r denotes @p relativeThreshold
   * and @e a @p absoluteThreshold. If @p windowColumns is
   * non-positive, the same value will be used for both window
   * dimensions.
   *
   * @param image the input image
   *
   * @param func a binary function whose return value replaces the
   * pixel value in @p image. Invoked as @f$\mathrm{func}(p,t)@f$,
   * where @p is the value of a pixel and @p t is a locally calculated
   * threshold. The type of the threshold is @p float for all other
   * image types except @p double, in which case a @p double will be
   * passed.
   *
   * @param relativeThreshold multiply local average with this value
   *
   * @param absoluteThreshold add this value to the multiplication
   * result
   *
   * @param windowRows the number of rows in the local averaging
   * window
   *
   * @param windowColumns the number of columns in the local averaging
   * window. If this value is non-positive, a @p windowRows - by - @p
   * windowRows square window will be used.
   *
   * @note Window size has no effect on processing time.
   *
   * @code
   * using namespace PiiImage;
   * PiiMatrix<float> img;
   * // Threshold at 0.8 times the local mean. Using
   * // InverseThresholdFunction as function sets values lower than
   * // the threshold to one.
   * img = adaptiveThreshold(img, InverseThresholdFunction<float>(), 0.8, 0.0, 65);
   * @endcode
   *
   * @note If the input image is integer-typed, you can avoid rounding
   * errors by providing a different input type for the thresholding
   * function, for example ThresholdFunction<float,int>.
   */
  template <class Matrix, class BinaryFunction>
  PiiMatrix<typename BinaryFunction::result_type> adaptiveThreshold(const PiiRandomAccessMatrix& image,
                                                                    BinaryFunction func,
                                                                    float relativeThreshold,
                                                                    float absoluteThreshold,
                                                                    int windowRows, int windowColumns = 0);

  /**
   * @overload This version calls @a func using the pixel value and
   * the local mean as parameters.
   */
  template <class Matrix, class BinaryFunction>
  PiiMatrix<typename BinaryFunction::result_type> adaptiveThreshold(const PiiRandomAccessMatrix& image,
                                                                    BinaryFunction func,
                                                                    int windowRows, int windowColumns = 0);
  /**
   * @overload This version only considers pixels whose corresponding
   * entry in @a roiMask evaluates to @p true. The @a image and @a
   * roiMask matrices must match in size.
   */
  template <class Matrix, class BinaryFunction>
  PiiMatrix<typename BinaryFunction::result_type> adaptiveThreshold(const PiiRandomAccessMatrix& image,
                                                                    const PiiMatrix<bool>& roiMask,
                                                                    BinaryFunction func,
                                                                    int windowRows, int windowColumns = 0);
  
  /**
   * A function that calculates a local threshold @e t as @f$t = \mu +
   * r \sigma@f$, where @e r stands for @p relativeThreshold. The
   * output of the function is <em>thresholdFunc(transformFunc(pixel),
   * t)</em>.
   *
   * @see adaptiveThreshold()
   */
  template <class BinaryFunction, class UnaryFunction>
  struct MeanStdThresholdFunction :
    Pii::TernaryFunction<typename UnaryFunction::argument_type,
                         double,
                         double,
                         typename BinaryFunction::result_type>
  {
    MeanStdThresholdFunction(const BinaryFunction& thresholdFunc,
                             const UnaryFunction& transformFunc,
                             double relativeThreshold) :
      _thresholdFunc(thresholdFunc),
      _transformFunc(transformFunc),
      _dR(relativeThreshold)
    {}

    typename BinaryFunction::result_type operator() (typename UnaryFunction::argument_type pixel,
                                                     double mu,
                                                     double var) const
    {
      return _thresholdFunc(_transformFunc(pixel),
                            typename BinaryFunction::second_argument_type(mu + _dR * sqrt(var)));
    }

  private:
    const BinaryFunction& _thresholdFunc;
    const UnaryFunction& _transformFunc;
    double _dR;
  };

  /**
   * Create a threshold function.
   *
   * @relates MeanStdThresholdFunction
   */
  template <class BinaryFunction, class UnaryFunction>
  MeanStdThresholdFunction<BinaryFunction, UnaryFunction> meanStdThresholdFunction(const BinaryFunction& threshold,
                                                                                   const UnaryFunction& transform,
                                                                                   double r)
  {
    return MeanStdThresholdFunction<BinaryFunction, UnaryFunction>(threshold, transform, r);
  }

  /**
   * A function that calculates a local threshold @e t as @f$t = \mu
   * (1 + r (\sigma/\sigma_{\mathrm max} - 1))@f$, where @e r stands
   * for @p relativeThreshold and @f$\sigma_{\mathrm max}@f$ for the
   * maximum possible standard deviation. The output of the function
   * is <em>thresholdFunc(transformFunc(pixel), t)</em>.
   *
   * @see adaptiveThreshold()
   */
  template <class BinaryFunction, class UnaryFunction>
  struct SauvolaThresholdFunction :
    Pii::TernaryFunction<typename UnaryFunction::argument_type,
                         double,
                         double,
                         typename BinaryFunction::result_type>
  {
    SauvolaThresholdFunction(const BinaryFunction& thresholdFunc,
                             const UnaryFunction& transformFunc,
                             double relativeThreshold,
                             double maxStd) :
      _thresholdFunc(thresholdFunc),
      _transformFunc(transformFunc),
      _dR(relativeThreshold),
      _dMaxStd(maxStd)
    {}
    
    typename BinaryFunction::result_type operator() (typename UnaryFunction::argument_type pixel,
                                                     double mu,
                                                     double var) const
    {
      return _thresholdFunc(_transformFunc(pixel),
                            typename BinaryFunction::second_argument_type(mu * (1 + _dR * (sqrt(var)/_dMaxStd - 1))));
    }
    
  private:
    const BinaryFunction& _thresholdFunc;
    const UnaryFunction& _transformFunc;
    double _dR, _dMaxStd;
  };

  /// @internal
  template <class T> struct MaximumStd
  {
    struct Int { static double value() { return 128; } };
    struct Float { static double value() { return 0.5; } };
    
    static double value()
    {
      return Pii::IfClass<Pii::IsInteger<T>, Int, Float>::Type::value();
    }
  };
  
  /**
   * Create a threshold function.
   *
   * @param threshold a thresholding function, such as
   * ThresholdFunction
   *
   * @param transform a function that transforms input data before
   * applying the threshold. If no transform is needed, use
   * Pii::Identity.
   *
   * @param r weight of local deviation
   *
   * @param maxStd maximum standar deviation. The default value is 0.5
   * for floating point types and 128 for others.
   *
   * @relates SauvolaThresholdFunction
   */
  template <class BinaryFunction, class UnaryFunction>
  SauvolaThresholdFunction<BinaryFunction, UnaryFunction> sauvolaThresholdFunction(const BinaryFunction& threshold,
                                                                                   const UnaryFunction& transform,
                                                                                   double r,
                                                                                   double maxStd = MaximumStd<typename UnaryFunction::argument_type>::value())
  {
    return SauvolaThresholdFunction<BinaryFunction, UnaryFunction>(threshold, transform, r, maxStd);
  }
  
  /**
   * Threshold an image adaptively. This function calculates local
   * mean and local variance for each pixel, and uses @p func as the
   * thresholding function.
   *
   * @param image the input image
   *
   * @param func a ternary function whose return value replaces the
   * pixel value in @p image. Invoked as
   * @f$\mathrm{func}(p,\mu,\sigma^2)@f$, where @f$p, \mu, \sigma^2@f$
   * stand for the pixel value, the local mean, and the local
   * variance, respectively.
   *
   * @param windowRows the number of rows in the local averaging
   * window
   *
   * @param windowColumns the number of columns in the local averaging
   * window. If this value is non-positive, a @p windowRows - by - @p
   * windowRows square window will be used.
   *
   * @code
   * // Document binarization
   * using namespace PiiImage;
   * PiiMatrix<int> image;
   * image = adaptiveThresholdVar(image,
   *                              sauvolaThresholdFunction(ThresholdFunction<int>(),
   *                                                       Pii::Identity<int>(),
   *                                                       0.34, 255),
   *                              31);
   * @endcode
   *
   * The #sauvolaThresholdFunction() and #meanStdThresholdFunction()
   * helper functions can be used to create suitable thresholding
   * functions.
   */
  template <class TernaryFunction, class T>
  PiiMatrix<typename TernaryFunction::result_type> adaptiveThresholdVar(const PiiMatrix<T>& image,
                                                                        TernaryFunction func,
                                                                        int windowRows, int windowColumns = 0);
}

#include "PiiThresholding-templates.h"

#endif //_PIITHRESHOLDING_H
