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

#include "PiiBinaryInputArchive.h"

PII_DEFINE_SERIALIZER(PiiBinaryInputArchive);
PII_DEFINE_FACTORY_MAP(PiiBinaryInputArchive);

PiiBinaryInputArchive::PiiBinaryInputArchive(QIODevice* d) : QDataStream(d)
{
  if (!d->isOpen())
    PII_SERIALIZATION_ERROR(StreamNotOpen);

  // Read and verify ID
  char id[PII_BINARY_ARCHIVE_ID_LEN];
  if (d->read(id, PII_BINARY_ARCHIVE_ID_LEN) != PII_BINARY_ARCHIVE_ID_LEN)
    PII_SERIALIZATION_ERROR(StreamError);
  if (std::strncmp(id, PII_BINARY_ARCHIVE_ID, PII_BINARY_ARCHIVE_ID_LEN))
    PII_SERIALIZATION_ERROR(UnrecognizedArchiveFormat);

  // Read and verify major and minor version
  int iVersion;
  *this >> iVersion;
  if (iVersion > PII_ARCHIVE_VERSION)
    PII_SERIALIZATION_ERROR(ArchiveVersionMismatch);
  setMajorVersion(iVersion);
  *this >> iVersion;
  if (iVersion > PII_BINARY_ARCHIVE_VERSION)
    PII_SERIALIZATION_ERROR(ArchiveVersionMismatch);
  setMinorVersion(iVersion);
}

void PiiBinaryInputArchive::readRawData(void* ptr, unsigned int size)
{
  if (QDataStream::readRawData(static_cast<char*>(ptr), size) != int(size))
    PII_SERIALIZATION_ERROR(StreamError);
}
  
PiiBinaryInputArchive& PiiBinaryInputArchive::operator>> (QString& value)
{
  // Read the raw bytes
  unsigned int len;
  char* ptr;
  this->readArray(ptr, len);
  if (ptr != 0)
    value = QString::fromUtf8(ptr, len);
  else
    value.clear();
  delete[] ptr;
  return *this;
}

PiiBinaryInputArchive& PiiBinaryInputArchive::operator>> (char*& value)
{
  unsigned int len;
  this->readArray(value, len);
  // Should already be zero, but it costs next to nothing to ensure.
  value[len-1] = '\0';
  return *this;
}
