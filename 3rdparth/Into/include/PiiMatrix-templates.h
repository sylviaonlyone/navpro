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
# error "Never use <PiiMatrix-templates.h> directly; include <PiiMatrix.h> instead."
#endif

/**** Iterators ****/

template <class T> class PiiMatrixIterator
{
public:
  typedef std::random_access_iterator_tag iterator_category;
  typedef long difference_type;
  typedef typename Pii::ToNonConst<T>::Type value_type;
  typedef typename Pii::IfClass<Pii::IsConst<T>,
                                const PiiMatrix<value_type>&,
                                PiiMatrix<value_type>&>::Type MatrixRef;
  typedef T* pointer;
  typedef T& reference;
    
  PiiMatrixIterator(pointer firstRow, pointer row, int columns, int stride) :
    _pFirstRow(firstRow), _pRow(row), _iColumn(0), _iColumns(columns), _iStride(stride)
  {}
  PiiMatrixIterator(const PiiMatrixIterator& other) :
    _pFirstRow(other._pFirstRow),
    _pRow(other._pRow),
    _iColumn(other._iColumn),
    _iColumns(other._iColumns),
    _iStride(other._iStride)
  {}
  PiiMatrixIterator(MatrixRef mat) :
    _pFirstRow(mat[0]), _pRow(_pFirstRow),
    _iColumn(0), _iColumns(mat.columns()),
    _iStride(mat.stride())
  {}
  PiiMatrixIterator(MatrixRef mat, int row) :
    _pFirstRow(mat[0]), _pRow(mat[row]),
    _iColumn(0), _iColumns(mat.columns()),
    _iStride(mat.stride())
  {}

  reference operator* () const { return _pRow[_iColumn]; }
  pointer operator-> () const { return _pRow + _iColumn; }
    
  PiiMatrixIterator& operator= (const PiiMatrixIterator& other)
  {
    _pFirstRow = other._pFirstRow;
    _pRow = other._pRow;
    _iColumn = other._iColumn;
    _iColumns = other._iColumns;
    _iStride = other._iStride;
    return *this;
  }
  pointer addPtr(int offset) const { return reinterpret_cast<pointer>((char*)_pRow + offset); }
  reference operator[] (int i) const { return *(*this + i); }
  bool operator== (const PiiMatrixIterator &other) const { return (_pRow + _iColumn) == (other._pRow + other._iColumn); }
  bool operator!= (const PiiMatrixIterator &other) const { return  (_pRow + _iColumn) != (other._pRow + other._iColumn); }
  bool operator< (const PiiMatrixIterator& other) const { return  (_pRow + _iColumn) < (other._pRow + other._iColumn); }
  bool operator<= (const PiiMatrixIterator& other) const { return  (_pRow + _iColumn) <= (other._pRow + other._iColumn); }
  bool operator> (const PiiMatrixIterator& other) const { return  (_pRow + _iColumn) > (other._pRow + other._iColumn); }
  bool operator>= (const PiiMatrixIterator& other) const { return  (_pRow + _iColumn) >= (other._pRow + other._iColumn); }

  PiiMatrixIterator& operator++ ()
  {
    if (++_iColumn >= _iColumns)
      {
        _iColumn = 0;
        _pRow = addPtr(_iStride);
      }
    return *this;
  }
  PiiMatrixIterator operator++(int)
  {
    PiiMatrixIterator tmp(*this);
    operator++();
    return tmp;
  }
  PiiMatrixIterator& operator--()
  {
    if (--_iColumn < 0)
      {
        _iColumn = _iColumns-1;
        _pRow = addPtr(-_iStride);
      }
    return *this;
  }
  PiiMatrixIterator operator--(int)
  {
    PiiMatrixIterator tmp(*this);
    operator--();
    return tmp;
  }
  PiiMatrixIterator& operator+= (int i)
  {
    int iNewColumn = _iColumn + i;
    if (iNewColumn >= 0)
      {
        _pRow = addPtr(iNewColumn/_iColumns * _iStride);
        _iColumn = iNewColumn % _iColumns;
      }
    else
      {
        ++iNewColumn;
        _pRow = addPtr((iNewColumn/_iColumns - 1) * _iStride);
        _iColumn = _iColumns - 1 + iNewColumn % _iColumns;
      }
    return *this;
  }
  PiiMatrixIterator& operator-= (int i) { return operator+= (-i); }
  PiiMatrixIterator operator+ (int i) const { PiiMatrixIterator tmp(*this); return tmp += i; }
  PiiMatrixIterator operator- (int i) const { PiiMatrixIterator tmp(*this); return tmp -= i; }
  long operator- (const PiiMatrixIterator& other) const
  {
    return ((char*)(_pRow) - (char*)(other._pRow))/_iStride * _iColumns + _iColumn - other._iColumn;
  }

  int column() const { return _iColumn; }
  int row() const { return ((char*)_pRow - (char*)_pFirstRow) / _iStride; }

private:
  pointer _pFirstRow, _pRow;
  int _iColumn, _iColumns, _iStride;
};


