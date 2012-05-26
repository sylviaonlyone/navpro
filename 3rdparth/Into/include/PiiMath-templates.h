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

#ifndef _PIIMATH_H
# error "Never use <PiiMath-templates.h> directly; include <PiiMath.h> instead."
#endif

#include <numeric>
#include <algorithm>
#include <functional>
#include "PiiIeeeFloat.h"
#include "PiiTransposedMatrix.h"

namespace Pii
{
  /*
   * Finds a "pivot" in a matrix. The "pivot" of a column is the
   * element that has the largest absolute value. This function 
   * looks the pivot only from diagonal and below.
   *
   * @return the index of the row with the largest element or -1 if
   * the largest element is 0 within tolerance.
   */
  template <class Iterator> int pivot(Iterator begin, const int n)
  {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    T maxVal(abs(*begin++));
    int maxIndex = 0;
    // Find the largest entry on this column (start at the diagonal)
    for (int i=1; i<n; ++i, ++begin)
      {
        T val = abs(*begin);
        if (val > maxVal)
          {
            maxVal = val;
            maxIndex = i;
          }
      }

    // If the largest element is zero, this is a singular matrix
    if (maxVal <= Numeric<T>::tolerance())
      return -1;

    return maxIndex;
  }


  template <class T> bool isSingular(const PiiMatrix<T>& mat)
  {
    if (mat.rows() != mat.columns()) return false;
    return (abs(determinant(mat)) <= Numeric<T>::tolerance());
  }

	
  template <class T> bool isDiagonal(const PiiMatrix<T>& mat)
  {
    if (mat.rows() != mat.columns())
      return false;
    for (int r=0; r<mat.rows(); r++)
      for (int c=0; c<mat.columns(); c++)
        if (r != c && mat(r,c) != T(0))
          return false;
    return true;
  }

  template <class T> bool isSymmetric(const PiiMatrix<T>& mat)
  {
    if (mat.rows() != mat.columns())
      return false;
    for (int r=0; r<mat.rows(); r++)
      for (int c=r+1; c<mat.columns(); c++)
        if (mat(r,c) != mat(c,r))
          return false;
    return true;
  }

  template <class T> bool isAntiSymmetric(const PiiMatrix<T>& mat)
  {
    if (mat.rows() != mat.columns())
      return false;
    for (int r=0; r<mat.rows(); r++)
      for (int c=r; c<mat.columns(); c++)
        if (mat(r,c) != -mat(c,r))
          return false;
    return true;
  }

  template <class T> bool isSquare(const PiiMatrix<T>& mat)
  {
    return (mat.rows() == mat.columns());
  }

  template <class T> bool isOrthogonalLike(const PiiMatrix<T>& mat, const T& tolerance)
  {
    if (mat.rows() >= mat.columns())
      return almostEqual(transpose(mat) * mat, PiiMatrix<T>::identity(mat.columns()), tolerance);
    else
      return almostEqual(mat * transpose(mat), PiiMatrix<T>::identity(mat.rows()), tolerance);
  }

  template <class Matrix1, class Matrix2>
  bool almostEqual(const PiiConceptualMatrix<Matrix1>& mat1,
                   const PiiConceptualMatrix<Matrix2>& mat2,
                   typename Matrix1::value_type tolerance)
  {
    if (mat1.rows() != mat2.rows() || mat1.columns() != mat2.columns())
      return false;
    return almostEqual(mat1.begin(), mat1.end(), mat2.begin(), tolerance);
  }

  template <class InputIterator1, class InputIterator2>
  bool almostEqual(InputIterator1 begin1, InputIterator1 end1,
                   InputIterator2 begin2,
                   typename std::iterator_traits<InputIterator1>::value_type tolerance)
  {
    for (; begin1 != end1; ++begin1, ++begin2)
      if (abs(*begin1 - *begin2) > tolerance)
        return false;
    return true;
  }

  template <class T> T determinant2x2(const PiiMatrix<T>& m)
  {
    return m(0,0)*m(1,1) - m(0,1)*m(1,0);
  }

  template <class T> T determinant3x3(const PiiMatrix<T>& m)
  {
    return   (m(0,0) * ( m(1,1)*m(2,2) - m(1,2)*m(2,1) )
            - m(0,1) * ( m(1,0)*m(2,2) - m(1,2)*m(2,0) )
            + m(0,2) * ( m(1,0)*m(2,1) - m(1,1)*m(2,0) ));
  }

  template <class T> T determinant(const PiiMatrix<T>& mat)
  {
    PII_MATRIX_CHECK_SQUARE(mat);
    const int iSize = mat.rows();

    if (iSize == 2)
      return determinant2x2(mat);
    
    if (iSize == 3)
      return determinant3x3(mat);

    T det(1);
    PiiMatrix<T> tmp(mat);
		
    for (int r=0; r<iSize; ++r)
      {
        int index = pivot(tmp.columnBegin(r) + r, iSize - r);
        if (index == -1)
          return T(0);
        if (index != 0)
          {
            tmp.swapRows(index + r, r);
            det = -det;
          }
        T pivotVal = tmp(r,r);
        det *= pivotVal;
        pivotVal = T(1.0)/pivotVal;
        for (int r2=r+1; r2<iSize; ++r2)
          {
            T *sourceRow = tmp.row(r);
            T *resultRow = tmp.row(r2);
            T val = resultRow[r] * pivotVal;
            for (int c=r+1; c<iSize; ++c)
              resultRow[c] -= val * sourceRow[c];
          }
      }
    return det;
  }

  template <class T> PiiMatrix<T> inverse(const PiiMatrix<T>& mat)
  {
    PII_MATRIX_CHECK_SQUARE(mat);
    const int iSize = mat.rows();

    // Gauss-Jordan elimination

    PiiMatrix<T> tmp(mat);
    PiiMatrix<T> result(PiiMatrix<T>::identity(mat.rows()));

    for(int r1=0; r1<iSize; r1++)
      {
        // Find the maximum magnitude at this column ("pivot")
        int index = pivot(tmp.columnBegin(r1) + r1, iSize - r1);
        if (index == -1)
          PII_THROW(PiiMathException, QCoreApplication::translate("PiiMath", "Matrix is singular and cannot be inverted."));
        // Interchange rows to put the pivot element on the diagonal
        if (index != 0)
          {
            index += r1;
            result.swapRows(index, r1);
            tmp.swapRows(index, r1);
          }

        T pivotVal = T(1.0)/tmp(r1, r1); // the pivot value is on the diagonal
        //qDebug("r1 = %d, pivot = %lf, pivot row = %d", r1, tmp(r1,r1), index);
        T* resultRow = result.row(r1);
        T* tmpRow = tmp.row(r1);
        for(int c=0; c<iSize; c++)
          { // divide row by the pivot
            resultRow[c] *= pivotVal;
            tmpRow[c] *= pivotVal;
          }
        
        // Subtract this row from others to make the rest of column r1 zero
        for(int r2=0; r2<iSize; r2++)
          {
            T* tmpRow2 = tmp.row(r2);
            T scale = tmpRow2[r1];
            
            // skip rows that are already zero
            if ((r1 != r2) && scale != 0)
              {
                T* resultRow2 = result.row(r2);
                for (int c=0; c<iSize; c++)
                  {
                    tmpRow2[c] -= scale * tmpRow[c];
                    resultRow2[c] -= scale * resultRow[c];
                  }
              }
          }
      }
    return result;
  }
	
