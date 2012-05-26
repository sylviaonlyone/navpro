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

#include "PiiException.h"

PiiException::Data::Data()
{}

PiiException::Data::Data(const QString& message, const QString& location) :
  strMessage(message), strLocation(location)
{}

PiiException::Data::~Data()
{}

PiiException::PiiException() :
  d(new Data)
{}

PiiException::PiiException(const QString& message) :
  d(new Data(message, ""))
{}

PiiException::PiiException(const QString& message, const QString& location) :
  d(new Data(message, location))
{}


PiiException::PiiException(const PiiException& other) :
  d(new Data(other.d->strMessage, other.d->strLocation))
{}

PiiException::PiiException(Data* data) :
  d(data)
{
}

PiiException& PiiException::operator= (const PiiException& other)
{
  d->strMessage = other.d->strMessage;
  d->strLocation = other.d->strLocation;
  return *this;
}

PiiException::~PiiException()
{
  delete d;
}

QString PiiException::message() const { return d->strMessage; }
void PiiException::setMessage(const QString& message) { d->strMessage = message; }
QString PiiException::location() const { return d->strLocation; }
QString PiiException::location(const QString& prefix, const QString& suffix) const
{
  if (d->strLocation.isEmpty())
    return QString();
  return prefix + d->strLocation + suffix;
}
void PiiException::setLocation(const QString& location) { d->strLocation = location; }

PiiInvalidArgumentException::PiiInvalidArgumentException(const QString& message, const QString& location) :
  PiiException(message, location)
{}

PiiInvalidArgumentException::PiiInvalidArgumentException(const PiiInvalidArgumentException& other) :
  PiiException(other)
{}

PiiInvalidArgumentException& PiiInvalidArgumentException::operator= (const PiiInvalidArgumentException& other)
{
  PiiException::operator= (other);
  return *this;
}
