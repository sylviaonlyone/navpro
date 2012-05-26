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

#ifndef _PIIFIFOBUFFER_H
#define _PIIFIFOBUFFER_H

#include <QIODevice>
#include <QMutex>
#include <QWaitCondition>
#include "PiiGlobal.h"

/**
 * A first in first out I/O device. PiiFifoBuffer is a thread-safe
 * buffer that can be read and written simultaneously. It reads and
 * writes data into a fixed array in memory. PiiFifoBuffer can work as
 * a replacement to QBuffer in situations where the amount of incoming
 * data is unlimited.
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiFifoBuffer : public QIODevice
{
  Q_OBJECT
  
public:
  /**
   * Create a new fifo buffer.
   *
   * @param size the number of bytes to reserve for the memory buffer
   */
  PiiFifoBuffer(qint64 size);

  /**
   * Destroy the buffer.
   */
  ~PiiFifoBuffer();

  /**
   * Set the number of millisecond a reader/writer will halted if no
   * data is available for reading or there is no free space for
   * writing. Reading/writing will continue as soon as new data is
   * available or something has been read out. Zero means that if data
   * is not immediately available, reading/writing will fail. The
   * default value is 100.
   */
  void setWaitTime(unsigned long readWaitTime);
  /**
   * Get the current wait time.
   */
  unsigned long waitTime() const;

  /**
   * Move both reading and writing position to the beginning of the buffer.
   */
  bool reset();

  /**
   * Move the @i reading position. Writing is always sequential, but
   * reading position can be changed in certain limits. Seeking
   * backwards is supported only within the unget buffer. Seeking
   * forwards is equal to skipping data, and can happen as far as the
   * writer provides new data to skip.
   */
  bool seek(qint64 pos);
  qint64 pos() const;
  bool atEnd() const;

  /**
   * Tell the buffer that everything has been written. This informs
   * possible readers that there is no more data to come, which helps
   * in avoiding unnecessary waiting. Call #reset() to start writing
   * again.
   */
  void finishWriting();
  
  /**
   * Returns true.
   */
  bool isSequential() const;

protected:
  /**
   * Read at most @p maxSize bytes into @p data. In any case, the
   * number of bytes that can be read is less than or equal to the
   * size of the memory buffer. If less data has been written to the
   * buffer, the number of bytes written so far limits the number of
   * readable bytes.
   *
   * If the buffer is empty, the calling thread will be halted for at
   * most @p waitTime milliseconds. The wait will be interrupted
   * as soon as new data is available. The function returns the number
   * of bytes read.
   */
  qint64 readData(char * data, qint64 maxSize);

  /**
   * Write at most @p maxSize bytes from @p data. The capacity of the
   * internal buffer limits the amount of data that can be written. 
   * Once the buffer is full, no data can be written before something
   * is read out.
   *
   * If the buffer is full, the calling thread will be blocked for at
   * most @p waitTime milliseconds. The functioning is analogous to
   * @ref readData().
   */
  qint64 writeData(const char * data, qint64 maxSize);

  qint64 bytesAvailable () const;

private:
  qint64 readBytes(char * data, qint64 maxSize);

  class Data
  {
  public:
    Data(qint64 size);
    qint64 iSize, iReadStart, iWriteStart, iFreeSpace, iReadTotal;
    char* pBuffer;
    unsigned long ulWaitTime;
    QMutex bufferLock;
    QWaitCondition dataWritten, dataRead;
    bool bWriteFinished;
  } *d;
};


#endif //_PIIFIFOBUFFER_H
