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

#ifndef _PIILOCALSERVER_H
#define _PIILOCALSERVER_H

#include "PiiNetworkServer.h"
#include <QLocalServer>

/**
 * A threaded server that runs an application layer protocol over a
 * local socket. On Unix, this means a Unix-domain socket. On Windows,
 * pipes are used.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiLocalServer : public PiiNetworkServer
{
  Q_OBJECT

public:
  PiiLocalServer(PiiNetworkProtocol* protocol);
  ~PiiLocalServer();  
  /**
   * Start the server. This function will create a local socket and
   * bind it to the Unix domain socket/pipe specified. If the server
   * cannot listen to the socket, it will return @p false.
   *
   * Note that this function will not block. If the server starts
   * successfully, it will continue running in the backround.
   */
  bool start();
  
  void stop(PiiNetwork::StopMode mode);

  /**
   * Set the name of the socket/pipe to bind to. On Unix, this is the
   * name of the socket file. On windows, use a pipe path.
   *
   * @see QLocalServer::listen()
   */
  bool setServerAddress(const QString& serverAddress);
  QString serverAddress() const;
  
  /**
   * Creates a new QLocalSocket and assigns @p socketDescriptor to it.
   */
  QIODevice* createSocket(PiiGenericSocketDescriptor socketDescriptor);
  
private:
  class EntryPoint : public QLocalServer
  {
  public:
    EntryPoint(PiiLocalServer* owner);
    
  protected:
    /// Calls PiiLocalServer::incomingConnection().
    void incomingConnection(quintptr socketDescriptor);
    
  private:
    PiiLocalServer* _pOwner;
  };

  /// @internal
  class Data : public PiiNetworkServer::Data
  {
  public:
    Data(PiiLocalServer* owner, PiiNetworkProtocol* protocol);
    
    EntryPoint server;
    QString strSocketName;
  };
  PII_D_FUNC;

  /**
   * Calls PiiNetworkServer::incomingConnection().
   */
  void incomingConnection(quintptr socketDescriptor);

};


#endif //_PIILOCALSERVER_H
