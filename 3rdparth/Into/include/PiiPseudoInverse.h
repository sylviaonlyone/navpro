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

#ifndef _PIIPSEUDOINVERSE_H
#define _PIIPSEUDOINVERSE_H

#include "PiiSvDecomposition.h"

namespace Pii
{
  /**
   * Returns the Moore-Penrose pseudoinverse of an m-by-n matrix. 
   * Pseudoinverse is the optimal solution (in least squares sense)
   * for an over-determined system of linear equations (i.e. m > n). 
   * The Moore-Penrose pseudoinverse of a matrix A has the following
   * properties:
   *
   * @li @f$AA^+A = A@f$
   * @li @f$A^+AA^+ = A^+@f$
   * @li @f$(AA^+)^H = AA^+@f$ (H denotes a Hermitian transpose)
   * @li @f$(A^+A)^H = A^+A@f$
   *
   * This function uses @ref svDecompose() "SVD" to calculate the
   * pseudoinverse in a numerically stable way.
   */
  template <class Matrix>
  PiiMatrix<typename Matrix::value_type> pseudoInverse(const PiiRandomAccessMatrix& A)
  {
    typedef typename Matrix::value_type T;
    // A = USV* -> A+ = VS+U*
    // A is m by n
    // U is m by m
    // S is m by n -> S+ is n by m
    // V is n by n
    // VS+ is n by m
    
    PiiMatrix<T> U, V;
    PiiMatrix<T> S = svDecompose(A, &U, &V);
    const T* pSingularValues = S[0];
    T zeroTolerance = epsilon<T>() * qMax(A.rows(), A.columns()) * maxAll(S);
    int iNonZeroSVs = 0;

    // The pseudoinverse of a diagonal rectangular matrix is
    // calculated by taking the reciprocals of non-zero diagonal
    // entries and transposing the matrix. First, count non-zero
    // singular values.
    int iColumns = S.columns();
    for (; iNonZeroSVs<iColumns && pSingularValues[iNonZeroSVs] > zeroTolerance; ++iNonZeroSVs) ;

    // The rightmost part will be zeroed out in multiplication anyway.
    V.resize(V.rows(), iColumns); 
    // Calculate V*S+
    for (int r=0; r<V.rows(); ++r)
      {
        T* pRow = V[r];
        int c=0;
        for (; c<iNonZeroSVs; ++c)
          pRow[c] /= pSingularValues[c];
        for (; c<iColumns; ++c)
          pRow[c] = T(0);
      }
    // The rightmost part will be zeroed out in multiplication anyway.
    U.resize(U.rows(), iColumns);
    return V * adjoint(U);
  }
}

#endif //_PIIPSEUDOINVERSE_H