  template <class T> T innerProduct(const PiiMatrix<T>& mat1, const PiiMatrix<T>& mat2)
  {
    PII_MATRIX_CHECK_EQUAL_SIZE(mat1,mat2);
    return innerProduct(mat1.begin(), mat1.end(), mat2.begin(), T(0));
  }

  template <class T> PiiMatrix<T> crossProduct(const PiiMatrix<T>& m1, const PiiMatrix<T>& m2)
  {
    PII_MATRIX_CHECK_EQUAL_SIZE(m1,m2);
    if (m1.rows() * m1.columns() != 3)
      PII_THROW(PiiMathException, QCoreApplication::translate("PiiMath", "Vectors must be 3-dimensional."));
    return PiiMatrix<T>(m1.rows(), m2.columns(),
                        m1(1)*m2(2) - m1(2)*m2(1),
                        m1(2)*m2(0) - m1(0)*m2(2),
                        m1(0)*m2(1) - m1(1)*m2(0));
  }

  template <class T> T gcd(T numerator, T denominator)
  {
    T larger, smaller, remainder;
    T tmpNumerator = abs(numerator);
    if (tmpNumerator > denominator)
      {
        larger = tmpNumerator;
        smaller = denominator;
      }
    else
      {
        larger = denominator;
        smaller = tmpNumerator;
      }

    if (smaller == 0)
      return larger;

    //Euclid's algorithm
    do
      {
        remainder = larger%smaller;
        larger = smaller;
        smaller = remainder;
      } while (remainder > 0);

    return larger;
  }

  template <class U, class Matrix> PiiMatrix<U> sum(const PiiRandomAccessMatrix& mat, MatrixDirection direction)
  {
    typedef typename Matrix::const_row_iterator Row;
    const int iRows = mat.rows(), iCols = mat.columns();
    if (direction == Vertically)
      {
        PiiMatrix<U> result(1, iCols);
        U* resultRow = result.row(0);
        for (int r=0; r<iRows; ++r)
          {
            Row sourceRow = mat.rowBegin(r);
            for (int c=0; c<iCols; ++c)
              resultRow[c] += U(sourceRow[c]);
          }
        return result;
      }
    else
      {
        PiiMatrix<U> result(PiiMatrix<U>::uninitialized(iRows, 1));
        for (int r=0; r<iRows; ++r)
          {
            U sum(0);
            Row row = mat.rowBegin(r);
            for (int c=0; c<iCols; ++c)
              sum += U(row[c]);
            *result[r] = sum;
          }
        return result;
      }
  }

  template <class UnaryFunction, class Matrix>
  PiiMatrix<typename UnaryFunction::result_type> cumulativeSum(const PiiRandomAccessMatrix& matrix,
                                                               UnaryFunction func,
                                                               CumulativeSumMode mode)
  {
    typedef typename UnaryFunction::result_type U;
    typedef typename Matrix::const_row_iterator Iterator;
    
    const int iRows = matrix.rows(), iCols = matrix.columns();

    // HACK the enum value equals to the size of border
    PiiMatrix<U> matResult(PiiMatrix<U>::uninitialized(iRows + int(mode), iCols + int(mode)));
    if (mode == ZeroBorderCumulativeSum)
      {
        matResult(0,0,1,-1) = 0;
        matResult(0,0,-1,1) = 0;
      }
    if (matrix.isEmpty())
      return matResult;
    
    // Handle first row specially
    Iterator pSource = matrix.rowBegin(0);
    U* pTarget = matResult[int(mode)] + int(mode);
    *pTarget = func(*pSource);
    for (int c=1; c<iCols; ++c)
      pTarget[c] = pTarget[c-1] + func(pSource[c]);

    // Pointer to the previous row. pTarget points to the current one.
    const U* pPrevTarget = pTarget;
    
    // Next rows add to the previous one
    for (int r=1; r<iRows; ++r)
      {
        // Pointers to the current rows
        pSource = matrix.rowBegin(r);
        pTarget = matResult[r+int(mode)] + int(mode);

        // First element of row just add to the one above it.
        U sum = func(*pSource);
        *pTarget = sum + *pPrevTarget;

        // Subsequent elements build up the row sum and add to the
        // elements above them.
        for (int c=1; c<iCols; c++)
          {
            sum += func(pSource[c]);
            pTarget[c] = sum + pPrevTarget[c];
          }

        pPrevTarget = pTarget;
      }
    return matResult;
  }

  template <class U, class Matrix> PiiMatrix<U> cumulativeSum(const PiiRandomAccessMatrix& mat,
                                                              MatrixDirection direction)
  {
    typedef typename Matrix::const_row_iterator Iterator;
    const int iRows = mat.rows(), iCols = mat.columns();
    PiiMatrix<U> result(PiiMatrix<U>::uninitialized(iRows, iCols));
    if (direction == Vertically)
      {
        U* resultRow = result[0], *resultRow2;
        Iterator sourceRow = mat.rowBegin(0);
        
        for (int c=0; c<iCols; ++c)
          resultRow[c] = sourceRow[c];
        for (int r=1; r<iRows; ++r)
          {
            sourceRow = mat.rowBegin(r);
            resultRow2 = result[r];
            for (int c=0; c<iCols; ++c)
              resultRow2[c] = U(sourceRow[c]) + resultRow[c];

            resultRow = resultRow2;
          }
      }
    else
      {
        for (int r=0; r<iRows; ++r)
          {
            Iterator sourceRow = mat.rowBegin(r);
            U* resultRow = result[r];
            resultRow[0] = sourceRow[0];
            for (int c=1; c<iCols; ++c)
              resultRow[c] = resultRow[c-1] + U(sourceRow[c]);
          }
      }
    return result;
  }

