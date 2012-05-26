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

#ifndef _PIISOCKETSTATE_H
#define _PIISOCKETSTATE_H

#include "PiiYdin.h"
#include "PiiVariant.h"

/**
 * A structure that stores information about a socket's state at run
 * time.
 *
 * @ingroup Ydin
 */
struct PII_YDIN_EXPORT PiiSocketState
{
  /**
   * Initialize a state with flow level @p l and delay @p d.
   */
  PiiSocketState(int l = 0, int d = 0) :
    flowLevel(l), delay(d)
  {}

  /// The current flow level.
  int flowLevel;
  /// The number of delayed objects.
  int delay;

private:
  friend struct PiiSerialization::Accessor;
  template <class Archive> void serialize(Archive& archive, const unsigned int)
  {
    PII_SERIALIZE(archive, flowLevel);
    PII_SERIALIZE(archive, delay);
  }
};

#endif //_PIISOCKETSTATE_H
