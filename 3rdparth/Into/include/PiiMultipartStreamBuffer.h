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

#ifndef _PIIMULTIPARTSTREAMBUFFER_H
#define _PIIMULTIPARTSTREAMBUFFER_H

#include "PiiStreamBuffer.h"

/**
 * An output filter that buffers data into itself until #flush() is
 * called. It sends a MIME header before the actual data.
 *
 * @code
 * void MyHandler::handleRequest(const QString& uri,
 *                               PiiHttpDevice* h,
 *                               PiiProgressController* controller)
 * {
 *   QString strBoundary("243F6A8885A308D31319");
 *   h->setHeader("Content-Type", "multipart/mixed; boundary=\"" + strBoundary + "\"");
 *
 *   QStringList lstResponses = QStringList() << "Response1" << "Response2";
 *   for (int i=0; i<lstResponses.size(); ++i)
 *     {
 *        PiiMultipartStreamBuffer* bfr = new PiiMultipartStreamBuffer(strBoundary);
 *        bfr->setHeader("Content-Type", "text/plain");
 *        h->startOutputFiltering(bfr);
 *        h->print(lstResponses[i]);
 *        h->endOutputFiltering();
 *     }
 *   h->print("\r\n--" + strBoundary + "--\r\n");
 * }
 * @endcode
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiMultipartStreamBuffer : public PiiStreamBuffer
{
public:
  /**
   * Create a new multipart stream buffer.
   */
  PiiMultipartStreamBuffer(const QString& boundary);

  ~PiiMultipartStreamBuffer();
  
  /**
   * Sends all buffered data to the output device.
   */
  qint64 flushFilter();

  /**
   * Set a header field.
   */
  void setHeader(const QString& name, const QString& value);
  
  /**
   * Returns the number of bytes currently in the buffer, including
   * header information.
   */
  qint64 bufferedSize() const;

private:
  /// @internal
  class Data : public PiiStreamBuffer::Data
  {
  public:
    Data(PiiMultipartStreamBuffer* owner, const QString& boundary);
    QString strHeader, strContentType, strBoundary;
    bool bFlushed;
  };
  inline Data* _d() { return static_cast<Data*>(PiiStreamFilter::d); }
  inline const Data* _d() const { return static_cast<const Data*>(PiiStreamFilter::d); }
};


#endif //_PIIMULTIPARTSTREAMBUFFER_H