  template <class U, class T>
  PiiMatrix<U> fastMovingAverage(const PiiMatrix<T>& cumulativeSum,
                                 int windowRows, int windowColumns)
  {
    // Calculate average with enough precision
    typedef typename Combine<T,U>::Type AvgType;
    
    // Initialize result matrix
    PiiMatrix<U> matResult(PiiMatrix<U>::uninitialized(cumulativeSum.rows()-1, cumulativeSum.columns()-1));

    if (windowColumns <= 0)
      windowColumns = windowRows;
    int iHalfRows = windowRows/2, iHalfCols = windowColumns/2;
    const int iRows = matResult.rows(), iCols = matResult.columns();
    int c1, c2, r1, r2;

    const T* pPrevRow, *pNextRow;
    U* pTarget;
    for (int r=0; r<iRows; ++r)
      {
        // Check image boundaries
        r1 = qMax(r-iHalfRows, 0);
        r2 = qMin(r+iHalfRows+1, iRows);
        pPrevRow = cumulativeSum[r1];
        pNextRow = cumulativeSum[r2];
        pTarget = matResult[r];
        for (int c=0; c<iCols; ++c)
          {
            c1 = qMax(c-iHalfCols, 0);
            c2 = qMin(c+iHalfCols+1, iCols);
            int iCount = (c2-c1) * (r2-r1);
            // Use the cumulative sum to calculate moving average.
            pTarget[c] = U(AvgType(pNextRow[c2] + pPrevRow[c1] - pNextRow[c1] - pPrevRow[c2]) / iCount);
          }
      }
    return matResult;
  }

  template <class InputIterator, class OutputIterator>
  void fastMovingAverage(InputIterator input, int n, OutputIterator output, int windowSize)
  {
    typedef typename std::iterator_traits<OutputIterator>::value_type T;

    if (n <= 0 || windowSize <= 0) return;

    if (n < windowSize)
      {
        T average(*input);
        for (int i=1; i<n; ++i)
          average += input[i];
        average /= n;
        for (int i=0; i<n; ++i)
          output[i] = average;
        return;
      }

    if (windowSize == 1)
      {
        for (int i=0; i<n; ++i)
          output[i] = T(input[i]);
        return;
      }

    const int iLeftWindow = windowSize / 2,
      iRightWindow = windowSize - iLeftWindow - 1;

    // Temporary storage for cumulative sum
    T* pSum = new T[n];
    *pSum = T(*input);
    for (int i=1; i<n; ++i)
      pSum[i] = pSum[i-1] + T(input[i]);
    
    // 1 2 3 4 5 6 7 8 9
    T boundaryAvg = pSum[iLeftWindow-1] / iLeftWindow;
    int i = 0;
    for (; i<iLeftWindow; ++i)
      output[i] = boundaryAvg;
    output[i] = pSum[windowSize-1] / windowSize;
    ++i;
    for (; i<n-iRightWindow; ++i)
      output[i] = (pSum[i+iRightWindow] - pSum[i-iLeftWindow-1]) / windowSize;
    if (iRightWindow > 0)
      {
        boundaryAvg = (pSum[n-1] - pSum[n-iRightWindow-1]) / iRightWindow;
        for (; i<n; ++i)
          output[i] = boundaryAvg;
      }
    
    delete[] pSum;
  }

  template <class T, class Matrix> T sumAll(const PiiConceptualMatrix<Matrix>& mat)
  {
    return Pii::accumulate(mat.begin(), mat.end(),
                           std::plus<T>(),
                           T(0));
  }

  template <class U, class T> U varAll(const PiiMatrix<T>& mat, U* mean)
  {
    U avg = meanAll<U>(mat);
    U sum(0);
    for (int r=mat.rows(); r--;)
      {
        const T* row = mat.row(r);
        for (int c=mat.columns(); c--; )
          {
            U diff = (U)row[c] - avg;
            sum += diff*diff;
          }
      }
    if (mean != 0)
      *mean = avg;
    return sum != 0 ? (sum / (mat.rows() * mat.columns())) : 0;
  }
  
  template <class U, class T> PiiMatrix<U> var(const PiiMatrix<T>& mat, MatrixDirection direction)
  {
    PiiMatrix<U> matMean(mean<U>(mat, direction));
    PiiMatrix<U> matVar(matMean.rows(), matMean.columns());

    const int iRows = mat.rows(), iColumns = mat.columns();
    if (direction == Vertically)
      {
        U* varRow = matVar.row(0);
        U* meanRow = matMean.row(0);
        for (int r=0; r<iRows; ++r)
          {
            const T *sourceRow = mat.row(r);
            for (int c=0; c<iColumns; ++c)
              {
                U diff = U(sourceRow[c]) - meanRow[c];
                varRow[c] += diff*diff;
              }
          }
        matVar /= mat.rows();
      }
    else
      {
        for (int r=0; r<iRows; ++r)
          {
            const T* sourceRow = mat.row(r);
            U tmpSum = 0;
            U tmpMean = matMean(r,0);
            for (int c=0; c<iColumns; ++c)
              {
                U diff = (U)sourceRow[c] - tmpMean;
                tmpSum += diff*diff;
              }
            matVar(r,0) = tmpSum / mat.columns();
          }
      }
    return matVar;
  }

  template <class T> PiiMatrix<double> covariance(const PiiMatrix<T>& mat,
                                                  PiiMatrix<double>* meanMatrix)
  {
    // Mean of all dimensions
    PiiMatrix<double> mu = mean<double>(mat, Vertically);
    PiiMatrix<double> result(mat.columns(), mat.columns());
    for (int r=mat.rows(); r--; )
      {
        PiiMatrix<double> x(mat(r,0,1,-1));
        x -= mu;
        // (x-mu)' * (x-mu) produces a M-by-M matrix
        result += transpose(x) * x;
      }
    result /= (mat.rows() - 1);
    // Store mean value
    if (meanMatrix != 0)
      *meanMatrix = mu;
    return result;
  }

  
  template <class T, class BinaryPredicate>
  PiiMatrix<T> minOrMax(const PiiMatrix<T>& mat, MatrixDirection direction,
                        BinaryPredicate compare)
  {
    const int iRows = mat.rows(), iCols = mat.columns();
    if (direction == Vertically)
      {
        PiiMatrix<T> matResult(PiiMatrix<T>::uninitialized(1, iCols));
        T* pResultRow = matResult.row(0);
        memcpy(pResultRow, mat[0], sizeof(T)*iCols);
        for (int r=1; r<iRows; ++r)
          {
            const T* pSourceRow = mat.row(r);
            for (int c=0; c<iCols; ++c)
              if (compare(pSourceRow[c], pResultRow[c]))
                pResultRow[c] = pSourceRow[c];
          }
        return matResult;
      }
    else
      {
        PiiMatrix<T> matResult(PiiMatrix<T>::uninitialized(iRows, 1));
        for (int r=0; r<iRows; ++r)
          {
            const T* pRow = mat.row(r);
            T extremum(pRow[0]);
            for (int c=1; c<iCols; ++c)
              if (compare(pRow[c], extremum))
                extremum = pRow[c];
            matResult(r,0) = extremum;
          }
        return matResult;
      }
  }

  template <class T> PiiMatrix<T> max(const PiiMatrix<T>& mat, MatrixDirection direction)
  {
    return minOrMax(mat, direction, std::greater<T>());
  }

