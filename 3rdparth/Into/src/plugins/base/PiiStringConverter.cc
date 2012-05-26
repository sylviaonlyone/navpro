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

#include "PiiStringConverter.h"

#include <PiiYdinTypes.h>

PiiStringConverter::Data::Data() :
  conversionMode(ConvertToInt)
{
}

PiiStringConverter::PiiStringConverter() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

void PiiStringConverter::process()
{
  PiiVariant obj = readInput();
  if (obj.type() != PiiYdin::QStringType)
    PII_THROW_UNKNOWN_TYPE(inputAt(0));
  if (_d()->conversionMode == ConvertToInt)
    emitObject(Pii::stringTo<int>(obj.valueAs<QString>()));
  else
    emitObject(Pii::stringTo<double>(obj.valueAs<QString>()));
}

void PiiStringConverter::setConversionMode(ConversionMode conversionMode) { _d()->conversionMode = conversionMode; }
PiiStringConverter::ConversionMode PiiStringConverter::conversionMode() const { return _d()->conversionMode; }
