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

#ifndef _PIINETWORKCLIENT_H
#define _PIINETWORKCLIENT_H

#include <QString>
#include <QObject>
#include <QAbstractSocket>

#include "PiiSocketDevice.h"

/**
 * A generic client for network connections. %PiiNetworkClient tries
 * to keep a connection open to avoid unnecessary reconnection delays.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiNetworkClient : public QObject
{
  Q_OBJECT
public:
  /**
   * Creates a new network client that is used to communicate with the
   * server at the given address. See PiiHttpServer for valid address
   * formats. Note that there must be no slash at the end of the
   * address.
   */
  PiiNetworkClient(const QString& serverAddress = "");

  /**
   * Destroys the device.
   */
  ~PiiNetworkClient();
  
  /**
   * Opens a connection to the server. The connection will be kept
   * alive if possible. Subsequent calls to this function check if the
   * connection is still open and try to reopen it if possible.
   *
   * @return a pointer to the communication device, or zero if the
   * connection failed. The QIODevice pointer in the returned device
   * is owned by %PiiNetworkClient and must not be deleted by the
   * caller.
   */
  PiiSocketDevice openConnection();

  /**
   * Sets the maximum number of milliseconds the %PiiNetworkClient
   * will wait for a successful connection. Set this value before
   * calling #openConnection(). The default value is 5000.
   */
  void setConnectionTimeout(int connectionTimeout);
  /**
   * Returns the connection time-out.
   */
  int connectionTimeout() const;

  /**
   * Sets the server address. The server at the new address will be
   * first contacted when #openConnection() is called next time. See
   * PiiHttpServer for valid address formats. If the server address
   * changes, the next #openConnection() call will delete the current
   * communication device and create a new one.
   *
   * @code
   * client.setServerAddress("tcp://127.0.0.1:3142");
   * @endcode
   */
  void setServerAddress(const QString& serverAddress);
  /**
   * Returns the current server address.
   */
  QString serverAddress() const;

public slots:
  /**
   * Closes the connection to the server.
   */
  void closeConnection();

private:
  PiiSocketDevice connectToServer();
  
  /// @internal
  class Data
  {
  public:
    Data(const QString& serverAddress);
    QString strServerAddress, strOldAddress;
    PiiSocketDevice pDevice;
    int iConnectionTimeout;
  } *d;
};

#endif //_PIINETWORKCLIENT_H
