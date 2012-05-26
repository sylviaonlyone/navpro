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

#ifndef _PIIPOINT_H
#define _PIIPOINT_H

/**
 * A two-dimensional point. In many occasions, matrices are used in
 * storing geometric objects like points, circles (PiiCircle), sizes
 * (PiiSize), rectangles (PiiRectangle) etc. The parameters of these
 * objects are represented as row vectors. The geometric object
 * structures are provided to conveniently handle such matrices.
 *
 * @code
 * // A matrix that stores two points
 * PiiMatrix<int> mat(2,  2,
 *                    1,  1,
 *                   -1, -1);
 * PiiPoint<int> *pt = mat.rowAs<PiiPoint<int> >(0);
 * pt->x = 2; // modifies mat(0,0)
 * pt->y = 0; // modifies mat(0,1)
 * pt = (PiiPoint<int>*)mat.row(1);
 * pt->x = 0; // modifies mat(1,0)
 *
 * // A circle
 * PiiMatrix<float> mat2(1, 3,
 *                       0.0, 0.0, 5.0);
 * PiiCircle<float> *circle = mat2.rowAs<PiiCircle<float> >(0);
 * circle->radius += 5; // radius is now 10
 * PiiPoint<float> *pt2 = mat2.rowAs<PiiPoint<float> >(0);
 * pt2->y = 2; // modifies mat2(0,1)
 *
 * // Store the matrix row into a circle structure
 * PiiCircle<float> circle2 = *circle;
 * @endcode
 *
 * @ingroup Core
 */
template <class T> struct PiiPoint
{
  PiiPoint(T xVal = 0, T yVal = 0) :
    x(xVal), y(yVal)
  {}
               
  /**
   * The x coordinate.
   */
  T x;
  /**
   * The y coordinate.
   */
  T y;

  bool operator== (const PiiPoint& other) const { return x == other.x && y == other.y; }
  bool operator!= (const PiiPoint& other) const { return x != other.x || y != other.y; }
};


#endif //_PIIPOINT_H
