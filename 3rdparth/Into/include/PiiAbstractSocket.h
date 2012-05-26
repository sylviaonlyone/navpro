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

#ifndef _PIIABSTRACTSOCKET_H
#define _PIIABSTRACTSOCKET_H

#include "PiiYdin.h"

class PiiSocket;
class PiiInputSocket;
class PiiOutputSocket;
class PiiProxySocket;

/**
 * A superclass for socket interfaces. Due to the fact that QObject
 * doesn't support virtual inheritance, all concrete implementations
 * of this interface also derive from PiiSocket. To avoid
 * dynamic_casts at run-time, this interfaces provides a virtual
 * function, #socket(), for performing a cross-cast.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiAbstractSocket
{
public:
  virtual ~PiiAbstractSocket();

  /**
   * Returns a pointer to this socket as a PiiSocket. This is the
   * preferred way of performing a cross-cast, even if @p dynamic_cast
   * would work in most cases.
   */
  virtual PiiSocket* socket() = 0;

  /**
   * Returns a pointer to this socket as a @p const PiiSocket. This
   * function calls the non-const version.
   */
  inline const PiiSocket* socket() const
  {
    return const_cast<PiiAbstractSocket*>(this)->socket();
  }

protected:
  PiiAbstractSocket();
  
  // No d-pointer here to avoid dynamic_cast at run time.
};

Q_DECLARE_METATYPE(PiiAbstractSocket*);

#endif //_PIIABSTRACTSOCKET_H
