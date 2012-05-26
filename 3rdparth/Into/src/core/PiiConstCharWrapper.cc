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

#include "PiiConstCharWrapper.h"

uint qHash(const PiiConstCharWrapper& key)
{
  uint h = 0;
  uint g;
  
  const char* p = key.ptr;
  int n = strlen(p);
  while (n--)
    {
      h = (h << 4) + (*p++);
      if ((g = (h & 0xf0000000)) != 0)
        h ^= g >> 23;
      h &= ~g;
    }
  return h;
}
