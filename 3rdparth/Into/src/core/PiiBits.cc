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

#include "PiiBits.h"

using namespace std;

namespace Pii
{
  int countOnes(unsigned int c, const unsigned char bits)
  {
    int count = 0;
    for (int i=0;i<bits;i++)
      {
        count += c & 1; //add the lsb
        c >>= 1; //take next bit to lsb
      }
    return count;
  }

  int countTransitions(unsigned int c, const unsigned char bits)
  {
    return countOnes(c ^ ror(c, 1, bits));
  }

  unsigned int rotateToMinimum(unsigned int n, const unsigned char bits)
  {
    unsigned int tmp = n << sizeof(int)*4;
    unsigned int lowmask = INT_MAX >> (sizeof(int)*4-1);
    unsigned int min = tmp;
    int minIndex=0;
    for (int i=1;i<bits;i++)
      {
        tmp >>= 1;
        tmp |= (tmp & lowmask) << bits;
        tmp &= ~lowmask;
        if (tmp<min)
          {
            min = tmp;
            minIndex = i;
          }
      }
    tmp = n << (sizeof(int)*4 - minIndex);
    tmp |= (tmp & lowmask) << bits;
    tmp &= ~lowmask;
    return tmp >> sizeof(int)*4;
  }

  int hammingDistance(unsigned int a, unsigned int b, const unsigned char bits)
  {
    return countOnes(a^b, bits); //find differing bits
  }
}

