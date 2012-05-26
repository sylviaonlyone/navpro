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

#ifndef _PIIFUNCTIONAL_H
#define _PIIFUNCTIONAL_H

/**
 * @file
 *
 * This file contains useful function objects missing from the
 * standard library. The @p typedefs required in adaptable functions
 * are provided both in STL and in Intopii/Qt style. The function
 * adaptors are modeled after SGI extensions to STL.
 */

namespace Pii
{
  /**
   * An stl-compatible model for a generator function that takes no
   * arguments.
   */
  template <class Result> struct Generator
  {
    /**
     * Typedef for the result type.
     */
    typedef Result ResultType;
    /**
     * Stl-style typedef for the result type.
     */
    typedef Result result_type;
  };

  /**
   * An stl-compatible model for an adaptable unary function.
   */
  template <class Arg, class Result = Arg> struct UnaryFunction
  {
    /**
     * Typedef for the argument type.
     */
    typedef Arg ArgumentType;
    /**
     * Stl-style typedef for the argument type.
     */
    typedef Arg argument_type;
    /**
     * Typedef for the result type.
     */
    typedef Result ResultType;
    /**
     * Stl-style typedef for the result type.
     */
    typedef Result result_type;
  };
  
  /**
   * An stl-compatible model for an adaptable binary function.
   */
  template <class Arg1, class Arg2 = Arg1, class Result = Arg1> struct BinaryFunction
  {
    /**
     * Typedef for the first argument type.
     */
    typedef Arg1 FirstArgumentType;
    /**
     * Stl-style typedef for the first argument type.
     */
    typedef Arg1 first_argument_type;
    /**
     * Typedef for the second argument type.
     */
    typedef Arg2 SecondArgumentType;
    /**
     * Stl-style typedef for the second argument type.
     */
    typedef Arg2 second_argument_type;
    /**
     * Typedef for the result type.
     */
    typedef Result ResultType;
    /**
     * Stl-style typedef for the result type.
     */
    typedef Result result_type;
  };

  /**
   * A model for an adaptable ternary function.
   */
  template <class Arg1, class Arg2 = Arg1, class Arg3 = Arg1, class Result = Arg1> struct TernaryFunction
  {
    /**
     * Typedef for the first argument type.
     */
    typedef Arg1 FirstArgumentType;
    /**
     * Stl-style typedef for the first argument type.
     */
    typedef Arg1 first_argument_type;
    /**
     * Typedef for the second argument type.
     */
    typedef Arg2 SecondArgumentType;
    /**
     * Stl-style typedef for the second argument type.
     */
    typedef Arg2 second_argument_type;
    /**
     * Typedef for the third argument type.
     */
    typedef Arg3 ThirdArgumentType;
    /**
     * Stl-style typedef for the third argument type.
     */
    typedef Arg3 third_argument_type;
    /**
     * Typedef for the result type.
     */
    typedef Result ResultType;
    /**
     * Stl-style typedef for the result type.
     */
    typedef Result result_type;
  };

  /**
   * An adaptable binary functor that performs the binary OR operation.
   */
  template <class T> struct BinaryOr : public BinaryFunction<T>
  {
    T operator() (const T& a, const T& b) const { return a | b; }
  };
  /**
   * An adaptable binary functor that performs the binary AND
   * operation.
   */
  template <class T> struct BinaryAnd : public BinaryFunction<T>
  {
    T operator() (const T& a, const T& b) const { return a & b; }
  };
  /**
   * An adaptable binary functor that performs the binary XOR
   * operation.
   */
  template <class T> struct BinaryXor : public BinaryFunction<T>
  {
    T operator() (const T& a, const T& b) const { return a ^ b; }
  };
  /**
   * An adaptable binary unary functor that performs the binary NOT
   * operation.
   */
  template <class T> struct BinaryNot : public BinaryFunction<T>
  {
    T operator() (const T& a) const { return ~a; }
  };
  /**
   * An adaptable binary binary functor that shifts the left operand
   * left as many times as indicated by the right operand.
   */
  template <class T> struct BinaryShiftLeft : public BinaryFunction<T,int>
  {
    T operator() (const T& a, int b) const { return a << b; }
  };
  /**
   * An adaptable binary binary functor that shifts the left operand
   * right as many times as indicated by the right operand.
   */
  template <class T> struct BinaryShiftRight : public BinaryFunction<T,int>
  {
    T operator() (const T& a, int b) const { return a >> b; }
  };

