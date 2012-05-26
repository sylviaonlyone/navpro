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

#ifndef _PIIMATHEIGENSYSTEM_H
#define _PIIMATHEIGENSYSTEM_H


#include <PiiMatrix.h>

/*
 * The routines are adapted from TNT(template numerical toolkit),
 * which in turn was
 * "Adapted from JAMA, a Java Matrix Library, developed by jointly 
 *	by the Mathworks and NIST; see http://math.nist.gov/javanumerics/jama)."
 * Both were in the public domain.
 *
 * Note: What comes to performance, it's worth noting that in the JAMA webpage
 * the implementations were descripted as pedagogical reference implementations.
 */


namespace Pii
{

  /**
   * Implements eigenvalue calculations and stores the results of the calculations
   * (eigenvalues and eigenvectors). Intended to be used through
   * @ref Pii::eigensolve() "eigensolve".
   *
   * @tparam T Matrix elementtype
   * @tparam Real Real number type used in calculations.
   */
  template<class tT, class tReal>
  class EigenSystem
  {
  public:
    typedef tReal Real;
    typedef tT T;

    /**
     * Solves eigenvalues and vectors for given matrix.
     */
    void solve(const PiiMatrix<T>& mat);

    /**
     * Returns row vector with complex eigenvalues.
     */
    PiiMatrix<std::complex<Real> > eigenvalues() const;

    /**
     * Returns real parts of eigenvalues as row vector.
     */
    const PiiMatrix<Real>& eigenvaluesR() const {return _matRealParts;}

    /**
     * Return imaginary parts of eigenvalues as row vector.
     */
    const PiiMatrix<Real>& eigenvaluesI() const {return _matImagParts;}

    /**
     * Returns eigenvectors as columnvectors in a matrix.
    */
    const PiiMatrix<Real>& eigenvectors() const {return _matEigenvectors;}

    /**
     * Creates block diagonal eigenvalue matrix from eigenvalues.
     * For real eigenvalues, returns diagonal matrix with eigenvalues on diagonal(1x1 blocks),
     * and for complex eigenvalues block on diagonal is of size 2x2. For example if the
     * complex eigenvalues are u+iv and u-iv, the block looks like 
     * @code
     *  u     v
     * -v     u
     * @endcode
     */
    //Corresponds to the "getD()" function in TNT.
    PiiMatrix<Real> blockDiagonalEigenvalMatrix();

    /**
     * Solves eigenvalues of given matrix.
     *
     * @param mat Square matrix whose eigenvalues is to be calculated.
     * @param pRealpartArray Null pointer or pointer to array where realparts
     *        of eigenvalues will be stored. Make sure that the array is large 
     *        enough(at least the size of matrix dimension).
     * @param pImagpartArray Null pointer or pointer to array where imaginaryparts
     *        of eigenvalues will be stored. Make sure that the array is large 
     *        enough(at least the size of matrix dimension).
     * @param pEigenvectors. Null pointer or pointer toMatrix where to store
              eigenvectors as column vectors.
     *
     */
    static void solve(const PiiMatrix<T>& mat, Real* const pRealpartArray,
                Real* const pImagpartArray, PiiMatrix<Real>* const pEigenvectors);

    

  //Private methods.
  private:
    typedef Real* const Array1D;
    typedef PiiMatrix<Real> Array2D;

    /** Symmetric Householder reduction to tridiagonal form (adapted from TNT function Eigenvalue::tred2()) */
    static void tred2(const int n, Array1D& d, Array1D& e, Array2D& V);

    /** Symmetric tridiagonal QL algorithm  (Adapted from TNT function Eigenvalue::tql2()) */
    static void tql2(const int n, Array1D& d, Array1D& e, Array2D& V);

    /** Nonsymmetric reduction to Hessenberg form (adapted from TNT function Eigenvalue::orthes()) */
    static void orthes(const int n, Array2D& V, Array2D& H);

