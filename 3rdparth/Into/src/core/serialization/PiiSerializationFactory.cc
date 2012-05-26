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

#include "PiiSerializationFactory.h"

PII_DEFINE_FACTORY_MAP(PiiSerialization::Void);

QList<const char*> PiiSerializationFactory::keys(MapType* map)
{
  QList<const char*> lstResult;
  for (MapType::const_iterator i = map->constBegin();
       i != map->constEnd();
       ++i)
    lstResult << i.key().ptr;
  return lstResult;
}


PiiSerializationFactory::~PiiSerializationFactory()
{
}

void PiiSerializationFactory::removeFrom(MapType* map)
{
  QMutableHashIterator<PiiConstCharWrapper, PiiSerializationFactory*> i(*map);
  while (i.hasNext())
    {
      i.next();
      if (i.value() == this)
        {
          i.remove();
          return;
        }
    }
}
