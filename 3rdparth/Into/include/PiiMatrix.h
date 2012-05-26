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

#ifndef _PIIMATRIX_H
#define _PIIMATRIX_H

#include "PiiMatrixData.h"
#include "PiiFunctional.h"
#include "Pii.h"
#include "PiiConceptualMatrix.h"
#include "PiiFilteredMatrix.h"
#include <functional>
#include <algorithm>

/// @cond null
template <class T> class PiiMatrixIterator;
template <class T> class PiiMatrixColumnIterator;

template <class T, int rowCount, int columnCount> class PiiMatrix;
template <class T, int rowCount, int columnCount, int level>
struct PiiMatrixTraits<PiiMatrix<T, rowCount, columnCount>, level>
{
  typedef T value_type;
  typedef T& reference;
  typedef T* iterator;
  typedef const T* const_iterator;
  typedef PiiMatrixColumnIterator<T> column_iterator;
  typedef PiiMatrixColumnIterator<const T> const_column_iterator;
  typedef T* row_iterator;
  typedef const T* const_row_iterator;
};
/// @endcond

template <class T, int rowCount = -1, int columnCount = -1> class PiiMatrix :
  public PiiConceptualMatrix<PiiMatrix<T, rowCount, columnCount>, Pii::RandomAccessMatrix>
{
public:
  typedef PiiMatrixTraits<PiiMatrix<T, rowCount, columnCount>, Pii::RandomAccessMatrix> Traits;
  typedef typename Pii::VaArg<T>::Type VaArgType;

  PiiMatrix()
  {
    memset(_data, 0, byteCount());
  }
  
  PiiMatrix(const PiiMatrix& other)
  {
    memcpy(_data, other._data, byteCount());
  }
  
  PiiMatrix(VaArgType firstElement, ...)
  {
    va_list argp;
    va_start(argp, firstElement);
    Pii::copyVaList(begin(), end(), firstElement, argp);
    va_end(argp);
  }

  PiiMatrix& operator= (const PiiMatrix& other)
  {
    memcpy(_data, other._data, byteCount());
    return *this;
  }
  PiiMatrix& operator= (T value) { return PiiConceptualMatrix<PiiMatrix<T,rowCount,columnCount> >::operator= (value); }

  int rows() const { return rowCount; }
  int columns() const { return columnCount; }

  T* begin() { return _data; }
  const T* begin() const { return _data; }
  T* end() { return row(rowCount); }
  const T* end() const { return row(rowCount); }
  
  T* rowBegin(int r) { return row(r); }
  const T* rowBegin(int r) const { return row(r); }
  T* rowEnd(int r) { return row(r+1); }
  const T* rowEnd(int r) const { return row(r+1); }

  typename Traits::column_iterator columnBegin(int colIndex)
  {
    return typename Traits::column_iterator(_data + colIndex, stride());
  }
  typename Traits::const_column_iterator columnBegin(int colIndex) const
  {
    return typename Traits::const_column_iterator(_data + colIndex, stride());
  }
  typename Traits::column_iterator columnEnd(int colIndex)
  {
    return typename Traits::column_iterator(row(rowCount) + colIndex, stride());
  }
  typename Traits::const_column_iterator columnEnd(int colIndex) const
  {
    return typename Traits::const_column_iterator(row(rowCount) + colIndex, stride());
  }

  T* row(int r) { return _data + r*columnCount; }
  const T* row(int r) const { return _data + r*columnCount; }

  static int stride() { return columnCount * sizeof(T); }

private:
  static inline int byteCount() { return rowCount * columnCount * sizeof(T); }
  
  T _data[rowCount * columnCount];
};

/**
 * A two-dimensional dynamic array of non-typed data. This class
 * provides basic functionality for accessing the data of a matrix in
 * a type-agnostic manner. PiiMatrix provides type-specific functions
 * and usual matrix manipulation operations.
 *
 * %PiiTypelessMatrix holds a pointer to a data structure that may be
 * shared among many matrices. The class implements the
 * @e copy-on-write paradigm; all copies are shallow until a modification
 * is going to happen. The data will be @ref PiiMatrix::detach() "detached"
 * in the beginning of all non-const functions.
 *
 * @ingroup Matrix
 */
class PII_CORE_EXPORT PiiTypelessMatrix
{
public:
  /// Releases the internal data pointer.
  ~PiiTypelessMatrix() { d->release(); }

  /**
   * Returns the number of rows in the matrix.
   */
  int rows() const { return d->iRows; }
  /**
   * Returns the number of columns in the matrix.
   */
  int columns() const { return d->iColumns; }

  /**
   * Returns the number of bytes between the beginnings of successive
   * rows. The stride may be different from sizeof(datatype) *
   * columns() for two reasons:
   *
   * @li Matrix rows are aligned to four-byte boundaries. For example,
   * if the data type is @p char, and the matrix has three columns
   * (three bytes per row), @e stride will be four.
   *
   * @li The matrix references external data. In this case the stride
   * may be anything, but always larger than or equal to the number of
   * columns.
   */
  int stride() const { return d->iStride; }

  /**
   * Returns the maximum number or rows that can be stored in the
   * matrix without reallocation. If the matrix references external
   * data, the capacity is zero.
   */
  int capacity() const { return d->iCapacity; }

  /**
   * Releases all memory allocated by the matrix and resizes the
   * matrix to 0-by-0.
   */
  void clear();

protected:
  /// @cond null
  PiiTypelessMatrix() : d(PiiMatrixData::sharedNull()) { d->reserve(); }

  PiiTypelessMatrix(const PiiTypelessMatrix& other) : d(other.d) { d->reserve(); }
  
  PiiTypelessMatrix(PiiMatrixData* data) : d(data) {}
  
  PiiMatrixData* createReference(int rows, int columns, void* buffer) const;
  void cloneAndReplaceData(int capacity, int bytesPerRow);

