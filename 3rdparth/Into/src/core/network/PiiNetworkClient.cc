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

#include "PiiNetworkClient.h"
#include <QIODevice>
#include <QUrl>
#include <QTcpSocket>
#include <QSslSocket>
#include <QLocalSocket>
#include <QCoreApplication>

PiiNetworkClient::Data::Data(const QString& serverAddress) :
  strServerAddress(serverAddress),
  iConnectionTimeout(5000)
{
}

PiiNetworkClient::PiiNetworkClient(const QString& serverAddress) :
  d(new Data(serverAddress))
{
}

PiiNetworkClient::~PiiNetworkClient()
{
  closeConnection();
  delete d->pDevice.device();
  delete d;
}

void PiiNetworkClient::closeConnection()
{
  if (d->pDevice == 0)
    return;
  d->pDevice.disconnect();
  d->pDevice->close();
}

PiiSocketDevice PiiNetworkClient::openConnection()
{
  if (d->strServerAddress.isEmpty())
    return PiiSocketDevice();

  if (d->pDevice == 0 || !d->pDevice.isWritable() || d->strServerAddress != d->strOldAddress)
    {
      //qDebug("PiiNetworkClient::openConnection(): creating new device");
      delete d->pDevice;
      d->pDevice = connectToServer();
      d->strOldAddress = d->strServerAddress;
    }
  return d->pDevice;
}

PiiSocketDevice PiiNetworkClient::connectToServer()
{
  QUrl serverUrl(d->strServerAddress);
  QString strScheme = serverUrl.scheme();

  if (strScheme == "tcp")
    {
      QTcpSocket* pSocket = new QTcpSocket;
      pSocket->connectToHost(serverUrl.host(), serverUrl.port());
      if (!pSocket->waitForConnected(d->iConnectionTimeout))
        {
          delete pSocket;
          return PiiSocketDevice();
        }
      return pSocket;
    }
#ifndef QT_NO_OPENSSL  
  else if (strScheme == "ssl")
    {
      QSslSocket* pSocket = new QSslSocket;
      pSocket->connectToHostEncrypted(serverUrl.host(), serverUrl.port());
      if (!pSocket->waitForEncrypted(d->iConnectionTimeout))
        {
          delete pSocket;
          return PiiSocketDevice();
        }
      return pSocket;
    }
#endif
  else if (strScheme == "local")
    {
      QLocalSocket* pSocket = new QLocalSocket;
      pSocket->connectToServer(d->strServerAddress.mid(8)); // strlen("local://")
      if (!pSocket->waitForConnected(d->iConnectionTimeout))
        {
          delete pSocket;
          return PiiSocketDevice();
        }
      return pSocket;
    }
  
  return PiiSocketDevice();
}

void PiiNetworkClient::setConnectionTimeout(int connectionTimeout) { d->iConnectionTimeout = connectionTimeout; }
int PiiNetworkClient::connectionTimeout() const { return d->iConnectionTimeout; }
void PiiNetworkClient::setServerAddress(const QString& serverAddress) { d->strServerAddress = serverAddress; }
QString PiiNetworkClient::serverAddress() const { return d->strServerAddress; }
