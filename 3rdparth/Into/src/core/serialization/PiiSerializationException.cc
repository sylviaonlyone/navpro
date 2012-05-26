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

#include "PiiSerializationException.h"

const char* PiiSerializationException::messageForCode(Code code)
{
  static const char* messages[] =
    {
      QT_TR_NOOP("Unknown error"),
      QT_TR_NOOP("Invalid data format"),
      QT_TR_NOOP("Unregistered class"),
      QT_TR_NOOP("Could not find a serializer"),
      QT_TR_NOOP("Object to be read is newer than our implementation"),
      QT_TR_NOOP("Stream input/output error"),
      QT_TR_NOOP("Stream must be open before constructing an archive"),
      QT_TR_NOOP("Unrecognized archive format"),
      QT_TR_NOOP("Input archive is newer than our implementation")
    };
  return messages[(int)code];
}


PiiSerializationException::Data::Data(Code c, const QString& location) :
  PiiException::Data(messageForCode(c), location),
  code(c)
{}

PiiSerializationException::Data::Data(const QString& message, const QString& location) :
  PiiException::Data(message, location),
  code(Unknown)
{}

PiiSerializationException::Data::Data(Code c, const QString& info, const QString& location) :
  PiiException::Data(messageForCode(c), location),
  code(c),
  strInfo(info)
{}

PiiSerializationException::PiiSerializationException(Code code, const QString& location) :
  PiiException(new Data(code, location))
{}

PiiSerializationException::PiiSerializationException(const QString& message, const QString& location) :
  PiiException(new Data(message, location))
{}

PiiSerializationException::PiiSerializationException(Code code, const QString& info, const QString& location) :
  PiiException(new Data(code, info, location))
{}

PiiSerializationException::PiiSerializationException(const PiiSerializationException& other) :
  PiiException(new Data(other.code(), other.info(), other.location()))
{
}

PiiSerializationException::Code PiiSerializationException::code() const { return _d()->code; }
QString PiiSerializationException::info() const { return _d()->strInfo; }
