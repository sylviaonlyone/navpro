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

#ifndef _PIICONCEPTUALMATRIX_H
#define _PIICONCEPTUALMATRIX_H

#include <functional>
#include <algorithm>
#include "PiiFunctional.h"
#include "PiiAlgorithm.h"
#include "PiiIterator.h"
#include "PiiMathException.h"
#include "PiiTypeTraits.h"
#include "PiiMetaTemplate.h"

#define PiiRandomAccessMatrix PiiConceptualMatrix<Matrix, Pii::RandomAccessMatrix>


/**
 * Throw a PiiMathException that complains about non-matching sizes.
 */
#define PII_MATRIX_SIZE_MISMATCH PII_THROW(PiiMathException, Pii::sizeMismatchErrorMessage())

/**
 * Check that the matrices (references) @p a and @p b are of equal
 * size. If they aren't, throw a PiiMathException that complains about
 * non-matching sizes.
 */
#define PII_MATRIX_CHECK_EQUAL_SIZE(a,b) if ((a).rows() != (b).rows() || (a).columns() != (b).columns()) PII_MATRIX_SIZE_MISMATCH

/**
 * Check that matrix @p a is square. If it isn't, throw a
 * PiiMathException.
 */
#define PII_MATRIX_CHECK_SQUARE(a) if ((a).rows() != (a).columns()) PII_THROW(PiiMathException, Pii::notSquareErrorMessage())

namespace Pii
{
  /**
   * Matrix implementation levels. See PiiConceptualMatrix.
   */
  enum MatrixImplementationLevel
    {
      IterableMatrix = 0,
      RandomAccessMatrix = 1
    };

  /// @internal
  PII_CORE_EXPORT QString notSquareErrorMessage();
  /// @internal
  PII_CORE_EXPORT QString sizeMismatchErrorMessage();
}

template <class T, int level=0> struct PiiMatrixTraits;

#define PII_MATRIX_SCALAR_ASSIGNMENT_OPERATOR(OPERATOR, FUNCTION) \
Derived& operator OPERATOR ## = (typename PiiMatrixTraits<Derived>::value_type value) \
{ \
  Pii::map(begin(), end(), \
           std::bind2nd(FUNCTION<typename PiiMatrixTraits<Derived>::value_type>(), value)); \
  return *self(); \
}

#define PII_MATRIX_MATRIX_ASSIGNMENT_OPERATOR(OPERATOR, FUNCTION) \
template <class Matrix> \
Derived& operator OPERATOR ## = (const PiiConceptualMatrix<Matrix>& other) \
{ \
  PII_MATRIX_CHECK_EQUAL_SIZE(*this, other); \
  Pii::map(begin(), end(), other.begin(), \
           FUNCTION<typename PiiMatrixTraits<Derived>::value_type>()); \
  return *self(); \
}

#define PII_BOTH_MATRIX_ASSIGNMENT_OPERATORS(OPERATOR, FUNCTION) \
  PII_MATRIX_MATRIX_ASSIGNMENT_OPERATOR(OPERATOR, FUNCTION) \
  PII_MATRIX_SCALAR_ASSIGNMENT_OPERATOR(OPERATOR, FUNCTION)
/// @endcond

