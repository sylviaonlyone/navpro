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

#ifndef _PIIMATRIXUTIL_H
# error "Never use <PiiMatrixUtil-templates.h> directly; include <PiiMatrixUtil.h> instead."
#endif

#include <PiiRandom.h>

namespace Pii
{
  template <class Stream, class Matrix, class ColSeparator, class RowSeparator>
  void printMatrix(Stream& out, const PiiConceptualMatrix<Matrix, RandomAccessMatrix>& mat,
                   const ColSeparator& columnSeparator,
                   const RowSeparator& rowSeparator)
  {
    for (int r=0; r<mat.rows(); ++r)
      {
        typename Matrix::const_row_iterator row = mat.rowBegin(r), end = mat.rowEnd(r);
        // Print each element on this row
        while (row != end)
          {
            out << *row;
            // Add the column separator in between
            if (++row != end)
              out << columnSeparator;
            else
              break;
          }
        // Add row separator after each row
        if (r != mat.rows()-1)
          out << rowSeparator;
      }
  }

  template <class Stream, class Matrix>
  void matlabPrint(Stream& out,
                   const PiiConceptualMatrix<Matrix, RandomAccessMatrix>& mat)
  {
    out << "[ ";
    printMatrix<Stream,Matrix,const char*,const char*>(out, mat, " ", "; ...\n  ");
    out << " ];\n";
  }

  template <class T> PiiMatrix<T> extend(const PiiMatrix<T>& mat,
                                         int amount,
                                         ExtendMode mode,
                                         int sides)
	{
		int leftCols = 0, rightCols = 0, topRows = 0, bottomRows = 0;

		if (sides & MatrixTop)
			topRows = amount;
		if (sides & MatrixBottom)
			bottomRows = amount;
		if (sides & MatrixLeft)
			leftCols = amount;
		if (sides & MatrixRight)
			rightCols = amount;

    return extend(mat, topRows, bottomRows, leftCols, rightCols, mode);
  }

  template <class T> PiiMatrix<T> extend(const PiiMatrix<T>& mat,
                                         int topRows, int bottomRows, int leftCols, int rightCols,
                                         ExtendMode mode)
  {
    if (mode == ExtendNot)
      return mat;
    
		int oldR = mat.rows(), oldC = mat.columns();
		int newR = oldR+topRows+bottomRows, newC = oldC+leftCols+rightCols;

		// Make all elements zero if extension mode is zero padding
		PiiMatrix<T> result;
    if (mode == ExtendZeros)
      result = PiiMatrix<T>(newR, newC);
    else
      result = PiiMatrix<T>::uninitialized(newR, newC);

		// Copy inner contents
    result(topRows, leftCols, oldR, oldC) << mat;
    
		switch (mode)
			{
			case ExtendSymmetric:
        {
          for (int c=leftCols; c--; )
            result(topRows, c, oldR, 1) << mat(0, Pii::min(leftCols-c, oldC)-1, oldR, 1);
          for (int c=rightCols; c--; )
            result(topRows, -c-1, oldR, 1) << mat(0, -Pii::min(rightCols-c, oldC), oldR, 1);
          int doubleRows = (topRows << 1) - 1;
          int upLimit = oldR + topRows - 1;
          for (int r=topRows; r--; )
            result(r, 0, 1, -1) << result(Pii::min(doubleRows-r, upLimit), 0, 1, -1);
          upLimit = oldR + topRows - bottomRows; 
          for (int r=bottomRows; r--; )
            result(-r-1, 0, 1, -1) << result(Pii::max(upLimit+r, topRows), 0, 1, -1);
        }
        break;
				
			case ExtendPeriodic:
        {
          int upLimit = oldC - leftCols;
          for (int c=leftCols; c--; )
            result(topRows, c, oldR, 1) << mat(0, (upLimit + c) % oldC, oldR, 1);
          upLimit = oldC + leftCols;
          for (int c=rightCols; c--; )
            result(topRows, upLimit+c, oldR, 1) << mat(0, c % oldC , oldR, 1);
          upLimit = oldR - topRows;
          for (int r=topRows; r--; )
            result(r, 0, 1, -1) << result((upLimit + r) % oldR + topRows, 0, 1, -1);
          upLimit = oldR + topRows;
          for (int r=bottomRows; r--; )
            result(upLimit + r, 0, 1, -1) << result(r % oldR + topRows, 0, 1, -1);
        }
				break;

			case ExtendReplicate:
        if (leftCols)
          {
            PiiMatrix<T> column(mat(0, 0, oldR, 1));
            for (int c=leftCols; c--; )
              result(topRows, c, oldR, 1) << column;
          }
        if (rightCols)
          {
            PiiMatrix<T> column(mat(0, -1, oldR, 1));
            for (int c=rightCols; c--; )
              result(topRows, -c-1, oldR, 1) << column;
          }
        if (topRows)
          {
            PiiMatrix<T> row(result(topRows, 0, 1, -1));
            for (int r=topRows; r--; )
              result(r, 0, 1, -1) << row;
          }
        if (bottomRows)
          {
            PiiMatrix<T> row(result(-bottomRows-1, 0, 1, -1));
            for (int r=bottomRows; r--; )
              result(-r-1, 0, 1, -1) << row;
          }
        break;

			case ExtendZeros:
      default:
        break;
			}

		return result;
	}

