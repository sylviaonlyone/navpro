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

#include "PiiMath.h"
#include "PiiPseudoInverse.h"

namespace Pii
{
  double calculateThirdSide(double a, double b, double angle)
  {
    return a*a + b*b - 2*a*b*cos(angle);
  }

  double calculateOppositeAngle(double a, double b, double c)
  {
    return acos((b*b + c*c - a*a) / 2*b*c);
  }

  double calculateArea(double a, double b, double c)
  {
    double s = (a + b + c) * 0.5; // semiperimeter
    return sqrt(s * (s-a) * (s-b) * (s-c));
  }

  double calculateInterveningAngle(double a, double b, double angle)
  {
    // Law of sines: a/sin(alpha) = b/sin(beta)
    // alpha opposes a and beta opposes b
    double dBeta = acos(b*sin(angle)/a);
    return M_PI - angle - dBeta;
  }

  PiiMatrix<double> fitPolynomial(unsigned int order, const PiiMatrix<double>& x, const PiiMatrix<double>& y)
  {
    int iMeasurements = y.isEmpty() ? x.rows() : x.columns();
    
    // Sanity check
    if (iMeasurements <= int(order) ||
        (y.isEmpty() &&  x.columns() != 2) ||
        (!y.isEmpty() && x.columns() != y.columns()))
      PII_THROW(PiiInvalidArgumentException, QCoreApplication::translate("PiiMath", "Input arguments to fitPolynomial() are malformed."));
    
    // Initialize estimation matrices
    PiiMatrix<double> matH(iMeasurements, order+1);
    PiiMatrix<double> matZ(1, iMeasurements);

    // Both x and y are provided in one matrix
    if (y.isEmpty())
      {
        // Fill in estimation matrices
        for (int r=0; r<x.rows(); ++r)
          {
            // The x coordinate of the measurement
            double dX = x(r,0);
            // The y coordinate of the measurement
            matZ(0,r) = x(r,1);
            // Powers of the x coordinate
            matH(r,0) = 1;
            for (unsigned int o=1; o<=order; ++o)
              matH(r,o) = matH(r,o-1) * dX;
          }
        return matZ * Pii::transpose(pseudoInverse(matH));
      }
    else
      {
        PiiMatrix<double> result(0, order+1);
        result.reserve(x.rows());
        // Initialize to all ones
        matZ = 1;
        for (int r=0; r<x.rows(); ++r)
          {
            for (int c=0; c<x.columns(); ++c)
              {
                // The x coordinate of the measurement
                double dX = x(r,c);
                // The y coordinate of the measurement
                matZ(0,c) = y(r,c);
                // Powers of the x coordinate
                matH(c,0) = 1;
                for (unsigned int o=1; o<=order; ++o)
                  matH(c,o) = matH(c,o-1) * dX;
              }
            // Fill in the result matrix with the polynomial coeffs
            // calculated from one row of measurements.
            result.appendRow(matZ * Pii::transpose(pseudoInverse(matH)));
          }
        return result;
      }
  }

  // Algorithm courtesy of Peter John Acklam, pjacklam@online.no
  double erf(double p)
  {
    static const double dLowLimit = 0.02425, dHighLimit = 1-dLowLimit;

    static const double a[] =
      {
        -3.969683028665376e+01,
        2.209460984245205e+02,
        -2.759285104469687e+02,
        1.383577518672690e+02,
        -3.066479806614716e+01,
        2.506628277459239e+00
      };

    static const double b[] =
      {
        -5.447609879822406e+01,
        1.615858368580409e+02,
        -1.556989798598866e+02,
        6.680131188771972e+01,
        -1.328068155288572e+01
      };

    static const double c[] =
      {
        -7.784894002430293e-03,
        -3.223964580411365e-01,
        -2.400758277161838e+00,
        -2.549732539343734e+00,
        4.374664141464968e+00,
        2.938163982698783e+00
      };

    static const double d[] =
      {
        7.784695709041462e-03,
        3.224671290700398e-01,
        2.445134137142996e+00,
        3.754408661907416e+00
      };
    
    if (p < 0 || p > 1)
      return NAN;
    else if (p == 0)
      return -INFINITY;
    else if (p == 1)
      return INFINITY;
    else if (p < dLowLimit)
      {
        // Rational approximation for lower region
        double q = sqrt(-2*log(p));
        return (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
          ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
      }
    else if (p > dHighLimit)
      {
        // Rational approximation for upper region
        double q  = sqrt(-2*log(1-p));
        return -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
          ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
      }
    else
      {
        // Rational approximation for central region
    		double q = p - 0.5, r = q*q;
        return (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q /
          (((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1);
      }
	}
}


