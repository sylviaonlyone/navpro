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

#include "PiiYdinResources.h"
#include <cstring>

namespace PiiYdin
{
  QString resourceConnectorName(const QString& resource1Name,
                                const QString& resource2Name,
                                const QString& role)
  {
    using namespace Pii;
    int id = resourceDatabase()->findFirst(subject == resource1Name &&
                                           predicate == role &&
                                           object == resource2Name);
    if (id == -1)
      return 0;

    QList<QString> lstConnectors = resourceDatabase()->select(object,
                                                              resourceIdToInt(subject) == id &&
                                                              predicate == connectorPredicate);
    if (lstConnectors.size() == 0)
      return QString();

    return lstConnectors[0];
  }

  static int pointerOffset(int id)
  {
    using namespace Pii;
    QList<int> lstOffsets = resourceDatabase()->select(resourceStringTo<int>(object),
                                                       resourceIdToInt(subject) == id &&
                                                       predicate == offsetPredicate);
    return lstOffsets.size() == 0 ? 0 : lstOffsets[0];
  }

  int pointerOffset(const char* superClass, const char* subClass)
  {
    // Superclass and subclass are the same.
    if (!strcmp(superClass, subClass))
      return 0;
    
    using namespace Pii;
    // First, we search all direct superclasses of the resource.
    QList<PiiResourceStatement> lstSuperClasses = resourceDatabase()->select(subject == subClass &&
                                                                             predicate == classPredicate);
    for (int i=0; i<lstSuperClasses.size(); ++i)
      if (lstSuperClasses[i].object() == superClass)
        return pointerOffset(lstSuperClasses[i].id());
    
    // Not a direct superclass. Recurse.
    for (int i=0; i<lstSuperClasses.size(); ++i)
      {
        int iOffset = pointerOffset(superClass, qPrintable(lstSuperClasses[i].object()));
        if (iOffset >= 0)
          return iOffset + pointerOffset(lstSuperClasses[i].id());
      }
    return -1;
  }
}

