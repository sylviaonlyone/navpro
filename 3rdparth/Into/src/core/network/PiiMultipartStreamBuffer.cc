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

#include "PiiMultipartStreamBuffer.h"

PiiMultipartStreamBuffer::Data::Data(PiiMultipartStreamBuffer* owner, const QString& boundary) :
  PiiStreamBuffer::Data(owner),
  strHeader(QString("\r\n"
                    "--%1\r\n"
                    "Content-Length: %2\r\n").arg(boundary)),
  bFlushed(false)
{
}

PiiMultipartStreamBuffer::PiiMultipartStreamBuffer(const QString& boundary) :
  PiiStreamBuffer(new Data(this, boundary))
{
}

PiiMultipartStreamBuffer::~PiiMultipartStreamBuffer()
{
}

void PiiMultipartStreamBuffer::setHeader(const QString& name, const QString& value)
{
  _d()->strHeader += name + ": " + value + "\r\n";
}

qint64 PiiMultipartStreamBuffer::flushFilter()
{
  PII_D;
  if (d->pOutputFilter)
    // Send MIME header and buffered body
    return d->pOutputFilter->filterData((d->strHeader.arg(PiiStreamBuffer::bufferedSize()) + "\r\n").toAscii()) +
      PiiStreamBuffer::flushFilter();
  d->bFlushed = true;
  return 0;
}

qint64 PiiMultipartStreamBuffer::bufferedSize() const
{
  const PII_D;
  if (d->bFlushed) return 0;
  
  qint64 iBodyLength = PiiStreamBuffer::bufferedSize();
  QString strBodyLength = QString::number(iBodyLength);
  // In d->strHeader, %2 needs to be taken away, but the final \r\n
  // added in flushFilter() cancels the effect.
  return iBodyLength + d->strHeader.size() + strBodyLength.size();
}