template <class T> class PiiMatrixColumnIterator
{
public:
  // stl compatibility typedefs
  typedef std::random_access_iterator_tag iterator_category;
  typedef long difference_type;
  typedef typename Pii::ToNonConst<T>::Type value_type;
  typedef typename Pii::IfClass<Pii::IsConst<T>,
                                const PiiMatrix<value_type>&,
                                PiiMatrix<value_type>&>::Type MatrixRef;
  typedef T* pointer;
  typedef T& reference;
    
  PiiMatrixColumnIterator(pointer ptr, int stride) :
    _ptr(ptr), _iStride(stride)
  {}
  PiiMatrixColumnIterator(const PiiMatrixColumnIterator &other) :
    _ptr(other._ptr), _iStride(other._iStride)
  {}
  PiiMatrixColumnIterator(MatrixRef mat, int column) :
    _ptr(mat[0]+column), _iStride(mat.stride())
  {}
  PiiMatrixColumnIterator(MatrixRef mat, int row, int column) :
    _ptr(mat[row]+column), _iStride(mat.stride())
  {}

  reference operator* () const { return *_ptr; }

  PiiMatrixColumnIterator& operator= (const PiiMatrixColumnIterator& other)
  {
    _ptr = other._ptr;
    _iStride = other._iStride;
    return *this;
  }
  reference operator[] (int i) const { return *reinterpret_cast<pointer>((char*)_ptr + i * _iStride); }
  bool operator== (const PiiMatrixColumnIterator& other) const { return _ptr == other._ptr; }
  bool operator!= (const PiiMatrixColumnIterator& other) const { return  _ptr != other._ptr; }
  bool operator< (const PiiMatrixColumnIterator& other) const { return _ptr < other._ptr; }
  bool operator<= (const PiiMatrixColumnIterator& other) const { return _ptr <= other._ptr; }
  bool operator> (const PiiMatrixColumnIterator& other) const { return _ptr > other._ptr; }
  bool operator>= (const PiiMatrixColumnIterator& other) const { return _ptr >= other._ptr; }
  PiiMatrixColumnIterator& operator++ ()
  {
    _ptr = reinterpret_cast<pointer>((char*)_ptr + _iStride);
    return *this;
  }
  PiiMatrixColumnIterator operator++ (int)
  {
    PiiMatrixColumnIterator tmp(*this);
    operator++();
    return tmp;
  }
  PiiMatrixColumnIterator& operator-- ()
  {
    _ptr = reinterpret_cast<pointer>((char*)_ptr - _iStride);
    return *this;
  }
  PiiMatrixColumnIterator operator-- (int)
  {
    PiiMatrixColumnIterator tmp(*this);
    operator--();
    return tmp;
  }
  PiiMatrixColumnIterator& operator+= (int i)
  {
    _ptr = &operator[](i);
    return *this;
  }
  PiiMatrixColumnIterator& operator-= (int i)
  {
    _ptr = &operator[](-i);
    return *this;
  }
  PiiMatrixColumnIterator operator+ (int i) const { PiiMatrixColumnIterator tmp(*this); return tmp += i; }
  PiiMatrixColumnIterator operator- (int i) const { PiiMatrixColumnIterator tmp(*this); return tmp -= i; }
  long operator- (const PiiMatrixColumnIterator& other) const { return ((char*)(_ptr) - (char*)(other._ptr))/_iStride; }

private:
  pointer _ptr;
  int _iStride;
};

/**** Matrix functions ****/

template <class T> PiiMatrix<T,-1,-1>::PiiMatrix(int rows, int columns, VaArgType firstElement, ...) :
  PiiTypelessMatrix(PiiMatrixData::createUninitializedData(rows, columns, columns * sizeof(T)))
{
  va_list argp;
  // initalize var ptr
  va_start(argp, firstElement);
  Pii::copyVaList(begin(), end(), firstElement, argp);
  // done with args
  va_end(argp);
}

