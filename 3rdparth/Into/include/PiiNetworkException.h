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

#ifndef _PIINETWORKEXCEPTION_H
#define _PIINETWORKEXCEPTION_H

#include <PiiException.h>
#include "PiiNetworkGlobal.h"

/**
 * Used to indicate errors in network connections.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiNetworkException : public PiiException
{
public:
  /**
   * Constructs a new PiiNetworkException with the given error
   * @a message and @a location.
   */
  PiiNetworkException(const QString& message = "", const QString& location = "");

  /**
   * Creates a copy of @a other.
   */
  PiiNetworkException(const PiiNetworkException& other);

protected:
  /// @internal
  PiiNetworkException(Data* d) : PiiException(d) {}
};

#endif //_PIINETWORKEXCEPTION_H
