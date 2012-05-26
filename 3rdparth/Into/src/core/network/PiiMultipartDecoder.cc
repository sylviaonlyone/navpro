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

#include "PiiMultipartDecoder.h"
#include "PiiMimeHeader.h"

PiiMultipartDecoder::Data::Data(QIODevice* device) :
  pDevice(device),
  bHeadersRead(false),
  iContentLength(-1),
  iCurrentMultipartDepth(0)
{
}

PiiMultipartDecoder::PiiMultipartDecoder(QIODevice* device) :
  d(new Data(device))
{
  open(device->openMode());
}

PiiMultipartDecoder::PiiMultipartDecoder(QIODevice* device, const QHttpHeader& header) :
  d(new Data(device))
{
  d->stkHeaders.push(header);
  updateBodyPartInfo();
  open(device->openMode());
}

PiiMultipartDecoder::~PiiMultipartDecoder()
{
  delete d;
}

qint64 PiiMultipartDecoder::readData(char* data, qint64 maxSize)
{
  // If content-length is given, trust it.
  if (d->iContentLength > 0)
    {
      qint64 iBytesRead = d->pDevice->read(data, qMin(maxSize, qint64(d->iContentLength)));
      if (iBytesRead < 0)
        return iBytesRead;
      d->iContentLength -= iBytesRead;
      if (d->iContentLength <= 0) // Done with the message body
        {
          d->iContentLength = -1;
          // This recursive call should read the boundary marker.
          read(d->aBfr.data(), d->aBoundary.size());
          // NOTE this should return 0 or the format is incorrect. We
          // don't check it here but hope the next header will fail
          // anyway.
        }
      
      return iBytesRead;
    }
  // Content-length is known, and no bytes can be read.
  else if (d->iContentLength == 0)
    return -1;
  // We are out of luck. Must filter the input for the boundary
  // delimiter.
  else if (d->aBoundary.size() > 0)
    { 
      // Read at most the size of the boundary bytes
      qint64 iBytesRead = d->pDevice->read(data, qMin(d->aBoundary.size(), int(maxSize)));
      if (iBytesRead <= 0)
        return iBytesRead;
      
      // Check received data for the boundary marker
      int i = 0, iTailLength = 0, iPeekLength = 0;
      while (i < iBytesRead)
        {
          // If the end of the read data matches the beginning of the
          // boundary marker, we must check if this is really the
          // start of the boundary.
          iTailLength = iBytesRead - i;
          iPeekLength = d->aBoundary.size()-iTailLength;
          if (memcmp(d->aBoundary.constData(), data + i, iTailLength) == 0)
            {
              // either the boundary was fully matched ...
              if (iPeekLength == 0)
                break;
              else
                {
                  // ... or the data to come next is the boundary
                  QByteArray aRest = d->pDevice->peek(iPeekLength);
                  if (aRest.size() == iPeekLength &&
                      memcmp(aRest.constData(), d->aBoundary.constData() + iTailLength, iPeekLength) == 0)
                    break;
                }
            }
          ++i;
        }

      // Great, we found the boundary
      if (i < iBytesRead)
        {
          // Put the boundary marker back. nextMessage() will handle it.
          while (iBytesRead > i)
            d->pDevice->ungetChar(data[--iBytesRead]);

          // This blocks reads beyond the boundary.
          d->iContentLength = 0;
          // This allows one to read a new header.
          d->bHeadersRead = false;
        }
      return iBytesRead;
    }
  // No boundary, no Content-Length. Too bad...
  else
    return d->pDevice->read(data, maxSize);
}

qint64 PiiMultipartDecoder::writeData(const char* data, qint64 maxSize)
{
  return d->pDevice->write(data, maxSize);
}

bool PiiMultipartDecoder::isSequential() const
{
  return d->pDevice->isSequential();
}

qint64 PiiMultipartDecoder::bytesAvailable() const
{
  return d->pDevice->bytesAvailable();
}

bool PiiMultipartDecoder::readPreamble()
{
  // Read preamble until the next delimiter.
  d->stkHeaders.top().setPreamble(readAll());
  return true;
}