/**
 * A superclass for classes that implement the @e matrix concept. The
 * matrix concept is divided into three @ref
 * Pii::MatrixImplementationLevel "implementation levels":
 *
 * @lip IterableMatrix (level 0) - the matrix provides at least input
 * iterators for accessing the data. The class must have #begin() and
 * #end() functions for getting the iterators. The iterators may be
 * bi-directional or random-access, but only forward iteration is a
 * requirement. At least @p const versions of the iterators must be
 * provided. The iterators must model the <em>forward iterator</em>
 * concept of the standard library, and must scan the matrix in
 * row-major order: the end of row N is immediately followed by the
 * start of row N+1. A level 0 matrix must also define functions
 * called #rows() and #columns(), for getting the number of rows and
 * columns, respectively. This basic functionality is required by
 * almost all matrix algorithms.
 *
 * @lip RandomAccessMatrix (level 1) - the matrix provides
 * iterators for accessing individual rows and columns. See @ref
 * PiiConceptualMatrix<Derived,Pii::RandomAccessMatrix>. This
 * functionality is required by algorithms such as matrix
 * multiplication and transposition.
 *
 * Independent of the implementation level, all models of the matrix
 * concept must provide a specialization of the PiiMatrixTraits
 * structure. The specialization must define types related to the
 * matrix model:
 *
 * @lip value_type for the content type.
 * @lip reference for a reference to the content type.
 * @lip const_iterator for read-only access.
 * @lip iterator for read-write access.
 *
 * Level 1 traits must add the following:
 *
 * @lip column_iterator for read-write column-wise iterators.
 * @lip const_column_iterator for read-only column-wise iterators.
 * @lip row_iterator for read-write row-wise iterators.
 * @lip const_row_iterator for read-only row-wise iterators.
 *
 * If you create a class that models the matrix concept, you can mark
 * it as such as using the curiously recurring template pattern (CRTP)
 * as follows:
 *
 * @code
 * // First declare types related to your matrix implementation. This
 * // needs to come first because PiiConceptualMatrix matrix needs it
 * // when instantiated. We declare the traits as a template wrt to
 * // implementation level so that it will be always used if we later
 * // improve the implementation. Since My3x3Matrix is not declared
 * // yet, a forward declaration is needed.
 * class My3x3Matrix;
 * template <int level> struct PiiMatrixTraits<My3x3Matrix, level>
 * {
 *   typedef int value_type;
 *   typedef int& reference;
 *   typedef int* iterator;
 *   typedef const int* const_iterator;
 * };
 *
 * class My3x3Matrix : public PiiConceptualMatrix<My3x3Matrix>
 * {
 * public:
 *   typedef PiiConceptualMatrix<My3x3Matrix> BaseType;
 *
 *   const_iterator begin() const { return _data; }
 *   const_iterator end() const { return _data + 9; }
 *   int rows() const { return 3; }
 *   int columns() const { return 3; }
 *   // Now we have the minimum functionality.
 *   // Other functions may be added to modify the data etc.
 * private:
 *   int _data[9];
 * };
 * @endcode
 *
 * Once you have done this, your matrix class can be used with other
 * matrix classes in arithmetic operations. For example, you can do
 * the following:
 *
 * @code
 * PiiMatrix<int> mat(3,3);
 * My3x3Matrix mat2;
 * mat += mat2 / 4 + 5;
 * mat -= mat2 + mat;
 * @endcode
 *
 * @ingroup Matrix
 */
template <class Derived, int level = Pii::IterableMatrix>
class PiiConceptualMatrix
{
public:
  enum { implementationLevel = level };
  
  typedef Derived DerivedType;
  typedef PiiMatrixTraits<Derived, level> Traits;
  typedef typename Traits::const_iterator const_iterator;
  typedef typename Traits::iterator iterator;
  typedef typename std::iterator_traits<iterator>::value_type value_type;
  typedef typename std::iterator_traits<iterator>::reference reference;
  
  inline DerivedType* self() { return static_cast<Derived*>(this); }
  inline const DerivedType* self() const { return static_cast<const Derived*>(this); }
  inline DerivedType& selfRef() { return *static_cast<Derived*>(this); }
  inline const DerivedType& selfRef() const { return *static_cast<const Derived*>(this); }

  int rows() const { return self()->rows(); }
  int columns() const { return self()->columns(); }

  /**
   * Returns @p true if the matrix is empty, and @p false otherwise. 
   * An empty matrix cannot hold a single element, i.e. there are
   * either zero rows or zero columns. Any access to an element within
   * an empty matrix will reference illegal memory.
   */
  bool isEmpty() const { return rows() * columns() == 0; }  
  
  const_iterator begin() const { return self()->begin(); }
  const_iterator end() const { return self()->end(); }

  const_iterator constBegin() const { return self()->begin(); }
  const_iterator constEnd() const { return self()->end(); }

  iterator begin() { return self()->begin(); }
  iterator end() { return self()->end(); }

  PII_BOTH_MATRIX_ASSIGNMENT_OPERATORS(+, std::plus)
  PII_BOTH_MATRIX_ASSIGNMENT_OPERATORS(-, std::minus)
  PII_BOTH_MATRIX_ASSIGNMENT_OPERATORS(|, Pii::BinaryOr)
  PII_BOTH_MATRIX_ASSIGNMENT_OPERATORS(&, Pii::BinaryAnd)

  PII_MATRIX_SCALAR_ASSIGNMENT_OPERATOR(*, std::multiplies)
  PII_MATRIX_SCALAR_ASSIGNMENT_OPERATOR(/, std::divides)

