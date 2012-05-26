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

#ifndef _PIIHTTPSERVER_H
#define _PIIHTTPSERVER_H

#include <QMutex>
#include <QMap>
#include "PiiHttpProtocol.h"
#include "PiiNetworkServer.h"

/**
 * A HTTP server. This class holds an application-wide registry of
 * HTTP servers. The servers can be created and accessed with
 * user-defined names.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiHttpServer
{
public:
  ~PiiHttpServer();

  /**
   * Returns the protocol instance that parses requests to this server
   * and communicates with the clients.
   */
  PiiHttpProtocol* protocol() const;

  /**
   * Returns the network server that manages incoming connections.
   */
  PiiNetworkServer* networkServer() const;

  /**
   * A shorthand for networkServer()->start().
   */
  bool start();
  
  /**
   * A shorthand for networkServer()->stop().
   */
  void stop(PiiNetwork::StopMode mode = PiiNetwork::WaitClients);

  /**
   * Adds a new server to the list of HTTP servers using the given @p
   * serverName and the binding address @p address. I there already
   * exists a server with the same name, the old one will be
   * destroyed. If there are no servers, the first one will be set as
   * the default server.
   *
   * @param serverName the name of the server. The server instance can
   * be later retrieved with #server(). If @p serverName is empty,
   * the new server will become the default server.
   *
   * @param address the low level protocol and the address to bind the
   * server to. The address may be either an IPv4 address
   * (tcp://123.123.123.123:80 or ssl://0.0.0.0:443), an IPv6 address
   * (tcp://[2001:db8::1428:57ab]:80 or ssl://[::1]:443), or the name
   * of a local socket (local:///tmp/server.sock on Linux,
   * local://\\\\.\\pipe\\socket on Windows). Network addresses must
   * contain a port number and no trailing slash. The server currently
   * supports @p tcp, @p ssl, and @p local connections.
   *
   * @return a pointer to a new PiiHttpServer instance, or zero if the
   * address is not valid. The pointer is still owned by
   * %PiiHttpServer; one should not delete it.
   *
   * @code
   * // Create a HTTPS server (0.0.0.0 binds to all network interfaces)
   * PiiHttpFileSystemHandler handler("/var/www/securehtml");
   * PiiHttpServer* server = PiiHttpServer::addServer("secure", "ssl://0.0.0.0:443/");
   * server->protocol()->addHandler("/", &handler);
   *
   * // Create another server that uses the same handler, but
   * // communicates through a local socket.
   * server = PiiHttpServer::addServer("local", "local:///tmp/http.sock");
   * server->protocol()->addHandler("/", &handler);
   * @endcode
   *
   * This function is thread-safe.
   */
  static PiiHttpServer* addServer(const QString& serverName, const QString& address);

  /**
   * @overload
   *
   * Same as addServer("", address).
   */
  static PiiHttpServer* addServer(const QString& address) { return addServer("", address); }
  
  /**
   * Creates a new instance of PiiHttpServer at the given address. See
   * #addServer() for valid addresses. This function does not add the
   * new server to the server list, and the caller is responsible for
   * deleting the server.
   *
   * This function is thread-safe.
   */
  static PiiHttpServer* createServer(const QString& address);
  
  /**
   * Returns the server called @p serverName. The server must have
   * been previously added with #addServer(). If @p serverName is
   * empty, the default server will be returned.
   *
   * This function is thread-safe.
   */
  static PiiHttpServer* server(const QString& serverName = "");

  /**
   * Removes the server called @p serverName from the list of HTTP
   * servers.
   *
   * This function is thread-safe.
   */
  static void removeServer(const QString& serverName);

private:
  class ServerMap : public QMap<QString, PiiHttpServer*>
  {
  public:
    ~ServerMap()
    {
      for (iterator i = begin(); i != end(); ++i)
        delete i.value();
    }
  };

  PiiHttpServer(PiiNetworkServer* server, PiiHttpProtocol* protocol);

  /// @internal
  class Data
  {
  public:
    Data(PiiNetworkServer* server, PiiHttpProtocol* protocol);
    virtual ~Data();
    PiiNetworkServer* pServer;
    PiiHttpProtocol* pProtocol;
  } *d;

  static void deleteServer(const QString& serverName);
  
  static ServerMap* serverMap();
  static QMutex _mapLock;
  static PiiHttpServer* _pDefaultServer;
};

#endif //_PIIHTTPSERVER_H
