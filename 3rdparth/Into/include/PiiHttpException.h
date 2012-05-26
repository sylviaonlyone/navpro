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

#ifndef _PIIHTTPEXCEPTION_H
#define _PIIHTTPEXCEPTION_H

#include "PiiNetworkException.h"

/**
 * Thrown when a HTTP request handler cannot complete a request.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiHttpException : public PiiNetworkException
{
public:
  /**
   * Constructs a new %PiiHttpException with the given @a statusCode
   * and @a location.
   */
  PiiHttpException(int statusCode, const QString& location = "");

  /**
   * Constructs a new %PiiHttpException with the given @a statusCode
   * and @a location.
   */
  PiiHttpException(int statusCode, const QString& message, const QString& location);

  /**
   * Creates a copy of @a other.
   */
  PiiHttpException(const PiiHttpException& other);

  /**
   * Returns the status code.
   */
  int statusCode() const;
  
private:
  /// @internal
  class Data : public PiiException::Data
  {
  public:
    Data(int statusCode, const QString& location);
    Data(int statusCode, const QString& message, const QString& location);
    int iStatusCode;
  };
  PII_D_FUNC;
};

#endif //_PIIHTTPEXCEPTION_H
