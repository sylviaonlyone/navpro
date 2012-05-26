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

#include "PiiHttpException.h"
#include "PiiHttpProtocol.h"

PiiHttpException::Data::Data(int statusCode, const QString& location) :
  PiiException::Data(PiiHttpProtocol::statusMessage(statusCode), location),
  iStatusCode(statusCode)
{
}

PiiHttpException::Data::Data(int statusCode, const QString& message, const QString& location) :
  PiiException::Data(message.isEmpty() ? PiiHttpProtocol::statusMessage(statusCode) : message, location),
  iStatusCode(statusCode)
{
}


PiiHttpException::PiiHttpException(int statusCode, const QString& location) :
  PiiNetworkException(new Data(statusCode, location))
{
}

PiiHttpException::PiiHttpException(int statusCode, const QString& message, const QString& location) :
  PiiNetworkException(new Data(statusCode, message, location))
{
}

PiiHttpException::PiiHttpException(const PiiHttpException& other) :
  PiiNetworkException(new Data(other.statusCode(), other.message(), other.location()))
{
}

int PiiHttpException::statusCode() const { return _d()->iStatusCode; }
