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

#ifndef _PIIRECTANGLE_H
#define _PIIRECTANGLE_H

/**
 * Stores the upper left corner and the size of a rectangle. See
 * PiiPoint for usage examples.
 *
 * @ingroup Core
 */
template <class T> struct PiiRectangle
{
  PiiRectangle(T xVal = 0, T yVal = 0, T w = 0, T h = 0) :
    x(xVal), y(yVal), width(w), height(h)
  {}

  /**
   * The x coordinate of the upper left corner.
   */
  T x;
  /**
   * The y coordinate of the upper left corner.
   */
  T y;
  /**
   * The width of the rectangle.
   */
  T width;
  /**
   * The height of the rectangle.
   */
  T height;

  /**
   * Return @p true if either @p width or @p height is non-positive,
   * @p false otherwise.
   */
  bool isEmpty() const { return width <= 0 || height <= 0; }
  
  /**
   * Returns true if the point (x,y) is within this rectangle.
   */
  bool contains(T x, T y) const
  {
    return
      this->x <= x &&
      this->y <= y &&
      this->x + width >= x &&
      this->y + height >= y;
  }

  /**
   * Returns true if @p other is fully contained by @p this.
   */
  bool contains(const PiiRectangle& other) const
  {
    T tx2 = x + width;
    T ty2 = y + height;
    T ox2 = other.x + other.width;
    T oy2 = other.y + other.height;

#define PII_CONTAINS(xc,yc) \
      this->x <= xc && \
      this->y <= yc && \
      tx2 >= xc && \
      ty2 >= yc

    return
      PII_CONTAINS(other.x, other.y) &&
      PII_CONTAINS(ox2, other.y) &&
      PII_CONTAINS(other.x, oy2) &&
      PII_CONTAINS(ox2, oy2);

#undef PII_CONTAINS
  }

  /**
   * Returns true if @p other intersects @p this.
   */
  bool intersects(const PiiRectangle& other) const
  {
    T tx2 = x + width;
    T ty2 = y + height;
    T ox2 = other.x + other.width;
    T oy2 = other.y + other.height;

    return !(other.x > tx2 || other.y > ty2 || ox2 < x || oy2 < y);
  }

  /**
   * Enlarges this rectangle to include both @p this and @p other.
   */
  PiiRectangle& operator|= (const PiiRectangle& other)
  {
    T tx2 = x + width;
    T ty2 = y + height;
    T ox2 = other.x + other.width;
    T oy2 = other.y + other.height;

    x = x < other.x ? x : other.x;
    y = y < other.y ? y : other.y;
    width = tx2 > ox2 ? tx2 - x : ox2 - x;
    height = ty2 > oy2 ? ty2 - y : oy2 - y;

    return *this;
  }

  bool operator== (const PiiRectangle& other) const
  {
    return x == other.x && y == other.y && width == other.width && height == other.height;
  }
  bool operator!= (const PiiRectangle& other) const
  {
    return x != other.x || y != other.y || width != other.width || height != other.height;
  }

  template <class Archive> void serialize(Archive& archive, const unsigned int)
  {
    archive & x & y & width & height;
  }
};


#endif //_PIIRECTANGLE_H