  void* appendRow(int bytesPerRow);
  void* insertRow(int index, int bytesPerRow);
  void removeRow(int index, int bytesPerRow);
  void removeRows(int index, int cnt, int bytesPerRow);
  void reserve(int rows, int bytesPerRow);
  void* appendColumn(int bytesPerItem);
  void* insertColumn(int index, int bytesPerItem);
  void removeColumn(int index, int bytesPerItem);
  void removeColumns(int index, int cnt, int bytesPerItem);
  void resize(int rows, int columns, int bytesPerItem);

  PiiMatrixData* d;
  /// @endcond
};

/// @cond null
template <class T, int level> struct PiiMatrixTraits<PiiMatrix<T,-1,-1>, level>
{
  typedef T value_type;
  typedef T& reference;
  typedef PiiMatrixIterator<T> iterator;
  typedef PiiMatrixIterator<const T> const_iterator;
  typedef PiiMatrixColumnIterator<T> column_iterator;
  typedef PiiMatrixColumnIterator<const T> const_column_iterator;
  typedef T* row_iterator;
  typedef const T* const_row_iterator;
};

template <class T> class PiiSubmatrix;
template <class T, int level> struct PiiMatrixTraits<PiiSubmatrix<T>, level> :
  PiiMatrixTraits<PiiMatrix<T,-1,-1>, level>
{
};
/// @endcond


/**
 * A two-dimensional array that models the @e matrix concept.
 *
 * %PiiMatrix only supports POD (plain old data) types as the content
 * type. It will call neither constructors nor destructors. The data of
 * a matrix is cleared by simply setting all bytes to zero, and
 * assignments may be performed with memcpy(). If matrix atrithmetic
 * is to be performed, the corresponding operators of the content type
 * must be defined. Furthermore, for some operations, there must be a
 * constructor for a single numeric argument. An example of a class
 * that can be used as the content type:
 *
 * @code
 * struct MyClass
 * {
 *   MyClass(int i=0, double d=0.0);
 *
 *   void operator+= (const MyClass&);
 *   void operator-= (const MyClass&);
 *   void operator*= (const MyClass&);
 *   void operator/= (const MyClass&);
 *   MyClass operator+ (const MyClass&) const;
 *   MyClass operator- (const MyClass&) const;
 *   MyClass operator* (const MyClass&) const;
 *   MyClass operator/ (const MyClass&) const;
 *
 *   int iValue;
 *   double dValue;
 * };
 *
 *
 * PiiMatrix<MyClass> mat(1,1); //resets both ivalue and dvalue to zero
 * MyClass mc(1, 2.0);
 * mat(0,0) = mc;
 * mat *= mc;
 * @endcode
 *
 * Row and column indices in PiiMatrix are always zero-based. The
 * convention in matrix math is to index rows first, and that is what
 * PiiMatrix also does. For performance reasons there is no bound
 * checking. Thus, make sure you don't reference memory outside of the
 * matrix.
 *
 * Most routines that perform matrix math may throw a
 * PiiMathException. Such an exception may be thrown if the sizes of
 * two matrices do not match for calculation.
 *
 * The data within a matrix is organized so that the items in a row
 * (scan line) always occupy adjacent memory locations. The pointer to
 * the beginning to each row is returned by #row(int). Each row is
 * aligned at a four-byte boundary (unless initialized with an
 * external non-aligned buffer). Therefore, if the data type is less
 * than four bytes wide, it may happen that rows are not stored
 * sequentially.
 *
 * Usually, matrices use an internally allocated buffer to store the
 * elements. It is however possible to create matrices that refer to
 * other matrices or externally allocated buffers. A modifiable
 * reference to another matrix is created by the
 * operator(int,int,int,int). Changing the contents of such a matrix
 * will change the contents of the original matrix. Reference can be
 * chained; a reference to a reference modifies the original.
 *
 * @code
 * PiiMatrix<char> mat(8,8);
 * mat = '+';
 * mat(2,1,3,3) = 'a';
 * mat(1,5,6,3) = 'b';
 * mat(2,6,4,1) = 'c';
 * mat(6,0,2,1) = 'x';
 * 
 * //mat looks like this now:
 * + + + + + + + +
 * + + + + + b b b
 * + a a a + b c b
 * + a a a + b c b
 * + a a a + b c b
 * + + + + + b c b
 * x + + + + b b b
 * x + + + + + + +
 * @endcode
 *
 * @ingroup Matrix
 */
