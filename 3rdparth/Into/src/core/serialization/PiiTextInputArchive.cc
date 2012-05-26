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

#include "PiiTextInputArchive.h"

PII_DEFINE_SERIALIZER(PiiTextInputArchive);
PII_DEFINE_FACTORY_MAP(PiiTextInputArchive);

PiiTextInputArchive::PiiTextInputArchive(QIODevice* d) : QTextStream(d)
{
  if (!d->isOpen())
    PII_SERIALIZATION_ERROR(StreamNotOpen);

  setCodec("UTF-8");

  // Read and verify ID
  char id[PII_TEXT_ARCHIVE_ID_LEN];
  if (d->read(id, PII_TEXT_ARCHIVE_ID_LEN) != PII_TEXT_ARCHIVE_ID_LEN)
    PII_SERIALIZATION_ERROR(StreamError);
  if (std::strncmp(id, PII_TEXT_ARCHIVE_ID, PII_TEXT_ARCHIVE_ID_LEN))
    PII_SERIALIZATION_ERROR(UnrecognizedArchiveFormat);

  // Read and verify version
  int iVersion;
  *this >> iVersion;
  // Version 0 had no major version number stored.
  if (iVersion > 0)
    {
      if (iVersion > PII_ARCHIVE_VERSION)
        PII_SERIALIZATION_ERROR(ArchiveVersionMismatch);
        
      setMajorVersion(iVersion);
      // Read text archive version
      *this >> iVersion;
    }
  else
    setMajorVersion(0);
  if (iVersion > PII_TEXT_ARCHIVE_VERSION)
    PII_SERIALIZATION_ERROR(ArchiveVersionMismatch);

  setMinorVersion(iVersion);
}

void PiiTextInputArchive::readRawData(void* ptr, unsigned int size)
{
  startDelim();
  // Base64 encoded data has no spaces.
  QString strEncoded;
  QTextStream::operator>>(strEncoded);
  QByteArray decoded(QByteArray::fromBase64(strEncoded.toAscii()));
  if (int(size) != decoded.size())
    PII_SERIALIZATION_ERROR(InvalidDataFormat);
  memcpy(ptr, decoded.constData(), size);
}

PiiTextInputArchive& PiiTextInputArchive::operator>> (QString& value)
{
  startDelim();
  // Read the raw bytes
  int len;
  *this >> len;
  if (len > 0)
    {
      // Read separator.
      char ws;
      QTextStream::operator>>(ws);
      // Read all characters
      value = QTextStream::read(len);
      if (value.size() != len)
        PII_SERIALIZATION_ERROR(InvalidDataFormat);
    }
  return *this;
}

PiiTextInputArchive& PiiTextInputArchive::operator>> (char*& value)
{
  int len;
  *this >> len;
  if (len < 0)
    PII_SERIALIZATION_ERROR(InvalidDataFormat);
  value = new char[len+1];
  if (len > 0)
    {
      startDelim();
      for (int i=0; i<len; i++)
        QTextStream::operator>>(value[i]);
    }
  // Terminate the string with null
  value[len] = '\0';
  return *this;
}