    /** Nonsymmetric reduction from Hessenberg to real Schur form(adapted from TNT function Eigenvalue::hqr2()) */
    static void hqr2(const int n, Array1D& d, Array1D& e, Array2D& V, Array2D& H);

    /** Complex scalar division, x/y (adapted from TNT function Eigenvalue::cdiv()) */
    static void cdiv(const Real xr, const Real xi, const Real yr, const Real yi, Real& cdivr, Real& cdivi);

  private:
    PiiMatrix<Real> _matRealParts;
    PiiMatrix<Real> _matImagParts;
    PiiMatrix<Real> _matEigenvectors;

  }; //End of EigenSystem class definition.


  template<class T, class Real>
  void EigenSystem<T, Real>::cdiv(const Real xr, const Real xi, const Real yr, const Real yi, Real& cdivr, Real& cdivi)
  {
    Real r,d;
    if (abs(yr) > abs(yi))
      {
        r = yi/yr;
        d = yr + r*yi;
        cdivr = (xr + r*xi)/d;
        cdivi = (xi - r*xr)/d;
      } 
    else
      {
        r = yr/yi;
        d = yi + r*yr;
        cdivr = (r*xr + xi)/d;
        cdivi = (r*xi - xr)/d;
      }
  }


  template<class T, class Real>
  void EigenSystem<T, Real>::tred2(const int n, Array1D& d, Array1D& e, Array2D& V)
  {
    //  This is derived from the Algol procedures tred2 by
    //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
    //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
    //  Fortran subroutine in EISPACK.

      for (int j = 0; j < n; j++)
        {
          d[j] = V[n-1][j];
        }

      // Householder reduction to tridiagonal form.
    
      for (int i = n-1; i > 0; i--)
        {
    
        // Scale to avoid under/overflow.
    
        Real scale = 0.0;
        Real h = 0.0;
        for (int k = 0; k < i; k++)
          {
            scale = scale + abs(d[k]);
          }
          if (scale == 0.0)
            {
              e[i] = d[i-1];
              for (int j = 0; j < i; j++)
                {
                  d[j] = V[i-1][j];
                  V[i][j] = 0.0;
                  V[j][i] = 0.0;
                }
            }
          else
            {
            // Generate Householder vector.
    
            for (int k = 0; k < i; k++)
              {
                d[k] /= scale;
                h += d[k] * d[k];
              }
            Real f = d[i-1];
            Real g = Pii::sqrt(h);
            if (f > 0)
              {
                g = -g;
              }
            e[i] = scale * g;
            h = h - f * g;
            d[i-1] = f - g;
            for (int j = 0; j < i; j++)
              {
                e[j] = 0.0;
              }
    
            // Apply similarity transformation to remaining columns.
    
            for (int j = 0; j < i; j++)
              {
                f = d[j];
                V[j][i] = f;
                g = e[j] + V[j][j] * f;
                for (int k = j+1; k <= i-1; k++)
                {
                  g += V[k][j] * d[k];
                  e[k] += V[k][j] * f;
                }
                e[j] = g;
              }
            f = 0.0;
            for (int j = 0; j < i; j++)
              {
                e[j] /= h;
                f += e[j] * d[j];
              }
            Real hh = f / (h + h);
            for (int j = 0; j < i; j++)
              {
                e[j] -= hh * d[j];
              }
            for (int j = 0; j < i; j++)
              {
                f = d[j];
                g = e[j];
                for (int k = j; k <= i-1; k++)
                  {
                    V[k][j] -= (f * e[k] + g * d[k]);
                  }
                d[j] = V[i-1][j];
                V[i][j] = 0.0;
              }
          }
          d[i] = h;
        }
    
      // Accumulate transformations.
    
      for (int i = 0; i < n-1; i++)
      {
        V[n-1][i] = V[i][i];
        V[i][i] = 1.0;
        Real h = d[i+1];
        if (h != 0.0)
          {
            for (int k = 0; k <= i; k++)
              {
                d[k] = V[k][i+1] / h;
              }
            for (int j = 0; j <= i; j++)
              {
                Real g = 0.0;
                for (int k = 0; k <= i; k++)
                  {
                    g += V[k][i+1] * V[k][j];
                  }
                for (int k = 0; k <= i; k++)
                  {
                    V[k][j] -= g * d[k];
                  }
              }
          }
          for (int k = 0; k <= i; k++)
            {
              V[k][i+1] = 0.0;
            }
      }
      for (int j = 0; j < n; j++)
        {
          d[j] = V[n-1][j];
          V[n-1][j] = 0.0;
        }
      V[n-1][n-1] = 1.0;
      e[0] = 0.0;
    } 



