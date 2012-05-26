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

#ifndef _PIIINPUTCONTROLLER_H
#define _PIIINPUTCONTROLLER_H

#include "PiiVariant.h"
#include "PiiExecutionException.h"

class PiiAbstractInputSocket;

/**
 * An interface for classes that receive objects from input sockets. 
 * Input controllers can implement any imaginable object passing
 * mechanisms. One must make sure that operations connected to each
 * other are driven by compatible input controllers. For example, any
 * operation connected to an operation derived from
 * PiiDefaultOperation must be able to handle synchronization tags and
 * stop tags.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiInputController
{
public:
  virtual ~PiiInputController();

  /**
   * Activate any functionality that should be performed when a new
   * object arrives into an input. This method is called by an input
   * socket whenever a new object is received on it.
   *
   * @param sender a pointer to the socket that received the object
   *
   * @param object the object that was received
   *
   * @return @p true if the object was accepted, @p false otherwise.
   * Typically, an object is accepted if there is no blocking object
   * in @p sender by calling @ref PiiInputSocket::receive(). Note that
   * @p true should be returned whenever the object is accepted for
   * processing, even if the processing itself will fail.
   *
   * Note that the function may not throw exceptions. If your
   * implementation passes the incoming objects right away, you must
   * catch the exceptions thrown by @ref
   * PiiOutputSocket::emitObject(), for example.
   */
  virtual bool tryToReceive(PiiAbstractInputSocket* sender, const PiiVariant& object) throw () = 0;
};

#endif //_PIIINPUTCONTROLLER_H
