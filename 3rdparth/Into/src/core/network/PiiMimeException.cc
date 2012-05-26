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

#include "PiiMimeException.h"

const char* PiiMimeException::messageForCode(Code code)
{
  static const char* messages[] =
    {
      QT_TR_NOOP("Header too large"),
      QT_TR_NOOP("Invalid message format")
    };
  return messages[code];
}

PiiMimeException::Data::Data(Code c, const QString& location) :
  PiiException::Data(messageForCode(c), location),
  code(c)
{}

PiiMimeException::PiiMimeException(Code code, const QString& location) :
  PiiException(new Data(code, location))
{}

PiiMimeException::Code PiiMimeException::code() const { return _d()->code; }