  template<class T, class Real>
  void EigenSystem<T, Real>::tql2(const int n, Array1D& d, Array1D& e, Array2D& V)
  {
    //  This is derived from the Algol procedures tql2, by
    //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
    //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
    //  Fortran subroutine in EISPACK.
    
    for (int i = 1; i < n; i++)
      {
        e[i-1] = e[i];
      }
    e[n-1] = 0.0;

    Real f = 0.0;
    Real tst1 = 0.0;
    Real eps = pow(2.0,-52.0);
    for (int l = 0; l < n; l++) 
      {
        // Find small subdiagonal element

        tst1 = max(tst1,abs(d[l]) + abs(e[l]));
        int m = l;

        // Original while-loop from Java code
        while (m < n) 
          {
            if (abs(e[m]) <= eps*tst1)
              break;

            m++;
          }


        // If m == l, d[l] is an eigenvalue,
        // otherwise, iterate.

        if (m > l)
          {
            int iter = 0;
            do
              {
                iter = iter + 1;  // (Could check iteration count here.)

                // Compute implicit shift

                Real g = d[l];
                Real p = (d[l+1] - g) / (2.0 * e[l]);
                Real r = hypot(p,1.0);
                if (p < 0)
                  r = -r;
              
                d[l] = e[l] / (p + r);
                d[l+1] = e[l] * (p + r);
                Real dl1 = d[l+1];
                Real h = g - d[l];
                for (int i = l+2; i < n; i++)
                  d[i] -= h;

                f = f + h;

                // Implicit QL transformation.

                p = d[m];
                Real c = 1.0;
                Real c2 = c;
                Real c3 = c;
                Real el1 = e[l+1];
                Real s = 0.0;
                Real s2 = 0.0;
                for (int i = m-1; i >= l; i--)
                  {
                  c3 = c2;
                  c2 = c;
                  s2 = s;
                  g = c * e[i];
                  h = c * p;
                  r = hypot(p,e[i]);
                  e[i+1] = s * r;
                  s = e[i] / r;
                  c = p / r;
                  p = c * d[i] - s * g;
                  d[i+1] = h + s * (c * g + s * d[i]);

                  // Accumulate transformation.

                  for (int k = 0; k < n; k++) 
                    {
                      h = V[k][i+1];
                      V[k][i+1] = s * V[k][i] + c * h;
                      V[k][i] = c * V[k][i] - s * h;
                    }
                  }
                p = -s * s2 * c3 * el1 * e[l] / dl1;
                e[l] = s * p;
                d[l] = c * p;

                // Check for convergence.

              } while (abs(e[l]) > eps*tst1);
          }
        d[l] = d[l] + f;
        e[l] = 0.0;
      }

    // Sort eigenvalues and corresponding vectors.

    for (int i = 0; i < n-1; i++)
      {
        int k = i;
        Real p = d[i];
        for (int j = i+1; j < n; j++) 
          {
            if (d[j] < p)
              {
                k = j;
                p = d[j];
              }
          }
          if (k != i)
            {
              d[k] = d[i];
              d[i] = p;
              for (int j = 0; j < n; j++)
                {
                  p = V[j][i];
                  V[j][i] = V[j][k];
                  V[j][k] = p;
                }
            }
      }

  } 





