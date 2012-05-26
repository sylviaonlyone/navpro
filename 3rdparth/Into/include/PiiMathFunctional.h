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

#ifndef _PIIMATHFUNCTIONAL_H
#define _PIIMATHFUNCTIONAL_H

#ifndef _PIIMATH_H
# error "Never use <PiiMathFunctional.h> directly; include <PiiMath.h> instead."
#endif

namespace Pii
{
  /**
   * An adaptable unary function for taking the absolute value.
   */
  template <class T> struct Abs : public Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return ::Pii::abs(value); }
  };

  /**
   * An adaptable unary function for taking the absolute value of a
   * complex number.
   */
  template <class T> struct Abs<std::complex<T> > : public Pii::UnaryFunction<std::complex<T>, T>
  {
    T operator() (const std::complex<T>& value) const { return std::abs(value); }
  };

  /**
   * An adaptable unary function for taking the absolute value of the
   * square of a real number.
   */
  template <class T> struct Abs2 : public Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return ::Pii::abs2(value); }
  };

  /**
   * An adaptable unary function for taking the absolute value of the
   * square of a complex number.
   */
  template <class T> struct Abs2<std::complex<T> > : public Pii::UnaryFunction<std::complex<T>, T>
  {
    T operator() (const std::complex<T>& value) const { return std::norm(value); }
  };
  
  /**
   * An adaptable unary function that returns the complex conjugate of
   * a real number (which is the argument itself) or a complex number.
   */
  template <class T> struct Conj : public Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return ::Pii::conj(value); }
  };
  
  /**
   * An adaptable binary function that raises the absolute value of its
   * first argument to the power of the second argument.
   *
   * @code
   * PiiMatrix<double> mat(...); // Initialize as you want
   *
   * // For each element, set m(i,j) = |m(i,j)|^n
   * mat.binaryOp(Pii::AbsPow<double>(), n);
   * @endcode
   */
  //template <class T, class U = T> struct AbsPow : public Pii::BinaryFunction<T,U>
  template <class T,
            class U = typename Pii::Abs<T>::result_type,
            class R = typename Pii::Abs<T>::result_type>
  struct AbsPow : public Pii::BinaryFunction<T,U,R>
  {
    R operator() (const T& value, const U& power) const { return pow(Pii::abs(value), power); }
  };

  /**
   * An adaptable binary function that calculates the sum of absolute
   * values, @f$|x| + |y|@f$.
   */
  template <class T> struct AbsSum : public Pii::BinaryFunction<T>
  {
    T operator() (const T& x, const T& y) const { return abs(x) + abs(y); }
  };

  /**
   * An adaptable binary function that calculates the absolute
   * difference of its arguments, @f$|x - y|@f$.
   */
  template <class T> struct AbsDiff : public Pii::BinaryFunction<T>
  {
    T operator() (const T& x, const T& y) const { return abs(x - y); }
  };

  /**
   * An adaptable binary function that calculates the squared
   * difference of its arguments, @f$(x - y)^2@f$.
   */
  template <class T> struct SquaredDiff : public Pii::BinaryFunction<T>
  {
    T operator() (const T& x, const T& y) const { T diff = (x - y); return diff*diff; }
  };

  /**
   * An adaptable unary function for calculating the natural logarithm.
   */
  template <class T> struct Log : public Pii::UnaryFunction<T,typename RealReturnValue<T>::Type>
  {
    typename RealReturnValue<T>::Type operator() (const T& value) const { return log(value); }
  };

  /**
   * An adaptable unary function for calculating the base-e
   * exponential.
   */
  template <class T> struct Exp : public Pii::UnaryFunction<T,typename RealReturnValue<T>::Type>
  {
    typename RealReturnValue<T>::Type operator() (const T& value) const { return exp(value); }
  };

  /**
   * An adaptable binary function that raises its first argument to the
   * power of the second argument.
   */
  template <class T, class U = T> struct Pow : public Pii::BinaryFunction<T,U>
  {
    T operator() (const T& value, const U& power) const { return pow(value, power); }
  };

  /**
   * An adaptable unary function that returns the square of its
   * argument.
   */
  template <class T> struct Square : public Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return value*value; }
  };

  /**
   * A unary function that returns the square root of its argument.
   */
  template <class T> struct Sqrt : public Pii::UnaryFunction<T,double>
  {
    double operator() (const T& value) const { return sqrt(value); }
  };
  /**
   * A specialization for floats. Sometimes faster than the @p double
   * version if the compiler supports the C99 standard.
   */
  template <> struct Sqrt<float> : public Pii::UnaryFunction<float>
  {
    float operator() (float value) const { return sqrt(value); }
  };

  /**
   * A unary function that returns @p sin(x).
   */
  template <class T> struct Sin : public Pii::UnaryFunction<T,double>
  {
    double operator() (const T& value) const { return sin(value); }
  };
  /**
   * A specialization for floats. Sometimes faster than the @p double
   * version if the compiler supports the C99 standard.
   */
  template <> struct Sin<float> : public Pii::UnaryFunction<float>
  {
    float operator() (float value) const { return sin(value); }
  };

  /**
   * A unary function that returns @p asin(x).
   */
  template <class T> struct Asin : public Pii::UnaryFunction<T,double>
  {
    double operator() (const T& value) const { return asin(value); }
  };
  /**
   * A specialization for floats. Sometimes faster than the @p double
   * version if the compiler supports the C99 standard.
   */
  template <> struct Asin<float> : public Pii::UnaryFunction<float>
  {
    float operator() (float value) const { return asin(value); }
  };

  /**
   * A unary function that returns @p cos(x).
   */
  template <class T> struct Cos : public Pii::UnaryFunction<T,double>
  {
    double operator() (const T& value) const { return cos(value); }
  };
  /**
   * A specialization for floats. Sometimes faster than the @p double
   * version if the compiler supports the C99 standard.
   */
  template <> struct Cos<float> : public Pii::UnaryFunction<float>
  {
    float operator() (float value) const { return cos(value); }
  };

  /**
   * A unary function that returns @p acos(x).
   */
  template <class T> struct Acos : public Pii::UnaryFunction<T,double>
  {
    double operator() (const T& value) const { return acos(value); }
  };
  /**
   * A specialization for floats. Sometimes faster than the @p double
   * version if the compiler supports the C99 standard.
   */
  template <> struct Acos<float> : public Pii::UnaryFunction<float>
  {
    float operator() (float value) const { return acos(value); }
  };

  /**
   * A unary function that returns @p tan(x).
   */
  template <class T> struct Tan : public Pii::UnaryFunction<T,double>
  {
    double operator() (const T& value) const { return tan(value); }
  };
  /**
   * A specialization for floats. Sometimes faster than the @p double
   * version if the compiler supports the C99 standard.
   */
  template <> struct Tan<float> : public Pii::UnaryFunction<float>
  {
    float operator() (float value) const { return tan(value); }
  };

  /**
   * A unary function that returns @p tan(x).
   */
  template <class T> struct Atan : public Pii::UnaryFunction<T,double>
  {
    double operator() (const T& value) const { return atan(value); }
  };
  /**
   * A specialization for floats. Sometimes faster than the @p double
   * version if the compiler supports the C99 standard.
   */
  template <> struct Atan<float> : public Pii::UnaryFunction<float>
  {
    float operator() (float value) const { return atan(value); }
  };

  /**
   * An adaptable binary function that calculates the arc tangent of two
   * variables.
   */
  template <class T> struct Atan2 : public Pii::BinaryFunction<T,T,double>
  {
    double operator() (const T& y, const T& x) const { return ::atan2(y, x); }
  };

  /**
   * Specialization for @p float arguments.
   */
  template <> struct Atan2<float> : public Pii::BinaryFunction<float>
  {
    float operator() (float y, float x) const { return ::atan2(y, x); }
  };

  /**
   * An adaptable binary function that uses a fast approximation (0.01
   * rad accuracy) to calculate the arc tangent of two variables.
   */
  template <class T> struct FastAtan2 : public Pii::BinaryFunction<T,T,float>
  {
    float operator() (const T& y, const T& x) const { return atan2((float)y, (float)x); }
  };

  /**
   * An adaptable binar function that uses a very fast but crude
   * approximation (0.07 rad accuracy) to calculate the arc tangent of
   * two variables.
   */
  template <class T> struct FastestAtan2 : public Pii::BinaryFunction<T,T,float>
  {
    float operator() (const T& y, const T& x) const { return fastAtan2((float)y, (float)x); }
  };

  /**
   * An adaptable binary function that calculates the hypotenuse given
   * two orthogonal sides of a triangle, @f$\sqrt{x^2 + y^2}@f$.
   */
  template <class T> struct Hypotenuse : public Pii::BinaryFunction<T>
  {
    T operator() (const T& x, const T& y) const { return T(sqrt(x*x + y*y)); }
  };

  /**
   * Specialization for integer arguments.
   */
  template<> struct Hypotenuse<int> : public Pii::BinaryFunction<int>
  {
    int operator() (int x, int y) const { return round<int>(sqrt(float(x*x) + float(y*y))); }
  };

  /**
   * A unary function that returns @p ceil(x).
   */
  template <class T> struct Ceil : public Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return ceil(value); }
  };

  /**
   * A unary function that returns @p floor(x).
   */
  template <class T> struct Floor : public Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return floor(value); }
  };
  
  /**
   * An adaptable unary function that rounds its argument to the closest
   * integer. Template parameter @p T determines the input type and @p
   * U the output type. The output type can be omitted as it defaults
   * to the input type.
   */
  template <class From, class To = From> struct Round : public Pii::UnaryFunction<From,To>
  {
    To operator() (const From& value) const { return round<To>(value); }
  };

  /**
   * An adaptable unary function that returns the inverse of its
   * argument (1/x).
   */
  template <class T> struct Invert : public Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return 1/value; }
  };

  /**
   * An adaptable binary function that returns the minimum of its two
   * arguments.
   */
  template <class T> struct Min : public Pii::BinaryFunction<T>
  {
    T operator() (const T& x, const T& y) const { return min(x,y); }
  };

  /**
   * An adaptable binary function that returns the maximum of its two
   * arguments.
   */
  template <class T> struct Max : public Pii::BinaryFunction<T>
  {
    T operator() (const T& x, const T& y) const { return max(x,y); }
  };

  /**
   * An adaptable unary function that calculates the sum of its
   * arguments in successive calls and records the number of times the
   * function has been called. As such, the function can be used to
   * calculate the mean value of data.
   *
   * @code
   * // Calculate the mean value of a matrix:
   * PiiMatrix<float> matrix(3,3);
   * Pii::Mean<float> mean;
   * Pii::forEach(matrix, mean);
   * std::cout << mean.mean() << std::endl;
   * @endcode
   */
  template <class T, class U = T> class Mean : public Pii::UnaryFunction<T,U>
  {
  public:
    /**
     * Create a new %Mean function that initializes its accumulator to
     * the given value.
     */
    Mean(U initialValue = 0) : _accumulator(initialValue), _iInvocations(0) {}
    
    /**
     * Increment the invocation count and returns the current sum of
     * values.
     */
    U operator() (const T& value) const
    {
      ++_iInvocations;
      return _accumulator += value;
    }

    /**
     * Get the number of times the function has been called.
     */
    int invocations() const { return _iInvocations; }

    /**
     * Calculate the current mean value by dividing the accumulator by
     * the number of invocations.
     */
    U mean() const { return _accumulator / _iInvocations; }
    
  private:
    mutable U _accumulator;
    mutable int _iInvocations;
  };

  /**
   * A unary function that returns the real part of its argument. The
   * default implementation returns its argument.
   */
  template <class T> struct Real : public Pii::UnaryFunction<T>
  {
    T operator() (const T& arg) const { return arg; }
  };
  
  /**
   * A unary function that returns the real part of its argument. This
   * specialization returns the real part of a complex number.
   */
  template <class T> struct Real<std::complex<T> > : public Pii::UnaryFunction<std::complex<T>,T>
  {
    T operator() (const std::complex<T>& arg) const { return arg.real(); }
  };

  /**
   * A unary function that returns the imaginary part of its argument. 
   * The default implementation returns zero.
   */
  template <class T> struct Imag : public Pii::UnaryFunction<T>
  {
    T operator() (const T&) const { return 0; }
  };
  
  /**
   * A unary function that returns the imaginary part of its argument. 
   * This specialization returns the imaginary part of a complex
   * number.
   */
  template <class T> struct Imag<std::complex<T> > : public Pii::UnaryFunction<std::complex<T>,T>
  {
    T operator() (const std::complex<T>& arg) const { return arg.imag(); }
  };

  /**
   * A unary function that returns the sign (1, 0, or -1) of its
   * argument.
   */
  template <class T> struct Sign : public Pii::UnaryFunction<T>
  {
    T operator() (const T& value) const { return sign(value); }
  };

  /**
   * A unary function that bounds its argument to a predefined range.
   * If the argument is less than @a min, @a min will be returned. If
   * the argument is larger than @a max, @a max will be returned.
   * Otherwise, the argument will be returned unchanged.
   */
  template <class T> struct Bound : public Pii::UnaryFunction<T>
  {
    Bound(T min, T max) : _min(min), _max(max) {}
    T operator() (const T& value) const { return qBound(_min, value, _max); }

  private:
    T _min, _max;
  };

  /**
   * Helper function for algorithms that involve scrolling elements.
   *
   * @tparam FuItem: Unary functor, takes (matrix) element as argument and the return value is used as second parameter in FuMem.
   * @tparam FuMem : Binaryfunctor: takes memoryobject and result of FuItem as arguments, and returns memoryobject.
   */
  template<class FuItem, class FuMemory>
  struct ForEachFunction
  {
  public:
    /**
     * @param fItem Functor 'reads' item and returns value to be handled by memorymodifier functor.
     * @param fMem Functor that handles internal memory updating.
     * @param initMem Initial value for memory.
     *
     * @note see @ref createForEachFunction() for creating object of this type more easily.
     */
    ForEachFunction(FuItem fItem, FuMemory fMem, typename FuMemory::result_type initMem)
      : _fuItem(fItem), _fuMem(fMem), _Mem(initMem) {}

    /**
     * Calls unary functor FuItem on (matrix) element, and passes its return value to 
     * second argument of binary functor FuMemory, whose return value in turn is set as
     * internal memory. 
     */
    void operator()(const typename FuItem::argument_type& val) {_Mem = _fuMem(_Mem, _fuItem(val));}

    /**
     * Returns memory object.
     *
     * @return Functor 'memory'.
     */
    typename FuMemory::result_type operator()() {return _Mem;}

  private:
    FuItem _fuItem;
    FuMemory _fuMem;
    typename FuMemory::result_type _Mem;
  };

  /**
   * Helper function for creating object of type ForEachFunction.
   *
   * @relates ForEachFunction
   */
  template<class FuItem, class FuMemory>
  ForEachFunction<FuItem, FuMemory> createForEachFunction(FuItem fi, FuMemory fm,
                                                          typename FuMemory::result_type initVal)
  {
    return ForEachFunction<FuItem, FuMemory>(fi, fm, initVal);
  }
}

#endif //_PIIMATHFUNCTIONAL_H