  /**
   * An adaptable unary function that casts its argument from type @p
   * From to type @p To.
   */
  template <class From, class To> struct Cast : public UnaryFunction<From,To>
  {
    To operator() (const From& value) const { return To(value); }
  };
  
  /**
   * A unary function adaptor that makes the result of one unary
   * function the argument of another. If function 1 is f(x) and
   * function 2 is g(x), the composition returns f(g(x)).
   *
   * @code
   * PiiMatrix<double> mat(1,5, 1.0, 2.0, 3.0, 4.0, 5.0);
   * // Calculate -sin(x)
   * mat.map(Pii::unaryCompose(std::negate(), std::ptr_fun(sin)));
   * @endcode
   *
   * @see unaryCompose()
   * @see BinaryCompose
   */
  template <class AdaptableUnaryFunction1, class AdaptableUnaryFunction2>
  class UnaryCompose : public UnaryFunction<typename AdaptableUnaryFunction2::argument_type,
                                            typename AdaptableUnaryFunction1::result_type>
  {
  public:
    /**
     * Construct a UnaryCompose object. It is seldom necessary to use
     * the constructor directly. Use the @ref unaryCompose() function
     * instead.
     */
    UnaryCompose(const AdaptableUnaryFunction1& op1,
                 const AdaptableUnaryFunction2& op2) :
      firstOp(op1), secondOp(op2)
    {}

    typename AdaptableUnaryFunction1::result_type operator()(const typename AdaptableUnaryFunction2::argument_type& value) const
    {
      return firstOp(secondOp(value));
    }

    /**
     * The first function.
     */
    AdaptableUnaryFunction1 firstOp;
    /**
     * The first function.
     */
    AdaptableUnaryFunction2 secondOp;
  };

  /**
   * Construct a composed unary function (UnaryCompose) out of two
   * adaptable unary functions.
   *
   * @relates UnaryCompose
   */
  template <class AdaptableUnaryFunction1, class AdaptableUnaryFunction2>
  inline UnaryCompose<AdaptableUnaryFunction1, AdaptableUnaryFunction2>
  unaryCompose(const AdaptableUnaryFunction1& op1, const AdaptableUnaryFunction2& op2)
  {
    return UnaryCompose<AdaptableUnaryFunction1,AdaptableUnaryFunction2>(op1, op2);
  }

  /**
   * A unary/binary function adaptor that makes the results of two
   * unary functions the arguments of a binary function. If the binary
   * function is f(x,y) and the two unary functions are g(x) and h(x),
   * the unary composition returns f(g(x),h(x)). If the object is used
   * as a binary function, the composition returns f(g(x), h(y)).
   *
   * @code
   * PiiMatrix<double> mat(1,5, 1.0, 2.0, 3.0, 4.0, 5.0);
   * // Calculate sin(mat) + cos(mat)
   * mat.map(Pii::binaryCompose(std::plus<double>(), std::ptr_fun(sin), std::ptr_fun(cos)));
   *
   * PiiMatrix<double> mat2(1,5, -1.0, 2.0, -3.0, 4.0, -5.0);
   * // Calculate mat - sqrt(mat2)
   * mat.map(Pii::binaryCompose(std::minus<double>(), Pii::Identity<double>, Pii::Sqrt<double>()),
   *         mat2);
   * @endcode
   *
   * @see binaryCompose()
   * @see UnaryCompose
   */
  template <class AdaptableBinaryFunction, class AdaptableUnaryFunction1, class AdaptableUnaryFunction2>
  class BinaryCompose :
    public UnaryFunction<typename AdaptableUnaryFunction1::argument_type,
                         typename AdaptableBinaryFunction::result_type>,
    public BinaryFunction<typename AdaptableUnaryFunction1::argument_type,
                          typename AdaptableUnaryFunction2::argument_type,
                          typename AdaptableBinaryFunction::result_type>
    
