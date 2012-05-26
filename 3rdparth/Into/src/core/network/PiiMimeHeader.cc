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

#include "PiiMimeHeader.h"
#include <PiiUtil.h>

PiiMimeHeader::Data::Data()
{}

PiiMimeHeader::Data::Data(const Data& other) :
  mapDispositionParams(other.mapDispositionParams),
  aPreamble(other.aPreamble),
  strContentDisposition(other.strContentDisposition),
  strBoundary(other.strBoundary)
{}

PiiMimeHeader::PiiMimeHeader() :
  d(new Data)
{}

PiiMimeHeader::PiiMimeHeader(const QHttpHeader& other) :
  QHttpHeader(other),
  d(new Data)
{
  parse();
}

PiiMimeHeader::PiiMimeHeader(const PiiMimeHeader& other) :
  QHttpHeader(other),
  d(new Data(*other.d))
{
  parse();
}

PiiMimeHeader::PiiMimeHeader(const QString& str) :
  QHttpHeader(str),
  d(new Data)
{
  parse();
}

PiiMimeHeader::~PiiMimeHeader()
{
  delete d;
}

void PiiMimeHeader::setPreamble(const QByteArray& preamble)
{
  d->aPreamble = preamble;
}
  
QByteArray PiiMimeHeader::preamble() const
{
  return d->aPreamble;
}

void PiiMimeHeader::parse()
{
  QString strDisposition = value("Content-Disposition");
  int semicolonIndex = strDisposition.indexOf(';');
  d->strContentDisposition = strDisposition.left(semicolonIndex).trimmed();
  if (semicolonIndex != -1)
    {
      d->mapDispositionParams = Pii::decodeProperties(strDisposition.mid(semicolonIndex + 1),
                                                      ';', '=', '\\',
                                                      Pii::TrimPropertyName |
                                                      Pii::TrimPropertyValue |
                                                      Pii::RemoveQuotes |
                                                      Pii::DowncasePropertyName);
    }

  QRegExp reBoundary("boundary=(\".[^\"]+\"|[^\r\n]+)", Qt::CaseInsensitive);
  if (reBoundary.indexIn(value("Content-Type")) != -1)
    {
      // The first subexpression
      d->strBoundary = reBoundary.cap(1);
      // Get rid of quotation marks
      if (d->strBoundary[0] == '"')
        d->strBoundary = d->strBoundary.mid(1, d->strBoundary.size()-2);
    }

  if (!hasContentType())
    setContentType("text/plain");
}

QString PiiMimeHeader::contentDisposition() const
{
  return d->strContentDisposition;
}

bool PiiMimeHeader::isMultipart() const
{
  return contentType().startsWith("multipart/");
}

bool PiiMimeHeader::isUploadedFile() const
{
  return contentDisposition() == "form-data" &&
    d->mapDispositionParams.contains("filename");
}

QString PiiMimeHeader::controlName() const
{
  return d->mapDispositionParams["name"].toString();
}

QString PiiMimeHeader::fileName() const
{
  return d->mapDispositionParams["filename"].toString();
}

QString PiiMimeHeader::boundary() const
{
  return d->strBoundary;
}

PiiMimeHeader& PiiMimeHeader::operator= (const PiiMimeHeader& other)
{
  QHttpHeader::operator= (other);
  d->mapDispositionParams = other.d->mapDispositionParams;
  d->aPreamble = other.d->aPreamble;
  d->strContentDisposition = other.d->strContentDisposition;
  d->strBoundary = other.d->strBoundary;
  return *this;
}

int PiiMimeHeader::majorVersion() const { return 1; }
int PiiMimeHeader::minorVersion() const { return 1; }


QByteArray PiiMimeHeader::readHeaderData(QIODevice* device, qint64 maxLength, qint64* bytesRead)
{
  static const qint64 iBufferSize = 4096;
  char lineBuffer[iBufferSize];
  qint64 iHeaderSize = 0;
  QByteArray aHeader;

  for (;;)
    {
      qint64 iCurrentBlockSize = qMin(maxLength, iBufferSize);
      // If buffered data is available, empty buffer first. The buffer
      // can contain a newline and we don't even need to consult the
      // low-level device.
      if (device->bytesAvailable() > 0)
        // Need +1 because readline reserves one byte for a linefeed character.
        iCurrentBlockSize = qMin(iCurrentBlockSize, device->bytesAvailable() + 1);
      //qDebug("readHeaderData(): Bytes available: %d. Block size: %d", int(device->bytesAvailable()), int(iCurrentBlockSize));
      // Read one line of HTTP header
      qint64 iBytes = device->readLine(lineBuffer, iCurrentBlockSize);
      //qDebug("readHeaderData(): got %d bytes", (int)bytes);

      // EOD or error in reading
      if (iBytes <= 0)
        break;

      iHeaderSize += iBytes;

      // Too many bytes in header
      if (iHeaderSize > maxLength)
        PII_THROW_MIME(HeaderTooLarge);

      // End of header
      if (*lineBuffer == '\r' || *lineBuffer == '\n')
        break;

      aHeader.append(lineBuffer, iBytes);
    }

  if (bytesRead != 0)
    *bytesRead = iHeaderSize;

  return aHeader;
}
