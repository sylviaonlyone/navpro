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

#ifndef _PIIMATRIXVALUE_H
#define _PIIMATRIXVALUE_H

/**
 * A structure that stores a value and its location in a matrix.
 *
 * @ingroup Matrix
 */
template <class T> struct PiiMatrixValue
{
  PiiMatrixValue(T val=0, int r=0, int c=0) :
    value(val), row(r), column(c)
  {}

  bool operator<(const PiiMatrixValue& other) const { return value < other.value; }
  bool operator>(const PiiMatrixValue& other) const { return value > other.value; }
  bool operator<=(const PiiMatrixValue& other) const { return value <= other.value; }
  bool operator>=(const PiiMatrixValue& other) const { return value >= other.value; }
  bool operator==(const PiiMatrixValue& other) const { return value == other.value; }
  bool operator!=(const PiiMatrixValue& other) const { return value != other.value; }
  
  T value;
  int row;
  int column;
};

#endif //_PIIMATRIXVALUE_H