template <class T> class PiiMatrix<T,-1,-1> :
  public PiiTypelessMatrix,
  public PiiConceptualMatrix<PiiMatrix<T,-1,-1>, Pii::RandomAccessMatrix>
{
public:
  typedef PiiMatrixTraits<PiiMatrix<T,-1,-1>, Pii::RandomAccessMatrix> Traits;

  /**
   * Type for arguments passed through va_args.
   */
  typedef typename Pii::VaArg<T>::Type VaArgType;

  /**
   * Constructs an empty matrix.
   */
  PiiMatrix() {}

  /**
   * Constucts a shallow copy of @a other. This constructor only
   * increases the reference count of the internal data structure.
   *
   * @code
   * PiiMatrix<int> a(5,5);
   * PiiMatrix<int> b(a(1,1,2,2)); // 2-by-2 reference to a
   * PiiMatrix<int> c(a);
   * PiiMatrix<int> c(b);          // deep copy of b
   * @endcode
   */
  PiiMatrix(const PiiMatrix& other) : PiiTypelessMatrix(other) {}

  /**
   * Constructs a deep copy of @a other by copying and typecasting
   * each individual element.
   */
  template <class Matrix> explicit PiiMatrix(const PiiConceptualMatrix<Matrix>& other) :
    PiiTypelessMatrix(PiiMatrixData::createUninitializedData(other.rows(),
                                                             other.columns(),
                                                             other.columns() * sizeof(T)))
  {
    Pii::transform(other.begin(), other.end(), begin(), Pii::Cast<typename Matrix::value_type,T>());
  }
  
  /**
   * Constucts a @a rows-by-@a columns matrix with all entries
   * initialized to zero.
   */
  PiiMatrix(int rows, int columns) :
    PiiTypelessMatrix(PiiMatrixData::createInitializedData(rows, columns, columns * sizeof(T)))
  {}

  /**
   * Constructs a @a rows-by-@a columns matrix whose initial contents
   * are taken from the array pointed to by @a data. The array must
   * hold at least @a rows * @a columns entries. If @a stride is set
   * to a value larger than @p sizeof(T) * @a columns, row padding is
   * assumed. The @a data pointer must remain valid throughout the
   * lifetime of the matrix and any shallow copies of it.
   *
   * @code
   * const double data[] = { 1, 2, 3, 4 };
   * PiiMatrix<double> mat(2, 2, data);
   * QCOMPARE(mat(1,1), 4);
   *
   * const char* padded = "ABCDEFGH";
   * PiiMatrix<char> mat(3, 3, padded, 4);
   * QCOMPARE(mat(1,0), 'E');
   * @endcode
   *
   * Note that @a stride is always in bytes.
   *
   * @see PiiTypelessMatrix::stride()
   */
  PiiMatrix(int rows, int columns, const T* data, int stride = 0) :
    PiiTypelessMatrix(PiiMatrixData::createReferenceData(rows, columns,
                                                         qMax(stride, int(sizeof(T)*columns)),
                                                         const_cast<T*>(data))->makeImmutable())
  {}
  
  /**
   * Constructs a @a rows-by-@a columns matrix that uses @a data as
   * its data buffer. This is an overloaded constructor that behaves
   * essentially the same way as the one above. The difference is that
   * accesses to this matrix will modify @a data. Furthermore, if you
   * set the @a ownership flag to @p Pii::ReleaseOwnership, the data
   * pointer will be deallocated with @p free() when the matrix is
   * destroyed.
   *
   * @code
   * double data[] = { 1, 2, 3, 4 };
   * PiiMatrix<double> mat(2, 2, data, Pii::RetainOwnership);
   * mat(1,1) = 3;
   * QCOMPARE(data[3], 3);
   *
   * void* bfr = malloc(sizeof(int) * 16);
   * // bfr will be deallocated with free() when the matrix is destroyed.
   * PiiMatrix<int> mat(2, 7, bfr, Pii::ReleaseOwnership, 8 * sizeof(int));
   * mat = 0; // sets all elements in bfr to zeros
   * @endcode
   */
  PiiMatrix(int rows, int columns, void* data, Pii::PtrOwnership ownership, int stride = 0) :
    PiiTypelessMatrix(PiiMatrixData::createReferenceData(rows, columns,
                                                         qMax(stride, int(sizeof(T)*columns)),
                                                         data))
  {
    if (ownership == Pii::ReleaseOwnership)
      d->bufferType = PiiMatrixData::ExternalOwnBuffer;
  }
  
  /**
   * Constructs a matrix with the given number of @a rows and @a
   * columns. Matrix contents are given as a variable-length parameter
   * list in a horizontal raster-scan order. It is handy if you know
   * what you do, but proper care must be taken to ensure correct
   * functioning.
   *
   * Only elementary types can be used with this constructor. Complex
   * types cause a compile-time error.
   *
   * @warning Take extreme care to ensure that the elements you give
   * in the parameter list are of correct type. Examples:
   *
   * @code
   * PiiMatrix<char> a(3, 1, 'a', 'b', 'c');     //correct, chars are passed as ints
   * PiiMatrix<char> b(3, 1, 1, 2, 3);           //correct
   * PiiMatrix<float> c(3, 1, 1, 2, 3);          //WRONG! values are passed as ints
   * PiiMatrix<float> d(3, 1, 1.0, 2.0, 3.0);    //correct
   * PiiMatrix<float> e(3, 1, 1.0f, 2.0f, 3.0f); //WRONG! float constants cannot be used as va_args
   * PiiMatrix<double> f(3, 1, 1.0, 2.0, 3.0);   //correct
   * PiiMatrix<int> g(3, 1, 1, 2, 3);            //correct
   * PiiMatrix<int> g(3, 1, 1, 2);               //WRONG! too few parameters
   * @endcode
   *
   * @param rows the number of rows
   *
   * @param columns the number of columns
   *
   * @param firstElement the first element
   *
   * @param ... rest of the matrix data
   */
  PiiMatrix(int rows, int columns, VaArgType firstElement, ...);

#ifdef PII_CXX0X
  /**
   * Moves the contents of @a other to @p this and leaves @a other
   * with null data.
   */
  PiiMatrix(PiiMatrix&& other)
  {
    std::swap(d, other.d);
  }
  /**
   * Moves the contents of @a other to @p this and leaves @a other
   * with null data.
   */
  PiiMatrix(PiiSubmatrix<T>&& other)
  {
    std::swap(d, other._matrix.d);
    d->makeImmutable();
  }

  /**
   * Moves the contents of @a other to @p this and leaves @a other
   * with null data. Returns a reference to @p this.
   */
  PiiMatrix& operator= (PiiMatrix&& other)
  {
    std::swap(d, other._matrix.d);
    return *this;
  }
  PiiMatrix& operator= (PiiSubmatrix<T>&& other)
  {
    std::swap(d, other._matrix.d);
    d->makeImmutable();
    return *this;
  }
#else
  PiiMatrix(const PiiSubmatrix<T>& other)
  {
    std::swap(d, const_cast<PiiSubmatrix<T>&>(other)._matrix.d);
    d->makeImmutable();
  }
  PiiMatrix& operator= (const PiiSubmatrix<T>& other)
  {
    std::swap(d, const_cast<PiiSubmatrix<T>&>(other)._matrix.d);
    d->makeImmutable();
    return *this;
  }
#endif

  /// Destroys the matrix.
  ~PiiMatrix() {}

  /**
   * Assigns @a other to @p this and returns a reference to @p this.
   */
  PiiMatrix& operator= (const PiiMatrix& other)
  {
    // Reserve/release is always safe, and costs a bit extra only in
    // the rare case of self-assignment.
    other.d->reserve();
    d->release();
    d = other.d;
    return *this;
  }

  /**
   * Creates a deep copy of @a other and returns a reference to @p
   * this.
   */
  template <class Matrix> PiiMatrix& operator= (const PiiConceptualMatrix<Matrix>& other);

  PiiMatrix& operator= (T value) { return PiiConceptualMatrix<PiiMatrix<T,-1,-1> >::operator= (value); }
  
  using PiiTypelessMatrix::rows;
  using PiiTypelessMatrix::columns;

  /**
   * Detaches the matrix from shared data. This function makes sure
   * that there are no other references to the shared data, and
   * creates a clone of the data if necessary.
   */
  void detach()
  {
    if (d->iRefCount != 1)
      cloneAndReplaceData(capacity(), columns() * sizeof(T));
  }

  /**
   * Allocates memory for at least @a rows matrix rows. If you know in
   * advance how large the matrix will be, you can avoid unnecessary
   * reallocations while adding new rows. Trying to set @a rows to a
   * value less than the current capacity has no effect.
   *
   * @see capacity()
   */
  void reserve(int rows) { PiiTypelessMatrix::reserve(rows, columns() * sizeof(T)); }
  
  /**
   * Returns a random-access iterator to the start of the matrix data.
   */
  typename Traits::iterator begin() { return typename Traits::iterator(*this); }
  /// @overload
  typename Traits::const_iterator begin() const { return typename Traits::const_iterator(*this); }
  /**
   * Returns a random-access iterator to the end of the matrix data.
   */
  typename Traits::iterator end() { return typename Traits::iterator(*this, rows()); }
  /// @overload
  typename Traits::const_iterator end() const { return typename Traits::const_iterator(*this, rows()); }
  
  /**
   * Returns a random-access iterator to the start of the row at @a
   * rowIndex.
   */
  typename Traits::row_iterator rowBegin(int rowIndex) { return row(rowIndex); }
  /// @overload
  typename Traits::const_row_iterator rowBegin(int rowIndex) const { return row(rowIndex); }
  /**
   * Returns a random-access iterator to the end of the row at @a
   * rowIndex.
   */
  typename Traits::row_iterator rowEnd(int rowIndex) { return row(rowIndex) + columns(); }
  /// @overload
  typename Traits::const_row_iterator rowEnd(int rowIndex) const { return row(rowIndex) + columns(); }

  /**
   * Returns a random-access iterator to the start of the column at @a
   * colIndex.
   */
  typename Traits::column_iterator columnBegin(int colIndex) { return typename Traits::column_iterator(*this, colIndex); }
  /// @overload
  typename Traits::const_column_iterator columnBegin(int colIndex) const { return typename Traits::const_column_iterator(*this, colIndex); }
  /**
   * Returns a random-access iterator to the end of the column at @a
   * colIndex.
   */
  typename Traits::column_iterator columnEnd(int colIndex) { return typename Traits::column_iterator(*this, rows(), colIndex); }
  /// @overload
  typename Traits::const_column_iterator columnEnd(int colIndex) const { return typename Traits::const_column_iterator(*this, rows(), colIndex); }

  /**
   * Returns a pointer to the beginning of row at @a index.
   */
  const T* row(int index) const { return static_cast<const T*>(d->row(index)); }
  /// @overload
  T* row(int index) { detach(); return static_cast<T*>(d->row(index)); }

  /**
   * A utility function that returns a reference to the memory
   * location at the beginning of the given row as the specified type.
   * Use this function only if you are absolutely sure that the memory
   * arrangement of the type @p T matches that of a matrix row.
   *
   * @code
   * PiiMatrix<int> mat(1,3, 1,2,3);
   * PiiVector<int,3>& vec = mat.rowAs<PiiVector<int,3> >(0);
   * vec[0] = 2;
   * QCOMPARE(mat(0,0), 2);
   * @endcode
   */
  template <class U> U& rowAs(int index) { detach(); return *static_cast<U*>(d->row(index)); }
  /// @overload
  template <class U> const U& rowAs(int index) const { return *static_cast<const U*>(d->row(index)); }


  /**
   * Returns a reference to an item in the matrix. Equal to row(@p
   * row)[@p column]. No bound checking will be done for performance
   * reasons.
   *
   * @param r the row index
   *
   * @param c the column index
   *
   * @return a reference to the matrix item at (r,c)
   */
  T& operator() (int r, int c) { return row(r)[c]; }

  /**
   * Returns a copy of an item in the matrix.
   *
   * @param r the row index
   *
   * @param c the column index
   *
   * @return the matrix item at (r,c)
   */
  T operator() (int r, int c) const { return row(r)[c]; }

  /**
   * Access the matrix along its first non-singleton dimension. If the
   * matrix is actually a vector, it is more convenient not to repeat
   * the zero row or column index. This function works with both row and
   * column vectors. If the matrix is not a vector, the first element
   * of the indexth row is returned.
   *
   * @code
   * // Row vector
   * PiiMatrix<int> mat(1, 3, 0,1,2);
   * QCOMPARE(mat(2), 2);
   * // Column vector
   * PiiMatrix<int> mat2(3, 1, 0,1,2);
   * QCOMPARE(mat2(2), 2);
   * // Matrix
   * PiiMatrix<int> mat3(3, 2,
   *                     0, 1, 2,
   *                     3, 4, 5);
   * QCOMPARE(mat3(1), 3);
   * @endcode
   */
  T& operator() (int index) { return (rows() > 1 ? row(index)[0] : row(0)[index]); }
  /// @overload
  T operator() (int index) const { return (rows() > 1 ? row(index)[0] : row(0)[index]); }


  /**
   * Returns a pointer to the beginning of the given row. Same as
   * row(r). The purpose of this function is to allow the use of a
   * matrix as a two-dimensional array:
   *
   * @code
   * PiiMatrix<int> mat(2, 2,
   *                    1, 2,
   *                    3, 4);
   * QVERIFY(mat[0][1] == mat(0,1));
   * @endcode
   */
  T* operator[] (int r) { return row(r); }
  /// @overload
  const T* operator[] (int r) const { return row(r); }
  
  /**
   * Returns a mutable reference to a sub-matrix. Take care that the
   * dimensions of the matrix are not exceeded. If you modify the
   * returned result, the data within this matrix will also change. 
   * The const version returns a copy.
   *
   * @param r the row of the upper left column of the sub-matrix. If
   * this is negative, it is treated as a backwards index from the
   * last row. -1 means the last row and so on.
   *
   * @param c the column of the upper left column of the sub-matrix. 
   * Negative index is relative to the last column.
   *
   * @param rows the number of rows to include. Negative value means
   * "up to the nth last row".
   *
   * @param columns the number of columns to include. Negative value
   * means "up to the nth last column".
   *
   * @code
   * PiiMatrix<int> a(3, 3,
   *                  1, 2, 3,
   *                  4, 5, 6,
   *                  7, 8, 9);
   *
   * a(0,1,1,2) = 8;
   * a(-2,-2,-1,1) = 0;
   *
   * // a = 1, 8, 8,
   * //     4, 0, 6,
   * //     7, 0, 9);
   * @endcode
   */
  PiiSubmatrix<T> operator() (int r, int c, int rows, int columns)
  {
    fixIndices(r, c, rows, columns);
    return PiiSubmatrix<T>(this->createReference(rows, columns, row(r) + c));
  }

  /// @overload
  PiiMatrix operator() (int r, int c, int rows, int columns) const
  {
    fixIndices(r, c, rows, columns);
    return PiiMatrix(this->createReference(rows, columns, const_cast<T*>(row(r) + c))->makeImmutable());
  }

  template <class Matrix> PiiFilteredMatrix<const PiiMatrix, Matrix>
  operator() (const PiiConceptualMatrix<Matrix>& mask) const;

  template <class Matrix> PiiFilteredMatrix<PiiMatrix, Matrix>
  operator() (const PiiConceptualMatrix<Matrix>& mask);

  /**
   * Cast the contents of a matrix to another type. If the contents
   * of the original matrix are not of any elementary type, then
   * an appropriate typecast operator for the content class must
   * be defined.
   */
  template <class U> operator PiiMatrix<U>() const
  {
    PiiMatrix<U> result(PiiMatrix<U>::uninitialized(rows(), columns()));
    Pii::transform(begin(), end(), result.begin(), Pii::Cast<T,U>());
    return result;
  }

  /**
   * Returns a @b row vector that contains the elements of the row at
   * @a index. If you need a column vector, use
   * #operator()(int,int,int,int).
   */
  PiiMatrix column(int index) const;

  /**
   * Resizes the matrix to @a rows-by-@a columns. The function
   * preserves as much data as possible and sets any new entries to
   * zero. If the currently reserved space is not large enough, matrix
   * data will be reallocated. No reallocation will occur if the size
   * is decreased.
   */
  void resize(int rows, int columns);

  /**
   * Appends a new row to the end of the matrix. The contents of the
   * new row will be set to zero. Returns a row iterator to the
   * beginning of the new row. New rows can be appended without
   * reallocation until #capacity() is exceeded.
   */
  typename Traits::row_iterator appendRow()
  {
    detach();
    int iBytesPerRow = sizeof(T) * d->iColumns;
    void* pRow = PiiTypelessMatrix::appendRow(iBytesPerRow);
    memset(pRow, 0, iBytesPerRow);
    return static_cast<T*>(pRow);
  }

  /**
   * Appends the given vector to the end of this matrix. The size of
   * the @a row matrix must be 1-by-columns(), unless this matrix is
   * empty. In that case this matrix will be set equal to @a row.
   *
   * @see insertRow(int, const PiiMatrix&)
   */
  typename Traits::row_iterator appendRow(const PiiMatrix& row)
  {
    if (this->isEmpty())
      {
        *this = row;
        return rowBegin(0);
      }
    return insertRow(-1, row);
  }

  /**
   * Appends the given vector to the end of this matrix.
   *
   * @see insertRow(int, const T*)
   */
  typename Traits::row_iterator appendRow(const T* row)
  {
    return insertRow(-1, row);
  }

  /**
   * Append all rows in @a other to the end of this matrix. The number
   * of columns in @a other must equal to that of this matrix, unless
   * this matrix is empty. In that case this matrix will be set equal
   * to @a other.
   */
  void appendRows(const PiiMatrix& other);

  /**
   * Appends the given elements as a new row to the end of this
   * matrix. The number of elements must match the number of columns
   * in this matrix.
   *
   * @see insertRow(int, VaArgType, ...)
   */
  typename Traits::row_iterator appendRow(VaArgType firstElement, ...);
  
  /**
   * Inserts a new at position @a index and moves all following rows
   * forwards. The contents of the new row will be set to zero. 
   * Returns a row iterator to the beginning of the new row.
   *
   * @param index the row index of the new row. -1 means last.
   *
   * @return an iterator to the beginning of the newly added row
   */
  typename Traits::row_iterator insertRow(int index);

  /**
   * Inserts the given row at @a index. The input matrix can be either
   * a column or a row vector.
   *
   * @overload
   */
  typename Traits::row_iterator insertRow(int index, const PiiMatrix& row);

  /**
   * Inserts the given row at the given row index. The data of the new
   * row will be copied from @a row, which must hold at least
   * #columns() elements.
   *
   * @overload
   */
  typename Traits::row_iterator insertRow(int index, const T* row);

  /**
   * Insert a row with the given data elements at the given row index. 
   * The number of element arguments must be equal to the number of
   * columns in the matrix.
   *
   * @overload
   */
  typename Traits::row_iterator insertRow(int index, VaArgType firstElement, ...);

  /**
   * Appends a new column to the right of the last column.
   *
   * @see insertColumn(int)
   */
  typename Traits::column_iterator appendColumn()
  {
    return insertColumn(-1);
  }

  /**
   * Appends the given vector (N-by-1 matrix) as a new column to the
   * right of the last column. The number of rows in this matrix must
   * be equal to the number of rows in @a column, unless this matrix
   * is empty. In that case this matrix will be set equal to @a
   * column.
   *
   * @see insertColumn(int, const PiiMatrix&)
   */
  typename Traits::column_iterator appendColumn(const PiiMatrix& column)
  {
    if (this->isEmpty())
      {
        *this = column;
        return columnBegin(0);
      }
    return insertColumn(-1, column);
  }

  /**
   * Appends the given vector as a new column to the right of the last
   * column.
   *
   * @see insertColumn(int, const T*)
   */
  typename Traits::column_iterator appendColumn(const T* column)
  {
    return insertColumn(-1, column);
  }
  
  /**
   * Appends the given elements as a new column to the right of the
   * last column.
   *
   * @see insertColumn(int, VaArgType, ...)
   */
  typename Traits::column_iterator appendColumn(VaArgType firstElement, ...);

  /**
   * Inserts an empty column at the given index. Any subsequent
   * columns will be shifted forwards. The elements on the new column
   * will be set to zeros.
   *
   * @note New columns can be added without reallocation if the stride
   * is large enough. If there is no more free space, the matrix data
   * will be reallocated with #resize().
   *
   * @param index the row index of the new column. -1 means last.
   *
   * @return an iterator to the beginning of the newly added column
   */
  typename Traits::column_iterator insertColumn(int index = -1);

  /**
   * Inserts the given column at the given index. Any subsequent
   * columns will be shifted forwards. The input matrix can be either
   * a column or a row vector.
   *
   * @param column a matrix whose first column or row will be copied
   * to the memory location of the newly added column.
   *
   * @param index the column index of the new column. -1 means last.
   *
   * @return an iterator to the beginning of the newly added column
   */
  typename Traits::column_iterator insertColumn(int index, const PiiMatrix& column);

  /**
   * Inserts the given column at the given column index. The data of
   * the new column will be copied from @a column, which must hold at
   * least #rows() elements.
   *
   * @overload
   */
  typename Traits::column_iterator insertColumn(int index, const T* column);
  
  /**
   * Inserts a column with the given data elements at the given column
   * index. Any subsequent columns will be shifted forwards. The
   * number of elements must be equal to the number of rows in the
   * matrix.
   *
   * @overload
   */
  typename Traits::column_iterator insertColumn(int index, VaArgType firstElement, ...);

  /**
   * Removes the row at @a index. All rows below the removed one will
   * be moved up.
   *
   * @code
   * PiiMatrix<int> mat(3,3, 1,2,3, 4,5,6, 7,8,9);
   * mat.removeRow(1);
   * // mat = 1 2 3
   * //       7 8 9
   * @endcode
   */
  void removeRow(int index) { detach(); PiiTypelessMatrix::removeRow(index, sizeof(T) * d->iColumns); }

  /**
   * Removes @a count successive rows starting at @a index. All rows
   * below the last removed one will be moved up.
   */
  void removeRows(int index, int count)
  {
    detach();
    PiiTypelessMatrix::removeRows(index, count, sizeof(T) * d->iColumns);
  }
  
  /**
   * Removes a column from the matrix. All data right of @a index will
   * be moved left. The stride of the matrix will not change. If you
   * later add a column to the matrix, the data will not be
   * reallocated.
   *
   * @code
   * PiiMatrix<int> mat(3,3, 1,2,3, 4,5,6, 7,8,9);
   * mat.removeColumn(1);
   * // mat = 1  3
   * //       4  6
   * //       7  9
   * @endcode
   */
  void removeColumn(int index) { detach(); PiiTypelessMatrix::removeColumn(index, sizeof(T)); }

  /**
   * Removes @a count successive columns starting at @a index. All columns
   * right of the last removed one will be moved left.
   */
  void removeColumns(int index, int count)
  {
    detach();
    PiiTypelessMatrix::removeColumns(index, count, sizeof(T));
  }
  
  /**
   * Assigns the elements of @a other to the corresponding elements of
   * this.
   *
   * @exception PiiMathException& if @a other and @p this are not
   * equal in size.
   */
  template <class Matrix> PiiMatrix& assign(const PiiConceptualMatrix<Matrix>& other);

  /**
   * Applies the <em>adaptable binary function</em> @p op to all
   * elements of this matrix and the corresponding elements in @a
   * other. The matrices must be of equal size. The result is returned
   * in a new matrix. For example, to explicitly apply the addition
   * operation, do this:
   *
   * @code
   * PiiMatrix<int> a, b;
   * PiiMatrix<int> c(a.mapped(std::plus<int>(), b));
   * @endcode
   *
   * @exception PiiMathException& if this matrix is not equal to @a
   * other in size.
   */
  template <class BinaryFunc, class Matrix> PiiMatrix<typename BinaryFunc::result_type>
  mapped(BinaryFunc op, const PiiConceptualMatrix<Matrix>& other) const
  {
    PII_MATRIX_CHECK_EQUAL_SIZE(other, *this);
    typedef typename BinaryFunc::result_type R;
    PiiMatrix<R> result(PiiMatrix<R>::uninitialized(rows(), columns()));
    Pii::transform(begin(), end(), other.begin(), result.begin(), op);
    return result;
  }

  /**
   * Applies the <em>adaptable binary function</em> to all elements of
   * this matrix and the correspondng element in @a other. The
   * matrices must be of equal size. The result is stored in this
   * matrix. For example, to apply the operator-= the other way, do
   * this:
   *
   * @code
   * PiiMatrix<int> a, b;
   * a.map(std::minus<int>(), b);
   * @endcode
   *
   * @exception PiiMathException& if this matrix is not equal to @a
   * other in size.
   */
  template <class BinaryFunc, class Matrix>
  PiiMatrix& map(BinaryFunc op, const PiiConceptualMatrix<Matrix>& other)
  {
    PII_MATRIX_CHECK_EQUAL_SIZE(other, *this);
    Pii::map(begin(), end(), other.begin(), op);
    return *this;
  }
  
  /**
   * Applies a binary function to all elements of this matrix and the
   * scalar @a value. The result is returned in a new matrix. An
   * example:
   *
   * @code
   * PiiMatrix<int> a;
   * PiiMatrix<int> c(a.mapped(std::plus<int>(), 5));
   * @endcode
   */
  template <class BinaryFunc>
  PiiMatrix<typename BinaryFunc::result_type> mapped(BinaryFunc op, typename BinaryFunc::second_argument_type value) const
  {
    typedef typename BinaryFunc::result_type R;
    PiiMatrix<R> result(PiiMatrix<R>::uninitialized(rows(), columns()));
    Pii::transform(begin(), end(), result.begin(), std::bind2nd(op, value));
    return result;
  }

  /**
   * Applies a binary function to all elements of this matrix and the
   * scalar @a value. The result is stored in this matrix. An example:
   *
   * @code
   * PiiMatrix<int> a;
   * a.map(std::minus<int>(), 5);
   * // The same can be achieved with
   * a.map(std::bind2nd(std::minus<int>(), 5));
   * // ... but which one is more readable?
   * @endcode
   */
  template <class BinaryFunc>
  PiiMatrix& map(BinaryFunc op, typename BinaryFunc::second_argument_type value)
  {
    Pii::map(begin(), end(), std::bind2nd(op, value));
    return *this;
  }

  /**
   * Applies a unary function to all elements in this matrix. For
   * example, to negate all elements in a matrix, do the following:
   *
   * @code
   * PiiMatrix<int> a;
   * a.map(std::negate<int>());
   * @endcode
   */
  template <class UnaryFunc> PiiMatrix& map(UnaryFunc op)
  {
    Pii::map(begin(), end(), op);
    return *this;
  }

  /**
   * Creates a matrix that contains the result of applying a unary
   * function to all elements in this matrix. For example, to create a
   * negation of a matrix, do the following:
   *
   * @code
   * PiiMatrix<int> a;
   * PiiMatrix<int> b(a.mapped(std::negate<int>());
   *
   * // Different result type (convert complex numbers to real numbers)
   * PiiMatrix<std::complex<float> > a;
   * PiiMatrix<float> b(a.mapped(Pii::Abs<std::complex<float> >()));
   * @endcode
   */
  template <class UnaryFunc> PiiMatrix<typename UnaryFunc::result_type> mapped(UnaryFunc op) const
  {
    typedef typename UnaryFunc::result_type R;
    PiiMatrix<R> result(PiiMatrix<R>::uninitialized(rows(), columns()));
    Pii::transform(begin(), end(), result.begin(), op);
    return result;
  }
  
  /**
   * Swaps the places of @a row1 and @a row2.
   */
  void swapRows(int row1, int row2);

  /**
   * Creates a @a size-by-@a size identity matrix.
   *
   * @code
   * PiiMatrix<int> mat(PiiMatrix<int>::identity(3));
   * // 1 0 0
   * // 0 1 0
   * // 0 0 1
   * @endcode
   */
  static PiiMatrix identity(int size);

  /**
   * Creates a @a rows-by-@a columns matrix whose initial contents are
   * set to @a value. This is faster than first creating and clearing
   * a matrix to zeros and then setting all entries.
   *
   * @code
   * PiiMatrix<float> mat(PiiMatrix<float>::constant(2, 3, 1.0);
   * // 1.0 1.0 1.0
   * // 1.0 1.0 1.0
   * @endcode
   */
  static PiiMatrix constant(int rows, int columns, T value)
  {
    PiiMatrix result(uninitialized(rows, columns));
    result = value;
    return result;
  }

  /**
   * Creates an uninitialized @a rows-by-@a columns matrix. This
   * function leaves the contents of the matrix in an unspecified
   * state. It is useful as an optimization if you know you are going
   * to set all matrix entries anyway. If @a stride is set to a value
   * larger than <tt>sizeof(T)*columns</tt>, matrix rows will be
   * padded to @a stride bytes.
   */
  static PiiMatrix uninitialized(int rows, int columns, int stride = 0)
  {
    return PiiMatrix(PiiMatrixData::createUninitializedData(rows, columns,
                                                            columns * sizeof(T),
                                                            stride));
  }

  /**
   * Creates @a rows-by-@a columns matrix with initial contents set to
   * zero. If @a stride is set to a value larger than
   * <tt>sizeof(T)*columns</tt>, matrix rows will be padded to
   * @a stride bytes.
   */
  static PiiMatrix padded(int rows, int columns, int stride)
  {
    return PiiMatrix(PiiMatrixData::createInitializedData(rows, columns,
                                                          columns * sizeof(T),
                                                          stride));
  }
  
private:
  friend class PiiSubmatrix<T>;
  
  PiiMatrix(PiiMatrixData* d) : PiiTypelessMatrix(d) {}

  void fixIndices(int &r, int &c, int &rows, int &columns) const
  {
    if (r < 0) r += this->rows();
    if (c < 0) c += this->columns();
    if (rows < 0) rows += this->rows() - r + 1;
    if (columns < 0) columns += this->columns() - c + 1;
  }
};

