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

#ifndef _PIIMETATYPEUTIL_H
#define _PIIMETATYPEUTIL_H

#include "PiiGlobal.h"
#include <QVariantList>
#include <QByteArray>

namespace Pii
{
  /**
   * Converts comma-separated type names to a list of type IDs. If any
   * of the types is invalid, or if @a types is empty, returns an
   * empty list.
   *
   * @code
   * QList<int> lstTypes = Pii::parseTypes("QString,int");
   * // lstTypes = (QMetaType::String, QMetaType::Int)
   * @endcode
   */
  PII_CORE_EXPORT QList<int> parseTypes(const QByteArray& types);

  /**
   * Returns a numerical score for the goodness of a parameter set
   * given a list of expected types. The returned value is the number
   * of elements in @a params whose type exactly matches the
   * corresponding type in @a types. If the length of @a params is
   * different from that of @a types, or any of the parameters cannot
   * be converted to the expected type, returns -1.
   *
   * @code
   * QList<int> lstTypes;
   * lstTypes << QMetaType::Int << QMetaType::String;
   * QVariantList lstParams;
   * lstParams << QString("abc") << QString("def");
   * int iScore = Pii:scoreOverload(lstParams, lstTypes);
   * // iScore == 1 because QVariant allows conversions from QString to int
   * @endcode
   */
  PII_CORE_EXPORT int scoreOverload(const QVariantList& params, const QList<int>& types);

  /**
   * Copies the object pointed to by @a source to the memory location
   * at @a target. Both @a source and @a target must point to an
   * object whose type matches @a type.
   *
   * @param type the type ID of the objects. See QMetaType::Type.
   *
   * @param source the source object
   *
   * @param target the target object
   *
   * @code
   * int iSource = 1, iTarget = 0;
   * Pii::copyMetaType(QVariant::Int, &iSource, &iTarget);
   * // iTarget == 1
   * @endcode
   */
  PII_CORE_EXPORT void copyMetaType(QVariant::Type type, const void* source, void* target);

  /// @internal
  PII_CORE_EXPORT bool copyMetaType(const QVariant& source, int expectedType, void** args);

  /**
   * Converts @a args to to a QVariantList, assuming that each
   * argument is a valid pointer of the type specified by @a types.
   *
   * @code
   * int i;
   * double d;
   * void* args[2] = { &i, &d };
   * QList<int> lstTypes;
   * lstTypes << QMetaType::Int << QMetaType::Double;
   * QVariantList lstArgs(PiiUniversalSlot::argsToList(lstTypes, args);
   * @endcode
   */
  PII_CORE_EXPORT QVariantList argsToList(const QList<int>& types, void** args);
}

#endif //_PIIMETATYPEUTIL_H
