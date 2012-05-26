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

#include "PiiMorphology.h"
#include <cmath>

namespace PiiImage
{
  PiiMatrix<int> borderMasks[8][2] =
    {
      {
        // Border detector no 0
        PiiMatrix<int>(3,3,
                       0,0,0,
                       0,1,0,
                       1,1,1),
        // The corresponding significance mask
        PiiMatrix<int>(3,3,
                       1,1,1,
                       0,1,0,
                       1,1,1)
        
      },
      {
        // Corner detector no 0
        PiiMatrix<int>(3,3,
                       0,0,0,
                       1,1,0,
                       1,1,0),
        // The corresponding significance mask
        PiiMatrix<int>(3,3,
                       0,1,1,
                       1,1,1,
                       0,1,0)
      },
      {
        // Border detector no 1
        PiiMatrix<int>(3,3,
                       1,0,0,
                       1,1,0,
                       1,0,0),
        // The corresponding significance mask
        PiiMatrix<int>(3,3,
                       1,0,1,
                       1,1,1,
                       1,0,1)
        
      },
      {
        // Corner detector no 1
        PiiMatrix<int>(3,3,
                       1,1,0,
                       1,1,0,
                       0,0,0),
        // The corresponding significance mask
        PiiMatrix<int>(3,3,
                       0,1,0,
                       1,1,1,
                       0,1,1)
      },
      {
        // Border detector 2
        PiiMatrix<int>(3,3,
                       1,1,1,
                       0,1,0,
                       0,0,0),
        PiiMatrix<int>(3,3,
                       1,1,1,
                       0,1,0,
                       1,1,1)
        
      },
      {
        // Corner detector 2
        PiiMatrix<int>(3,3,
                       0,1,1,
                       0,1,1,
                       0,0,0),
        PiiMatrix<int>(3,3,
                       0,1,0,
                       1,1,1,
                       1,1,0)
      },
      {
        // Border detector 3
        PiiMatrix<int>(3,3,
                       0,0,1,
                       0,1,1,
                       0,0,1),
        PiiMatrix<int>(3,3,
                       1,0,1,
                       1,1,1,
                       1,0,1)
        
      },
      {
        // Corner detector 2
        PiiMatrix<int>(3,3,
                       0,0,0,
                       0,1,1,
                       0,1,1),
        PiiMatrix<int>(3,3,
                       1,1,0,
                       1,1,1,
                       0,1,0)
      }          
    };

  PiiMatrix<int> createMask(MaskType type, int rows, int columns)
  {
    return createMask<int>(type,rows,columns);
  }

  // Export an explicit instantiation.
  PII_DEFINE_EXPORTED_FUNCTION_TEMPLATE(PiiMatrix<unsigned char>, createMask<unsigned char>, (MaskType type, int rows, int columns));
}
