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

#include "PiiHttpServer.h"
#include "PiiTcpServer.h"
#include "PiiLocalServer.h"

QMutex PiiHttpServer::_mapLock;
PiiHttpServer* PiiHttpServer::_pDefaultServer = 0;

PiiHttpServer::Data::Data(PiiNetworkServer* server,
                          PiiHttpProtocol* protocol) :
  pServer(server), pProtocol(protocol)
{
}

PiiHttpServer::Data::~Data()
{
  delete pServer;
  delete pProtocol;
}


PiiHttpServer::PiiHttpServer(PiiNetworkServer* server, PiiHttpProtocol* protocol) :
  d(new Data(server, protocol))
{
  server->setBusyMessage("HTTP/1.1 503 Service Unavailable");
}

PiiHttpServer::~PiiHttpServer()
{
  delete d;
}

PiiHttpProtocol* PiiHttpServer::protocol() const
{
  return d->pProtocol;
}

PiiNetworkServer* PiiHttpServer::networkServer() const
{
  return d->pServer;
}

PiiHttpServer::ServerMap* PiiHttpServer::serverMap()
{
  static ServerMap map;
  return &map;
}

PiiHttpServer* PiiHttpServer::addServer(const QString& serverName, const QString& address)
{
  QMutexLocker lock(&_mapLock);

  deleteServer(serverName);
  PiiHttpServer* pServer = createServer(address);
  if (pServer != 0)
    {
      // First will become default. So will an empty name.
      if (serverMap()->size() == 0 || serverName.isEmpty())
        _pDefaultServer = pServer;
      serverMap()->insert(serverName, pServer);
    }
  return pServer;
}

PiiHttpServer* PiiHttpServer::server(const QString& serverName)
{
  QMutexLocker lock(&_mapLock);
  if (serverName.isEmpty())
    return _pDefaultServer;
  return serverMap()->value(serverName, 0);
}

void PiiHttpServer::removeServer(const QString& serverName)
{
  QMutexLocker lock(&_mapLock);
  deleteServer(serverName);
}

void PiiHttpServer::deleteServer(const QString& serverName)
{
  ServerMap* pMap = serverMap();
  if (pMap->contains(serverName))
    {
      PiiHttpServer* pServer = pMap->take(serverName);
      pServer->networkServer()->stop(PiiNetwork::InterruptClients);
      // Default was deleted
      if (pServer == _pDefaultServer)
        {
          // Reassign default if possible
          if (pMap->size() > 0)
            _pDefaultServer = *pMap->begin();
          else
            _pDefaultServer = 0;
        }
      delete pServer;
    }
}

PiiHttpServer* PiiHttpServer::createServer(const QString& address)
{
  int schemeIndex = address.indexOf("://");
  if (schemeIndex == -1)
    return 0;
  
  QString strScheme = address.left(schemeIndex);
  PiiHttpProtocol* pProtocol = new PiiHttpProtocol;
  PiiNetworkServer* pServer = 0;

  if (strScheme == "tcp")
    pServer = new PiiTcpServer(pProtocol);
  else if (strScheme == "ssl")
    pServer = new PiiTcpServer(pProtocol, PiiTcpServer::SslEncryption);
  else if (strScheme == "local")
    pServer = new PiiLocalServer(pProtocol);
  else
    {
      delete pProtocol;
      return 0;
    }

  if (!pServer->setServerAddress(address.mid(schemeIndex+3)))
    {
      delete pServer;
      delete pProtocol;
      return 0;
    }

  return new PiiHttpServer(pServer, pProtocol);
}

bool PiiHttpServer::start() { return d->pServer->start(); }
void PiiHttpServer::stop(PiiNetwork::StopMode mode) { return d->pServer->stop(mode); }
