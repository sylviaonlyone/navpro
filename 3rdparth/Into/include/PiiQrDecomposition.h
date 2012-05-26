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

#ifndef _PIIQRDECOMPOSITION_H
#define _PIIQRDECOMPOSITION_H

#include "PiiHouseholderTransform.h"

namespace Pii
{
  /**
   * Unpacking styles for QR decomposition. Assume the input matrix A
   * is m-by-n.
   *
   * @lip UnpackEconomyQR - Either Q or R is m-by-n, and the other one
   * is a o-by-o matrix, where o = min(m,n). If m >= n: Q will be
   * m-by-n, R n-by-n. If m <= n: Q will be m-by-m, R m-by-n.
   *
   * @lip UnpackFullQR - Q will be m-by-m, R m-by-n. If m is large,
   * this style easily eats up all memory. Use with care.
   */
  enum QrUnpackStyle
    {
      UnpackEconomyQR = 0,
      UnpackFullQR = 1
    };
  
  /**
   * Unpacks the result of QR decomposition. Given a set of elementary
   * reflectors in A, this function calculates the reflector matrix Q
   * in the form Q = I + VTV'. This function requires preallocated
   * temporary storage and is intended to be used in nested loops.
   *
   * @param A QR decomposed matrix in compact form. This matrix will
   * be modified so that it only contains the reflector vectors below
   * and on the diagonal. That is, A will be converted to V.
   *
   * @param tau a pointer to the beginning of the tau vector
   *
   * @param T an output-value matrix. This must be a square matrix (@a
   * columns by @a columns).
   *
   * @param gram a temporary storage for a Gram matrix. Same size as
   * T.
   *
   * @param bfr a temporary storage of at least @a columns entries.
   *
   * @see unpackColumns()
   */
  template <class Real>
  void qrUnpack(PiiMatrix<Real>& A,
                const Real* tau,
                PiiMatrix<Real>& T,
                PiiMatrix<Real>& gram)
  {
    const int iCols = A.columns();
    // Turn A into V: zero the upper triangle and place ones on the
    // diagonal. After this we have the reflector vectors in the lower
    // triangle starting at the diagonal.
    for (int r=0; r<iCols; ++r)
      {
        typename PiiMatrix<Real>::row_iterator row = A.rowBegin(r) + r;
        *row++ = 1;
        fillN(row, iCols - r - 1, Real(0));
      }

    unpackReflectors(Vertically, A, tau, T, gram);
  }

  /**
   * Unpacks the result of QR decomposition. Given a set of elementary
   * reflectors in @a A and the @a tau array, this function calculates
   * the reflector matrix Q as follows:
   *
   * @f[
   * Q = H_0  H_1  \ldots H_k,
   * @f]
   *
   * where k is either the number of rows or columns in A, whichever
   * is smaller.
   *
   * Each H matrix can be represented as
   *
   * @f[
   * H_i = I - \tau_i v_i v_i^T,
   * @f]
   *
   * where @f$v_i@f$ represents the ith elementary reflector vector. 
   * The elementary reflectors are stored in @a A under the main
   * diagonal as columns, and the corresponding values of @f$\tau@f$
   * are in @a tau.
   *
   * @param A QR decomposed matrix in compact form (m-by-n). This
   * matrix will be modified so that it only contains the reflector
   * vectors below and on the diagonal.
   *
   * @param tau the tau vector as returned by qrDecompose().
   *
   * @param R a pointer to a matrix that will store the upper diagonal
   * part of the decomposition result.
   *
   * @param style unpacking style
   *
   * @return a m-by-m orthogonal matrix Q
   */
  template <class Real> PiiMatrix<Real> qrUnpack(PiiMatrix<Real>& A,
                                                 const PiiMatrix<Real>& tau,
                                                 PiiMatrix<Real>* R = 0,
                                                 QrUnpackStyle style = UnpackEconomyQR)
  {
    bool bMoreRows = true;
    const int iRows = A.rows(), iCols = A.columns();
    int iSize = iCols;
    if (iRows < iCols)
      bMoreRows = false, iSize = iRows;
    
    PiiMatrix<Real> matT(iSize, iSize), matGram(iSize, iSize);
    // If R is needed, copy the upper triangle from A.
    if (R != 0)
      {
        int iRRows, iRCols;
        if (style == UnpackEconomyQR && bMoreRows)
          iRRows = iRCols = iSize;
        else
          iRRows = iRows, iRCols = iCols;
        R->resize(iRRows, iRCols);
        *R = 0;
        for (int r=0; r<iSize; ++r)
          copyN(A.rowBegin(r)+r, iRCols - r, R->rowBegin(r)+r);
      }
    // Discard the rightmost columns of A (all zeros)
    if (!bMoreRows)
      A.resize(iSize, iSize);
    qrUnpack(A, tau[0], matT, matGram);
    PiiMatrix<Real> matQ;
    if (style == UnpackEconomyQR && bMoreRows)
      matQ = A * matT * transpose(A(0,0,iSize,iSize));
    else
      matQ = A * matT * transpose(A);
    for (int i=0; i<iSize; ++i)
      matQ(i,i) += 1;
    return matQ;
  }

