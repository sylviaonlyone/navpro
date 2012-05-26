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

#ifndef _PIINETWORKCAMERAOPERATION_H
#define _PIINETWORKCAMERAOPERATION_H

#include <PiiImageReaderOperation.h>
#include <PiiFifoBuffer.h>
#include <QHttp>
#include <QImageReader>
#include "PiiCameraGlobal.h"
#include <PiiWaitCondition.h>
#include <QTcpSocket>

/**
 * A camera interface that reads images from a network camera. 
 * Currently, the operation only supports HTTP as the transfer
 * protocol.
 *
 * @inputs
 *
 * @in url - an optional image URL input. If this input is connected,
 * the QString read from it will override #imageUrl.
 *
 * @ingroup PiiCameraPlugin
 */
class PII_CAMERA_EXPORT PiiNetworkCameraOperation : public PiiImageReaderOperation
{
  Q_OBJECT

  /**
   * The URL of an image that is in some supported format. The syntax
   * is http://[user:password@]host[:port]/path. Bracketed parts are
   * optional. Example of valid URLs:
   *
   * @li http://me:password@my.host.com:88/image.jpg
   *
   * @li http://my.host.com/image.bmp
   *
   * @li http://10.10.10.7/JPEG.CGI
   */
  Q_PROPERTY(QString imageUrl READ imageUrl WRITE setImageUrl);

  /**
   * The URL of a HTTP proxy server. Use the same syntax as for @p
   * imageUrl, but do not provide a path. A valid proxy URL looks like
   * this: http://my.proxy.com:8088.
   */
  Q_PROPERTY(QString proxyUrl READ proxyUrl WRITE setProxyUrl);

  /**
   * Streaming mode flag. If the image URL is not a single image but
   * rather a continuous stream of images such as MJPEG, one may want
   * to set this flag for better performance. In stream mode, only one
   * HTTP request is performed and the stream is read forever,
   * provided that any further operations are able to process the
   * data. If they are not, the internal buffer of
   * PiiHttpCameraOperation may overflow which causes the operation to
   * abort. The default value is @p false.
   */
  Q_PROPERTY(bool streamMode READ streamMode WRITE setStreamMode);

  /**
   * Should the operation ignore network and image decoding errors? 
   * Set this flag to @p true if it does not harm to skip a few frames
   * due to bad network connections etc. The default is @p false.
   */
  Q_PROPERTY(bool ignoreErrors READ ignoreErrors WRITE setIgnoreErrors);

  /**
   * The maximum amount of time (in seconds) to wait for network I/O. 
   * Image decoder will wait at most this many seconds for data before
   * it cancels its operation. The default value is one. Use a larger
   * value for slow network connections.
   */
  Q_PROPERTY(double maxIoDelay READ maxIoDelay WRITE setMaxIoDelay);
  
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  PiiNetworkCameraOperation();
  ~PiiNetworkCameraOperation();

  QString imageUrl() const;
  void setImageUrl(const QString& url);

  QString proxyUrl() const;
  void setProxyUrl(const QString& url);

  bool streamMode() const;
  void setStreamMode(bool streamMode);

  bool ignoreErrors() const;
  void setIgnoreErrors(bool ignoreErrors);

  double maxIoDelay() const;
  void setMaxIoDelay(double maxIoDelay);

  void check(bool reset);

protected:
  void process();

private:
  /// @internal
  class Data : public PiiImageReaderOperation::Data
  {
  public:
    Data();
    
    QTcpSocket* pSocket;
    QImageReader *pImageReader;
    QString strImageUrl, strProxyUrl, strPath;
    QString strHost;
    qint16 iPort;
    double dMaxIoDelay;
    bool bStreamMode, bIgnoreErrors;
    PiiInputSocket* pUrlInput;
    QString strPreviousUrl;
  };
  PII_D_FUNC;

  void disconnectSocket();
  void checkUrl(const QUrl& url);
};

#endif //_PIINETWORKCAMERAOPERATION_H