  template <class T> PiiMatrix<T> min(const PiiMatrix<T>& mat, MatrixDirection direction)
  {
    return minOrMax(mat, direction, std::less<T>());
  }


  template <class T> void minMax(const PiiMatrix<T>& mat,
                                 T* minimum, T* maximum,
                                 int* minR, int* minC,
                                 int* maxR, int* maxC)
  {
    *minimum = *maximum = mat(0,0);
    if (minR) *minR = 0;
    if (minC) *minC = 0;
    if (maxR) *maxR = 0;
    if (maxC) *maxC = 0;
    for (int r=mat.rows(); r--;)
      {
        const T* row = mat.row(r);
        for (int c=mat.columns(); c--; )
          {
            if (row[c] < *minimum)
              {
                *minimum = row[c];
                if (minR) *minR = r;
                if (minC) *minC = c;
              }
            else if (row[c] > *maximum)
              {
                *maximum = row[c];
                if (maxR) *maxR = r;
                if (maxC) *maxC = c;
              }
          }
      }
  }

  template <class T> PiiMatrix<T> diff(const PiiMatrix<T>& mat, int step, int order, MatrixDirection direction)
  {
    if (direction == Horizontally)
      {
        if (step >= mat.columns())
          return PiiMatrix<T>(mat.rows(), 0);
        
        PiiMatrix<T> result(PiiMatrix<T>::uninitialized(mat.rows(), mat.columns()-step));
    
        for (int r=mat.rows(); r--; )
          {
            T* target = result.row(r);
            const T* source = mat.row(r);
            for (int c=mat.columns()-step; c--; )
              target[c] = source[c+step] - source[c];
          }

        // Recurse if order is larger than one
        return order == 1 ? result : diff(result, step, order-1, direction);
      }
    else
      {
        if (step >= mat.rows())
          return PiiMatrix<T>(0, mat.columns());
        
        PiiMatrix<T> result(PiiMatrix<T>::uninitialized(mat.rows()-step, mat.columns()));
    
        for (int r=step; r<mat.rows(); ++r)
          {
            T* target = result.row(r-step);
            const T* source1 = mat.row(r);
            const T* source2 = mat.row(r-step);
            for (int c=mat.columns(); c--; )
              target[c] = source1[c] - source2[c];
          }
    
        // Recurse if order is larger than one
        return order == 1 ? result : diff(result, step, order-1, direction);
      }
  }

  template <class T, class U> PiiMatrix<T> centralDiff(const PiiMatrix<U>& mat, MatrixDirection direction)
  {
    PiiMatrix<T> matResult(mat.rows(), mat.columns());

    if (direction == Horizontally)
      {
        if (mat.columns() < 2)
          return matResult;
        
        for (int r=mat.rows(); r--; )
          {
            T* pResultRow = matResult[r];
            const U* pSourceRow = mat[r];
            pResultRow[0] = pSourceRow[1] - pSourceRow[0];
            for (int c=1; c<mat.columns()-1; ++c)
              pResultRow[c] = (T(pSourceRow[c+1]) - T(pSourceRow[c-1])) / 2;
            pResultRow[mat.columns()-1] = pSourceRow[mat.columns()-1] - pSourceRow[mat.columns()-2];
          }
      }
    else
      {
        if (mat.rows() < 2)
          return matResult;

        
        const U* pSourceRow1 = mat[0], *pSourceRow2 = mat[1];
        T* pTargetRow = matResult[0];

        for (int c=0; c<mat.columns(); ++c)
          pTargetRow[c] = (T(pSourceRow2[c]) - T(pSourceRow1[c]));

        for (int r=1; r<mat.rows()-1; ++r)
          {
            pTargetRow = matResult[r];
            pSourceRow1 = mat[r-1];
            pSourceRow2 = mat[r+1];
            for (int c=0; c<mat.columns(); ++c)
              pTargetRow[c] = (T(pSourceRow2[c]) - T(pSourceRow1[c])) / 2;
          }

        pTargetRow = matResult[mat.rows()-1];
        pSourceRow1 = mat[mat.rows()-2];
        for (int c=0; c<mat.columns(); ++c)
          pTargetRow[c] = (T(pSourceRow2[c]) - T(pSourceRow1[c]));
      }
    
    return matResult;
  }

  template <class T, class UnaryPredicate> PiiMatrix<int> find(const PiiMatrix<T>& mat, UnaryPredicate predicate, int cnt)
  {
    //First calculate the number of matching entries
    int matches = 0;
    for (int r=mat.rows(); r--; )
      {
        const T* row = mat.row(r);
        for (int c=mat.columns(); c--; )
          if (predicate(row[c]))
            matches++;
      }

    // Negative count -> omit last entries
    if (cnt < 0)
      matches += cnt;
    // Positive value limits matches only if it is not too large
    else if (cnt > 0 && cnt < matches)
      matches = cnt;

    if (matches <= 0)
      return PiiMatrix<int>();

    // If the input matrix is a vector, return linear indices
    if (mat.rows() == 1 || mat.columns() == 1)
      {
        PiiMatrix<int> result(PiiMatrix<int>::uninitialized(1, matches));
        int length = mat.rows() * mat.columns();
        int index = 0;
        for (int i=0; i<length; i++)
          {
            if (predicate(mat(i)))
              {
                result(0,index++) = i;
                if (index >= matches)
                  break;
              }
          }
        return result;
      }
    // For a matrix, row and column coordinates are returned
    else
      {
        PiiMatrix<int> result(PiiMatrix<int>::uninitialized(matches, 2));
        int index = 0;
        for (int r=0; r<mat.rows(); r++)
          {
            const T* row = mat.row(r);
            for (int c=0; c<mat.columns(); c++)
              if (predicate(row[c]))
                {
                  result(index,0) = r;
                  result(index,1) = c;
                  index++;
                  if (index >= matches)
                    goto find_out1;
                }
          }
      find_out1:
        return result;
      }
  }

  template <class T> void findExtrema(const PiiMatrix<T>& mat, PiiHeap<PiiMatrixValue<T>,16>& heap)
  {
    const int iRows = mat.rows(), iCols = mat.columns();
    for (int r=0; r<iRows; ++r)
      {
        const T* pRow = mat[r];
        for (int c=0; c<iCols; ++c)
          heap.put(PiiMatrixValue<T>(pRow[c], r, c));
      }
    heap.sort();
  }
    
