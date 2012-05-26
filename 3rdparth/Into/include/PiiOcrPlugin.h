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

#ifndef _PIIOCRPLUGIN_H
#define _PIIOCRPLUGIN_H

#include <PiiGlobal.h>

#ifdef PII_BUILD_OCR
#  define PII_OCR_EXPORT PII_DECL_EXPORT
#  define PII_BUILDING_OCR 1
#else
#  define PII_OCR_EXPORT PII_DECL_IMPORT
#  define PII_BUILDING_OCR 0
#endif/**
 * @defgroup PiiOcrPlugin
 *
 * Description here.
 *
 * @dependencies PiiOcrPlugin
 *
 * @runtime PiiOtherPlugin Optional reason.
 * @compile PiiYetAnotherPlugin Optional reason
 */


#endif //_PIIOCRPLUGIN_H