template <class T> template <class Matrix>
PiiMatrix<T>& PiiMatrix<T,-1,-1>::operator= (const PiiConceptualMatrix<Matrix>& other)
{
  d->release();
  d = PiiMatrixData::createUninitializedData(other.rows(), other.columns(), other.columns() * sizeof(T));
  Pii::transform(other.begin(), other.end(), begin(), Pii::Cast<typename Matrix::value_type,T>());
  return *this;
}

template <class T> template <class Matrix>
PiiMatrix<T>& PiiMatrix<T,-1,-1>::assign(const PiiConceptualMatrix<Matrix>& other)
{
  PII_MATRIX_CHECK_EQUAL_SIZE(*this, other);
  if (d->iRefCount != 1)
    {
      d->release();
      d = createUninitializedData(other.rows(), other.columns(), other.columns() * sizeof(T));
    }
  Pii::transform(other.begin(), other.end(), begin(), Pii::Cast<typename Matrix::value_type,T>());
  return *this;
}

template <class T>
template <class Matrix> PiiFilteredMatrix<const PiiMatrix<T>, Matrix>
PiiMatrix<T,-1,-1>::operator() (const PiiConceptualMatrix<Matrix>& mask) const
{
  PII_MATRIX_CHECK_EQUAL_SIZE(*this, mask);
  return Pii::filteredMatrix(*this, *mask.self());
}

template <class T>
template <class Matrix> PiiFilteredMatrix<PiiMatrix<T>, Matrix>
PiiMatrix<T,-1,-1>::operator() (const PiiConceptualMatrix<Matrix>& mask)
{
  PII_MATRIX_CHECK_EQUAL_SIZE(*this, mask);
  return Pii::filteredMatrix(*this, *mask.self());
}

template <class T> typename PiiMatrix<T>::Traits::row_iterator PiiMatrix<T,-1,-1>::insertRow(int index, const PiiMatrix<T>& row)
{
  detach();
  T* pNewRow = static_cast<T*>(PiiTypelessMatrix::insertRow(index, columns() * sizeof(T)));
  // Not a column vector
  if (row.columns() != 1)
    memcpy(pNewRow, row[0], qMin(row.columns(), columns()) * sizeof(T));
  // Column vector
  else
    Pii::copyN(row.columnBegin(0), qMin(row.rows(), columns()), pNewRow);

  return pNewRow;
}

template <class T> typename PiiMatrix<T>::Traits::row_iterator PiiMatrix<T,-1,-1>::insertRow(int index, const T* row)
{
  detach();
  int iBytesPerRow = columns() * sizeof(T);
  T* pNewRow = static_cast<T*>(PiiTypelessMatrix::insertRow(index, iBytesPerRow));
  memcpy(pNewRow, row, iBytesPerRow);
  return pNewRow;
}

template <class T> typename PiiMatrix<T>::Traits::row_iterator PiiMatrix<T,-1,-1>::insertRow(int index)
{
  detach();
  int iBytesPerRow = columns() * sizeof(T);
  T* pNewRow = static_cast<T*>(PiiTypelessMatrix::insertRow(index, iBytesPerRow));
  memset(pNewRow, 0, iBytesPerRow);
  return pNewRow;
}

template <class T> typename PiiMatrix<T>::Traits::row_iterator PiiMatrix<T,-1,-1>::insertRow(int index, VaArgType firstElement, ...)
{
  detach();
  T* pNewRow = static_cast<T*>(PiiTypelessMatrix::insertRow(index, columns() * sizeof(T)));
  va_list argp;
  va_start(argp, firstElement);
  Pii::copyVaList(pNewRow, pNewRow + columns(), firstElement, argp);
  va_end(argp);
  return pNewRow;
}

template <class T> typename PiiMatrix<T>::Traits::row_iterator PiiMatrix<T,-1,-1>::appendRow(VaArgType firstElement, ...)
{
  detach();
  T* pNewRow = static_cast<T*>(PiiTypelessMatrix::appendRow(columns() * sizeof(T)));
  va_list argp;
  va_start(argp, firstElement);
  Pii::copyVaList(pNewRow, pNewRow + columns(), firstElement, argp);
  va_end(argp);
  return pNewRow;
}

template <class T> void PiiMatrix<T,-1,-1>::appendRows(const PiiMatrix<T>& other)
{
  if (this->isEmpty())
    {
      *this = other;
      return;
    }

  reserve(rows() + other.rows());
  int iBytesPerRow = columns() * sizeof(T);
  for (int i=0; i<other.rows(); ++i)
    memcpy(PiiTypelessMatrix::insertRow(-1, iBytesPerRow),
           other[i],
           iBytesPerRow);
}


