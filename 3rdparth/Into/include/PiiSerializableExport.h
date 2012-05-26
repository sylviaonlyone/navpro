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

#ifndef _PIISERIALIZABLEEXPORT_H
#define _PIISERIALIZABLEEXPORT_H

#include "PiiSerializationFactory.h"
#include "PiiSerializer.h"
#include "PiiGenericInputArchive.h"
#include "PiiGenericOutputArchive.h"

/**
 * @file
 *
 * Macros for exporting serializable classes from dlls.
 *
 * @ingroup Serialization
 */

/**
 * Make a class known to the serialization system. This macro must be
 * included in library class definitions (in a .cc file) that may be
 * serialized via a base class pointer. The macro instantiates a
 * serializer for type @a CLASS_NAME to PiiGenericInputArchive and
 * PiiGenericOutputArchive. It also instantiates a factory object for
 * the class.
 */
#define PII_SERIALIZABLE_EXPORT(CLASS_NAME) \
  PII_INSTANTIATE_SERIALIZER(PiiGenericInputArchive, CLASS_NAME);	\
  PII_INSTANTIATE_SERIALIZER(PiiGenericOutputArchive, CLASS_NAME); \
  PII_INSTANTIATE_FACTORY(CLASS_NAME)


#endif //_PIISERIALIZABLEEXPORT_H
