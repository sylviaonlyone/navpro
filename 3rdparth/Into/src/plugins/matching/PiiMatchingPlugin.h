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

#ifndef _PIIMATCHINGPLUGIN_H
#define _PIIMATCHINGPLUGIN_H

#include <PiiGlobal.h>

#ifdef PII_BUILD_MATCHING
#  define PII_MATCHING_EXPORT PII_DECL_EXPORT
#else
#  define PII_MATCHING_EXPORT PII_DECL_IMPORT
#endif

/**
 * @defgroup PiiMatchingPlugin
 *
 * This plug-in contains functions and operations for matching images. 
 * It provides point and boundary based algorithms to match templates
 * against images, and algorithms for matching point correspondences
 * in stereo images.
 */

#endif //_PIIMATCHINGPLUGIN_H
