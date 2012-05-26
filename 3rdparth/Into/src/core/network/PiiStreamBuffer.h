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

#ifndef _PIISTREAMBUFFER_H
#define _PIISTREAMBUFFER_H

#include "PiiStreamFilter.h"
#include <QBuffer>

/**
 * An output filter that buffers data into itself until #flushFilter()
 * is called.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiStreamBuffer : public QByteArray, public PiiDefaultStreamFilter
{
public:
  /**
   * Creates a new output buffer.
   */
  PiiStreamBuffer();

  ~PiiStreamBuffer();

  /**
   * Writes @p maxSize bytes of @p data to the itself.
   */
  qint64 filterData(const char* data, qint64 maxSize);

  /**
   * Sends all buffered data to the output device.
   */
  qint64 flushFilter();

  /**
   * Returns the number of bytes currently in the buffer.
   */
  qint64 bufferedSize() const;

protected:
  /// @internal
  class Data : public PiiDefaultStreamFilter::Data
  {
  public:
    Data(PiiStreamBuffer* owner);
    QBuffer* pBuffer;
  };
  inline Data* _d() { return static_cast<Data*>(PiiStreamFilter::d); }
  inline const Data* _d() const { return static_cast<const Data*>(PiiStreamFilter::d); }
  
  /// @internal
  PiiStreamBuffer(Data* d);
};

#endif //_PIISTREAMBUFFER_H
