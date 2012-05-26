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

#include "PiiExecutionException.h"
#include <PiiOperation.h>

PiiExecutionException::Data::Data(const QString& message, const QString& location, Code c) :
  PiiException::Data(message, location),
  code(c)
{}

PiiExecutionException::Data::Data(const QString& message, const QString& location) :
  PiiException::Data(message, location),
  code(Error)
{}

PiiExecutionException::Data::Data(const Data& other) :
  PiiException::Data(other.strMessage, other.strLocation),
  code(other.code)
{}

PiiExecutionException::PiiExecutionException(const QString& message, const QString& location) :
  PiiException(new Data(message, location))
{}

PiiExecutionException::PiiExecutionException(Code code) :
  PiiException(new Data("", "", code))
{}

PiiExecutionException::PiiExecutionException(Code code, const QString& message, const QString& location) :
  PiiException(new Data(message, location, code))
{}

PiiExecutionException::PiiExecutionException(const PiiExecutionException& other) :
  PiiException(new Data(other.message(), other.location(), other.code()))
{}

PiiExecutionException::PiiExecutionException(Data* data) :
  PiiException(data)
{}

PiiExecutionException::~PiiExecutionException()
{}

PiiExecutionException::Code PiiExecutionException::code() const { return _d()->code; }
void PiiExecutionException::setCode(Code code) { _d()->code = code; }

const char* PiiExecutionException::errorName(PiiExecutionException::Code code)
{
  static const char* errorNames[] = { "Error", "Finished", "Interrupted", "Paused" };
  return errorNames[code];
}



PiiCompoundExecutionException::Data::Data(const QString& message, const QString& location) :
  PiiExecutionException::Data(message, location)
{}

PiiCompoundExecutionException::Data::Data(const Data& other) :
  PiiExecutionException::Data(other),
  lstExceptions(other.lstExceptions)
{}

PiiCompoundExecutionException::PiiCompoundExecutionException(const QString& message, const QString& location) :
  PiiExecutionException(new Data(message, location))
{}

PiiCompoundExecutionException::PiiCompoundExecutionException(const PiiCompoundExecutionException& other) :
  PiiExecutionException(new Data(*other._d()))
{}

void PiiCompoundExecutionException::addException(PiiOperation* operation,
                                                 const PiiExecutionException& ex)
{
  _d()->lstExceptions.append(qMakePair(operation, ex));
}

PiiCompoundExecutionException::ExceptionList PiiCompoundExecutionException::exceptions() const { return _d()->lstExceptions; }