  /**
   * Sets all elements to @a value and returns a reference to self().
   */
  Derived& operator= (value_type value)
  {
    std::fill(begin(), end(), value);
    return *self();
  }

  template <class Matrix>
  Derived& operator<< (const PiiConceptualMatrix<Matrix>& other)
  {
    PII_MATRIX_CHECK_EQUAL_SIZE(*this, other);
    std::transform(other.begin(), other.end(), begin(),
                   Pii::Cast<typename Matrix::value_type, value_type>());
    return *self();
  }
};

template <class Derived>
class PiiConceptualMatrix<Derived, Pii::RandomAccessMatrix> :
  public PiiConceptualMatrix<Derived, Pii::IterableMatrix>
{
public:
  enum { implementationLevel = Pii::RandomAccessMatrix };
  
  typedef PiiMatrixTraits<Derived, Pii::RandomAccessMatrix> Traits;
  typedef typename Traits::column_iterator column_iterator;
  typedef typename Traits::const_column_iterator const_column_iterator;
  typedef typename Traits::row_iterator row_iterator;
  typedef typename Traits::const_row_iterator const_row_iterator;

  using PiiConceptualMatrix<Derived>::self;

  const_row_iterator rowBegin(int index) const { return self()->rowBegin(index); }
  const_row_iterator operator[] (int index) const { return self()->rowBegin(index); }
  const_row_iterator rowEnd(int index) const { return self()->rowEnd(index); }
  const_row_iterator constRowBegin(int index) const { return self()->rowBegin(index); }
  const_row_iterator constRowEnd(int index) const { return self()->rowEnd(index); }
  row_iterator rowBegin(int index) { return self()->rowBegin(index); }
  row_iterator operator[] (int index) { return self()->rowBegin(index); }
  row_iterator rowEnd(int index) { return self()->rowEnd(index); }
  const_column_iterator columnBegin(int index) const { return self()->columnBegin(index); }
  const_column_iterator columnEnd(int index) const { return self()->columnEnd(index); }
  const_column_iterator constColumnBegin(int index) const { return self()->columnBegin(index); }
  const_column_iterator constColumnEnd(int index) const { return self()->columnEnd(index); }
  column_iterator columnBegin(int index) { return self()->columnBegin(index); }
  column_iterator columnEnd(int index) { return self()->columnEnd(index); }

  typename Traits::value_type operator() (int r, int c) const { return rowBegin(r)[c]; }
  typename Traits::reference operator() (int r, int c) { return rowBegin(r)[c]; }

  /**
   * Returns a copy of an item in the matrix.
   */
  typename Traits::value_type at(int r, int c) const { return rowBegin(r)[c]; }
};

template <class Matrix, class UnaryFunction> class PiiUnaryMatrixTransform;
template <class Matrix, class UnaryFunction, int level>
struct PiiMatrixTraits<PiiUnaryMatrixTransform<Matrix, UnaryFunction>, level>
{
  typedef typename UnaryFunction::result_type value_type;
  typedef value_type reference;
  typedef PiiUnaryFunctionIterator<typename Matrix::const_iterator, UnaryFunction> const_iterator;
  typedef const_iterator iterator;

  typedef const void* const_row_iterator;
  typedef const void* const_column_iterator;
  typedef const_row_iterator row_iterator;
  typedef const_column_iterator column_iterator;
};

template <class Matrix, class UnaryFunction>
struct PiiMatrixTraits<PiiUnaryMatrixTransform<Matrix, UnaryFunction>, Pii::RandomAccessMatrix> :
  PiiMatrixTraits<PiiUnaryMatrixTransform<Matrix, UnaryFunction>, Pii::IterableMatrix>
{
  typedef PiiUnaryFunctionIterator<typename Matrix::const_row_iterator, UnaryFunction> const_row_iterator;
  typedef PiiUnaryFunctionIterator<typename Matrix::const_column_iterator, UnaryFunction> const_column_iterator;
  typedef const_row_iterator row_iterator;
  typedef const_column_iterator column_iterator;
};

/**
 * A matrix that models the @e matrix concept by applying a unary
 * function to another matrix. Given a model of the matrix concept, @p
 * mat, and a function @p func, this matrix works as if it was the
 * result of calculating <tt>func(mat)</tt>. Note that
 * %PiiUnaryMatrixTransform is immutable. It provides no way of
 * modifying the source data.
 *
 * @see PiiConceptualMatrix
 *
 * @ingroup Matrix
 */
