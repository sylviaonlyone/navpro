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

#ifndef _PIISOCKETDEVICE_H
#define _PIISOCKETDEVICE_H

#include <QThread>
#include <QMutex>
#include "PiiNetworkGlobal.h"
#include <PiiTimer.h>
#include <PiiDelay.h>

class PiiProgressController;
class QIODevice;
class QAbstractSocket;
class QLocalSocket;

/**
 * A wrapper class that works as a generic socket device. The purpose
 * of this class is to work around Qt's lack of a common socket-type
 * superclass for QLocalSocket and QAbstractSocket. These cannot be
 * used interchangeably because their common superclass, QIODevice,
 * provides no general way of checking if the device is
 * readable/writable. For example, TCP sockets are "open" (isOpen()
 * returns @p true) even after the connection has been broken. This
 * class provides the #isReadable() and #isWritable() functions that
 * can be used to check if the socket is really available for reading
 * and writing. This class also provides functionality for reading and
 * writing data to/from devices that may not have everything available
 * at once.
 *
 * PiiSocketDevice is special in that it can be used in most places as
 * if it was a QIODevice pointer. Once initialized with a QIODevice
 * pointer, the class masquerades as one itself.
 *
 * @ingroup Network
 */
class PII_NETWORK_EXPORT PiiSocketDevice
{
public:
  /**
   * Constructs a null socket, which behaves like a null pointer. Any
   * access to the socket will cause a segfault.
   */
  PiiSocketDevice();
  /**
   * Constructs a PiiSocketDevice that wraps the given @a device. The
   * type of the device will be automatically determined. All Qt
   * socket types are recognized.
   */
  PiiSocketDevice(QIODevice* device);
  /**
   * Constructs a PiiSocketDevice that wraps the given @a socket.
   */
  PiiSocketDevice(QAbstractSocket* socket);
  /**
   * Constructs a PiiSocketDevice that wraps the given @a socket.
   */
  PiiSocketDevice(QLocalSocket* socket);
  /**
   * Constructs a copy of @a other.
   */
  PiiSocketDevice(const PiiSocketDevice& other);
  ~PiiSocketDevice();

  PiiSocketDevice& operator= (const PiiSocketDevice& other);

  /**
   * Returns @p true if it is possible that the socket still has data
   * and @p false otherwise. This function handles special cases such
   * as a disconnected socket with unread data in its receive buffer.
   */
  bool isReadable() const;
  
  /**
   * Returns @p true if the socket can be written to and @p false
   * otherwise.
   */
  bool isWritable() const;

  /**
   * Reads at most @a maxSize bytes of data from the socket and places
   * it to @a data. If not enough data is immediately available, waits
   * at most @a waitTime milliseconds to get more. If @a controller is
   * given, it can be used to terminate a long wait.
   */
  qint64 readWaited(char* data, qint64 maxSize, int waitTime = 5000, PiiProgressController* controller = 0);
  /**
   * Writes at most @a maxSize bytes of data from @a data to the
   * socket. If all data cannot be immediately written, waits at most
   * @a waitTime milliseconds. If @a controller is given, it can be
   * used to terminate a long wait.
   */
  qint64 writeWaited(const char* data, qint64 maxSize, int waitTime = 5000, PiiProgressController* controller = 0);

  /**
   * Disconnects the socket if it is connected to a server.
   */
  void disconnect();

  QIODevice* device() const;
  operator QIODevice* () const;
  QIODevice* operator-> () const;

private:
  enum Type { IODevice, AbstractSocket, LocalSocket };
  class Data : public PiiSharedD<Data>
  {
  public:
    Data();
    Data(QIODevice*);
    Data(QIODevice*, Type);
    QIODevice* pDevice;
    Type type;
  } *d;
  
  inline bool waitForDataAvailable(int waitTime, PiiProgressController* controller);
  inline bool waitForDataWritten(int waitTime, PiiProgressController* controller);
};

#endif //_PIISOCKETDEVICE_H