  template<class T, class Real>
  void EigenSystem<T, Real>::orthes(const int n, Array2D& V, Array2D& H)
  {
    //  This is derived from the Algol procedures orthes and ortran,
    //  by Martin and Wilkinson, Handbook for Auto. Comp.,
    //  Vol.ii-Linear Algebra, and the corresponding
    //  Fortran subroutines in EISPACK.
    int low = 0;
    int high = n-1;
    Real* ort = new Real[n]; //Working storage.

    for (int m = low+1; m <= high-1; m++)
      {

        // Scale column.

        Real scale = Real(0.0);
        for (int i = m; i <= high; i++)
          scale = scale + abs(H[i][m-1]);
        
        if (scale != Real(0.0))
          {

            // Compute Householder transformation.

            Real h = Real(0.0);
            for (int i = high; i >= m; i--)
              {
                ort[i] = H[i][m-1]/scale;
                h += ort[i] * ort[i];
              }
            Real g = sqrt(h);
            if (ort[m] > 0)
              g = -g;
          
            h = h - ort[m] * g;
            ort[m] = ort[m] - g;

            // Apply Householder similarity transformation
            // H = (I-u*u'/h)*H*(I-u*u')/h)

            for (int j = m; j < n; j++) 
              {
                Real f = Real(0.0);
                for (int i = high; i >= m; i--)
                  f += ort[i]*H[i][j];
              
                f = f/h;
                for (int i = m; i <= high; i++) 
                  H[i][j] -= f*ort[i];
              }

            for (int i = 0; i <= high; i++)
              {
                Real f = Real(0.0);
                for (int j = high; j >= m; j--)
                  f += ort[j]*H[i][j];
              
                f = f/h;
                for (int j = m; j <= high; j++)
                  H[i][j] -= f*ort[j];
              }
            ort[m] = scale*ort[m];
            H[m][m-1] = scale*g;
        }
    }

    // Accumulate transformations (Algol's ortran).

    for (int i = 0; i < n; i++)
      {
        for (int j = 0; j < n; j++)
          {
            V[i][j] = (i == j ? Real(1.0) : Real(0.0));
          }
      }

    for (int m = high-1; m >= low+1; m--)
      {
        if (H[m][m-1] != Real(0.0))
          {
            for (int i = m+1; i <= high; i++) 
              ort[i] = H[i][m-1];
          
            for (int j = m; j <= high; j++)
              {
                Real g = Real(0.0);
                for (int i = m; i <= high; i++)
                  g += ort[i] * V[i][j];

                // Double division avoids possible underflow
                g = (g / ort[m]) / H[m][m-1];
                for (int i = m; i <= high; i++)
                  {
                    V[i][j] += g * ort[i];
                  }
              }
          }
      }
    delete[] ort;
  }

