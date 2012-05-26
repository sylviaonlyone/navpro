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

#ifndef _PIISIZE_H
#define _PIISIZE_H

/**
 * Stores the width and height of an area. See PiiPoint for usage
 * examples.
 *
 * @ingroup Core
 */
template <class T> struct PiiSize
{
  PiiSize(T w = 0, T h = 0) :
    width(w), height(h) {}
  /**
   * The width of the area.
   */
  T width;
  /**
   * The height of the area.
   */
  T height;
};


#endif //_PIISIZE_H
