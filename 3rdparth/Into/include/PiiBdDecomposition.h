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

#ifndef _PIIBDDECOMPOSITION_H
#define _PIIBDDECOMPOSITION_H

namespace Pii
{
  /**
   * Reduces a rectangular matrix to a bidiagonal form. This algorithm
   * decomposes an m-by-n matrix @a A into three matrices Q, B, and P
   * so that A = QBP. Q and P are orthogonal matrices and B is a
   * bidiagonal matrix. If m < n, B will be a lower bidiagonal matrix,
   * otherwise it will be an upper bidiagonal matrix.
   *
   * The algorithm uses a two-sided series of @ref
   * householderTransform() "Householder reflections" to reduce the
   * input matrix to a bidiagonal form. The generated elementary
   * reflectors will be stored in a compact form to A upon return. The
   * lower triangle will store the left-hand reflectors just as with
   * @ref qrDecompose() "the QR decomposition". The upper triangle
   * will store the right-hand reflectors as row vectors. In the
   * illustration below, column (left-hand) reflectors are denoted by
   * v, row (right-hand) reflectors by u, the main diagonal by d, and
   * the super/subdiagonal by e.
   *
@verbatim
m >= n                       m < n

( d   e   u1  u1  u1 )      ( d   u1  u1  u1  u1  u1 )
( v1  d   e   u2  u2 )      ( e   d   u2  u2  u2  u2 )
( v1  v2  d   e   u3 )      ( v1  e   d   u3  u3  u3 )
( v1  v2  v3  d   e  )      ( v1  v2  e   d   u4  u4 )
( v1  v2  v3  v4  d  )      ( v1  v2  v3  e   d   u5 )
( v1  v2  v3  v4  v5 )
@endverbatim
   *
   * @code
   * PiiMatrix<double> matA(10,10); // input matrix
   * PiiMatrix<double> matTauQ, matTauP;
   * Pii::bdDecompose(matA, matTauQ, matTauP);
   * PiiMatrix<double> matQ = Pii::bdUnpackQ(matA, matTauQ[0]);
   * PiiMatrix<double> matP = Pii::bdUnpackP(matA, matTauP[0]);
   * // since A = Q * B * P,
   * // inverse(matQ) * matA * inverse(matP) is a bidiagonal matrix
   * // note that since Q and P are orthogonal,
   * // inverse(matQ) = transpose(matQ) and
   * // inverse(matP) = transpose(matP) and
   * @endcode
   *
   * @param A the input matrix. This matrix will be modified to store
   * Q, B, and P in a compact form as described above.
   *
   * @param tauQ an output-value matrix that will store the
   * coefficients for building Q.
   *
   * @param tauR an output-value matrix that will store the
   * coefficients for building R.
   *
   * @see bdUnpackQ()
   * @see bdUnpackP()
   * @see bdUnpackB()
   * @see bdUnpackDiagonals()
   */
  template <class Real>
  void bdDecompose(PiiMatrix<Real>& A,
                   PiiMatrix<Real>& tauQ,
                   PiiMatrix<Real>& tauP)
  {
    const int iRows = A.rows(), iCols = A.columns(),
      iMinDimension = qMin(iRows, iCols);

    if (iMinDimension == 0) return;

    Real* pBfr = new Real[qMax(iRows, iCols) + 1];
    tauQ.resize(1, iMinDimension);
    tauP.resize(1, iMinDimension);
    Real* pTauQ = tauQ[0], *pTauP = tauP[0];
    
    if (iRows >= iCols)
      {
        // Reduce to upper bidiagonal using a two-sided series of
        // Householder reflections.
        for (int i = 0; i < iCols; ++i)
          {
            // See qrDecompose() for an thorough explanation.
            typename PiiMatrix<Real>::column_iterator column = A.columnBegin(i)+i;
        
            int iRowsLeft = iRows - i, iColsLeft = iCols - i - 1;
            Real beta;
            // This transform nulls out the current column vector
            // below the diagonal.
            householderTransform(column, iRowsLeft, pTauQ + i, &beta);
            // Apply the reflection transform to the rest of A.
            if (iColsLeft > 0)
              {
                // Apply the reflection to the rest of the matrix...
                reflectColumns((PiiMatrix<Real>&)A(i, i+1, -1, -1), column, pTauQ[i], pBfr);
                // ... and this column.
                *column = beta;
                
                typename PiiMatrix<Real>::row_iterator row = A.rowBegin(i)+i+1;
                // This transform nulls out the current row vector
                // right of superdiagonal.
                householderTransform(row, iColsLeft, pTauP + i, &beta);
                // Apply the reflection to the rest of the matrix...
                reflectRows((PiiMatrix<Real>&)A(i+1, i+1, -1, -1), row, pTauP[i]);
                // ... and this row
                *row = beta;
              }
            else
              {
                *column = beta;
                pTauP[i] = 0;
              }
          }
      }
    else // iRows < iCols
      {
        // Reduce to lower bidiagonal
        for (int i = 0; i < iRows; ++i)
          {
            typename PiiMatrix<Real>::row_iterator row = A.rowBegin(i)+i;
            int iRowsLeft = iRows - i - 1, iColsLeft = iCols - i;
            Real beta;

            householderTransform(row, iColsLeft, pTauP + i, &beta);
            if (iRowsLeft > 0)
              {
                reflectRows((PiiMatrix<Real>&)A(i+1, i, -1, -1), row, pTauP[i]);
                *row = beta;

                typename PiiMatrix<Real>::column_iterator column = A.columnBegin(i)+i+1;
                householderTransform(column, iRowsLeft, pTauQ + i, &beta);
                reflectColumns((PiiMatrix<Real>&)A(i+1, i+1, -1, -1), column, pTauQ[i], pBfr);
                *column = beta;
              }
            else
              {
                *row = beta;
                pTauQ[i] = 0;
              }
          }
      } // iRows < iCols
    delete[] pBfr;
  } // bdDecompose()

