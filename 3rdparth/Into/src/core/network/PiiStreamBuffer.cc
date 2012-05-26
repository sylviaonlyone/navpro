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

#include "PiiStreamBuffer.h"

PiiStreamBuffer::Data::Data(PiiStreamBuffer* owner) :
  pBuffer(new QBuffer(owner))
{
  pBuffer->open(QIODevice::WriteOnly);
}

PiiStreamBuffer::PiiStreamBuffer() :
  PiiDefaultStreamFilter(new Data(this))
{
}

PiiStreamBuffer::PiiStreamBuffer(Data* data) :
  PiiDefaultStreamFilter(data)
{
}

PiiStreamBuffer::~PiiStreamBuffer()
{
  delete _d()->pBuffer;
}

qint64 PiiStreamBuffer::filterData(const char* bytes, qint64 maxSize)
{
  return _d()->pBuffer->write(bytes, maxSize);
}

qint64 PiiStreamBuffer::flushFilter()
{
  PII_D;
  if (d->pOutputFilter)
    {
      qint64 iBytesWritten = d->pOutputFilter->filterData(*this);
      clear();
      d->pBuffer->seek(0);
      return iBytesWritten;
    }
  return 0;
}

qint64 PiiStreamBuffer::bufferedSize() const
{
  return size();
}