template <class Matrix, class UnaryFunction> class PiiUnaryMatrixTransform :
  public PiiConceptualMatrix<PiiUnaryMatrixTransform<Matrix, UnaryFunction>,
                             Matrix::implementationLevel>
{
public:
  typedef PiiMatrixTraits<PiiUnaryMatrixTransform<Matrix, UnaryFunction>,
                          Matrix::implementationLevel> Traits;
  
  PiiUnaryMatrixTransform(const Matrix& mat, const UnaryFunction& func) :
    _mat(mat), _func(func)
  {}

  PiiUnaryMatrixTransform(const PiiUnaryMatrixTransform& other) :
    _mat(other._mat), _func(other._func)
  {}

  PiiUnaryMatrixTransform& operator= (const PiiUnaryMatrixTransform& other)
  {
    _mat = other._mat;
    _func = other._func;
    return *this;
  }

  int rows() const { return _mat.rows(); }
  int columns() const { return _mat.columns(); }

  typename Traits::value_type operator() (int r, int c) const { return _func(_mat(r,c)); }

  typename Traits::const_iterator begin() const
  {
    return typename Traits::const_iterator(_mat.begin(), _func);
  }
  typename Traits::const_iterator end() const
  {
    return typename Traits::const_iterator(_mat.end(), _func);
  }

  typename Traits::const_row_iterator rowBegin(int index) const
  {
    return typename Traits::const_row_iterator(_mat.rowBegin(index), _func);
  }
  typename Traits::const_column_iterator columnBegin(int index) const
  {
    return typename Traits::const_column_iterator(_mat.columnBegin(index), _func);
  }

private:
  Matrix _mat;
  UnaryFunction _func;
};

template <class Matrix1, class Matrix2, class BinaryFunction> class PiiBinaryMatrixTransform;
template <class Matrix1, class Matrix2, class BinaryFunction, int level>
struct PiiMatrixTraits<PiiBinaryMatrixTransform<Matrix1, Matrix2, BinaryFunction>, level>
{
  typedef PiiBinaryFunctionIterator<typename Matrix1::const_iterator,
                                    typename Matrix2::const_iterator,
                                    BinaryFunction> const_iterator;
  typedef const_iterator iterator;
  typedef typename BinaryFunction::result_type value_type;
  typedef value_type reference;
};

template <class Matrix1, class Matrix2, class BinaryFunction>
struct PiiMatrixTraits<PiiBinaryMatrixTransform<Matrix1, Matrix2, BinaryFunction>, Pii::RandomAccessMatrix> :
  PiiMatrixTraits<PiiBinaryMatrixTransform<Matrix1, Matrix2, BinaryFunction>, Pii::IterableMatrix>
{
  typedef PiiBinaryFunctionIterator<typename Matrix1::const_row_iterator,
                                    typename Matrix2::const_row_iterator,
                                    BinaryFunction> const_row_iterator;
  typedef PiiBinaryFunctionIterator<typename Matrix1::const_column_iterator,
                                    typename Matrix2::const_column_iterator,
                                    BinaryFunction> const_column_iterator;
  typedef const_row_iterator row_iterator;
  typedef const_column_iterator column_iterator;
};


/**
 * A matrix that models the @e matrix concept by using a binary
 * function that operates on two other matrices. Given two models of
 * the matrix concept, @p mat1 and @p mat2, and a binary function @p
 * func, this matrix works as if it was the result of calculating
 * <tt>func(mat1, mat2)</tt>. Note that %PiiBinaryMatrixTransform is
 * immutable. It provides no way of modifying the source data.
 *
 * @see PiiConceptualMatrix
 *
 * @ingroup Matrix
 */