/**
 * A matrix that provides a mutable reference to a PiiMatrix. 
 * Sub-matrices are temporary in nature; they only exist as return
 * values from @ref PiiMatrix::operator()(int,int,int,int). 
 * Sub-matrices cannot be copied. Unfortunately, there is no elegant
 * way of making a movable but non-copyable type in C++ prior to
 * C++0x. Therefore, this class uses a bit of hackery to implement
 * move semantics with the old C++ standard.
 *
 * @code
 * PiiMatrix<int> mat(5,5);
 * // Constructing a copy of a PiiSubmatrix is possible prior to
 * // C++0x. This is however strongly discouraged. The following
 * // won't compile if C++0x is enabled.
 * PiiSubMatrix<int> sub(mat(1,1,2,2)); // WRONG!
 * PiiSubMatrix<int> sub2(sub); // WRONG!
 * // As a result, sub will be left in an empty state.
 *
 * // The following is legal in both old C++ and C++0x
 * PiiMatrix<int> sub(mat(1,1,2,2)); // immutable shallow copy
 * // Modifies the central portion of mat
 * mat(1,1,3,3) *= 5;
 * @endcode
 *
 * @ingroup Matrix
 */
template <class T> class PiiSubmatrix :
  public PiiConceptualMatrix<PiiSubmatrix<T>, Pii::RandomAccessMatrix>
{
public:
  typedef PiiMatrixTraits<PiiSubmatrix<T>, Pii::RandomAccessMatrix> Traits;
  
#ifdef PII_CXX0X
  /**
   * Moves the contents of @a other to this and leaves @a other with
   * null data.
   */
  PiiSubmatrix(PiiSubmatrix&& other)
  {
    // Swap our matrix (null) with other's.
    std::swap(_matrix.d, other._matrix.d);
  }
  PII_DISABLE_COPY(PiiSubmatrix);
#else 
  PiiSubmatrix(const PiiSubmatrix& other)
  {
    // HACK Same as above, but now we really have no way of ensuring
    // "other" is not going to be used later.
    std::swap(_matrix.d, const_cast<PiiSubmatrix&>(other)._matrix.d);
  }
  
private: PiiSubmatrix& operator= (const PiiSubmatrix& other);
#endif
public:
  /**
   * Sets all elements to @a value and returns a reference to @p this.
   */
  PiiSubmatrix& operator= (typename Traits::value_type value) { return PiiConceptualMatrix<PiiSubmatrix<T> >::operator=(value); }
  
  typename Traits::iterator begin() { return _matrix.begin(); }
  typename Traits::iterator end() { return _matrix.end(); }
  typename Traits::const_iterator begin() const { return _matrix.begin(); }
  typename Traits::const_iterator end() const { return _matrix.end(); }
  typename Traits::row_iterator rowBegin(int index) { return _matrix.rowBegin(index); }
  typename Traits::row_iterator rowEnd(int index) { return _matrix.rowEnd(index); }
  typename Traits::const_row_iterator rowBegin(int index) const { return _matrix.rowBegin(index); }
  typename Traits::const_row_iterator rowEnd(int index) const { return _matrix.rowEnd(index); }
  typename Traits::column_iterator columnBegin(int index) { return _matrix.columnBegin(index); }
  typename Traits::column_iterator columnEnd(int index) { return _matrix.columnEnd(index); }
  typename Traits::const_column_iterator columnBegin(int index) const { return _matrix.columnBegin(index); }
  typename Traits::const_column_iterator columnEnd(int index) const { return _matrix.columnEnd(index); }

  int rows() const { return _matrix.rows(); }
  int columns() const { return _matrix.columns(); }
  
  T& operator() (int r, int c) { return _matrix(r,c); }
  T operator() (int r, int c) const { return _matrix(r,c); }

  /**
   * Returns a reference to the wrapped matrix. This function makes it
   * possible to pass a sub-matrix to a function that takes a
   * PiiMatrix as a parameter.
   *
   * @code
   * template <class T> void modify(PiiMatrix<T>& mat);
   *
   * // ...
   * PiiMatrix<float> mat(3,5);
   * // You need to explicitly specify the template parameter for
   * // overload resolution.
   * modify<float>(mat(1,0,1,-1));
   * @endcode
   */
  operator PiiMatrix<T>& () { return _matrix; }

private:
  friend class PiiMatrix<T>;
  PiiSubmatrix(PiiMatrixData* data) : _matrix(data) {}

  PiiMatrix<T> _matrix;
};