  template<class T, class Real>
  void EigenSystem<T, Real>::hqr2(const int nn, Array1D& d, Array1D& e, Array2D& V, Array2D& H)
  {
    //  This is derived from the Algol procedure hqr2,
    //  by Martin and Wilkinson, Handbook for Auto. Comp.,
    //  Vol.ii-Linear Algebra, and the corresponding
    //  Fortran subroutine in EISPACK.
     
    // Initialize

    int n = nn-1;
    int low = 0;
    int high = nn-1;
    Real eps = pow(2.0,-52.0);
    Real exshift = Real(0.0);
    Real p=0,q=0,r=0,s=0,z=0,t,w,x,y;
    Real cdivr, cdivi;

    // Store roots isolated by balanc and compute matrix norm

    Real norm = Real(0.0);
    for (int i = 0; i < nn; i++)
      {
        if ((i < low) || (i > high))
          {
            d[i] = H[i][i];
            e[i] = Real(0.0);
          }
        for (int j = qMax(i-1, 0); j < nn; j++)
          norm = norm + abs(H[i][j]);
      }

    // Outer loop over eigenvalue index

    int iter = 0;
    while (n >= low)
      {

        // Look for single small sub-diagonal element

        int l = n;
        while (l > low)
          {
            s = abs(H[l-1][l-1]) + abs(H[l][l]);
            if (s == Real(0.0))
              s = norm;
          
            if (abs(H[l][l-1]) < eps * s)
              break;
            l--;
          }
      
        // Check for convergence
        // One root found

        if (l == n)
          {
            H[n][n] = H[n][n] + exshift;
            d[n] = H[n][n];
            e[n] = Real(0.0);
            n--;
            iter = 0;

          // Two roots found

          } 
        else if (l == n-1)
          {
            w = H[n][n-1] * H[n-1][n];
            p = (H[n-1][n-1] - H[n][n]) / 2.0;
            q = p * p + w;
            z = sqrt(abs(q));
            H[n][n] = H[n][n] + exshift;
            H[n-1][n-1] = H[n-1][n-1] + exshift;
            x = H[n][n];

          // Real pair

          if (q >= 0)
            {
              if (p >= 0)
                {
                  z = p + z;
                }
              else 
                {
                  z = p - z;
                }
              d[n-1] = x + z;
              d[n] = d[n-1];
              if (z != Real(0.0))
                {
                  d[n] = x - w / z;
                }
              e[n-1] = Real(0.0);
              e[n] = Real(0.0);
              x = H[n][n-1];
              s = abs(x) + abs(z);
              p = x / s;
              q = z / s;
              r = sqrt(p * p+q * q);
              p = p / r;
              q = q / r;

              // Row modification

              for (int j = n-1; j < nn; j++)
                {
                  z = H[n-1][j];
                  H[n-1][j] = q * z + p * H[n][j];
                  H[n][j] = q * H[n][j] - p * z;
                }

              // Column modification

              for (int i = 0; i <= n; i++)
                {
                  z = H[i][n-1];
                  H[i][n-1] = q * z + p * H[i][n];
                  H[i][n] = q * H[i][n] - p * z;
                }

              // Accumulate transformations

              for (int i = low; i <= high; i++)
                {
                  z = V[i][n-1];
                  V[i][n-1] = q * z + p * V[i][n];
                  V[i][n] = q * V[i][n] - p * z;
                }

          // Complex pair

            }
          else
            {
              d[n-1] = x + p;
              d[n] = x + p;
              e[n-1] = z;
              e[n] = -z;
            }
            n = n - 2;
            iter = 0;

        // No convergence yet

          } 
        else
          {

            // Form shift

            x = H[n][n];
            y = Real(0.0);
            w = Real(0.0);
            if (l < n)
              {
                y = H[n-1][n-1];
                w = H[n][n-1] * H[n-1][n];
              }

            // Wilkinson's original ad hoc shift

            if (iter == 10)
              {
                exshift += x;
                for (int i = low; i <= n; i++)
                  {
                    H[i][i] -= x;
                  }
                s = abs(H[n][n-1]) + abs(H[n-1][n-2]);
                x = y = Real(0.75) * s;
                w = Real(-0.4375) * s * s;
              }

            // MATLAB's new ad hoc shift

            if (iter == 30)
              {
                s = (y - x) / 2.0;
                s = s * s + w;
                if (s > 0)
                  {
                    s = sqrt(s);
                    if (y < x)
                      {
                        s = -s;
                      }
                    s = x - w / ((y - x) / 2.0 + s);
                    for (int i = low; i <= n; i++)
                      {
                        H[i][i] -= s;
                      }
                    exshift += s;
                    x = y = w = Real(0.964);
                  }
              }

            iter = iter + 1;   // (Could check iteration count here.)

            // Look for two consecutive small sub-diagonal elements

            int m = n-2;
            while (m >= l)
              {
                z = H[m][m];
                r = x - z;
                s = y - z;
                p = (r * s - w) / H[m+1][m] + H[m][m+1];
                q = H[m+1][m+1] - z - r - s;
                r = H[m+2][m+1];
                s = abs(p) + abs(q) + abs(r);
                p = p / s;
                q = q / s;
                r = r / s;
                if (m == l)
                  {
                    break;
                  }
                if (abs(H[m][m-1]) * (abs(q) + abs(r)) <
                  eps * (abs(p) * (abs(H[m-1][m-1]) + abs(z) +
                  abs(H[m+1][m+1]))))
                  {
                    break;
                  }
                m--;
              }

            for (int i = m+2; i <= n; i++)
              {
                H[i][i-2] = Real(0.0);
                if (i > m+2)
                  {
                    H[i][i-3] = Real(0.0);
                  }
              }

            // Double QR step involving rows l:n and columns m:n

            for (int k = m; k <= n-1; k++)
              {
                int notlast = (k != n-1);
                if (k != m)
                  {
                    p = H[k][k-1];
                    q = H[k+1][k-1];
                    r = (notlast ? H[k+2][k-1] : Real(0.0));
                    x = abs(p) + abs(q) + abs(r);
                    if (x != Real(0.0))
                    {
                      p = p / x;
                      q = q / x;
                      r = r / x;
                    }
                  }
                if (x == Real(0.0))
                  break;
                
                s = sqrt(p * p + q * q + r * r);
                if (p < 0)
                  s = -s;
              
                if (s != 0)
                  {
                    if (k != m)
                      H[k][k-1] = -s * x;
                    else if (l != m)
                      {
                        H[k][k-1] = -H[k][k-1];
                      }
                    p = p + s;
                    x = p / s;
                    y = q / s;
                    z = r / s;
                    q = q / p;
                    r = r / p;

                    // Row modification

                    for (int j = k; j < nn; j++)
                      {
                        p = H[k][j] + q * H[k+1][j];
                        if (notlast)
                        {
                          p = p + r * H[k+2][j];
                          H[k+2][j] = H[k+2][j] - p * z;
                        }
                        H[k][j] = H[k][j] - p * x;
                        H[k+1][j] = H[k+1][j] - p * y;
                      }

                    // Column modification

                    for (int i = 0; i <= min(n,k+3); i++)
                      {
                        p = x * H[i][k] + y * H[i][k+1];
                        if (notlast)
                          {
                            p = p + z * H[i][k+2];
                            H[i][k+2] = H[i][k+2] - p * r;
                          }
                        H[i][k] = H[i][k] - p;
                        H[i][k+1] = H[i][k+1] - p * q;
                      }

                    // Accumulate transformations

                    for (int i = low; i <= high; i++)
                      {
                        p = x * V[i][k] + y * V[i][k+1];
                        if (notlast)
                          {
                            p = p + z * V[i][k+2];
                            V[i][k+2] = V[i][k+2] - p * r;
                          }
                          V[i][k] = V[i][k] - p;
                          V[i][k+1] = V[i][k+1] - p * q;
                      }
                  }  // (s != 0)
            }  // k loop
          }  // check convergence
      }  // while (n >= low)
    
    // Backsubstitute to find vectors of upper triangular form

    if (norm == Real(0.0))
      return;

    for (n = nn-1; n >= 0; n--)
      {
        p = d[n];
        q = e[n];

        // Real vector

        if (q == 0)
          {
            int l = n;
            H[n][n] = Real(1.0);
            for (int i = n-1; i >= 0; i--)
              {
                w = H[i][i] - p;
                r = Real(0.0);
                for (int j = l; j <= n; j++)
                  {
                    r = r + H[i][j] * H[j][n];
                  }
              if (e[i] < Real(0.0))
                {
                  z = w;
                  s = r;
                }
              else
                {
                  l = i;
                  if (e[i] == Real(0.0))
                    {
                      if (w != Real(0.0))
                        {
                          H[i][n] = -r / w;
                        }
                      else
                        {
                        H[i][n] = -r / (eps * norm);
                        }
                    // Solve real equations
                    }
                  else
                    {
                      x = H[i][i+1];
                      y = H[i+1][i];
                      q = (d[i] - p) * (d[i] - p) + e[i] * e[i];
                      t = (x * s - z * r) / q;
                      H[i][n] = t;
                      if (abs(x) > abs(z))
                        H[i+1][n] = (-r - w * t) / x;
                      else
                        H[i+1][n] = (-s - y * t) / z;  
                    }

                  // Overflow control

                  t = abs(H[i][n]);
                  if ((eps * t) * t > 1)
                    {
                      for (int j = i; j <= n; j++)
                        H[j][n] = H[j][n] / t;
                      
                    }
                }
              }

        // Complex vector

          } 
        else if (q < 0)
          {
            int l = n-1;

            // Last vector component imaginary so matrix is triangular

            if (abs(H[n][n-1]) > abs(H[n-1][n]))
              {
                H[n-1][n-1] = q / H[n][n-1];
                H[n-1][n] = -(H[n][n] - p) / H[n][n-1];
              }
            else 
              {
                cdiv(Real(0.0),-H[n-1][n],H[n-1][n-1]-p,q, cdivr, cdivi);
                H[n-1][n-1] = cdivr;
                H[n-1][n] = cdivi;
              }
            H[n][n-1] = Real(0.0);
            H[n][n] = Real(1.0);
            for (int i = n-2; i >= 0; i--)
              {
                Real ra,sa,vr,vi;
                ra = Real(0.0);
                sa = Real(0.0);
                for (int j = l; j <= n; j++)
                  {
                    ra = ra + H[i][j] * H[j][n-1];
                    sa = sa + H[i][j] * H[j][n];
                  }
                w = H[i][i] - p;
                if (e[i] < Real(0.0))
                  {
                    z = w;
                    r = ra;
                    s = sa;
                  }
                else
                  {
                    l = i;
                    if (e[i] == 0)
                      {
                        cdiv(-ra,-sa,w,q, cdivr, cdivi);
                        H[i][n-1] = cdivr;
                        H[i][n] = cdivi;
                      }
                    else
                      {
                        // Solve complex equations

                        x = H[i][i+1];
                        y = H[i+1][i];
                        vr = (d[i] - p) * (d[i] - p) + e[i] * e[i] - q * q;
                        vi = (d[i] - p) * 2.0 * q;
                        if ((vr == Real(0.0)) && (vi == Real(0.0)))
                          {
                            vr = eps * norm * (abs(w) + abs(q) +
                            abs(x) + abs(y) + abs(z));
                          }
                        cdiv(x*r-z*ra+q*sa,x*s-z*sa-q*ra,vr,vi, cdivr, cdivi);
                        H[i][n-1] = cdivr;
                        H[i][n] = cdivi;
                        if (abs(x) > (abs(z) + abs(q)))
                          {
                            H[i+1][n-1] = (-ra - w * H[i][n-1] + q * H[i][n]) / x;
                            H[i+1][n] = (-sa - w * H[i][n] - q * H[i][n-1]) / x;
                          }
                        else
                          {
                            cdiv(-r-y*H[i][n-1],-s-y*H[i][n],z,q, cdivr, cdivi);
                            H[i+1][n-1] = cdivr;
                            H[i+1][n] = cdivi;
                          }
                      }

                    // Overflow control

                    t = max(abs(H[i][n-1]),abs(H[i][n]));
                    if ((eps * t) * t > 1)
                      {
                        for (int j = i; j <= n; j++)
                          {
                            H[j][n-1] = H[j][n-1] / t;
                            H[j][n] = H[j][n] / t;
                          }
                      }
                  }
              }
          }
    }

    // Vectors of isolated roots

    for (int i = 0; i < nn; i++)
      {
        if (i < low || i > high)
          {
            for (int j = i; j < nn; j++)
              {
                V[i][j] = H[i][j];
              }
          }
      }

    // Back transformation to get eigenvectors of original matrix

    for (int j = nn-1; j >= low; j--)
      {
        for (int i = low; i <= high; i++)
          {
            z = Real(0.0);
            for (int k = low; k <= min(j,high); k++)
              {
                z = z + V[i][k] * H[k][j];
              }
            V[i][j] = z;
          }
      }
  }


  
  template<class T, class Real>
  void EigenSystem<T, Real>::solve(const PiiMatrix<T>& mat)
  {
    if(!Pii::isSquare(mat) || mat.rows() == 0)
      return;
    const int n = mat.rows();
    _matRealParts.resize(1, n);
    _matImagParts.resize(1, n);
    _matEigenvectors.resize(n,n);
    solve(mat, _matRealParts.row(0),_matImagParts.row(0), &_matEigenvectors);
  }