template <class Matrix1, class Matrix2, class BinaryFunction> class PiiBinaryMatrixTransform :
public PiiConceptualMatrix<PiiBinaryMatrixTransform<Matrix1, Matrix2, BinaryFunction>,
                           Pii::MinInt<Matrix1::implementationLevel, Matrix2::implementationLevel>::intValue>
{
public:
  typedef PiiMatrixTraits<PiiBinaryMatrixTransform<Matrix1, Matrix2, BinaryFunction>,
                          Pii::MinInt<Matrix1::implementationLevel, Matrix2::implementationLevel>::intValue> Traits;
  
  PiiBinaryMatrixTransform(const Matrix1& mat1, const Matrix2& mat2, const BinaryFunction& func) :
    _mat1(mat1), _mat2(mat2), _func(func)
  {}

  PiiBinaryMatrixTransform(const PiiBinaryMatrixTransform& other) :
    _mat1(other._mat1), _mat2(other._mat2), _func(other._func)
  {}

  PiiBinaryMatrixTransform& operator= (const PiiBinaryMatrixTransform& other)
  {
    _mat1 = other._mat1;
    _mat2 = other._mat2;
    _func = other._func;
    return *this;
  }

  int rows() const { return _mat1.rows(); }
  int columns() const { return _mat1.columns(); }

  typename Traits::value_type operator() (int r, int c) const { return _func(_mat1(r,c), _mat2(r,c)); }

  typename Traits::const_iterator begin() const
  {
    return typename Traits::const_iterator(_mat1.begin(), _mat2.begin(), _func);
  }
  typename Traits::const_iterator end() const { return typename Traits::const_iterator(_mat1.end(), _mat2.end(), _func); }

  typename Traits::const_row_iterator rowBegin(int index) const
  {
    return typename Traits::const_row_iterator(_mat1.rowBegin(index), _mat2.rowBegin(index), _func);
  }
  typename Traits::const_row_iterator rowEnd(int index) const
  {
    return typename Traits::const_row_iterator(_mat1.rowEnd(index), _mat2.rowEnd(index), _func);
  }
  typename Traits::const_column_iterator columnBegin(int index) const
  {
    return typename Traits::const_column_iterator(_mat1.columnBegin(index), _mat2.columnBegin(index), _func);
  }
  typename Traits::const_column_iterator columnEnd(int index) const
  {
    return typename Traits::const_column_iterator(_mat1.columnEnd(index), _mat2.columnEnd(index), _func);
  }

private:
  Matrix1 _mat1;
  Matrix2 _mat2;
  BinaryFunction _func;
};


namespace Pii
{
  /**
   * Creates a PiiUnaryMatrixTransform that returns @a func(@a mat).
   *
   * @relates PiiUnaryMatrixTransform
   */
  template <class Matrix, class UnaryFunction>
  inline PiiUnaryMatrixTransform<Matrix, UnaryFunction> unaryMatrixTransform(const Matrix& mat,
                                                                             const UnaryFunction& func)
  {
    return PiiUnaryMatrixTransform<Matrix, UnaryFunction>(mat, func);
  }
  
  /**
   * Creates a PiiBinaryMatrixTransform that returns @a func(@a mat1,
   * @a mat2). Note that this function does not check that @a matrix
   * and @a filter are equal in size.
   *
   * @relates PiiBinaryMatrixTransform
   */
  template <class Matrix1, class Matrix2, class BinaryFunction>
  inline PiiBinaryMatrixTransform<Matrix1, Matrix2, BinaryFunction> binaryMatrixTransform(const Matrix1& mat1,
                                                                                          const Matrix2& mat2,
                                                                                          const BinaryFunction& func)
  {
    return PiiBinaryMatrixTransform<Matrix1, Matrix2, BinaryFunction>(mat1, mat2, func);
  }

}

/// @cond null
#define PII_COMBINE_TYPES(T,U) typename Pii::Combine<T,U>::Type

// For operators like operator- () and operator! ()
#define PII_UNARY_MATRIX_OPERATOR(OPERATOR, FUNCTION) \
template <class Matrix> \
PiiUnaryMatrixTransform<Matrix, FUNCTION<typename Matrix::value_type> > \
operator OPERATOR (const PiiConceptualMatrix<Matrix>& matrix) \
{ \
  return Pii::unaryMatrixTransform(*matrix.self(), FUNCTION<typename Matrix::value_type>()); \
} namespace PiiDummy {}

// For operators like operator+ (matrix, scalar)
#define PII_MATRIX_SCALAR_OPERATOR(OPERATOR, FUNCTION) \
template <class Matrix> \
PiiUnaryMatrixTransform<Matrix, std::binder2nd<FUNCTION<typename Matrix::value_type> > > \
operator OPERATOR (const PiiConceptualMatrix<Matrix>& matrix, typename Matrix::value_type value) \
{ \
  return Pii::unaryMatrixTransform(*matrix.self(), std::bind2nd(FUNCTION<typename Matrix::value_type>(), value)); \
} namespace PiiDummy {}

