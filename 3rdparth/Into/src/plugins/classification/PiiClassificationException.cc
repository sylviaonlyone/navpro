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

#include "PiiClassificationException.h"


const char* PiiClassificationException::messageForCode(Code code)
{
  static const char* messages[] =
    {
      QT_TRANSLATE_NOOP("PiiClassificationException", "Unknown error"),
      QT_TRANSLATE_NOOP("PiiClassificationException", "Learning was interrupted."),
    };
  return messages[code];
}

PiiClassificationException::Data::Data(Code c, const QString& location) :
  PiiException::Data(messageForCode(c), location),
  code(c)
{}

PiiClassificationException::Data::Data(const QString& message, const QString& location) :
  PiiException::Data(message, location),
  code(Unknown)
{}

PiiClassificationException::PiiClassificationException(Code code, const QString& location) :
  PiiException(new Data(code, location))
{}

PiiClassificationException::PiiClassificationException(const QString& message, const QString& location) :
  PiiException(new Data(message, location))
{}
  
PiiClassificationException::Code PiiClassificationException::code() const { return _d()->code; }
