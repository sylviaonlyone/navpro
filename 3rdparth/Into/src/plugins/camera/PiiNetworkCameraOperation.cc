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

#include "PiiNetworkCameraOperation.h"
#include <QUrl>
#include <QFile>
#include <cstdio>

PiiNetworkCameraOperation::Data::Data() :
  pSocket(0),
  pImageReader(0),
  iPort(0),
  dMaxIoDelay(1.0),
  bStreamMode(false),
  bIgnoreErrors(false)
{
}

PiiNetworkCameraOperation::PiiNetworkCameraOperation() :
  PiiImageReaderOperation(new Data)
{
  PII_D;
  setMaxIoDelay(3.0);
  addSocket(d->pUrlInput = new PiiInputSocket("url"));
  d->pUrlInput->setOptional(true);
}

PiiNetworkCameraOperation::~PiiNetworkCameraOperation()
{
  PII_D;
  delete d->pSocket;
  delete d->pImageReader;
}

void PiiNetworkCameraOperation::setMaxIoDelay(double delay)
{
  PII_D;
  d->dMaxIoDelay = delay;
  //if (d->pSocket)
  //  d->pSocket->setWaitTime((int)(delay*1000));
}

void PiiNetworkCameraOperation::checkUrl(const QUrl& url)
{
  if (url.isEmpty())
    PII_THROW(PiiExecutionException, tr("Image URL is empty."));
  if (!url.isValid())
    PII_THROW(PiiExecutionException, tr("Image URL (%1) is not valid.").arg(url.toString()));
  if (url.scheme().toLower() != "http")
    PII_THROW(PiiExecutionException, tr("Unsupported protocol \"%1\" in image URL.").arg(url.scheme()));
}

void PiiNetworkCameraOperation::check(bool reset)
{
  PII_D;
  PiiImageReaderOperation::check(reset);
  
  QUrl proxy(d->strProxyUrl);
  if (!proxy.isEmpty())
    {
      if (!proxy.isValid())
        PII_THROW(PiiExecutionException, tr("The given proxy URL (%1) is not valid.").arg(d->strProxyUrl));
      if (proxy.scheme().toLower() != "http")
        PII_THROW(PiiExecutionException, tr("Unsupported protocol \"%1\" in proxy URL.").arg(proxy.scheme()));
    }

  if (!d->pUrlInput->isConnected())
    {
      QUrl image(d->strImageUrl);
      checkUrl(image);
      
      d->strHost = image.host();
      d->iPort = image.port(80);
      d->strPath = image.path();
    }

  if (reset)
    d->strPreviousUrl = "";
  
  //d->http.setHost(image.host(), (qint16)image.port(80));
  //d->http.setUser(image.userName(), image.password());
  //d->http.setProxy(proxy.host(), proxy.port(), proxy.userName(), proxy.password());
  //d->imageReader.setFormat(d->strImageFormat.toAscii());
  //d->imageReader.setFormat("jpeg");
}