// For operators like operator+ (matrix, matrix)
#define PII_MATRIX_MATRIX_OPERATOR(OPERATOR, FUNCTION) \
template <class Matrix1, class Matrix2> \
PiiBinaryMatrixTransform<Matrix1, Matrix2, FUNCTION<PII_COMBINE_TYPES(typename Matrix1::value_type, \
                                                                      typename Matrix2::value_type)> > \
operator OPERATOR (const PiiConceptualMatrix<Matrix1>& matrix1, const PiiConceptualMatrix<Matrix2>& matrix2) \
{ \
  PII_MATRIX_CHECK_EQUAL_SIZE(matrix1, matrix2); \
  return Pii::binaryMatrixTransform(*matrix1.self(), *matrix2.self(),    \
                                    FUNCTION<PII_COMBINE_TYPES(typename Matrix1::value_type, \
                                                               typename Matrix2::value_type)>()); \
}

#define PII_BOTH_MATRIX_OPERATORS(OPERATOR, FUNCTION) \
  PII_MATRIX_MATRIX_OPERATOR(OPERATOR, FUNCTION); \
  PII_MATRIX_SCALAR_OPERATOR(OPERATOR, FUNCTION)

PII_BOTH_MATRIX_OPERATORS(+, std::plus);
PII_BOTH_MATRIX_OPERATORS(-, std::minus);

PII_MATRIX_SCALAR_OPERATOR(*, std::multiplies);
PII_MATRIX_SCALAR_OPERATOR(/, std::divides);

PII_BOTH_MATRIX_OPERATORS(<, std::less);
PII_BOTH_MATRIX_OPERATORS(<=, std::less_equal);
PII_BOTH_MATRIX_OPERATORS(>, std::greater);
PII_BOTH_MATRIX_OPERATORS(>=, std::greater_equal);
PII_BOTH_MATRIX_OPERATORS(==, std::equal_to);
PII_BOTH_MATRIX_OPERATORS(!=, std::not_equal_to);

PII_BOTH_MATRIX_OPERATORS(&&, std::logical_and);
PII_BOTH_MATRIX_OPERATORS(||, std::logical_or);

PII_UNARY_MATRIX_OPERATOR(-, std::negate);
PII_UNARY_MATRIX_OPERATOR(!, std::logical_not);
PII_UNARY_MATRIX_OPERATOR(~, Pii::BinaryNot);

template <class Matrix>
PiiUnaryMatrixTransform<Matrix, std::binder2nd<std::plus<typename Matrix::value_type> > >
operator+ (typename Matrix::value_type value, const PiiConceptualMatrix<Matrix>& matrix)
{
  return Pii::unaryMatrixTransform(*matrix.self(), std::bind2nd(std::plus<typename Matrix::value_type>(), value));
}

template <class Matrix>
PiiUnaryMatrixTransform<Matrix,
                        Pii::UnaryCompose<std::binder2nd<std::plus<typename Matrix::value_type> >,
                                          std::negate<typename Matrix::value_type> > >
operator- (typename Matrix::value_type value, const PiiConceptualMatrix<Matrix>& matrix)
{
  typedef typename Matrix::value_type T;
  return Pii::unaryMatrixTransform(*matrix.self(),
                                   Pii::unaryCompose(std::bind2nd(std::plus<T>(), value),
                                                     std::negate<T>()));
}

template <class Matrix>
PiiUnaryMatrixTransform<Matrix, std::binder2nd<std::multiplies<typename Matrix::value_type> > >
operator* (typename Matrix::value_type value, const PiiConceptualMatrix<Matrix>& matrix)
{
  return Pii::unaryMatrixTransform(*matrix.self(), std::bind2nd(std::multiplies<typename Matrix::value_type>(), value));
}

/// @endcond

namespace Pii
{
  /**
   * Compare two matrices. Matrices are equal if all of their entries
   * are equal. All entries are equal if the memory allocated by them
   * matches byte-by-byte.
   */
  template <class Matrix1, class Matrix2> bool equals(const PiiConceptualMatrix<Matrix1>& mat1,
                                                      const PiiConceptualMatrix<Matrix2>& mat2)
  {
    if (mat1.rows() != mat2.rows() || mat1.columns() != mat2.columns())
      return false;
    return std::equal(mat1.begin(), mat1.end(), mat2.begin());
  }
}

#endif //_PIICONCEPTUALMATRIX_H
