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

#ifndef _PIISERIALIZATIONGLOBAL_H
#define _PIISERIALIZATIONGLOBAL_H

#include <PiiGlobal.h>

#define PII_SERIALIZATION_EXPORT PII_CORE_EXPORT
#ifdef PII_BUILD_CORE
#  define PII_BUILD_SERIALIZATION
#  define PII_BUILDING_SERIALIZATION 1
#else
#  define PII_BUILDING_SERIALIZATION 0
#endif

#endif //_PIISERIALIZATIONGLOBAL_H
