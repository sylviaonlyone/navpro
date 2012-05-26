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

#ifndef _PIICOLORTABLE_H
#define _PIICOLORTABLE_H

#include <QVector>
#include <QRgb>
#include "PiiGlobal.h"

/**
 * @file Functions for creating color tables.
 *
 * @ingroup Core
 */
namespace Pii
{
  /**
   * Returns a color table with shades of gray from zero to 255. 
   * PiiQImage uses this color table when initialized with a
   * PiiMatrix, because there is no color table in matrices.
   */
  PII_CORE_EXPORT const QVector<QRgb>& grayColorTable();
};

#endif //_PIICOLORTABLE_H