template <class T> typename PiiMatrix<T>::Traits::column_iterator PiiMatrix<T,-1,-1>::insertColumn(int index, const PiiMatrix<T>& column)
{
  detach();
  T* pColumnStart = static_cast<T*>(PiiTypelessMatrix::insertColumn(index, sizeof(T)));
  typename Traits::column_iterator ci(pColumnStart, stride());
  if (column.rows() == 1)
    {
      typename Traits::const_row_iterator r = column.rowBegin(0);
      Pii::copyN(r, qMin(column.columns(), rows()), ci);
    }
  else
    {
      typename Traits::const_column_iterator c = column.columnBegin(0);
      Pii::copyN(c, qMin(column.rows(), rows()), ci);
    }
  return ci;
}


template <class T> typename PiiMatrix<T>::Traits::column_iterator PiiMatrix<T,-1,-1>::insertColumn(int index, const T* column)
{
  detach();
  T* pColumnStart = static_cast<T*>(PiiTypelessMatrix::insertColumn(index, sizeof(T)));
  typename Traits::column_iterator ci(pColumnStart, stride());
  Pii::copy(column, column + rows(), ci);
  return ci;
}

template <class T> typename PiiMatrix<T>::Traits::column_iterator PiiMatrix<T,-1,-1>::insertColumn(int index)
{
  detach();
  T* pColumnStart = static_cast<T*>(PiiTypelessMatrix::insertColumn(index, sizeof(T)));
  typename Traits::column_iterator ci(pColumnStart, stride());
  std::fill(ci, ci + rows(), T(0));
  return ci;
}

template <class T> typename PiiMatrix<T>::Traits::column_iterator PiiMatrix<T,-1,-1>::insertColumn(int index, VaArgType firstElement, ...)
{
  detach();
  va_list rest;
  va_start(rest, firstElement);
  T* pColumnStart = static_cast<T*>(PiiTypelessMatrix::insertColumn(index, sizeof(T)));
  typename Traits::column_iterator ci(pColumnStart, stride());
  Pii::copyVaList(ci, ci + rows(), firstElement, rest);
  va_end(rest);
  return ci;
}

template <class T>
typename PiiMatrix<T>::Traits::column_iterator PiiMatrix<T,-1,-1>::appendColumn(VaArgType firstElement, ...)
{
  detach();
  va_list rest;
  va_start(rest, firstElement);
  T* pColumnStart = static_cast<T*>(PiiTypelessMatrix::insertColumn(-1, sizeof(T)));
  typename Traits::column_iterator ci(pColumnStart, stride());
  Pii::copyVaList(ci, ci + rows(), firstElement, rest);
  va_end(rest);
  return ci;
}

template <class T> void PiiMatrix<T,-1,-1>::resize(int rows, int columns)
{
  detach();
  int iOldR = this->d->iRows, iOldC = this->d->iColumns;
  PiiTypelessMatrix::resize(rows, columns, sizeof(T));
  if (columns > iOldC)
    {
      const int iBytes = sizeof(T) * (columns - iOldC);
      for (int i=0; i<rows; ++i)
        memset(static_cast<T*>(this->d->row(i)) + iOldC, 0, iBytes);
    }
  if (rows > iOldR)
    {
      const int iBytes = sizeof(T) * columns;
      if (iBytes > 0)
        for (int i=iOldR; i<rows; ++i)
          memset(static_cast<T*>(this->d->row(i)), 0, iBytes);
    }
}

template <class T> PiiMatrix<T> PiiMatrix<T,-1,-1>::identity(int size)
{
  PiiMatrix<T> result(size, size);
  for (int d=0; d<size; ++d)
    result(d,d) = T(1);
  return result;
}

template <class T> PiiMatrix<T> PiiMatrix<T,-1,-1>::column(int column) const
{
  PiiMatrix<T> result(uninitialized(1, rows()));
  Pii::copy(columnBegin(column), columnEnd(column), result.rowBegin(0));
  return result;
}

template <class T> void PiiMatrix<T,-1,-1>::swapRows(int r1, int r2)
{
  detach();
  T* p1 = row(r1), *p2 = row(r2);
  const int iColumns = columns();
  for (int i=0; i<iColumns; ++i)
    qSwap(p1[i], p2[i]);
}
