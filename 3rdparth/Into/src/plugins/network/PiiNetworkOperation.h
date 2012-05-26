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

#ifndef _PIINETWORKOPERATION_H
#define _PIINETWORKOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiHttpDevice.h>

#include <QHttpHeader>

#include "PiiNetworkPlugin.h"

/**
 * A base class for network input/output operations. Subclasses of
 * this class work as end points to network connections and make it
 * possible to send/receive data to and from remote computers.
 *
 * @inputs
 *
 * @in body - the body of a request or a response. Any data written to
 * this input will be sent as the request/response body to the HTTP
 * server/client. The body must be convertible to a QString. Any
 * primitive type will do.
 *
 * @in content type - the MIME type of the message body as a QString. 
 * This input is optional and cannot be connected alone. If the input
 * is not connected, the #contentType property will be used.
 *
 * @in inputX - a configurable number of optional input sockets. If
 * these inputs are connected, @p body and <tt>content type</tt> must
 * be left disconnected. The operation will encode the objects as
 * defined by the #messageEncoding property. Use the #inputNames
 * property to change the number of inputs and their names.
 *
 * @outputs
 *
 * @out outputX - a configurable number of outputs. X is a zero-based
 * output index. You can assign arbitrary alias names to outputs with
 * the #outputNames property.
 *
 * In PiiNetworkInputOperation, inputs should be connected as a
 * feed-back loop. Incoming network requests will be decoded and sent
 * to outputs. If no response is required, inputs can be left
 * disconnected.
 *
 * In PiiNetworkOutputOperation, inputs are encoded and sent to a
 * server, whose response will be decoded and sent to the output
 * sockets. If no response is received, no output will be produced.
 *
 * @ingroup PiiNetworkPlugin
 */
class PII_NETWORKPLUGIN_EXPORT PiiNetworkOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Output socket names. In PiiNetworkInputOperation, decoded HTTP
   * request variables will be sent to the corresponding named
   * outputs. In PiiNetworkOutputOperation, the variables decoded from
   * the server's response will be sent.
   */
  Q_PROPERTY(QStringList outputNames READ outputNames WRITE setOutputNames);

  /**
   * Input socket names. In PiiNetworkInputOperation, objects received
   * in the named inputs will be encoded and sent back to the HTTP
   * client as a response. In PiiNetworkOutputOperation, object
   * received in the inputs will be encoded and sent to the server as
   * a request.
   */
  Q_PROPERTY(QStringList inputNames READ inputNames WRITE setInputNames);

  /**
   * The MIME type of the request/response body. Used only if the @p
   * body input is connected. If the <tt>content type</tt> input is
   * connected, it overrides this value. The default value is @p
   * text/plain.
   */
  Q_PROPERTY(QString contentType READ contentType WRITE setContentType);

  /**
   * Control the behavior of the operation in error conditions. If the
   * flag is @p false (the default) the operation will signal an error
   * and stop if the network connection is lost or the server/client
   * sends invalid data.
   */
  Q_PROPERTY(bool ignoreErrors READ ignoreErrors WRITE setIgnoreErrors);

  /**
   * The maximum number of millisecond response data will be waited
   * for. In PiiNetworkInputOperation this is the time the operation
   * will wait for data to appear in its loop-back input socket. If a
   * response object is not received within time limits, the client
   * will be sent an error message. In PiiNetworkOutputOperation, this
   * is the time the operation will wait for a HTTP server to respond
   * after the operation has sent a request.
   *
   * The default value is 5000.
   */
  Q_PROPERTY(int responseTimeout READ responseTimeout WRITE setResponseTimeout);

public:
  ~PiiNetworkOperation();
  
  void check(bool reset);

  /**
   * Finds input sockets by their alias names.
   */
  PiiInputSocket* input(const QString& name) const;
  /**
   * Finds output sockets by their alias names.
   */
  PiiOutputSocket* output(const QString& name) const;

  void setInputNames(const QStringList& inputNames);
  QStringList inputNames() const;
  void setOutputNames(const QStringList& outputNames);
  QStringList outputNames() const;
  void setContentType(const QString& contentType);
  QString contentType() const;
  void setIgnoreErrors(bool ignoreErrors);
  bool ignoreErrors() const;
  void setResponseTimeout(int responseTimeout);
  int responseTimeout() const;

protected:
  /// Emit collected output values to named output sockets.
  void emitOutputValues();
  /**
   * Read and decode an object from @p device and add it to the output
   * value map with @p name.
   */
  void addToOutputMap(const QString& name, QIODevice& device);
  /**
   * Add variables to the output map.
   */
  void addToOutputMap(const QVariantMap& variables);
  /**
   * Add a variable to the output map. This function tries to convert
   * @p value into a PiiVariant. It recognizes integers, doubles and
   * strings.
   */
  void addToOutputMap(const QString& name, const QVariant& value);
  /**
   * Read and decode objects from @p h and add them to the output
   * value map.
   */
  bool decodeObjects(PiiHttpDevice& h, const QHttpHeader& header);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    
    /// Names of dynamic output sockets.
    QStringList lstOutputNames;
    /// Names of dynamic input sockets.
    QStringList lstInputNames;
    
    /// @p true iff the @p body input is connected.
    bool bBodyConnected;
    /// @p true iff the @p content @p type input is connected.
    bool bTypeConnected;
    /// @p false iff errors should cause termination of the operation
    bool bIgnoreErrors;
    /// Default content type if @p content @p type is not connected
    QString strContentType;
    /// The number of static inputs (not configured with #inputNames)
    int iStaticInputCount;
    /// The number of static outputs (not configured with #outputNames)
    int iStaticOutputCount;
    /// A pointer to the @p body input.
    PiiInputSocket* pBodyInput;
    /// A pointer to the @p content @p type input.
    PiiInputSocket* pTypeInput;
    /// Map of decoded output values.
    QMap<QString,PiiVariant> mapOutputValues;
    int iResponseTimeout;
  };
  PII_D_FUNC;
  /// @internal
  PiiNetworkOperation(Data *data);
};


#endif //_PIINETWORKOPERATION_H