  /**
   * Unpacks the result of bidiagonal decomposition. This function
   * returns the components of the left reflector matrix Q as @f$Q = I
   * + V T V^T@f$.
   *
   * @param V the result of bidiagonal decomposition in a compact
   * form. This matrix will be modified so that it contains contains
   * V.
   *
   * @param tauQ the coefficients needed for reconstructing Q
   *
   * @param diagonal 0, if reflector vectors start on the main
   * diagonal, 1 if they start at the first sub-diagonal.
   *
   * @param T a return-value matrix that will store the triangular
   * component of Q as an upper triangular matrix.
   *
   * @see bdDecompose()
   */
  template <class Real>
  void bdUnpackQ(PiiMatrix<Real>& V,
                 const Real* tauQ,
                 const int diagonal,
                 PiiMatrix<Real>& T)
  {
    const int iRows = V.rows();
    int iCols = V.columns();

    if (iCols > iRows)
      {
        V.resize(iRows, iRows);
        iCols = iRows;
      }
    if (diagonal > 0)
      fillN(V[0], iCols, Real(0));
    for (int d=0; d<iCols-diagonal; ++d)
      {
        Real* pRow = V[d+diagonal];
        pRow[d] = 1;
        fillN(pRow+d+1, iCols-d-1, Real(0));
      }
    int iSize = qMin(iRows, iCols-diagonal);
    PiiMatrix<Real> matGram(iSize, iSize);
    T.resize(iCols, iCols);
    unpackReflectors(Vertically, V, tauQ, T, matGram, diagonal);
  }

  /**
   * Unpacks the result of bidiagonal decomposition. This function
   * returns the left reflector matrix Q.
   *
   * @param A the result of bidiagonal decomposition in a compact
   * form.
   *
   * @param tauQ the coefficients needed for reconstructing Q
   *
   * @return the left reflector matrix Q
   *
   * @see bdDecompose()
   */
  template <class Real>
  PiiMatrix<Real> bdUnpackQ(const PiiMatrix<Real>& A,
                            const Real* tauQ)
  {
    PiiMatrix<Real> matT, matV;
    int iDiagonal = 0;
    if (A.rows() >= A.columns())
      matV = A;
    else
      {
        matV = A(0, 0, A.rows(), A.rows());
        iDiagonal = 1;
      }
    bdUnpackQ(matV, tauQ, iDiagonal, matT);
    PiiMatrix<Real> matQ(matV * matT * transpose(matV));
    for (int d=0; d<matQ.rows(); ++d)
      matQ(d,d) += 1;
    return matQ;
  }