void PiiNetworkCameraOperation::process()
{
  PII_D;
  // PENDING rewrite to use PiiNetworkClient and PiiHttpDevice
  // If the socket does not exist, create
  if (!d->pSocket)
    {
      d->pSocket = new QTcpSocket;
      d->pImageReader = new QImageReader(d->pSocket);
    }

  QString strHost = d->strHost, strPath = d->strPath;
  qint16 iPort = d->iPort;
  QString strUrl = d->strImageUrl;

  if (d->pUrlInput->isConnected())
    {
      PiiVariant urlObj = d->pUrlInput->firstObject();
      if (urlObj.type() != PiiYdin::QStringType)
        PII_THROW_UNKNOWN_TYPE(d->pUrlInput);
      strUrl = urlObj.valueAs<QString>();
      QUrl imageUrl(strUrl);
      checkUrl(imageUrl);

      strHost = imageUrl.host();
      iPort = imageUrl.port(80);
      strPath = imageUrl.path();
    }

  // If URL has changed, we need to reconnect
  if (strUrl != d->strPreviousUrl)
    {
      disconnectSocket();
      d->strPreviousUrl = strUrl;
    }
      
  // If the socket is unconnected, connect and send HTTP request
  if (d->pSocket->state() == QAbstractSocket::UnconnectedState)
    {
      d->pSocket->connectToHost(strHost, iPort);
      if (!d->pSocket->waitForConnected(10000))
        {
          qWarning("Could not connect to host.");
          PII_THROW(PiiExecutionException, tr("Could not connect to %1:%2: %3.").arg(strHost).arg(iPort).arg(d->pSocket->errorString()));
        }
      d->pSocket->write(QString("GET %1 HTTP/1.1\r\nHost: %2\r\nConnection: Keep-Alive\r\n\r\n").arg(strPath).arg(strHost).toAscii());
    }

  // Read headers
  bool keepAlive = false;
  QString headerStr;

  while (true)
    {
      QString line = d->pSocket->readLine();
      if (line == "\r\n" || line == "\n" || line.size() == 0)
        break;
      else if (line.isEmpty())
        {
          disconnectSocket();
          if (!d->bIgnoreErrors)
            PII_THROW(PiiExecutionException, tr("HTTP connection timed out."));
          return;
        }

      headerStr += line;
    } // while headers

  // Parse response headers
  QHttpResponseHeader header(headerStr);

  if (header.statusCode() != 200) // 200 OK
    {
      disconnectSocket();
      if (!d->bIgnoreErrors)
        PII_THROW(PiiExecutionException, tr("HTTP error: %1").arg(header.reasonPhrase()));
      return;
    }
  
  if (header.hasContentType() && header.contentType().startsWith("image/") && header.contentType().size() > 6)
    {
      d->pImageReader->setFormat(header.contentType().mid(6).toAscii());
      //qDebug("Image format: %s", header.contentType().mid(6).toAscii().constData());
    }
  
  if (header.hasContentLength())
    {
      //qDebug("Content-Length: %d", header.contentLength());
      //d->pSocket->setDataSize(header.contentLength());
    }

  keepAlive = header.value("Connection") == "Keep-Alive";

  /*
  unsigned char bfr[2];
  d->pSocket->peek((char*)bfr,2);
  qDebug("0x%x 0x%x", (unsigned int)bfr[0], (unsigned int)bfr[1]);
  */

  // Decode image
  QImage image = d->pImageReader->read();

  // Reset (and disconnect) socket
  //d->pSocket->setDataSize(0);
  if (!d->bStreamMode && !keepAlive)
    disconnectSocket();

  //qDebug("process(): Socket is in  state %d", d->pSocket->state());

  // Check image for errors
  if (image.isNull() || image.width() == 0 || image.height() == 0)
    {
      //qWarning("Image decoding error: %s", d->pImageReader->errorString().toAscii().constData());
      disconnectSocket();
      if (!d->bIgnoreErrors)
        PII_THROW(PiiExecutionException, tr("Could not decode image at %1. Error message: %2")
                  .arg(strUrl).arg(d->pImageReader->errorString()));
    }
  // All is fine
  else
    {
      if (d->imageType == GrayScale)
        emitGrayImage(image);
      else
        emitColorImage(image);
    }
}

void PiiNetworkCameraOperation::disconnectSocket()
{
  PII_D;
  d->pSocket->disconnectFromHost();
}

QString PiiNetworkCameraOperation::imageUrl() const { return _d()->strImageUrl; }
void PiiNetworkCameraOperation::setImageUrl(const QString& url) { _d()->strImageUrl = url; }
QString PiiNetworkCameraOperation::proxyUrl() const { return _d()->strProxyUrl; }
void PiiNetworkCameraOperation::setProxyUrl(const QString& url) { _d()->strProxyUrl = url; }
bool PiiNetworkCameraOperation::streamMode() const { return _d()->bStreamMode; }
void PiiNetworkCameraOperation::setStreamMode(bool streamMode) { _d()->bStreamMode = streamMode; }
bool PiiNetworkCameraOperation::ignoreErrors() const { return _d()->bIgnoreErrors; }
void PiiNetworkCameraOperation::setIgnoreErrors(bool ignoreErrors) { _d()->bIgnoreErrors = ignoreErrors; }
double PiiNetworkCameraOperation::maxIoDelay() const { return _d()->dMaxIoDelay; }
