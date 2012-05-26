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

#include "PiiTcpServer.h"
#include <QTcpSocket>
#include <QSslSocket>

PiiTcpServer::Data::Data(PiiNetworkProtocol* protocol, PiiTcpServer* owner, Encryption encryption) :
  PiiNetworkServer::Data(protocol),
  server(owner),
  strBindAddress("0.0.0.0"),
  iPort(0),
  iReadTimeout(20000),
  encryption(encryption)
{
}

PiiTcpServer::PiiTcpServer(PiiNetworkProtocol* protocol, Encryption encryption) :
  PiiNetworkServer(new Data(protocol, this, encryption))
{
}

PiiTcpServer::~PiiTcpServer()
{
}

bool PiiTcpServer::start()
{
  PII_D;
  PiiNetworkServer::start();
  return d->server.listen(QHostAddress(d->strBindAddress), d->iPort);
}

void PiiTcpServer::stop(PiiNetwork::StopMode mode)
{
  PiiNetworkServer::stop(mode);
  _d()->server.close();
}
  
void PiiTcpServer::incomingConnection(int socketDescriptor)
{
  PiiNetworkServer::incomingConnection(PiiGenericSocketDescriptor(socketDescriptor));
}

QIODevice* PiiTcpServer::createSocket(PiiGenericSocketDescriptor socketDescriptor)
{
  PII_D;
  if (d->encryption == NoEncryption)
    {
      QTcpSocket* pSocket = new QTcpSocket;
      if (!pSocket->setSocketDescriptor(socketDescriptor.iSocketDescriptor))
        {
          delete pSocket;
          return 0;
        }
      return pSocket;
    }
#ifndef QT_NO_OPENSSL
  else
    {
      QSslSocket* pSocket = new QSslSocket;
      if (!pSocket->setSocketDescriptor(socketDescriptor.iSocketDescriptor) ||
          !pSocket->waitForEncrypted(d->iReadTimeout))
        {
          delete pSocket;
          return 0;
        }
      return pSocket;
    }
#else
  return 0;
#endif
}

bool PiiTcpServer::setServerAddress(const QString& serverAddress)
{
  PII_D;
  int colonIndex = serverAddress.lastIndexOf(':');
  if (colonIndex == -1)
    return false;
  
  QHostAddress address;
  if (!address.setAddress(serverAddress.left(colonIndex)))
    return false;

  d->strBindAddress = address.toString();
  bool ok = false;
  d->iPort = serverAddress.mid(colonIndex+1).toInt(&ok);
  if (!ok)
    return false;

  return true;
}

QString PiiTcpServer::serverAddress() const
{
  const PII_D;
  QHostAddress address;
  if (!address.setAddress(d->strBindAddress))
    return "0.0.0.0:0";
  if (address.protocol() == QAbstractSocket::IPv6Protocol)
    return QString("[%1]:%2").arg(address.toString()).arg(d->iPort);
  else
    return QString("%1:%2").arg(address.toString()).arg(d->iPort);
}


PiiTcpServer::EntryPoint::EntryPoint(PiiTcpServer* owner) : _pOwner(owner) {}
void PiiTcpServer::EntryPoint::incomingConnection(int sockedFd) { _pOwner->incomingConnection(sockedFd); }

void PiiTcpServer::setBindAddress(const QString& bindAddress) { _d()->strBindAddress = bindAddress; }
QString PiiTcpServer::bindAddress() const { return _d()->strBindAddress; }
void PiiTcpServer::setPort(int port) { if (port > 0 && port < 65536) _d()->iPort = port; }
int PiiTcpServer::port() const { return _d()->iPort; }
void PiiTcpServer::setReadTimeout(int readTimeout) { _d()->iReadTimeout = readTimeout; }
int PiiTcpServer::readTimeout() const { return _d()->iReadTimeout; }
void PiiTcpServer::setEncryption(const Encryption& encryption) { _d()->encryption = encryption; }
PiiTcpServer::Encryption PiiTcpServer::encryption() const { return _d()->encryption; }