  /**
   * QR decomposition. This function factorizes @a A into Q and R
   * using a series of Householder reflections. Upon return, @a A will
   * be modified so that it holds both Q and R in a compact form.
   *
   * @param A a m-by-n matrix to be decomposed. Upon return, this
   * matrix will store the matrices Q and R in a compact form.
   *
   * @param tau a vector that can be used to form Q. This array must
   * be at least min(m,n) elements long.
   *
   * @param bfr a temporary buffer of at least A.rows() elements.
   *
   * @see qrDecompose()(PiiMatrix<Real>&, PiiMatrix<double>&)
   * @see qrUnpack()
   */
  template <class Matrix>
  void qrDecompose(PiiRandomAccessMatrix& A, typename Matrix::value_type* tau, typename Matrix::value_type* bfr)
  {
    typedef typename Matrix::value_type Real;
    
    const int iRows = A.rows(), iCols = A.columns(),
      iMinDimension = qMin(iRows, iCols);

    // Iteratively apply Householder transformations to eliminate
    // elements below the diagonal.
    for (int i = 0; i < iMinDimension; ++i)
      {
        typename Matrix::column_iterator column = A.columnBegin(i)+i;
        
        /* Create a Householder transform out of the ith column of A
           (below diagonal) and store the resulting transform vector
           to its place.
        
           Assume i is one. We'll store the reflection vector v to
           column number one, starting at the diagonal. The submatrix
           marked with o's will be transformed using the reflection
           vector. The column itself won't be transformed as we
           already know the result: [ beta 0 ... 0 ]. The lower
           triangle is used to store the reflection vectors.

           . . . .
           . 1 o o <- the first element of v is always one
           . v o o
           . v o o
           . v o o
        */
           
        int iRowsLeft = iRows-i;
        Real beta;
        householderTransform(column, iRowsLeft, tau + i, &beta);
        // Apply the reflection transform to the rest of A.
        if (i < iCols-1)
          reflectColumns((PiiMatrix<Real>&)A.selfRef()(i, i+1, -1, -1), column, tau[i], bfr);

        /* Transform the current column as well. Actually, we only
           store the first element as all the rest are zeros. The rest
           of the column i is used to store v, whose ith element is
           always one.

           The result (t stands for transformed):
           . . . .
           . B t t
           . v t t
           . v t t
           . v t t
        */
        *column = beta;
      }
  }

