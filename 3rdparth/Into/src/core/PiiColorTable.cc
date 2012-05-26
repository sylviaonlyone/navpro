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

#include "PiiColorTable.h"

namespace Pii
{
  static QVector<QRgb> createGrayColorTable()
  {
    QVector<QRgb> vecGray;
    vecGray.reserve(256);
    for (int i=0; i<256; ++i)
      vecGray << qRgb(i,i,i);
    return vecGray;
  }
  
  const QVector<QRgb>& grayColorTable()
  {
    static QVector<QRgb> vecGray(createGrayColorTable());
    return vecGray;
  }
}
