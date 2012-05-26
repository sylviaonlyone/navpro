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

#ifndef _PIICLASSINFOREGISTERER_H
#define _PIICLASSINFOREGISTERER_H

#include "PiiYdin.h"

/// @internal
class PII_YDIN_EXPORT PiiClassInfoRegisterer
{
public:
  PiiClassInfoRegisterer(const char* parent,
                         const char* child,
                         const char* superClass = 0,
                         unsigned long offset = 0);

  ~PiiClassInfoRegisterer();

private:
  class Data
  {
  public:
    QList<int> lstIds;
  } *d;
};

#endif //_PIICLASSINFOREGISTERER_H