  /**
   * Unpacks the result of bidiagonal decomposition. This function
   * returns the components of the right reflector matrix P as @f$P =
   * I + V^T T^T V@f$.
   *
   * @param V the result of bidiagonal decomposition in a compact
   * form. This matrix will be modified so that it contains contains
   * V.
   *
   * @param tauP the coefficients needed for reconstructing P
   *
   * @param diagonal 0, if reflector vectors start on the main
   * diagonal, 1 if they start at the first superdiagonal.
   *
   * @param T a return-value matrix that will store the triangular
   * component of P as an upper diagonal matrix.
   *
   * @see bdDecompose()
   */
  template <class Real>
  void bdUnpackP(PiiMatrix<Real>& V,
                 const Real* tauP,
                 const int diagonal,
                 PiiMatrix<Real>& T)
  {
    const int iCols = V.columns();
    int iRows = V.rows();

    if (iCols < iRows)
      {
        V.resize(iCols, iCols);
        iRows = iCols;
      }
    if (diagonal > 0)
      fillN(V.columnBegin(0), iRows, Real(0));
    int iSize = qMin(iRows-diagonal, iCols);
    for (int d=0; d<iSize; ++d)
      {
        Real* pDiagonal = fillN(V[d], d+diagonal, Real(0));
        *pDiagonal = 1;
      }

    PiiMatrix<Real> matGram(iSize, iSize);
    T.resize(iRows, iRows);
    unpackReflectors(Horizontally, V, tauP, T, matGram, diagonal);
  }

  /**
   * Unpacks the result of bidiagonal decomposition. This function
   * returns the right reflector matrix P.
   *
   * @param A the result of bidiagonal decomposition in a compact
   * form.
   *
   * @param tauP the coefficients needed for reconstructing P
   *
   * @return the left reflector matrix P
   *
   * @see bdDecompose()
   */
  template <class Real>
  PiiMatrix<Real> bdUnpackP(const PiiMatrix<Real>& A,
                            const Real* tauP)
  {
    PiiMatrix<Real> matT, matV;
    int iDiagonal = 0;
    if (A.rows() >= A.columns())
      {
        iDiagonal = 1;
        matV = A(0, 0, A.columns(), A.columns());
      }
    else
      matV = A;

    bdUnpackP(matV, tauP, iDiagonal, matT);
    PiiMatrix<Real> matP(transpose(matV) * transpose(matT) * matV);
    for (int d=0; d<matP.columns(); ++d)
      matP(d,d) += 1;
    return matP;
  }

  /**
   * Unpacks the result of bidiagonal decomposition. This function
   * modifies @p A so that it contains the bidiagonal part of the
   * decomposition result, B.
   *
   * @param A the result of bidiagonal decomposition in a compact
   * form. The reflector vectors will be nulled out, retaining only
   * the two diagonals.
   *
   * @see bdDecompose()
   */
  template <class Real>
  void bdUnpackB(PiiMatrix<Real>& A)
  {
    const int iRows = A.rows(), iCols = A.columns();
    if (iRows >= iCols)
      {
        for (int r=0; r<iRows; ++r)
          // Zero beginning of row, leave two diagonals, zero the rest
          fillN(fillN(A[r], qMin(r,iCols), Real(0)) + 2, iCols-r-2, Real(0));
      }
    else
      {
        fillN(A[0]+1, iCols-1, Real(0));
        for (int r=1; r<iRows; ++r)
          // Zero beginning of row, leave two diagonals, zero the rest
          fillN(fillN(A[r], r-1, Real(0)) + 2, iCols-r-1, Real(0));
      }
  }

  /**
   * Unpacks the result of bidiagonal decomposition. Fills the two
   * non-zero diagonals into the given arrays.
   *
   * @param A the result of bidiagonal decomposition in a compact form
   * (m by n matrix).
   *
   * @param d an array of at least min(m,n) entries. This will be
   * filled with the values on the main diagonal.
   *
   * @param e an array of at least min(m,n)-1 entries. This will be
   * filled with the values on the sub/superdiagonal, depending on the
   * size of @a A.
   *
   * @see bdDecompose()
   */
  template <class Real>
  void bdUnpackDiagonals(const PiiMatrix<Real>& A,
                         Real* d, Real* e)
  {
    const int iRows = A.rows(), iCols = A.columns(), iSize = qMin(iRows, iCols);
    int iRowOffset = 0, iColumnOffset = 1;
    if (iRows < iCols)
      qSwap(iRowOffset, iColumnOffset);
    for (int i=0; i<iSize; ++i)
      {
        d[i] = A(i,i);
        if (d < iSize-1)
          e[i] = A(i+iRowOffset, i+iColumnOffset);
      }
  }
}


#endif //_PIIBDDECOMPOSITION_H
