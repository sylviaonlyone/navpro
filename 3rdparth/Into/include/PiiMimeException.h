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

#ifndef _PIIMIMEEXCEPTION_H
#define _PIIMIMEEXCEPTION_H

#include <PiiException.h>
#include "PiiNetworkGlobal.h"

#define PII_THROW_MIME(CODE) PII_THROW(PiiMimeException, PiiMimeException::CODE)

/**
 * Used to indicate error conditions in MIME headers.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiMimeException : public PiiException
{
public:
  /**
   * Codes for different exception types.
   *
   * @lip HeaderTooLarge - header size exceeds limit
   *
   * @lip InvalidFormat - the MIME message has invalid format.
   */
  enum Code { HeaderTooLarge, InvalidFormat };
  
  /**
   * Constructs a new PiiMimeException with the given code, error
   * message and location.
   */
  PiiMimeException(Code code, const QString& location = "");
  
  /**
   * Get the cause of the exception.
   */
  Code code() const;

  /**
   * Returns a textual representation of the error associated with @a
   * code.
   */
  static const char* messageForCode(Code code);
  
private:
  class Data : public PiiException::Data
  {
  public:
    Data(Code code, const QString& location);
    Data(const QString& message, const QString& location);
    Code code;
  };
  PII_D_FUNC;
};

#endif //_PIIMIMEEXCEPTION_H