#include "PiiMatrix-templates.h"

namespace Pii
{
  /**
   * Returns a deep copy of @p mat. This function is useful if you
   * need a concrete copy of a matrix concept.
   *
   * @code
   * template <class T> void func(const PiiMatrix<T>& mat);
   *
   * // ...
   * PiiMatrix<int> mat1, mat2;
   * func(mat1 + mat2); // Error, mat1 + mat2 is a PiiBinaryMatrixTransform
   * func(Pii::matrix(mat1 + mat2)); // calls func(const PiiMatrix<int>&)
   * @endcode
   *
   * @relates PiiMatrix
   */
  template <class Matrix>
  inline PiiMatrix<typename Matrix::value_type> matrix(const PiiConceptualMatrix<Matrix>& mat)
  {
    return PiiMatrix<typename Matrix::value_type>(mat);
  }

  /**
   * @overload
   *
   * This specialization just returns @p mat.
   */
  template <class T>
  inline PiiMatrix<T> matrix(const PiiMatrix<T>& mat) { return mat; }
  
  /**
   * Returns a deep copy of @p mat allocated from the heap.
   *
   * @see matrix()
   * @relates PiiMatrix
   */
  template <class Matrix>
  inline PiiMatrix<typename Matrix::value_type>* newMatrix(const PiiConceptualMatrix<Matrix>& mat)
  {
    return new PiiMatrix<typename Matrix::value_type>(mat);
  }
}

#endif //_PIIMATRIX_H