  template <class T> T kthSmallest(T* data,int size, int k)
  {
    if(size <= 6)
      {
        //Case when there is not so many elements we just sort data
        //and take k:th element. 
        
        insertionSort(data,size);
        return data[k];
       
      }
    else 
      {
        
        //Lets divide data to to five element groups. 

        int overElements = size % 5;
        int groups = size / 5;

        //Here we go through data and divide it to groups (which
        //contains five elements) and we search median value to each group. 
        //after that we will swap median value to begin of each data.

        for (int i = 0; i < groups; ++i)
          {
            median5(data + i*5);
            qSwap(data[i], data[i*5+2]);
          }

        if (overElements != 0)
          {
            T* lastElements = data + groups*5;
            T value = median(lastElements, overElements);
            for (int i=0; i<overElements; ++i)
              {
                if (lastElements[i] == value)
                  {
                    qSwap(lastElements[i], data[groups]);
                    break;
                  }
              }
            ++groups;
          }
                
        //Then we will search median of medians.

        T m = median(data, groups);

        //Now we will use partition function to divide data so that at
        //position x there will be median of medians (m) and on
        //position 0 to x-1 there will be smaller values and at x+1 to
        //groups there will be greater values. 

        int index = 0;
        for(int p = 0; p < size; ++p)
          {
            if(data[p] == m)
              {
                index = p;
                break;
              }
          }

        int x = partition(data, size, index); 

        if (k == x)
          return m;
        else if (k < x)
          return kthSmallest(data, x, k);
        else
          return kthSmallest(data + x+1, size-x-1, k-x-1);
          
        
      }
  }
  template <class T> int partition(T* data, int size, int pivot)
  {

    T pivotValue = data[pivot];
    qSwap(data[pivot], data[size-1]);
    int storePos = 0;
    int elements = size-1;
    for(int loadPos = 0; loadPos < elements; ++loadPos)
      {
        if (data[loadPos] < pivotValue)
          {
            qSwap(data[loadPos], data[storePos]);
            storePos++;
          }
      }

    qSwap(data[storePos], data[elements]);

    return storePos;
        
  }

  // template <class Iterator, class Comparator> void
  // insertionSort(Iterator start, Iterator end, Comparator greater)
  template <class T> void insertionSort(T* data, int size)
  {
    T tmp = T(0);

    // Iterator j;
    // for (Iterator i = start + 1; i != end; ++i)
    for(int i = 1, j = 0; i < size; ++i)
      {
        tmp = data[i];
        j = i-1;
        // while (j != start && greater(*j, tmp))
        while(j >= 0 && data[j] > tmp)
          {
            //*(j+1) = *j
            data[j+1] = data[j];
            --j;
          }
        // *(j+1) = tmp
        data[j+1] = tmp;
      }

   }

  template <class T> T gaussian(T x, double mu, double sigma)
  {
    //sqrt(2*M_PI) \approx 2.506628275

    double a = 1.0/(sigma*2.506628275);
    double b = (x-mu)*(x-mu);
    return static_cast<T>(a*exp(-b/(2.0*(sigma*sigma))));
    
  }
  
  template <class T> T medianAll(const PiiMatrix<T>& mat)
  {
    return median(mat.begin(), mat.rows() * mat.columns());
  }

  /*
   * This median routine is based on the algorithm described in
   * "Numerical recipes in C", Second Edition,
   * Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
   * This code by Nicolas Devillard - 1998. Public domain.
   */

#define MEDIAN_ELEM_SWAP(a,b) { tmp=(a);(a)=(b);(b)=tmp; }
#define MEDIAN_SORT(a,b) { if ((a)>(b)) MEDIAN_ELEM_SWAP((a),(b)); }

  template <class Iterator> typename std::iterator_traits<Iterator>::value_type fastMedian(Iterator data, int len)
  {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    int low = 0, high = len-1;
    int medianIndex = (low + high) >> 1;
    int middle, ll, hh;
    T tmp;

    for (;;)
      {
        if (high <= low) /* One element only */
          return data[medianIndex] ;
        if (high == low + 1)
          { /* Two elements only */
            if (data[low] > data[high])
              MEDIAN_ELEM_SWAP(data[low], data[high]) ;
            return data[medianIndex] ;
          }
        /* Find median of low, middle and high items; swap into position low */
        middle = (low + high) / 2;
        if (data[middle] > data[high])    MEDIAN_ELEM_SWAP(data[middle], data[high]) ;
        if (data[low] > data[high])       MEDIAN_ELEM_SWAP(data[low], data[high]) ;
        if (data[middle] > data[low])     MEDIAN_ELEM_SWAP(data[middle], data[low]) ;
        /* Swap low item (now in position middle) into position (low+1) */
        MEDIAN_ELEM_SWAP(data[middle], data[low+1]) ;
        /* Nibble from each end towards middle, swapping items when stuck */
        ll = low + 1;
        hh = high;
        for (;;)
          {
            do ll++; while (data[low] > data[ll]) ;
            do hh--; while (data[hh] > data[low]) ;
            if (hh < ll)
              break;
            MEDIAN_ELEM_SWAP(data[ll], data[hh]) ;
          }
        /* Swap middle item (in position low) back into correct position */
        MEDIAN_ELEM_SWAP(data[low], data[hh]) ;
        /* Re-set active partition */
        if (hh <= medianIndex)
          low = ll;
        if (hh >= medianIndex)
          high = hh - 1;
      }
  }

  /*
   * The following code is public domain.
   * Algorithm by Torben Mogensen, implementation by N. Devillard.
   * This code in public domain.
   */

  template <class Iterator> typename std::iterator_traits<Iterator>::value_type median(Iterator data, int len)
  {
    typedef typename std::iterator_traits<Iterator>::value_type T;
    int i, less, greater, equal;
    T min, max, guess, maxltguess, mingtguess;
    min = max = data[0] ;
    for (i=1 ; i<len ; i++)
      {
        if (data[i]<min) min=data[i];
        if (data[i]>max) max=data[i];
      }
    for (;;)
      {
        guess = (min+max)/2;
        less = 0; greater = 0; equal = 0;
        maxltguess = min ;
        mingtguess = max ;
        for (i=0; i<len; i++)
          {
            if (data[i]<guess)
              {
                less++;
                if (data[i]>maxltguess) maxltguess = data[i] ;
              }
            else if (data[i]>guess)
              {
                greater++;
                if (data[i]<mingtguess) mingtguess = data[i] ;
              } else equal++;
          }
        if (less <= (len+1)/2 && greater <= (len+1)/2) break ;
        else if (less>greater) max = maxltguess ;
        else min = mingtguess;
      }
    if (less >= (len+1)/2) return maxltguess;
    else if (less+equal >= (len+1)/2) return guess;
    else return mingtguess;
  }

