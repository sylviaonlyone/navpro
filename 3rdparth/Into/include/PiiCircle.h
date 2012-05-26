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

#ifndef _PIICIRCLE_H
#define _PIICIRCLE_H

/**
 * Stores the center and radius of a circle. See PiiPoint for usage
 * examples.
 *
 * @ingroup Core
 */
template <class T> struct PiiCircle
{
  PiiCircle(T xVal = 0, T yVal = 0, T r = 0) :
    x(xVal), y(yVal), radius(r)
  {}

  /**
   * The x coordinate of the center.
   */
  T x;
  /**
   * The y coordinate of the center.
   */
  T y;
  /**
   * The radius of the circle.
   */
  T radius;

  bool operator== (const PiiCircle& other) const { return x == other.x && y == other.y && radius == other.radius; }
  bool operator!= (const PiiCircle& other) const { return x != other.x || y != other.y || radius != other.radius; }
};


#endif //_PIICIRCLE_H
