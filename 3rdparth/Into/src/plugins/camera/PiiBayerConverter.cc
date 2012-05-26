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

#include "PiiBayerConverter.h"

namespace PiiCamera
{
  PiiMatrix<PiiColor4<> > rggbToRgb(const PiiMatrix<unsigned char>& encoded)
  {
    return bayerToRgb(encoded, RggbDecoder<unsigned char>(), Rgb4Pixel<>());
  }

  PiiMatrix<PiiColor4<> > grbgToRgb(const PiiMatrix<unsigned char>& encoded)
  {
    return bayerToRgb(encoded, GrbgDecoder<unsigned char>(), Rgb4Pixel<>());
  }
}
