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

#ifndef _PIIREMOTEOBJECTCLIENT_H
#define _PIIREMOTEOBJECTCLIENT_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QCoreApplication>
#include <QMutex>
#include <QWaitCondition>

#include <PiiProgressController.h>

#include "PiiNetworkClient.h"
#include "PiiHttpDevice.h"

/**
 * A class that provides a client-side interface for a
 * PiiQObjectServer. When intialized, %PiiRemoteObjectClient inspects
 * the functions and properties provided by the server and reflects
 * them as dynamic signals, slots and properties on the client side. 
 * The remote object can then be used as if it was on the client side.
 *
 * The reflected object is identified by an URI that speficies the
 * protocol, address, and relative path of the server. The server at
 * the specified URI must implement the interface speficied by
 * PiiRemoteObjectServer.
 *
 * @code
 * PiiRemoteObjectClient client("tcp://intopii.com:3142/valuesetter/");
 * connect(pSlider, SIGNAL(valueChanged(int)), &client, SLOT(setValue(int)));
 * connect(&client, SIGNAL(valueChanged(int)), pSlider2, SLOT(setValue(int)));
 * @endcode
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiRemoteObjectClient :
  public QObject,
  private PiiProgressController
{
public:
  /// @cond null
  // No Q_OBJECT macro here. Instead, the meta-object interface is
  // implemented manually.
  const QMetaObject* metaObject() const;
  void* qt_metacast(const char *className);
  int qt_metacall(QMetaObject::Call call, int id, void** arguments);
  static inline QString tr(const char *s) { return QCoreApplication::translate("PiiRemoteObjectClient", s); }
  /// @endcond
  
  /**
   * Creates a new %PiiRemoteObjectClient. The object becomes
   * functional only after #setServerUri() has been called.
   */
  PiiRemoteObjectClient();

  ~PiiRemoteObjectClient();
  
  /**
   * Creates a new PiiRemoteObjectClient and sets the server URI at
   * the same time. See #setServerUri().
   *
   * @exception PiiNetworkException& if the server cannot be connected
   * @exception PiiInvalidArgumentException& if @a uri is incorrecly formatted
   */
  PiiRemoteObjectClient(const QString& serverUri);

  /**
   * Sets the server URI. The URI must contain a protocol, a host
   * name, and a path (a single slash at a minimum). Other components
   * (username, password and port) are optional. This function will
   * open a connection to the server and update the local meta object
   * based on the server's response.
   *
   * @code
   * PiiRemoteObjectClient client;
   * client.setServerUri("tcp://intopii.com:3142/valuesetter/");
   * @endcode
   *
   * @note Reassigning signals and slots makes all previously
   * requested meta objects invalid. You can no longer call the
   * functions or slots that were previously available, and no old
   * properties can be read or written to. If you had connected a
   * sender to a dynamic slot and the meta information changed,
   * invoking the slot later would crash your application. Therefore,
   * changing the server URI will not recreate the meta-object. It is
   * assumed that the same object is present in the new URI. This
   * makes it possible to move remote objects.
   *
   * @exception PiiNetworkException& if the server cannot be connected
   * @exception PiiInvalidArgumentException& if @a uri is incorrecly formatted
   */
  void setServerUri(const QString& uri);

  /**
   * Returns the URI of the server object.
   */
  QString serverUri() const;

  /**
   * Sets the number of times the class will try to connect to a
   * server if the first attempt fails. The default is 2. The maximum
   * is 5 to avoid hangs.
   */
  void setRetryCount(int retryCount);
  int retryCount() const;

  /**
   * Sets the number of milliseconds between reconnection attempts. 
   * The default is 1000. The maximum is 2000 to avoid hangs.
   */
  void setRetryDelay(int retryDelay);
  int retryDelay() const;  

protected:
  /**
   * Adds the given @a uri to the resources pushed from the server.
   */
  bool addToChannel(const QString& uri);
  /**
   * Removes the given @a uri from the resources pushed from the
   * server.
   */
  bool removeFromChannel(const QString& uri);

  /**
   * Decodes a message received from a return channel. The default
   * implementation recognizes signals emitted by the server and calls
   * the corresponding slot with decoded function arguments. Override
   * this function to handle other pushed resources.
   *
   * @param uri the identifier of the pushed resource
   *
   * @param data encoded data
   *
   * @return @p true if @a data was successfully decoded, @p false
   * otherwise.
   *
   * @see PiiRemoteObjectServer::enqueuePushData()
   */
  virtual bool decodePushData(const QString& uri, const QByteArray& data);

  /// @internal
  void connectNotify(const char* signal);
  /// @internal
  void disconnectNotify(const char* signal);
  /// @internal
  void timerEvent(QTimerEvent*);
  
private:
  struct Function
  {
    Function(const char* sig, int r, const QString& n, const QList<int>& t) :
      aSignature(sig), returnType(r), strName(n), lstParamTypes(t)
    {}
    QByteArray aSignature;
    int returnType;
    QString strName;
    QList<int> lstParamTypes;
  };

  struct Signal : Function
  {
    Signal(const char* sig, int r, const QString& n, const QList<int>& t) :
      Function(sig,r,n,t),
      bConnected(false)
    {}
    bool bConnected;
  };

  struct Property
  {
    Property(int t, const QString& n) : type(t), strName(n) {}
    int type;
    QString strName;
  };
  
  class Data
  {
  public:
    Data();
    
    QByteArray stringData;
    QVector<uint> vecMetaData;

    PiiNetworkClient networkClient;
    PiiHttpDevice* pHttpDevice;
    QString strPath;
    QMetaObject metaObject;
    QList<Function> lstFunctions;
    QList<Signal> lstSignals;
    QList<Property> lstProperties;

    QString strChannelId;
    int iAddToChannelIndex, iRemoveFromChannelIndex, iCloseChannelIndex;
    QThread* pChannelThread;
    QMutex channelMutex;
    QWaitCondition channelUpCondition;
    volatile bool bChannelRunning;
    int iRetryCount;
    int iRetryDelay;
  } *d;

  int metaCall(QMetaObject::Call call, int id, void** arguments);
  void createMetaObject();
  PiiHttpDevice& openConnection();
  QList<QByteArray> readDirectoryList(const QString& path);
  void collectProperties();
  void collectFunctions(bool listSignals);
  inline bool manageChannel(int functionIndex, const QString& uri);
  bool emitSignal(int id, const QByteArray& data);
  bool openChannel();
  void closeChannel();
  void readChannel();
  void stopChannelThread();
  bool checkChannelResponse(PiiHttpDevice& dev);
  bool canContinue(double progressPercentage);
};

#endif //_PIIREMOTEOBJECTCLIENT_H
