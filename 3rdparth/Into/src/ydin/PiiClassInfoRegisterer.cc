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

#include "PiiClassInfoRegisterer.h"
#include <PiiResourceStatement.h>

PiiClassInfoRegisterer::PiiClassInfoRegisterer(const char* parent,
                                               const char* child,
                                               const char* superClass,
                                               unsigned long offset) :
  d(new Data)
{
  QList<PiiResourceStatement> lstStatements;
  if (parent != 0)
    lstStatements << PiiResourceStatement(child,
                                          PiiYdin::parentPredicate,
                                          parent,
                                          PiiResourceStatement::ResourceType);
  if (superClass != 0)
    {
      lstStatements << PiiResourceStatement(child,
                                            PiiYdin::classPredicate,
                                            superClass,
                                            PiiResourceStatement::ResourceType);
      if (offset != 0)
        lstStatements << PiiResourceStatement("#",
                                              PiiYdin::offsetPredicate,
                                              QString::number(offset),
                                              PiiResourceStatement::LiteralType);
    }

  d->lstIds = PiiYdin::resourceDatabase()->addStatements(lstStatements);
}

PiiClassInfoRegisterer::~PiiClassInfoRegisterer()
{
  PiiYdin::resourceDatabase()->removeStatements(d->lstIds);
  delete d;
}