  template <class Iterator> typename std::iterator_traits<Iterator>::value_type median3(Iterator p)
  {
    typename std::iterator_traits<Iterator>::value_type tmp;
    MEDIAN_SORT(p[0],p[1]) ; MEDIAN_SORT(p[1],p[2]) ; MEDIAN_SORT(p[0],p[1]) ;
    return(p[1]) ;
  }
  template <class Iterator> typename std::iterator_traits<Iterator>::value_type median5(Iterator p)
  {
    typename std::iterator_traits<Iterator>::value_type tmp;
    MEDIAN_SORT(p[0],p[1]) ; MEDIAN_SORT(p[3],p[4]) ; MEDIAN_SORT(p[0],p[3]) ;
    MEDIAN_SORT(p[1],p[4]) ; MEDIAN_SORT(p[1],p[2]) ; MEDIAN_SORT(p[2],p[3]) ;
    MEDIAN_SORT(p[1],p[2]) ; return(p[2]) ;
  }
  template <class Iterator> typename std::iterator_traits<Iterator>::value_type median7(Iterator p)
  {
    typename std::iterator_traits<Iterator>::value_type tmp;
    MEDIAN_SORT(p[0], p[5]) ; MEDIAN_SORT(p[0], p[3]) ; MEDIAN_SORT(p[1], p[6]) ;
    MEDIAN_SORT(p[2], p[4]) ; MEDIAN_SORT(p[0], p[1]) ; MEDIAN_SORT(p[3], p[5]) ;
    MEDIAN_SORT(p[2], p[6]) ; MEDIAN_SORT(p[2], p[3]) ; MEDIAN_SORT(p[3], p[6]) ;
    MEDIAN_SORT(p[4], p[5]) ; MEDIAN_SORT(p[1], p[4]) ; MEDIAN_SORT(p[1], p[3]) ;
    MEDIAN_SORT(p[3], p[4]) ; return (p[3]) ;
  }
  template <class Iterator> typename std::iterator_traits<Iterator>::value_type median9(Iterator p)
  {
    typename std::iterator_traits<Iterator>::value_type tmp;
    MEDIAN_SORT(p[1], p[2]) ; MEDIAN_SORT(p[4], p[5]) ; MEDIAN_SORT(p[7], p[8]) ;
    MEDIAN_SORT(p[0], p[1]) ; MEDIAN_SORT(p[3], p[4]) ; MEDIAN_SORT(p[6], p[7]) ;
    MEDIAN_SORT(p[1], p[2]) ; MEDIAN_SORT(p[4], p[5]) ; MEDIAN_SORT(p[7], p[8]) ;
    MEDIAN_SORT(p[0], p[3]) ; MEDIAN_SORT(p[5], p[8]) ; MEDIAN_SORT(p[4], p[7]) ;
    MEDIAN_SORT(p[3], p[6]) ; MEDIAN_SORT(p[1], p[4]) ; MEDIAN_SORT(p[2], p[5]) ;
    MEDIAN_SORT(p[4], p[7]) ; MEDIAN_SORT(p[4], p[2]) ; MEDIAN_SORT(p[6], p[4]) ;
    MEDIAN_SORT(p[4], p[2]) ; return(p[4]) ;
  }
  template <class Iterator> typename std::iterator_traits<Iterator>::value_type median25(Iterator p)
  {
    typename std::iterator_traits<Iterator>::value_type tmp;
    MEDIAN_SORT(p[0], p[1]) ;   MEDIAN_SORT(p[3], p[4]) ;   MEDIAN_SORT(p[2], p[4]) ;
    MEDIAN_SORT(p[2], p[3]) ;   MEDIAN_SORT(p[6], p[7]) ;   MEDIAN_SORT(p[5], p[7]) ;
    MEDIAN_SORT(p[5], p[6]) ;   MEDIAN_SORT(p[9], p[10]) ;  MEDIAN_SORT(p[8], p[10]) ;
    MEDIAN_SORT(p[8], p[9]) ;   MEDIAN_SORT(p[12], p[13]) ; MEDIAN_SORT(p[11], p[13]) ;
    MEDIAN_SORT(p[11], p[12]) ; MEDIAN_SORT(p[15], p[16]) ; MEDIAN_SORT(p[14], p[16]) ;
    MEDIAN_SORT(p[14], p[15]) ; MEDIAN_SORT(p[18], p[19]) ; MEDIAN_SORT(p[17], p[19]) ;
    MEDIAN_SORT(p[17], p[18]) ; MEDIAN_SORT(p[21], p[22]) ; MEDIAN_SORT(p[20], p[22]) ;
    MEDIAN_SORT(p[20], p[21]) ; MEDIAN_SORT(p[23], p[24]) ; MEDIAN_SORT(p[2], p[5]) ;
    MEDIAN_SORT(p[3], p[6]) ;   MEDIAN_SORT(p[0], p[6]) ;   MEDIAN_SORT(p[0], p[3]) ;
    MEDIAN_SORT(p[4], p[7]) ;   MEDIAN_SORT(p[1], p[7]) ;   MEDIAN_SORT(p[1], p[4]) ;
    MEDIAN_SORT(p[11], p[14]) ; MEDIAN_SORT(p[8], p[14]) ;  MEDIAN_SORT(p[8], p[11]) ;
    MEDIAN_SORT(p[12], p[15]) ; MEDIAN_SORT(p[9], p[15]) ;  MEDIAN_SORT(p[9], p[12]) ;
    MEDIAN_SORT(p[13], p[16]) ; MEDIAN_SORT(p[10], p[16]) ; MEDIAN_SORT(p[10], p[13]) ;
    MEDIAN_SORT(p[20], p[23]) ; MEDIAN_SORT(p[17], p[23]) ; MEDIAN_SORT(p[17], p[20]) ;
    MEDIAN_SORT(p[21], p[24]) ; MEDIAN_SORT(p[18], p[24]) ; MEDIAN_SORT(p[18], p[21]) ;
    MEDIAN_SORT(p[19], p[22]) ; MEDIAN_SORT(p[8], p[17]) ;  MEDIAN_SORT(p[9], p[18]) ;
    MEDIAN_SORT(p[0], p[18]) ;  MEDIAN_SORT(p[0], p[9]) ;   MEDIAN_SORT(p[10], p[19]) ;
    MEDIAN_SORT(p[1], p[19]) ;  MEDIAN_SORT(p[1], p[10]) ;  MEDIAN_SORT(p[11], p[20]) ;
    MEDIAN_SORT(p[2], p[20]) ;  MEDIAN_SORT(p[2], p[11]) ;  MEDIAN_SORT(p[12], p[21]) ;
    MEDIAN_SORT(p[3], p[21]) ;  MEDIAN_SORT(p[3], p[12]) ;  MEDIAN_SORT(p[13], p[22]) ;
    MEDIAN_SORT(p[4], p[22]) ;  MEDIAN_SORT(p[4], p[13]) ;  MEDIAN_SORT(p[14], p[23]) ;
    MEDIAN_SORT(p[5], p[23]) ;  MEDIAN_SORT(p[5], p[14]) ;  MEDIAN_SORT(p[15], p[24]) ;
    MEDIAN_SORT(p[6], p[24]) ;  MEDIAN_SORT(p[6], p[15]) ;  MEDIAN_SORT(p[7], p[16]) ;
    MEDIAN_SORT(p[7], p[19]) ;  MEDIAN_SORT(p[13], p[21]) ; MEDIAN_SORT(p[15], p[23]) ;
    MEDIAN_SORT(p[7], p[13]) ;  MEDIAN_SORT(p[7], p[15]) ;  MEDIAN_SORT(p[1], p[9]) ;
    MEDIAN_SORT(p[3], p[11]) ;  MEDIAN_SORT(p[5], p[17]) ;  MEDIAN_SORT(p[11], p[17]) ;
    MEDIAN_SORT(p[9], p[17]) ;  MEDIAN_SORT(p[4], p[10]) ;  MEDIAN_SORT(p[6], p[12]) ;
    MEDIAN_SORT(p[7], p[14]) ;  MEDIAN_SORT(p[4], p[6]) ;   MEDIAN_SORT(p[4], p[7]) ;
    MEDIAN_SORT(p[12], p[14]) ; MEDIAN_SORT(p[10], p[14]) ; MEDIAN_SORT(p[6], p[7]) ;
    MEDIAN_SORT(p[10], p[12]) ; MEDIAN_SORT(p[6], p[10]) ;  MEDIAN_SORT(p[6], p[17]) ;
    MEDIAN_SORT(p[12], p[17]) ; MEDIAN_SORT(p[7], p[17]) ;  MEDIAN_SORT(p[7], p[10]) ;
    MEDIAN_SORT(p[12], p[18]) ; MEDIAN_SORT(p[7], p[12]) ;  MEDIAN_SORT(p[10], p[18]) ;
    MEDIAN_SORT(p[12], p[20]) ; MEDIAN_SORT(p[10], p[20]) ; MEDIAN_SORT(p[10], p[12]) ;
    return (p[12]);
  }

#undef MEDIAN_ELEM_SWAP
#undef MEDIAN_SWAP

