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

#include "PiiBinaryOutputArchive.h"

PII_DEFINE_SERIALIZER(PiiBinaryOutputArchive);
PII_DEFINE_FACTORY_MAP(PiiBinaryOutputArchive);

PiiBinaryOutputArchive::PiiBinaryOutputArchive(QIODevice* d) : QDataStream(d)
{
  if (!d->isOpen())
    PII_SERIALIZATION_ERROR(StreamNotOpen);

  // Store archive ID
  if (d->write(PII_BINARY_ARCHIVE_ID, PII_BINARY_ARCHIVE_ID_LEN) != PII_BINARY_ARCHIVE_ID_LEN)
    PII_SERIALIZATION_ERROR(StreamError);

  // Store archive version
  *this << PII_ARCHIVE_VERSION;
  *this << PII_BINARY_ARCHIVE_VERSION;
}

void PiiBinaryOutputArchive::writeRawData(const void* ptr, unsigned int size)
{
  // Store bytes
  if (QDataStream::writeRawData(reinterpret_cast<const char*>(ptr), size) != int(size))
    PII_SERIALIZATION_ERROR(StreamError);
}

PiiBinaryOutputArchive& PiiBinaryOutputArchive::operator<< (const QString& value)
{
  QByteArray utf8Data = value.toUtf8();
  writeArray(utf8Data.constData(), utf8Data.size());
  return *this;
}

PiiBinaryOutputArchive& PiiBinaryOutputArchive::operator<< (const char* value)
{
  // Write also the null byte at the end
  writeArray(value, strlen(value)+1);
  return *this;
}