bool PiiMultipartDecoder::nextMessage()
{
  // Can't reread headers
  if (d->bHeadersRead)
    return false;

  for (;;)
    {
      QByteArray aHeader(PiiMimeHeader::readHeaderData(d->pDevice, 4096));
      // The first line of the header can be a message end boundary.
      while (d->aBoundary.size() > 0 && aHeader.startsWith(d->aBoundary))
        {
          int iBoundarySize = d->aBoundary.size();
          int iHeaderSize = aHeader.size();

          // The boundary marker must be followed by either \r\n or
          // --\r\n. We also support \n linefeeds.
          if (iHeaderSize < iBoundarySize + 1)
            PII_THROW_MIME(InvalidFormat);
          switch (aHeader.at(iBoundarySize))
            {
            case '-': // Double dash finishes a multipart message.
              if (iHeaderSize < iBoundarySize + 2 || aHeader[iBoundarySize + 1] != '-')
                PII_THROW_MIME(InvalidFormat);
              popHeader();
              // This was the final boundary
              if (d->stkHeaders.isEmpty())
                return false;
              // Intermediate boundary markers must be followed by a newline.
              iBoundarySize += 2;
              if (iHeaderSize < iBoundarySize + 1)
                PII_THROW_MIME(InvalidFormat);
              if (aHeader[iBoundarySize] == '\n')
                break;
              if (aHeader[iBoundarySize] != '\r')
                PII_THROW_MIME(InvalidFormat);
            case '\r':
              if (iHeaderSize < iBoundarySize + 2 || aHeader[iBoundarySize + 1] != '\n')
                PII_THROW_MIME(InvalidFormat);
              ++iBoundarySize;
              break;
            case '\n':
              break;
            default:
              PII_THROW_MIME(InvalidFormat);
            }
          aHeader = aHeader.mid(iBoundarySize+1);
        }
      if (aHeader.isEmpty())
        return false;

      PiiMimeHeader header(aHeader);
      if (!header.isValid())
        PII_THROW_MIME(InvalidFormat);

      // If the topmost header represents a multipart message, build
      // the stack.
      if (d->stkHeaders.size() == 0 || d->stkHeaders.top().isMultipart())
        d->stkHeaders.push(header);
      // Otherwise replace the previous header with the newly read
      // one.
      else
        d->stkHeaders.top() = header;
      updateBodyPartInfo();
      if (header.isMultipart())
        {
          if (!readPreamble()) return false;
        }
      else
        break;
    }
  
  d->bHeadersRead = true;
  return true;
}

void PiiMultipartDecoder::popHeader()
{
  while (d->stkHeaders.size() >= d->iCurrentMultipartDepth)
    d->stkHeaders.pop();
  d->bHeadersRead = false;
  updateBodyPartInfo();
}

void PiiMultipartDecoder::updateBodyPartInfo()
{
  if (d->stkHeaders.size() > 0 &&
      d->stkHeaders.top().hasContentLength() &&
      !d->stkHeaders.top().isMultipart()) // Multipart message should not have Content-length
    d->iContentLength = d->stkHeaders.top().contentLength();
  else
    d->iContentLength = -1;
  
  // If the next body part is part of a multipart message, read until
  // a delimiter is found. The delimiter is that of the topmost
  // multipart message.
  for (int i=d->stkHeaders.size(); i--; )
    {
      if (d->stkHeaders[i].isMultipart())
        {
          d->aBoundary = d->stkHeaders[i].boundary().toAscii();
          d->aBoundary.prepend("--");
          d->aBfr.resize(d->aBoundary.size());
          d->iCurrentMultipartDepth = i+1;
          return;
        }
    }
  d->aBoundary.clear();
}

PiiMimeHeader PiiMultipartDecoder::header(int level)
{
  if (level >= 0 && level < d->stkHeaders.size())
    return d->stkHeaders[d->stkHeaders.size()-1-level];
  
  return PiiMimeHeader();
}

int PiiMultipartDecoder::depth()
{
  return d->stkHeaders.size();
}