  template <class T, class Collection> PiiMatrix<T> listToMatrix(const Collection& lst, Pii::MatrixDirection direction)
  {
    if (direction == Pii::Horizontally)
      {
        PiiMatrix<T> result(1, lst.size());
        Pii::copy(lst.begin(), lst.end(), result.rowBegin(0));
        return result;
      }
    else
      {
        PiiMatrix<T> result(lst.size(), 1);
        Pii::copy(lst.begin(), lst.end(), result.columnBegin(0));
        return result;
      }
  }

  template <class Collection, class T> Collection rowToList(const PiiMatrix<T>& matrix, int row)
  {
    Collection result;
    for (int c=0; c<matrix.columns(); c++)
      result << matrix(row, c);
    return result;
  }

  template <class Collection, class T> Collection columnToList(const PiiMatrix<T>& matrix, int column)
  {
    Collection result;
    for (int r=0; r<matrix.rows(); r++)
      result << matrix(r, column);
    return result;
  }

  template <class Rule, class T> void findFirst(const PiiMatrix<T>& matrix, T value, int &r, int& c)
  {
    Rule rule;
    for (r = 0; r<matrix.rows(); r++)
      {
        const T* row = matrix.row(r);
        for (c = 0; c<matrix.columns(); c++)
          if (rule(row[c],value))
            return;
      }
    r = c = -1;
  }

  template <class Rule, class T> void findLast(const PiiMatrix<T>& matrix, T value, int &r, int& c)
  {
    Rule rule;
    for (r = matrix.rows(); r--; )
      {
        const T* row = matrix.row(r);
        for (c = matrix.columns(); c--; )
          if (rule(row[c],value))
            return;
      }
  }

  template <class T> PiiMatrix<T> replicate(const PiiMatrix<T>& matrix, int vertical, int horizontal)
  {
    PiiMatrix<T> result(PiiMatrix<T>::uninitialized(matrix.rows()*vertical, matrix.columns()*horizontal));
    if (result.rows() == 0 || result.columns() == 0)
      return result;
    for (int r=0; r<vertical; ++r)
      for (int c=0; c<horizontal; ++c)
        result(r*matrix.rows(), c*matrix.columns(), matrix.rows(), matrix.columns()) << matrix;
    return result;
  }

  template <class Matrix1, class Matrix2>
  PiiMatrix<typename Matrix1::value_type> concatenate(const PiiConceptualMatrix<Matrix1>& mat1,
                                                      const PiiConceptualMatrix<Matrix2>& mat2,
                                                      Pii::MatrixDirection direction)
  {
    typedef typename Matrix1::value_type T;

    if (mat2.rows() == 0 || mat2.columns() == 0)
      return PiiMatrix<T>(*mat1.self());
    if (mat1.rows() == 0 || mat1.columns() == 0)
      return PiiMatrix<T>(*mat2.self());

    if (direction == Pii::Vertically)
      {
        if (mat1.columns() != mat2.columns())
          PII_MATRIX_SIZE_MISMATCH;
        
        PiiMatrix<T> result(PiiMatrix<T>::uninitialized(mat1.rows() + mat2.rows(), mat1.columns()));
        result(0, 0, mat1.rows(), -1) << mat1;
        result(mat1.rows(), 0, -1, -1) << mat2;
        
        return result;
      }
    else
      {
        if (mat1.rows() != mat2.rows())
          PII_MATRIX_SIZE_MISMATCH;
        
        PiiMatrix<T> result(PiiMatrix<T>::uninitialized(mat1.rows(), mat1.columns() + mat2.columns()));
        result(0, 0, -1, mat1.columns()) << mat1;
        result(0, mat1.columns(), -1, -1) << mat2;

        return result;
      }
  }

  template <class T> void randomizeMatrix(PiiMatrix<T>& mat, const T& lowerBound, const T& upperBound)
  {
    for (typename PiiMatrix<T>::iterator i = mat.begin(); i != mat.end(); ++i)
      *i = T(uniformRandom(lowerBound, upperBound));
  }