  template<class T, class Real>
  void EigenSystem<T, Real>::solve(const PiiMatrix<T>& mat, Real* const pRealpartArray,
                                    Real* const pImagpartArray, Array2D* const pEigenvectors)
  {
    const int n = min(mat.rows(), mat.columns());
    
    Array2D* pV = (pEigenvectors != 0) ? pEigenvectors : new Array2D;
    pV->resize(n,n);

    Array1D realArray = (pRealpartArray != 0) ? pRealpartArray : new Real[n];
    Array1D imagArray = (pImagpartArray != 0) ? pImagpartArray : new Real[n];

    if(isSymmetric(mat))
    {
      for (int i = 0; i < n; i++)
        {
          for (int j = 0; j < n; j++)
            {
              (*pV)[i][j] = mat(i,j);
            }
        }
      tred2(n, realArray, imagArray, *pV);

      // Diagonalize.
      // Symmetric tridiagonal QL algorithm.
      tql2(n, realArray, imagArray, *pV);
    }
    else //Non symmetric matrix.
    {
      Array2D H = Array2D(n,n);
         
      for (int j = 0; j < n; j++)
        {
          for (int i = 0; i < n; i++)
            {
              H[i][j] = mat(i,j);
            }
        }

      // Reduce to Hessenberg form.
      orthes(n, *pV, H);

      // Reduce Hessenberg to real Schur form.
      hqr2(n, realArray, imagArray, *pV, H);
    }

    if(pRealpartArray == 0)
      delete[] realArray;
    if(pImagpartArray == 0)
      delete[] imagArray;
    if(pEigenvectors == 0)
      delete pV;
  }



template<class T, class Real>
PiiMatrix<Real> EigenSystem<T, Real>::blockDiagonalEigenvalMatrix()
{
  const int n = _matRealParts.columns();
  if(n == 0 || n != _matImagParts.columns()) 
    return PiiMatrix<Real>();

  const Real* const d = _matRealParts.row(0);
  const Real* const e = _matImagParts.row(0);
  
  PiiMatrix<Real> D(n,n);
  for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < n; j++)
        {
          D[i][j] = 0.0;
        }
      D[i][i] = d[i];
      if (e[i] > 0 && i+1 < n)
        {
          D[i][i+1] = e[i];
        }
      else if (e[i] < 0 && i-1 >= 0)
        {
          D[i][i-1] = e[i];
        }
    }
  return D;
}

template<class T, class Real>
PiiMatrix<std::complex<Real> > EigenSystem<T, Real>::eigenvalues() const
{
  const int n = _matRealParts.columns();
  if(n == _matImagParts.columns() && n >= 1)
    {
      PiiMatrix<std::complex<Real> > mat(1,n);
      for(int i = n; i--;) mat(0,i) = std::complex<Real>(_matRealParts(0,i), _matImagParts(0,i));
      return mat;
    }
  else
    return PiiMatrix<std::complex<Real> >();
}

}; //namespace Pii




#endif // _PIIMATHEIGENSYSTEM_H
