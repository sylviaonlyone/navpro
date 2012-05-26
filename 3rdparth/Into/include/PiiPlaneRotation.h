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

#ifndef _PIIPLANEROTATION_H
#define _PIIPLANEROTATION_H

#include <PiiMath.h>

/**
 * Represents a rotation parallel to a plane spanned by two
 * coordinates axes. The rotation can be represented by a matrix of
 * the form
 *
 * @f[
 * R = \left(
 * \begin{array}{ccccccc}
 * 1 & \cdots & 0 & \cdots & 0 & \cdots & 0 \         \
 * \vdots & \ddots & \vdots & & \vdots & & \vdots \   \
 * 0 & \cdots & c & \cdots & s & \cdots & 0 \        \
 * \vdots & & \vdots & \ddots & \vdots & & \vdots \   \
 * 0 & \cdots & -s & \cdots & c & \cdots & 0 \         \
 * \vdots & & \vdots & & \vdots & \ddots & \vdots \   \
 * 0 & \cdots & 0 & \cdots & 0 & \cdots & 1
 * \end{array}
 * \right)
 * @f]
 *
 * Note that the rotation only affects the rows and columns that have
 * non-zero off-diagonal entries in R. Therefore, it only affects a
 * two-dimensional subspace of the full n-dimensional space. When
 * applied from the left, the rotation matrix rotates column vectors
 * and touches only two rows in the matrix. When applied from the
 * right, row vectors will be rotated and only two columns will be
 * affected.
 *
 * This class does not store the full rotation matrix. Instead, just
 * the coefficients @e c and @e s are stored.
 *
 * @ingroup Matrix
 */
template <class T> class PiiPlaneRotation
{
public:
  PiiPlaneRotation() : c(0), s(0) {}
  PiiPlaneRotation(const PiiPlaneRotation& other) : c(other.c), s(other.s) {}
  PiiPlaneRotation(T c_, T s_) : c(c_), s(s_) {}

  PiiPlaneRotation& operator= (const PiiPlaneRotation& other)
  {
    c = other.c;
    s = other.s;
    return *this;
  }

  /**
   * Combines two rotations to one as if the rotation matrices were
   * multiplied.
   */
  PiiPlaneRotation operator* (const PiiPlaneRotation& other) const
  {
    return PiiPlaneRotation(c * other.c - Pii::conj(s) * other.s,
                            Pii::conj(c * Pii::conj(other.s) + Pii::conj(s) * Pii::conj(other.c)));
  }

  /**
   * Returns @p true if @a other is equal to this, and @p false
   * otherwise.
   */
  bool operator== (const PiiPlaneRotation& other) const { return c == other.c && s == other.s; }

  /**
   * Applies the rotation matrix to @a mat from the left. The vectors
   * to be rotated are stored as column vectors in @a mat, hence the
   * name. The rotation will affect all columns but only rows @a p and
   * @p q (p < q).
   */
  template <class Matrix> void rotateColumns(PiiConceptualMatrix<Matrix, Pii::RandomAccessMatrix>& mat,
                                             int p, int q) const
  {
    rotateInPlane(mat.rowBegin(p), mat.rowBegin(q), mat.columns(), c, s);
  }

  /**
   * Applies the rotation matrix to @a mat from the right. The vectors
   * to be rotated are stored as row vectors in @a mat, hence the
   * name. The rotation will affect all rows but only columns @a p and
   * @p q (p < q).
   */
  template <class Matrix> void rotateRows(PiiConceptualMatrix<Matrix, Pii::RandomAccessMatrix>& mat,
                                          int p, int q) const
  {
    rotateInPlane(mat.columnBegin(p), mat.columnBegin(q), mat.rows(), c, -Pii::conj(s));
  }

  T c, s;

private:
  template <class Iterator> static void rotateInPlane(Iterator x, Iterator y, int n, T c, T s)
  {
    T conjS = Pii::conj(s);
    T conjC = Pii::conj(c);
    for (; n>0; --n, ++x, ++y)
    {
      T xTmp = *x, yTmp = *y;
      *x = c * xTmp + conjS * yTmp;
      *y = -s * xTmp + conjC * yTmp;
    }
  }
};

namespace Pii
{
  /**
   * Returns the transpose of @a rotation. Since rotation matrices are
   * orthogonal, the transposition of a rotation matrix equals its
   * inverse (rotation to the opposite direction).
   *
   * @relates PiiPlaneRotation
   */
  template <class T> PiiPlaneRotation<T> transpose(const PiiPlaneRotation<T>& rotation)
  {
    return PiiPlaneRotation<T>(rotation.c, -conj(rotation.s));
  }

  /**
   * Creates a matrix that rotates a column vector [a, b]' so that it
   * becomes [r, 0]' when applied from the left.
   *
   * @relates PiiPlaneRotation
   */
  template <class T> PiiPlaneRotation<T> givensRotation(T a, T b, T* r = 0)
  {
    PiiPlaneRotation<T> rotation;
    if (b == 0)
      {
        if (r != 0) *r = abs(a);
        rotation.c = sign(a);
      }
    else if (a == 0)
      {
        if (r != 0) *r = abs(b);
        rotation.s = sign(b);
      }
    else if (abs(b) > abs(a))
      {
        T t = a/b;
        T u = copySign(sqrt(1+t*t), b);
        if (r != 0) *r = b*u;
        rotation.s = 1/u;
        rotation.c = rotation.s * t;
      }
    else
      {
        T t = b/a;
        T u = copySign(sqrt(1+t*t), a);
        if (r != 0) *r = a*u;
        rotation.c = 1/u;
        rotation.s = rotation.c * t;
      }
    return rotation;
  }
   
  /**
   * Creates a matrix that, when applied as a similarity transform,
   * zeroes a pair of off-diagonal entries in a Hermitian (or
   * self-adjoint, in the real case symmetric) 2-by-2 matrix.
   *
   * @f[
   * A = \left( \begin{array}{cc}
   * a & b \\
   * b^* & c
   * \end{array} \right),
   * @f]
   *
   * where * denotes a complex conjugate.
   *
   * The rotation matrix R is chosen so that a similarity transform on
   * A yields a diagonal matrix:
   *
   * @f[
   * R^H A R = \left( \begin{array}{cc}
   * d & 0 \\
   * 0 & e
   * \end{array} \right),
   * @f]
   *
   * where @e d and @e e are non-zero. @f$R^H@f$ is the Hermitian
   * transpose of R.
   *
   * @relates PiiPlaneRotation
   */
  template <class T> PiiPlaneRotation<T> jacobiRotation(T a, T b, T c)
  {
    PiiPlaneRotation<T> rotation;
    if (b == 0)
      rotation.c = 1;
    else
      {
        T beta = (a-c) / (T(2) * abs(b));
        T tau = T(1) / (beta + copySign(sqrt(abs2(beta) + 1), beta));

        rotation.c = T(1) / sqrt(abs2(tau) + 1);
        rotation.s = -sign(tau) * (conj(b) / abs(b)) * abs(tau) * rotation.c;
      }
    return rotation;
  }
}

#endif //_PIIPLANEROTATION_H
