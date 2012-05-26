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

#include "PiiSerializationUtil.h"
#include "PiiSerializableExport.h"
#include <PiiConstCharWrapper.h>

PII_SERIALIZABLE_EXPORT(QObject);

namespace PiiSerialization
{
  const PiiMetaObject* metaObjectPointer(const QObject* obj)
  {
    // Static map of already-created PiiMetaObjects.
    static QHash<PiiConstCharWrapper, PiiMetaObject*> map;

    // First see if we already have fetched a meta-object for this
    // type.
    const QMetaObject* pQMetaObj = obj->metaObject();
    const char* pClassName = pQMetaObj->className();
    const PiiMetaObject* pMetaObj = map.value(pClassName, 0);
    if (pMetaObj != 0)
      return pMetaObj;

    unsigned uiVersion = 0;
    // Find the version class info.
    for (int i=0; i<pQMetaObj->classInfoCount(); ++i)
      {
        if (!strcmp(pQMetaObj->classInfo(i).name(), "version"))
          {
            uiVersion = QString(pQMetaObj->classInfo(i).value()).toUInt();
            break;
          }
      }
    // Now we have all the information. Create a new meta-object and
    // store it to the map.
    PiiMetaObject* pNewObj = new PiiMetaObject(pClassName, uiVersion, true);
    map.insert(pClassName, pNewObj);
    return pNewObj;
  }
}
