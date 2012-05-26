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

#ifndef _PIIYDINUTIL_H
#define _PIIYDINUTIL_H

#include "PiiYdin.h"
#include "PiiOperationCompound.h"
#include <QString>

/**
 * @file
 *
 * Utility functions for mainly debugging purposes.
 */

namespace PiiYdin
{
  enum IllustrationFlag
    {
      NoFlags = 0x0,
      ShowInputQueues = 0x1,
      ShowOutputStates = 0x2,
      ShowState = 0x3
    };

  Q_DECLARE_FLAGS(IllustrationFlags, IllustrationFlag);
  Q_DECLARE_OPERATORS_FOR_FLAGS(IllustrationFlags);
  
  /**
   * Create an ascii-graphics illustration of an operation. The result
   * can be printed on a console. This function is mainly useful for
   * debugging purposes.
   *
   * @param op the operation to illustrate
   *
   * @param showQueues enable/disable displaying of input queues.
   *
   * @return an ascii-graphics illustration.
   */
  PII_YDIN_EXPORT QString illustrateOperation(PiiOperation* op, IllustrationFlags flags = NoFlags);

  /**
   * Print the operation and child operations if necessary depends on
   * given level and the illustration flags.
   *
   * @param op the operation to illustrate
   *
   * @param level the level we start the dumping
   * 
   * @param flags illustration flags
   */
  PII_YDIN_EXPORT void dumpOperation(PiiOperation *op, int level = 0, IllustrationFlags flags = NoFlags);

  /**
   * Dump the state of an operation and all of its child operations. 
   * This function is mainly useful for debugging purposes.
   *
   * @param stream write state information to this stream
   *
   * @param op the operation whose state is to be dumped
   *
   * @param indent indentation depth
   */
  template <class Stream> void dumpState(Stream stream, PiiOperation* op, int indent = 0)
  {
    for (int i=indent; i--; )
      stream << ' ';
    
    stream << op->metaObject()->className() << "(" << op->objectName() << "): " << PiiOperation::stateName(op->state()) << "\n";
    PiiOperationCompound* compound = qobject_cast<PiiOperationCompound*>(op);
    if (compound != 0)
      {
        QList<PiiOperation*> children = compound->childOperations();
        ++indent;
        for (int i=0; i<children.size(); ++i)
          dumpState(stream, children[i], indent);
      }
  }

}

#endif //_PIIYDINUTIL_H