  /**
   * QR decomposition. The QR algorighm is used to decompose a matrix
   * @a A into two matrices @a Q and @a R so that A = QR. If A is
   * m-by-n, Q is a m-by-m orthogonal matrix, and R is a m-by-n upper
   * triangular matrix.
   *
   * This function uses a blockwise version of the method of
   * Householder reflections to perform the decomposition.
   *
   * @param A a m-by-n matrix to be decomposed. Upon return, this
   * matrix will store the matrices Q and R in a compact form.
   *
   * @param tau a vector that can be used to form Q. This matrix will
   * be resized to 1-by-min(m,n).
   *
   * The non-zero elements of R will be stored on and above the main
   * diagonal of A. The lower triangle of A will store a set of
   * elementary reflector vectors (see householderTransform()) that
   * can be used to form Q together with the @a tau vector. Each
   * column represents one of the vectors, excluding the first
   * dimension, which is always one.
   *
@verbatim
m >= n                          m < n

(  r   r   r   r   r  )         (  r   r   r   r   r   r  )
(  v1  r   r   r   r  )         (  v1  r   r   r   r   r  )
(  v1  v2  r   r   r  )         (  v1  v2  r   r   r   r  )
(  v1  v2  v3  r   r  )         (  v1  v2  v3  r   r   r  )
(  v1  v2  v3  v4  r  )         (  v1  v2  v3  v4  r   r  )
(  v1  v2  v3  v4  v5 )
@endverbatim
   *
   * @see qrUnpack()
   */
  template <class Matrix>
  void qrDecompose(PiiRandomAccessMatrix& A,
                   PiiMatrix<typename Matrix::value_type>& tau)
  {
    typedef typename Matrix::value_type Real;
    /* Iteratively partition A so that A11 is a square matrix. Once
       done, partition A22 similarly and so on.
     
      +-----+---------+
      | A11 |   A12   |
      |     |         |
      +-----+---------+
      | A21 |   A22   |
      |     |         |
      |     |         |
      |     |         |
      |     |         |
      +-----+---------+

      (A11) = A1
      (A21)

      (A12) = A2
      (A22)
    */
    static const int iBlockSize = 8;

    const int iRows = A.rows(), iCols = A.columns(),
      iMinDimension = qMin(iRows, iCols);

    if (iMinDimension == 0) return;
    
    Real* pBfr = new Real[qMax(iRows, iCols)+1];
    tau.resize(1, iMinDimension);

    // If the matrix is small enough, just use the non-blocked
    // version.
    if (iMinDimension < iBlockSize)
      {
        qrDecompose(A, tau.row(0), pBfr);
        delete[] pBfr;
        return;
      }

    // Holds the current block (A11 and A21)
    PiiMatrix<Real> matA1(iRows, iBlockSize);
    // Space for a block reflector matrix T.
    PiiMatrix<Real> matT(iBlockSize, iBlockSize);
    // Temporary storage for block reflector calculation.
    PiiMatrix<Real> matGram(iBlockSize, iBlockSize);
    
    int iBlockStart = 0;
    // Block-based QR
    while (iBlockStart < iMinDimension)
      {
        int iCurrentBlockSize = qMin(iMinDimension-iBlockStart, iBlockSize);
        int iRowsLeft = iRows-iBlockStart;
        Real* pTauRow = tau.row(0) + iBlockStart;
      
        /* Decompose the current block (A1).
          
           Alglib implementation suggests that the submatrix should be
           copied to a temporary storage to "solve some TLB issues
           arising from non-contiguous memory access pattern". We take
           the advice.
        */
        matA1.resize(iRowsLeft, iCurrentBlockSize);
        matA1 << A.selfRef()(iBlockStart, iBlockStart, iRowsLeft, iCurrentBlockSize);
        qrDecompose(matA1, pTauRow, pBfr);
        A.selfRef()(iBlockStart, iBlockStart, iRowsLeft, iCurrentBlockSize) << matA1;

        // Done with the block. Now update blocks A12 and A22 (A2)
        if (iBlockStart+iCurrentBlockSize < iCols)
          {
            // If the remaining part is large, create a block
            // reflector matrix and apply it to A2.
            if (iCols-iBlockStart-iCurrentBlockSize >= 2*iBlockSize ||
                iRowsLeft >= 4*iBlockSize)
              {
                /* Prepare a reflector matrix Q based on A1
                   Q = H1 * H2 * ... * Hn, where Hx are the elementary
                   reflectors calculated for A1.
                   
                   It can be shown (they say) that Q = I + VTV', where
                   T is an upper triangular matrix and V has the
                   reflector vectors as its columns.

                   This function converts A1 to V and fills T.
                */
                qrUnpack(matA1, pTauRow, matT, matGram);
                
                /* Multiply the rest of A (that is, A2) by Q'.
                   Since V is now actually in A1, we get:
                
                   Q  = I + A1 T A1'
                   Q' = I + A1 T'A1'

                   We are doing this:
                   
                   A2 <- (I + A1 T'A1') A2
                   A2 <- A2 + A1 T'A1'A2
                */
                
                A.selfRef()(iBlockStart, iBlockStart + iCurrentBlockSize, -1, -1) +=      // A2  +=
                  matA1 *                                                       // A1  *
                  transpose(matT(0,0, iCurrentBlockSize, iCurrentBlockSize)) *  // T'  *
                  transpose(matA1) *                                            // A1' *
                  A.selfRef()(iBlockStart, iBlockStart + iCurrentBlockSize, -1, -1);      // A2
              }
            // The remaining part is small. Use the reflector vectors
            // directly.
            else
              {
                for (int i=0; i < iCurrentBlockSize; ++i)
                  {
                    matA1(i,i) = 1; // Reflector vector has one as its first element
                    reflectColumns((PiiMatrix<Real>&)A.selfRef()(iBlockStart+i, iBlockStart+iCurrentBlockSize, -1, -1),
                                   matA1.columnBegin(i)+i, pTauRow[i], pBfr);
                  }
              }
          }
      
        iBlockStart += iCurrentBlockSize;
      }
    delete[] pBfr;
  }

  /**
   * @overload
   *
   * This version does not modify the input @a A, and returns the Q
   * matrix unpacked. Optionally, the R matrix will also be filled.
   */
  template <class Matrix>
  PiiMatrix<typename Matrix::value_type> qrDecompose(const PiiRandomAccessMatrix& A,
                                                     PiiMatrix<typename Matrix::value_type>* R = 0,
                                                     QrUnpackStyle style = UnpackEconomyQR)
  {
    typedef typename Matrix::value_type Real;
    PiiMatrix<Real> matCopy(A);
    PiiMatrix<Real> matTau;
    qrDecompose(matCopy, matTau);
    return qrUnpack(matCopy, matTau, R, style);
  }
} //namespace Pii

#endif //_PIIQRDECOMPOSITION_H