  template <class Matrix>
  void normalize(PiiConceptualMatrix<Matrix, RandomAccessMatrix>& mat,
                 MatrixDirection direction)
  {
    const int iRows = mat.rows(), iColumns = mat.columns();
    if (direction == Horizontally)
      {
        for (int r=0; r<iRows; ++r)
          {
            typename Matrix::row_iterator row = mat.rowBegin(r);
            // Calculate the squared length of a vector
            double dNormalizer = innerProductN(row, iColumns, row, 0.0);
            if (dNormalizer != 0)
              // Divide each element by the length
              transformN(row, iColumns, row, std::bind2nd(std::multiplies<double>(), 1.0/sqrt(dNormalizer)));
          }
      }
    else
      {
        for (int c=0; c<iColumns; ++c)
          {
            typename Matrix::column_iterator column = mat.columnBegin(c);
            // Calculate the length of a vector
            double dNormalizer = innerProductN(column, iRows, column, 0.0);
            if (dNormalizer != 0)
              // Divide each element by the length
              transformN(column, iRows, column, std::bind2nd(std::multiplies<double>(), 1.0/sqrt(dNormalizer)));
          }
      }
  }

  template <class Matrix>
  PiiMatrix<double> normalized(const PiiConceptualMatrix<Matrix, RandomAccessMatrix>& mat,
                               MatrixDirection direction)
  {
    PiiMatrix<double> result(mat);
    normalize(result, direction);
    return result;
  }

  template <class UnaryOp, class Storage> void combinations(int n, int k, UnaryOp unaryOp, Storage storage)
  {
    while (n-- >= k)
      {
        if (k > 1)
          {
            storage[k-1] = n;
            combinations<UnaryOp,Storage>(n, k-1, unaryOp, storage);
          }
        else
          {
            storage[0] = n;
            unaryOp(storage);
          }
      }
  }

  template <class UnaryOp, class Storage> void permutations(int n, UnaryOp unaryOp, Storage storage)
  {
    // Break recursion
    if (n == 1)
      {
        unaryOp(storage);
        return;
      }
    --n;
    permutations(n, unaryOp, storage);
    for (int i=n; i--;)
      {
        qSwap(storage[n], storage[n-1]);
        permutations<UnaryOp,Storage>(n, unaryOp, storage);
      }
  }

  template <class UnaryOp> void permutations(int n, UnaryOp unaryOp)
  {
    int *storage = new int[n];
    // Initialize storage to 0123...
    for (int i=n; i--; storage[i] = i) {}

    permutations<UnaryOp,int*>(n, unaryOp, storage);
    
    delete[] storage;
  }

  template <class T> T factorial(T n)
  {
    T result(1);
    while (n > 1)
      result *= n--;
    return result;
  }

  template <class T, class U> class Averager
  {
  public:
    typedef U InputType;
    typedef T OutputType;
    
    Averager(const PiiMatrix<U>& mat) : _matInput(mat) {}
    PiiMatrix<T> averageMatrix() const { return _matAverage; }

  protected:
    const PiiMatrix<U>& _matInput;
    PiiMatrix<T> _matAverage;
  };
  
  template <class T, class U> class RowAverager : public Averager<T,U>
  {
  public:
    RowAverager(const PiiMatrix<U>& mat) : Averager<T,U>(mat), _pCurrentRow(0), _pTargetRow(0) {}

    bool isValidWidth(int width) const { return width <= this->_matInput.columns(); }
    int signalLength() const { return this->_matInput.columns(); }
    int signalCount() const { return this->_matInput.rows(); }
    void selectSignal(int signalIndex) { _pCurrentRow = this->_matInput[signalIndex]; _pTargetRow = this->_matAverage[signalIndex]; }
    T value(int index) const { return _pCurrentRow[index]; }

    void initAverageMatrix(int signalLength) { this->_matAverage.resize(this->_matInput.rows(), signalLength); }
    void setAverage(int index, T average) { _pTargetRow[index] = average; }

  private:
    const U* _pCurrentRow;
    T* _pTargetRow;
  };

  template <class T, class U> class ColumnAverager : public Averager<T,U>
  {
  public:
    ColumnAverager(const PiiMatrix<U>& mat) : Averager<T,U>(mat), _iCurrentColumn(-1) {}

    bool isValidWidth(int width) const { return width <= this->_matInput.rows(); }
    int signalLength() const { return this->_matInput.rows(); }
    int signalCount() const { return this->_matInput.columns(); }
    void selectSignal(int signalIndex) { _iCurrentColumn = signalIndex; }
    T value(int index) const { return this->_matInput(index, _iCurrentColumn); }

    void initAverageMatrix(int signalLength) { this->_matAverage.resize(signalLength, this->_matInput.columns()); }
    void setAverage(int index, T average) { this->_matAverage(index, _iCurrentColumn) = average; }

  private:
    int _iCurrentColumn;
  };
  
