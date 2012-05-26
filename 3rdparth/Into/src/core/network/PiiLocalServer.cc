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

#include "PiiLocalServer.h"
#include <QLocalSocket>

PiiLocalServer::Data::Data(PiiLocalServer* owner, PiiNetworkProtocol* protocol) :
  PiiNetworkServer::Data(protocol),
  server(owner)
{
}

PiiLocalServer::PiiLocalServer(PiiNetworkProtocol* protocol) :
  PiiNetworkServer(new Data(this, protocol))
{
}

PiiLocalServer::~PiiLocalServer()
{
}

bool PiiLocalServer::start()
{
  PII_D;
  PiiNetworkServer::start();
  return d->server.listen(d->strSocketName);
}

void PiiLocalServer::stop(PiiNetwork::StopMode mode)
{
  PiiNetworkServer::stop(mode);
  _d()->server.close();
}
  
void PiiLocalServer::incomingConnection(quintptr socketDescriptor)
{
  PiiNetworkServer::incomingConnection(PiiGenericSocketDescriptor(socketDescriptor));
}

QIODevice* PiiLocalServer::createSocket(PiiGenericSocketDescriptor socketDescriptor)
{
  QLocalSocket* pSocket = new QLocalSocket;
  pSocket->setSocketDescriptor(socketDescriptor.pSocketDescriptor);
  return pSocket;
}

bool PiiLocalServer::setServerAddress(const QString& serverAddress)
{
  _d()->strSocketName = serverAddress;
  return true;
}

QString PiiLocalServer::serverAddress() const
{
  return _d()->strSocketName;
}


PiiLocalServer::EntryPoint::EntryPoint(PiiLocalServer* owner) : _pOwner(owner) {}
void PiiLocalServer::EntryPoint::incomingConnection(quintptr sockedFd) { _pOwner->incomingConnection(sockedFd); }

