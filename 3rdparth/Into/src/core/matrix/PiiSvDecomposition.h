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

#ifndef _PIISVDECOMPOSITION_H
#define _PIISVDECOMPOSITION_H

#include "PiiPlaneRotation.h"
#include "PiiQrDecomposition.h"

namespace Pii
{
  enum SvdOption
    {
      SvdThinU = 0,
      SvdThinV = 0,
      SvdFullU = 1,
      SvdFullV = 2
    };
  Q_DECLARE_FLAGS(SvdOptions, SvdOption);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Pii::SvdOptions)

namespace Pii
{
  /// @internal 2x2 SVD
  template <class T>
  void svDecompose(PiiMatrix<T,2,2>& mat,
                   PiiPlaneRotation<T>& leftRotation,
                   PiiPlaneRotation<T>& rightRotation)
  {
    PiiPlaneRotation<T> rotation;
    T t = mat(0,0) + mat(1,1);
    T d = mat(1,0) - mat(0,1);

    // Diagonal entries are zero -> turn 90 dgr
    if (t == 0)
      rotation.s = sign(d);
    else
      {
        T u = d / t;
        rotation.c = T(1) / sqrt(abs2(u) + 1);
        rotation.s = rotation.c * u;
      }
    
    rotation.rotateColumns(mat, 0, 1);
    rightRotation = jacobiRotation(mat(0,0), mat(0,1), mat(1,1));
    leftRotation = rotation * transpose(rightRotation);
  }

  /**
   * @overload
   *
   * This version requires a preallocated temporary storage @a tmp, a
   * k-by-k square matrix in which k is the minimum of A's dimensions.
   */
  template <class Matrix>
  PiiMatrix<typename Matrix::value_type> svDecompose(const PiiRandomAccessMatrix& A,
                                                     PiiMatrix<typename Matrix::value_type>& tmp,
                                                     PiiMatrix<typename Matrix::value_type>* U,
                                                     PiiMatrix<typename Matrix::value_type>* V,
                                                     SvdOptions options = SvdFullU | SvdFullV)
  {
    typedef typename Matrix::value_type Real;
    const int iRows = A.rows(), iCols = A.columns();
    const int iMinSize = tmp.rows();
    // 2 * epsilon is enough
    const Real precision = epsilon<Real>() * 2;

    // Square matrix doesn't need preconditioning
    if (iRows == iCols)
      {
        tmp << A.selfRef()(0, 0, iMinSize, iMinSize);
        if (U != 0)
          {
            U->resize(iRows, options & SvdFullU ? iRows : iMinSize);
            setIdentity(*U);
          }
        if (V != 0)
          {
            V->resize(iCols, options & SvdFullV ? iCols : iMinSize);
            setIdentity(*V);
          }
      }
    else
      {
        // Use QR decomposition to reduce the problem to a square case.

        // This will store the upper triangular matrix R to tmp.
        PiiMatrix<Real> matQ = qrDecompose(A, &tmp, UnpackEconomyQR);
        SvdOption fullOption = SvdFullU;
        // If the input has more columns than rows, swap the roles of
        // V and U.
        if (iCols > iRows)
          {
            qSwap(V,U);
            fullOption = SvdFullV;
          }
        if (U != 0)
          {
            if (options & fullOption)
              *U = matQ;
            else
              {
                U->resize(iRows, iMinSize);
                setIdentity(*U);
                *U *= matQ;
              }
          }
        if (V != 0)
          {
            V->resize(iCols, options & SvdFullV ? iCols : iMinSize);
            setIdentity(*V);
          }
      }

    // Iterate until the work matrix is diagonal.
    
    PiiMatrix<Real,2,2> mat2x2;
    PiiPlaneRotation<Real> leftRotation, rightRotation;
    
    bool bDone = false;
    while (!bDone)
      {
        bDone = true;

        /*
          Pick all possible pairs of indices (p,q) along the diagonal
          and perform a 2x2 SVD on the selected sub-matrix. In a 4x4
          case, if q = 0 and p = 2:

          +.+.
          ....
          +.+.
          ....

          The elements marked with + are taken as the 2x2 sub-matrix
          to decompose.
        */

        for (int p=1; p<iMinSize; ++p)
          {
            Real* pPRow = tmp[p];
            for(int q=0; q<p; ++q)
              {
                Real* pQRow = tmp[q];
                // If either of the non-diagonal entries differs
                // significantly from zero, we need to decompose the
                // sub-matrix.
                if (max(abs(pPRow[q]), abs(pQRow[p])) > max(abs(pPRow[p]), abs(pQRow[q])) * precision)
                  {
                    bDone = false;

                    mat2x2(0,0) = pPRow[p], mat2x2(0,1) = pPRow[q];
                    mat2x2(1,0) = pQRow[p], mat2x2(1,1) = pQRow[q];

                    // Apply Jacobi rotations to the sub-matrix to make it diagonal.
                    // PENDING Complex matrices need special handling here
                    svDecompose(mat2x2, leftRotation, rightRotation);

                    // Apply the rotations to the whole matrix
                    leftRotation.rotateColumns(tmp, p, q);
                    if (U != 0) transpose(leftRotation).rotateRows(*U, p, q);

                    rightRotation.rotateRows(tmp, p, q);
                    if (V != 0) rightRotation.rotateRows(*V, p, q);
                  }
              }
          }
      }

    // Make all singular values positive (and fix signs in U if needed)
    PiiMatrix<Real> matSingularValues(1, iMinSize);
    for (int i=0; i<iMinSize; ++i)
      {
        Real singularValue = matSingularValues(0,i) = abs(tmp(i,i));
        if (U != 0 && singularValue != 0)
          ::Pii::map(U->columnBegin(i), U->columnEnd(i),
                     std::bind2nd(std::multiplies<Real>(), tmp(i,i)/singularValue));
      }

    // Finally, sort singular values in descending order and swap
    // colums in U and V respectively.

    Real *pValuesBegin = matSingularValues[0],
      *pValuesEnd = pValuesBegin + iMinSize;
    for (int i=0; i<iMinSize; ++i, ++pValuesBegin)
      {
        Real* pMax = findSpecialValue(pValuesBegin, pValuesEnd, std::greater<Real>(), Identity<Real>());
        if (*pMax == 0)
          break;
        if (pMax != pValuesBegin)
          {
            qSwap(*pMax, *pValuesBegin);
            int j = i + pMax - pValuesBegin;
            if (U != 0) swap(U->columnBegin(i), U->columnEnd(i), U->columnBegin(j));
            if (V != 0) swap(V->columnBegin(i), V->columnEnd(i), V->columnBegin(j));
          }
      }
    return matSingularValues;
  }