  template <class Iterator, class UnaryFunction> void windowSum(Iterator begin, Iterator end, int width,
                                                                UnaryFunction function)
  {
    typedef typename UnaryFunction::argument_type T;
    T* bfr = new T[width];
    for (int i=0; i<width; ++i) bfr[i] = 0;
    int halfWidth = width/2;
    int bfrIndex = 0;
    T sum = 0;
    Iterator head = begin, center = begin;
    int i=0;
    for (;;)
      {
        if (head != end)
          {
            sum -= bfr[bfrIndex];
            bfr[bfrIndex] = *head;
            bfrIndex = (bfrIndex+1) % width;
            sum += T(*head);
            ++head;
          }
        else
          {
            sum -= bfr[bfrIndex];
            bfrIndex = (bfrIndex+1) % width;
          }
        

        if (center != end)
          {
            if (++i > halfWidth)
              {
                *center = function(sum);
                ++center;
              }
          }
        else
          break;
      }
    delete[] bfr;
  }
  
  template <class Averager> PiiMatrix<typename Averager::OutputType> movingAverage(Averager avg,
                                                                                   int width,
                                                                                   EndPointHandling endPointHandling)
  {
    // Force width to be odd in this mode
    if (endPointHandling == ShrinkWindowSymmetrically)
      width |= 1;
    
    if (!avg.isValidWidth(width))
      {
        avg.initAverageMatrix(0);
        return avg.averageMatrix();
      }

    // Number of steps left/right from the current element
    int left = width/2, right = width - left;
    int signalLength = avg.signalLength();

    // In this case the result signals are shorter
    if (endPointHandling == OnlyValidPart)
      {
        signalLength -= width-1;
        left = 0;
        right = width;
        // Otherwise the functioning is the same as assuming zeros
        endPointHandling = AssumeZeros;
      }

    avg.initAverageMatrix(signalLength);

    for (int signal=0; signal < avg.signalCount(); ++signal)
      {
        avg.selectSignal(signal);
        for (int t=0; t<signalLength; ++t)
          {
            typename Averager::OutputType localAverage(0);
            // Find boundaries of the local averaging window
            int first = t-left, last = t+right;
            if (first < 0)
              {
                if (endPointHandling == ShrinkWindowSymmetrically)
                  last += first;
                first = 0;
              }
            else if (last > avg.signalLength())
              {
                if (endPointHandling == ShrinkWindowSymmetrically)
                  first += last - avg.signalLength();
                last = avg.signalLength(); 
              }
            // Sum up
            for (int i=first; i<last; ++i)
              localAverage += avg.value(i);
            // Divide based on selected mode
            if (endPointHandling == AssumeZeros)
              avg.setAverage(t, localAverage/width);
            else if (endPointHandling >= ShrinkWindow)
              avg.setAverage(t, localAverage/(last-first));
          }
      }
    
    return avg.averageMatrix();
  }

  template <class T, class U> PiiMatrix<T> movingAverage(const PiiMatrix<U>& mat,
                                                         int width,
                                                         MatrixDirection direction,
                                                         EndPointHandling endPointHandling)
  {
    if (direction == Horizontally)
      return movingAverage(RowAverager<T,U>(mat), width, endPointHandling);
    return movingAverage(ColumnAverager<T,U>(mat), width, endPointHandling);
  }

  template <class Matrix1, class Matrix2>
  PiiMatrix<double> squaredDistance(const PiiConceptualMatrix<Matrix1, RandomAccessMatrix>& points1,
                                    const PiiConceptualMatrix<Matrix2, RandomAccessMatrix>& points2,
                                    MatrixDirection direction)
  {
    if (points1.rows() != points2.rows() || points1.columns() != points2.columns())
      return PiiMatrix<double>();

    double dInitialValue = 0.0;
    const int iRows = points1.rows(), iColumns = points2.columns();
    if (direction == Horizontally)
      {
        PiiMatrix<double> result(iRows, 1);
        // First column on each row stores the distance between row vectors
        for (int r=0; r<iRows; ++r)
          result(r,0) = squaredDistanceN(points1.rowBegin(r), iColumns,
                                         points2.rowBegin(r), dInitialValue);
        return result;
      }
    else
      {
        PiiMatrix<double> result(1, iColumns);
        // The first row sums up squared distances between
        // corresponding dimensions
        double* pResultRow = result[0];
        for (int r=0; r<iRows; ++r)
          {
            typename Matrix1::const_row_iterator pRow1 = points1.rowBegin(r);
            typename Matrix2::const_row_iterator pRow2 = points2.rowBegin(r);
            for (int c=0; c<iColumns; ++c)
              pResultRow[c] += square(pRow2[c] - pRow1[c]);
          }

        return result;
      }
  }

  template <class Real> Real epsilon(Real value)
  {
    IeeeFloat<Real> eps(std::numeric_limits<Real>::epsilon());
    IeeeFloat<Real> val(abs(value));
    eps.addExponent(val.exponent());
    return eps.value();
  }

  template <class Matrix> int rank(const PiiConceptualMatrix<Matrix, Pii::RandomAccessMatrix>& mat,
                                   typename Matrix::value_type tolerance)
  {
    typedef typename Matrix::value_type Real;
    const int iRows = mat.rows(), iCols = mat.columns();
    PiiMatrix<Real> tmp(mat);
    if (isNan(tolerance))
      tolerance = epsilon(Real(0.0));

    int r = 0;
    for (int c=0; c<iCols && r<iRows; ++c)
      {
        // Find the maximum magnitude at this column ("pivot")
        int index = pivot(tmp.columnBegin(c) + r, iRows - r);
        if (index == -1)
          continue;
        // Interchange rows to put the pivot element on the diagonal
        if (index != 0)
          tmp.swapRows(r + index, r);

        Real* tmpRow = tmp.row(r);
        mapN(tmpRow, iCols, std::bind2nd(std::multiplies<Real>(), Real(1.0)/tmp(r,c)));
        
        // Subtract this row from others to make the rest of column r1 zero
        for (int r2=r+1; r2<iRows; ++r2)
          {
            Real* tmpRow2 = tmp.row(r2);
            Real scale = tmpRow2[c];
            
            // skip rows that are already zero
            if (abs(scale) < tolerance)
              for (int c2=0; c2<iCols; ++c2)
                tmpRow2[c2] -= scale * tmpRow[c2];
          }
        ++r;
      }
    //std::cout << r << "\n";
    //matlabPrint(std::cout, tmp);
    return r;
 
    /*
    PiiMatrix<Real> singularValues = svd(mat);

    if (isNan(tolerance))
      tolerance = qMax(mat.rows(), mat.columns()) * epsilon(singularValues(0,0));

    int rank = 0;
    for (int i=0; i<singularValues.columns(); ++i)
      {
        if (singularValues(0,i) > tolerance)
          ++rank;
      }
    return rank;
    */
  }
} //namespace Pii