  template <class T> void flip(PiiMatrix<T>& mat, Pii::MatrixDirections directions)
  {
    const int iRows = mat.rows(), iCols = mat.columns();
    if (directions == (Pii::Horizontally | Pii::Vertically))
      {
        T* p1,*p2;
        for (int r=0; r<iRows/2; ++r)
          {
            p1 = mat.row(r);
            p2 = mat.row(iRows-1-r) + iCols-1;
            for (int c=0; c<iCols; ++c, ++p1, --p2)
              qSwap(*p1, *p2);
          }
        if (mat.rows() & 1)
          {
            p1 = mat.row(iRows/2);
            p2 = p1 + iCols-1;
            for (; p1 < p2; ++p1, --p2)
              qSwap(*p1, *p2);
          }        
      }
    else if (directions == Pii::Horizontally)
      {
        T* p1,*p2;
        for (int r=0; r<iRows; ++r)
          {
            p1 = mat.row(r);
            p2 = p1 + iCols-1;
            for (int c=0; c<iCols/2; ++c, ++p1, --p2)
              qSwap(*p1, *p2);
          }
      }
    else if (directions == Pii::Vertically)
      {
        for (int r=0; r<iRows/2; ++r)
          mat.swapRows(r, iRows-1-r);
      }
  }

  template <class T> PiiMatrix<T> flipped(const PiiMatrix<T>& mat, Pii::MatrixDirections directions)
  {
    const int iRows = mat.rows(), iCols = mat.columns();
    PiiMatrix<T> result(PiiMatrix<T>::uninitialized(iRows, iCols));
    if (directions == (Pii::Horizontally | Pii::Vertically))
      {
        const T* p1;
        T* p2;
        for (int r=0; r<iRows; ++r)
          {
            p1 = mat.row(r);
            p2 = result.row(iRows-1-r) + iCols-1;
            for (int c=iCols; c--; p1++, p2--)
              *p2 = *p1;
          }
      }
    else if (directions == Pii::Horizontally)
      {
        const T* p1;
        T* p2;
        for (int r=0; r<iRows; ++r)
          {
            p1 = mat.row(r);
            p2 = result.row(r) + iCols-1;
            for (int c=iCols; c--; p1++, p2--)
              *p2 = *p1;
          }
      }
    else if (directions == Pii::Vertically)
      {
        int rowLength = iCols*sizeof(T);
        for (int r=0; r<iRows; ++r)
          memcpy(result.row(iRows-1-r), mat.row(r), rowLength);
      }

    return result;
  }

  /**
   * @internal
   * Optimized quicksort for matrix rows.
   */
  template <class T, class LessThan> void sortRows(PiiMatrix<T>& matrix, int column,
                                                   T* pivotRow,
                                                   int firstRow, int lastRow,
                                                   LessThan lessThan)
  {
    if (firstRow < lastRow)
      {
        int iStartIndex, iEndIndex;
        const int iColumns = matrix.columns(), iBytesPerRow = iColumns * sizeof(T);
        iStartIndex = firstRow;
        iEndIndex = lastRow;
        // Take the last row as a pivot
        memcpy(pivotRow, matrix.row(lastRow), iBytesPerRow);
        do
          {
            // Find first row that is not smaller than pivot.
            while (iStartIndex < iEndIndex &&
                   !lessThan(pivotRow[column], matrix.at(iStartIndex, column)))
              ++iStartIndex;
            // Find last row that is not greater
            while ((iEndIndex > iStartIndex) &&
                   !lessThan(matrix.at(iEndIndex, column), pivotRow[column]))
              --iEndIndex;
            // Swap rows
            if (iStartIndex < iEndIndex)
              {
                T *pFirstRow = matrix.rowBegin(iStartIndex),
                  *pLastRow = matrix.rowBegin(iEndIndex);
                for (int i=0; i<iColumns; ++i)
                  qSwap(pFirstRow[i], pLastRow[i]);
              }
          } while (iStartIndex < iEndIndex);

        // Now, rows above iStartIndex are smaller or equal than the
        // pivot, and rows below (or at iStartIndex) are greater or
        // equal.
        memcpy(matrix.rowBegin(lastRow), matrix.constRowBegin(iStartIndex), iBytesPerRow);
        memcpy(matrix.rowBegin(iStartIndex), pivotRow, iBytesPerRow);
        
        sortRows(matrix, column, pivotRow, firstRow, iStartIndex-1, lessThan);
        sortRows(matrix, column, pivotRow, iStartIndex+1, lastRow, lessThan);
      }
  }
  
  template <class T, class LessThan> void sortRows(PiiMatrix<T>& matrix, LessThan lessThan, int column)
  {
    if (matrix.isEmpty()) return;
    T* pPivotRow = new T[matrix.columns()];
    sortRows(matrix, column,
             pPivotRow,
             0, matrix.rows()-1,
             lessThan);
    delete[] pPivotRow;
  }
}