  {
  public:
    /**
     * Construct a %BinaryCompose object. It is seldom necessary to use
     * the constructor directly. Use the @ref binaryCompose() function
     * instead.
     */
    BinaryCompose(const AdaptableBinaryFunction& op1,
                  const AdaptableUnaryFunction1& op2,
                  const AdaptableUnaryFunction2& op3) :
      firstOp(op1), secondOp(op2), thirdOp(op3)
    {}

    typename AdaptableBinaryFunction::result_type operator()
      (const typename AdaptableUnaryFunction1::argument_type& value) const
    {
      return firstOp(secondOp(value), thirdOp(value));
    }

    typename AdaptableBinaryFunction::result_type operator()
      (const typename AdaptableUnaryFunction1::argument_type& value1,
       const typename AdaptableUnaryFunction2::argument_type& value2) const
    {
      return firstOp(secondOp(value1), thirdOp(value2));
    }

    /**
     * The first function.
     */
    AdaptableBinaryFunction firstOp;
    /**
     * The second function.
     */
    AdaptableUnaryFunction1 secondOp;
    /**
     * The third function.
     */
    AdaptableUnaryFunction2 thirdOp;
  };

  
  /**
   * Construct a composed unary function (BinaryCompose) out of an
   * adaptable binary function and two adaptable unary functions.
   *
   * @relates BinaryCompose
   */
  template <class AdaptableBinaryFunction, class AdaptableUnaryFunction1, class AdaptableUnaryFunction2>
  inline BinaryCompose<AdaptableBinaryFunction, AdaptableUnaryFunction1, AdaptableUnaryFunction2>
  binaryCompose(const AdaptableBinaryFunction& op1,
                const AdaptableUnaryFunction1& op2,
                const AdaptableUnaryFunction2& op3)
  {
    return BinaryCompose<AdaptableBinaryFunction, AdaptableUnaryFunction1, AdaptableUnaryFunction2>(op1, op2, op3);
  }

  /**
   * A unary function that increments/decrements its internal counter
   * by a predefined value each time the function is called.
   *
   * @code
   * PiiMatrix<float> matrix(1,3);
   * // Fill matrix
   * Pii::fill(matrix.begin(), matrix.end(), Pii::CountFunction<float>(0,2));
   * // matrix = 0 2 4
   * @endcode
   */
  template <class T> class CountFunction : public Generator<T>
  {
  public:
    /**
     * Create a new count function with the given initial value and
     * increment.
     */
    CountFunction(int initialValue = 0, int increment = 1) : _iCount(initialValue), _iIncrement(increment) {}

    /**
     * Increase the internal counter by the increment.
     */
    T operator() () const
    {
      int iCnt = _iCount;
      _iCount += _iIncrement;
      return iCnt;
    }

    /**
     * Get the current value of the internal counter.
     */
    int count() const { return _iCount; }

    /**
     * Set the value of the internal counter.
     */
    void setCount(int count) { _iCount = count; }
    
    /**
     * Get value of the increment.
     */
    int increment() const { return _iIncrement; }

    /**
     * Set the value of the increment.
     */
    void setIncrement(int increment) { _iIncrement = increment; }

  private:
    mutable int _iCount;
    int _iIncrement;
  };

  /**
   * A unary function that returns its argument: f(x) = x.
   */
  template <class T> struct Identity : public UnaryFunction<T>
  {
    T operator()(const T& value) const { return value; }
  };

  /**
   * A binary function that returns its first argument: f(x,y) = x.
   */
  template <class T, class U=T> struct SelectFirst : public BinaryFunction<T,U>
  {
    T operator()(const T& value1, const U&) const { return value1; }
  };

  /**
   * A binary function that returns its second argument: f(x,y) = y.
   */
  template <class T, class U=T> struct SelectSecond : public BinaryFunction<T,U,U>
  {
    U operator()(const T&, const U& value2) const { return value2; }
  };
  
  /**
   * A unary function (predicate) that always returns @p true.
   */
  template <class T> struct YesFunction : public UnaryFunction<T,bool>
  {
    bool operator()(const T&) const { return true; }
  };
  
  /**
   * A unary function (predicate) that always returns @p false.
   */
  template <class T> struct NoFunction : public UnaryFunction<T,bool>
  {
    bool operator()(const T&) const { return false; }
  };
};

#endif //_PIIFUNCTIONAL_H
