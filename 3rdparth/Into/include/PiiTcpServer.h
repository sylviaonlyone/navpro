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

#ifndef _PIITCPSERVER_H
#define _PIITCPSERVER_H

#include <QTcpServer>
#include "PiiNetworkServer.h"

/**
 * A threaded TCP server. This class can be used to run an application
 * layer protocol over TCP. Use PiiLocalServer to run the protocols
 * over local sockets/pipes. To create a minimalistic HTTP server, do
 * this:
 *
 * @code
 * #include <QCoreApplication>
 * #include <PiiHttpProtocol.h>
 * #include <PiiTcpServer.h>
 *
 * int main(int argc, char* argv[])
 * {
 *   QCoreApplication app(argc, argv);
 *   PiiHttpProtocol p;
 *   PiiTcpServer server(&p);
 *   server.setPort(8080); // unprivileged port
 *   if (!server.start())
 *     return 1;
 *   return app.exec();
 * }
 * @endcode
 *
 * The server will run eternally and answer "404 Not Found" to all
 * requests. Look at the documentation of PiiHttpProtocol on how to
 * add URI handlers to the server.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiTcpServer : public PiiNetworkServer
{
  Q_OBJECT

  /**
   * The IP address of the network interface this server binds to. By
   * default, the address is "0.0.0.0", which causes the server to
   * bind to all interfaces. The bind address and the port can both be
   * set with the #serverAddress property.
   */
  Q_PROPERTY(QString bindAddress READ bindAddress WRITE setBindAddress);

  /**
   * The TCP port number to bind to. The default value is 0 and must
   * be changed before the server can work.
   */
  Q_PROPERTY(int port READ port WRITE setPort);

  /**
   * The number of milliseconds the server will wait for client input. 
   * If nothing has been received within the time-out period, the
   * socket device (see #createSocket()) will return with zero bytes
   * read. It is up to the protocol implementation to deal with
   * time-outs. The default value is 20000.
   */
  Q_PROPERTY(int readTimeout READ readTimeout WRITE setReadTimeout);

  /**
   * The encryption mode. The default value is @p NoEncryption. The
   * encryption mode only affects new connection attempts, not already
   * accepted connections.
   */
  Q_PROPERTY(Encryption encryption READ encryption WRITE setEncryption);
  Q_ENUMS(Encryption);
 
public:
  /**
   * Supported encryption modes.
   *
   * @lip NoEncryption - the connection will not be encrypted.
   *
   * @lip SslEncryption - the connection will be encrypted with SSLv3.
   */
  enum Encryption { NoEncryption = 0, SslEncryption };
  
  /**
   * Create a new TCP server that communicates with its clients with
   * the given application layer protocol and encryption mode.
   */
  PiiTcpServer(PiiNetworkProtocol* protocol, Encryption = NoEncryption);

  ~PiiTcpServer();
  
  /**
   * Start the server. This function will create a TCP socket and bind
   * it to the interface(s) and port specified. If the server cannot
   * listen to the socket, it will return @p false.
   *
   * Note that this function will not block. If the server starts
   * successfully, it will continue running in the backround.
   */
  bool start();
  
  void stop(PiiNetwork::StopMode mode);

  /**
   * Creates a new QTcpSocket or QSslSocket and assigns @p
   * socketDescriptor to it.
   */
  QIODevice* createSocket(PiiGenericSocketDescriptor socketDescriptor);
  
  void setBindAddress(const QString& bindAddress);
  QString bindAddress() const;
  void setPort(int port);
  int port() const;
  void setReadTimeout(int readTimeout);
  int readTimeout() const;
  void setEncryption(const Encryption& encryption);
  Encryption encryption() const;
  /**
   * Set the server's bind address. Format IPv4 addresses like
   * "123.123.123.123:80" and IPv6 addresses like
   * "[2001:db8::1428:57ab]:443".
   */
  bool setServerAddress(const QString& serverAddress);
  QString serverAddress() const;

private:
  class EntryPoint : public QTcpServer
  {
  public:
    EntryPoint(PiiTcpServer* owner);
    
  protected:
    /// Calls PiiTcpServer::incomingConnection().
    void incomingConnection(int sockedFd);
    
  private:
    PiiTcpServer* _pOwner;
  };
  
  friend class EntryPoint;
  
  /// @internal
  class Data : public PiiNetworkServer::Data
  {
  public:
    Data(PiiNetworkProtocol* protocol, PiiTcpServer* owner, Encryption encryption);
    
    EntryPoint server;
    QString strBindAddress;
    int iPort;
    int iReadTimeout;
    Encryption encryption;
  };
  PII_D_FUNC;

  /**
   * Calls PiiNetworkServer::incomingConnection().
   */
  void incomingConnection(int socketDescriptor);
};

#endif //_PIITCPSERVER_H