  /**
   * Calculates the Singular Value Decomposition (SVD) of a matrix.
   * The SVD decomposes an m-by-n matrix @a A into @f$A = U*S*V^T@f$
   * with the following properties:
   *
   * If @f$m \ge n@f$
   * @li U is an m-by-n matrix with orthonormal column vectors.
   * @li S is a diagonal n-by-n matrix with the singular values on its
   * diagonal in descending order (the largest at (0,0)).
   * @li V is an orthogonal n-by-n matrix.
   * 
   * If @f$m < n@f$
   * @li U is an orthogonal m-by-m matrix
   * @li S is a diagonal m-by-m matrix with the singular values on its
   * diagonal in descending order (the largest at (0,0)).
   * @li V is an n-by-m matrix with orthonormal column vectors.
   *
   * The algorithm used for calculating the SVD is based on two-sided
   * @ref jacobiRotation "Jacobi rotations". If the input matrix is
   * not square, @ref qrDecompose() "QR decomposition" is first used
   * to first reduce the input matrix to a square upper diagonal
   * matrix.
   *
   * @tparam Real the number type used in calculations (should be a
   * floating-point type)
   * 
   * @param A the input matrix
   *
   * @param U pointer to a matrix that will store U.
   *
   * @param V pointer to a matrix that will store V.
   *
   * @return a @e row @e vector that stores the singular values in
   * descending magnitude order. Since the matrix S is always
   * diagonal, this function just stores the diagonal entries as a row
   * vector.
   *
   * @note Output matrices U and V will be automatically resized.
   */
  template <class Matrix>
  PiiMatrix<typename Matrix::value_type> svDecompose(const PiiRandomAccessMatrix& A,
                                                     PiiMatrix<typename Matrix::value_type>* U = 0,
                                                     PiiMatrix<typename Matrix::value_type>* V = 0,
                                                     SvdOptions options = SvdFullU | SvdFullV)
  {
    const int iRows = A.rows(), iCols = A.columns();
    int iMinSize = qMin(iRows, iCols);
    PiiMatrix<typename Matrix::value_type> matTmp(iMinSize, iMinSize);
    return svDecompose(A, matTmp, U, V, options);
  }  
}

#endif //_PIISVDECOMPOSITION_H
