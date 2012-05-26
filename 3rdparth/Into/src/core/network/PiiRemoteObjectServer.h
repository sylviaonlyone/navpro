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

#ifndef _PIIREMOTEOBJECTSERVER_H
#define _PIIREMOTEOBJECTSERVER_H

#include "PiiHttpProtocol.h"
#include <PiiUniversalSlot.h>
#include <PiiTimer.h>

#include <QMetaMethod>
#include <QVariant>
#include <QWaitCondition>
#include <QQueue>
#include <QStringList>
#include <QHash>

/**
 * An object that maps QObject's functions and properties into the URI
 * space of a PiiHttpProtocol.
 *
 * @par Usage example
 *
 * If you just need to start quick, do this:
 *
 * @code
 * QTimer t; // ... or any other class derived from QObject
 * PiiHttpServer* pHttpServer = PiiHttpServer::addServer("My server", "tcp://0.0.0.0:3142");
 * PiiRemoteObjectServer* pRemoteObjectServer = new PiiRemoteObjectServer(&t);
 * pHttpServer->protocol()->registerUriHandler("/timer/", pRemoteObjectServer);
 * pHttpServer->start();
 * @endcode
 *
 * See PiiRemoteObjectClient for the client side. If you want the
 * details, read on.
 *
 * @par Mapping to URIs
 *
 * %PiiRemoteObjectServer makes QObject's properties, signals, slots,
 * and other invokable functions available to client software by
 * mapping the object to an URI in PiiHttpServer. By default, all
 * properties (including dynamic ones) and public invokable methods
 * will be made available as sub-URIs. For example, a QTimer
 * registered at /timer/ would create the following URI structure:
 *
 * @li /timer/
 * @li /timer/functions/
 * @li /timer/functions/start
 * @li /timer/functions/stop
 * @li /timer/signals/
 * @li /timer/signals/timeout
 * @li /timer/properties/
 * @li /timer/properties/active
 * @li /timer/properties/interval
 * @li /timer/properties/singleShot
 *
 * Note that the QTimer::start() function has two overloaded versions,
 * which both can be accessed through the same URI. The server
 * inspects the parameter types to find out which function to call.
 *
 * PiiHttpServer is configured so that it returns a list of sub-URIs
 * if no handler is defined for a specific URI. That is, a request to
 * /timer/ would list the root "folders":
 *
@verbatim
GET /timer/ HTTP/1.1
@endverbatim
 *
 * Response:
 *
@verbatim
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 31

functions/
signals/
properties/
@endverbatim
 *
 * @par Function calls
 *
 * A GET request to the /functions/ URI lists all callable functions:
 *
@verbatim
GET /timer/functions/ HTTP/1.1
@endverbatim
 *
 * Response:
 *
@verbatim
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 40

start()
start(int)
stop()
@endverbatim
 *
 * A call to a function with no parameters is a simple GET request:
 *
@verbatim
GET /timer/functions/start HTTP/1.1
@endverbatim
 *
 * Note that the Host header is not necessary, although HTTP 1.1
 * dictates one. The response is equally simple (no return value):
 *
@verbatim
HTTP/1.1 200 OK
Content-Length: 0
@endverbatim
 *
 * If the function has parameters, they can be specified in the URI:
 *
@value
GET /timer/functions/start?msec=1000 HTTP/1.1
@endverbatim
 *
 * The server requires that parameters are given in the order they are
 * declared in the function signature. It ignores the parameter name,
 * which can therefore be omitted. The example above is equivalent to:
 *
@verbatim
GET /timer/functions/start?1000 HTTP/1.1
@endverbatim
 *
 * Parameters are automatically decoded using @ref PiiHttpDevice::decodeVariant().
 * If the function has a return value, the same encoding will be used
 * in the return message body.
 *
 * @par Properties
 *
 * The properties of a QObject are provided under the "/properties/"
 * URI. Each property has a URI that corresponds to its name. By
 * default, both static and dynamic properties will be listed. 
 * Property definitions can be queried with a GET request to
 * /properties/:
 *
@verbatim
GET /timer/properties/ HTTP/1.1
@endverbatim
 *
 * Response:
 * 
@verbatim
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 41

bool active
int interval
bool singleShot
@endverbatim
 *
 * Individual property values can be retrieved with a GET request to
 * the property URI:
 *
@verbatim
GET /timer/properties/active HTTP/1.1
@endverbatim
 *
 * Response:
 * 
@verbatim
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 4

true
@endverbatim
 *
 * Setting property values may or may not be allowed by the server. 
 * The value of an individual property can be set either with a GET or
 * a POST request:
 *
@verbatim
GET /timer/properties/interval?12345 HTTP/1.1

POST /timer/properties/interval HTTP/1.1
Content-Type: application/x-www-form-urlencoded
Content-Length: 15

interval=12345
@endverbatim
 *
 * Many properties can be set at once with either a GET request or a
 * POST (x-www-form-urlencoded) request.
 *
@verbatim
GET /timer/properties?interval=1000&singleShot=false HTTP/1.1

POST /timer/properties HTTP/1.1
Content-Type: application/x-www-form-urlencoded
Content-Length: 30

interval=1000&singleShot=false
@endverbatim
 *
 * @par Channels and signals
 *
 * Connecting to a signal requires a persistent return channel the
 * server can use to push data to the client. A return channel must be
 * set up before a signal can be connected to. Then, the return
 * channel can be used to deliver any number of signals and also other
 * data.
 *
 * To continue with the timer example, the URI structure in fact
 * contains the following additional entries:
 *
 * @li /timer/functions/register
 * @li /timer/functions/unregister
 * @li /timer/functions/close
 * @li /timer/channels/
 * @li /timer/channels/new
 * @li /timer/channels/reconnect
 *
 * Channels can be connected to under the /channels/ URI. The three
 * functions are used to control what data is sent through the
 * channel:
 *
 * @li register(QString id, QString uri) - registers a resource to be
 * pushed to a channel. The first parameter is a channel ID and the
 * second one the URI of a resource (such as a signal). This is
 * equivalent to connecting a slot to a signal.
 *
 * @li unregister(QString id, QString uri) - the opposite: the given
 * resource will no longer be pushed through the indicated channel. 
 * This is equivalent to disconnecting a slot from a signal.
 *
 * @li close(QString) - closes the channel associated with the given
 * channel ID and releases all resources allocated to it.
 *
 * Now, the apparent question is how to get the channel id. The ID of
 * a newly created channel is the first row of the return body (the
 * preamble of a multipart message) when the client requests
 * /channels/new. The server leaves the requesting socket open, and
 * pushes new data to it when needed.
 *
@verbatim
GET /timer/channels/new HTTP/1.1
@endverbatim
 *
 * Response:
 *
@verbatim
HTTP/1.1 200 OK
Content-Type: multipart/mixed-replace; boundary="ural"

4A40938-2229-9F31-D008-2EFA98EC4E6C
--ural
X-URI: signals/timeout
Content-Length: 0

--ural
...
@endverbatim
 *
 * Now that the channel has been set up, one can add pushable
 * resources to the channel. To request the server to push the data
 * emitted through the timeout signal, the following request must be
 * made:
 *
@verbatim
GET /functions/register?4A40938-2229-9F31-D008-2EFA98EC4E6C&/signals/timeout() HTTP/1.1
@endverbatim
 *
 * The first parameter to the @p register function is the channel ID. 
 * The second parameter identifies the pushable resource as a URI. In
 * the case of a signal, the URI must specify the exact signature of
 * the signal to avoid ambiguous overloads. Note that the URI is not
 * percent-encoded in the example for readability although it should
 * really be.
 *
 * Whenever new registered data becomes available it will be written
 * to the channel. The URI of the resource will be sent as in a
 * non-standard "X-URI" MIME header. The actual data is written in a
 * serialized format (see @ref Serialization). If the pushed resource
 * is a signal, its parameters will be encoded as a QVariantList. The
 * client and the server must agree on the data format with other
 * pushable resources that may be provided by subclasses.
 *
 * The reconnect function reconnects the client to a disconnected
 * channel. It takes the channel ID as a parameter. The server keeps
 * unclosed channels in memory for a while allowing clients to recover
 * from network failures. If the channel is still alive, this call
 * will re-establish it as if /channels/new was requested. The only
 * differences are that all previously added pushable resources are
 * still in effect and that the channel ID will not be returned.
 *
 * @par Multiple Instances
 *
 * By default, there is a single instance of a remote object. All
 * clients will be connected to the same object, and changes made by
 * one will affect others. It is however possible to create a new
 * instance for each client. In this mode, the remote object server is
 * not given an instance of a QObject. The server initially works as a
 * primary server that either creates new server instances or passes
 * requests to existing ones. There will be no "default" server
 * object, but each client must create a instance by requesting /new. 
 * The server uses #createServer() function to create a new secondary
 * server and assigns a unique ID to it. The ID is then returned to
 * the client.
 *
@verbatim
GET /new HTTP/1.1
@endverbatim
 *
 * Response:
 *
@verbatim
HTTP/1.1 200 OK
Content-Length: 35

243F6A8-885A-308D-3131-98A2E0370734
@endverbatim
 *
 * The returned ID can now be used as the URI of the new instance:
 *
@verbatim
GET /243F6A8-885A-308D-3131-98A2E0370734/properties/ HTTP/1.1
@endverbatim
 *
 * Inactive object instances will be kept alive only a limited amount
 * of time. If no accesses have been made to the object for a
 * specified maximum number of milliseconds (see #instanceTimeout()),
 * the object will be automatically deleted. If no other requests are
 * made, the instance can be kept alive by requesting /ping. This is a
 * dummy URI whose sole purpose is to indicate the server that the
 * client is still alive. It can also be used to check the
 * availability of a remote object in a non-intrusive manner.
 *
@verbatim
GET /243F6A8-885A-308D-3131-98A2E0370734/ping HTTP/1.1
@endverbatim
 *
 * Response:
 *
@verbatim
HTTP/1.1 200 OK
Content-Length: 0
@endverbatim
 *
 * The remote object instance can be explicitly destroyed by
 * requesting /delete:
 *
@verbatim
GET /delete?243F6A8-885A-308D-3131-98A2E0370734 HTTP/1.1
@endverbatim
 *
 * It is possible to pass parameters to the new object instance by
 * encoding them into the request (GET or POST). This makes it
 * possible to even create totally different remote objects based on
 * the parameters. Suppose a server that can create QObjects based on
 * class name. Then, the following request would create an instance of
 * a QTimer.
 *
@verbatim
GET /new?className=QTimer HTTP/1.1
@endverbatim
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiRemoteObjectServer :
  public QObject,
  public PiiHttpProtocol::UriHandler
{
  Q_OBJECT
public:
  /**
   * Creates a new %PiiRemoteObjectServer that maps HTTP request to
   * the given @a object. There will be only one instance of the
   * remote object, and all client requests will use it.
   */
  PiiRemoteObjectServer(QObject* object);

  ~PiiRemoteObjectServer();

  void handleRequest(const QString& uri, PiiHttpDevice* dev, PiiHttpProtocol::TimeLimiter* controller);

  /**
   * Returns a list of accessible property names with their types. 
   * Each entry in this list consists of a type and a property name,
   * e.g. "int value".
   */
  QStringList propertyDeclarations() const;
  /**
   * Returns the signatures of all invokable functions and slots. Each
   * signature contains a return type (if there is one), function
   * name, and a list of parameter types. For example "start(int)" is
   * a valid signature.
   */
  QStringList functionSignatures() const;

  /**
   * Returns the signatures of all signals.
   */
  QStringList signalSignatures() const;

  Q_INVOKABLE bool addToChannel(const QString& channelId, const QString& uri);
  Q_INVOKABLE bool removeFromChannel(const QString& channelId, const QString& uri);
  Q_INVOKABLE void closeChannel(const QString& channelId);

  /**
   * Sets the number of millisecond a channel will be kept alive after
   * a client breaks connection to it without explicitly closing the
   * channel. The default is 10000.
   */
  void setChannelTimeout(int channelTimeout);
  /**
   * Returns the current channel time-out.
   */
  int channelTimeout() const;

  /**
   * Sets the number of millisecond an object instance will be kept
   * alive after a client breaks connection to it without explicitly
   * deleting the instance. The default is 10000.
   */
  void setInstanceTimeout(int instanceTimeout);
  /**
   * Returns the instance time-out.
   */
  int instanceTimeout() const;

  /**
   * Sets the maximum number of remote object instances the server
   * will manage concurrently. Once this limit is reached, the server
   * will refuse to create new object instances. The default value is
   * 100.
   */
  void setMaxInstances(int maxInstances);
  /**
   * Returns the maximum number of remote object instances.
   */
  int maxInstances() const;
  
protected:
  /**
   * Creates a new %PiiRemoteObjectServer with no associated server
   * object. The server will work as a primary server that creates a
   * new secondary server for each client using the #createServer()
   * function.
   */
  PiiRemoteObjectServer();
  
  /**
   * Puts @a data to the send queue of all channels to which the
   * pushable resource identified by @a uri has been added to.
   */
  void enqueuePushData(const QString& uri, const QByteArray& data);

  /**
   * Returns a "directory list" of the given folder. This function
   * makes it possible for subclasses to add functionality to the
   * default URI tree. If you override this function, add your new
   * entries to the list returned by the default implementation. It is
   * also possible to remove entries from the list.
   *
   * @param uri the URI whose sub-URIs are to be listed, for example
   * "/" or "/functions/".
   *
   * @return a list of sub-URIs. URIs that may contain sub-URIs should
   * end with a slash (/).
   */
  virtual QStringList listFolder(const QString& uri) const;

  /**
   * Returns a new server instance given the @a parameters passed in
   * the HTTP request. If the server is created without an associated
   * server object, this function must be overridden to create a new
   * server object for each client. The default implementation returns
   * 0.
   */
  virtual PiiRemoteObjectServer* createServer(const QVariantMap& parameters);

  /// @internal
  void timerEvent(QTimerEvent*);
  
private:
  struct Function
  {
    inline Function(QObject* o, const QMetaMethod& m);

    QObject* pObject;
    QMetaMethod method;
    QString strName;
    QList<int> lstParamTypes;
    bool bIsVoid;
  };

  class Channel : public PiiUniversalSlot
  {
  public:
    Channel();
    
    void push(PiiHttpDevice* dev, PiiHttpProtocol::TimeLimiter* controller);
    void enqueuePushData(const QString& uri, const QByteArray& data);
    void removeAll(const QString& uri);
    bool isAlive(int timeout) const;
    void quit();
    void wait();

  protected:
    bool invokeSlot(int id, void** args);
    
  private:
    bool _bPushing, _bKilled;
    QThread* _pThread;
    QMutex _queueMutex;
    QWaitCondition _queueCondition;
    QQueue<QPair<QString,QByteArray> > _dataQueue;
    PiiTimer _idleTimer;
  };

  struct Instance
  {
    Instance(PiiRemoteObjectServer* server) : pServer(server)
    {
      idleTime.start();
    }
    
    QTime idleTime;
    PiiRemoteObjectServer* pServer;
  };

  class Data
  {
  public:
    Data(QObject* object);
    QObject *pObject;
    QList<Function> lstFunctions, lstSignals;
    QMutex channelMutex, instanceMutex;
    QHash<QString,Channel*> hashChannelsById;
    QMultiHash<QString,Channel*> hashChannelsByUri;
    int iChannelTimeout, iInstanceTimeout;
    int iMaxInstances, iInstanceCount;
    QHash<QString,Instance> hashInstances;
    QStringList lstEnums;
    QHash<QString,QStringList> hashEnums;
    QHash<QString,int> hashEnumValues;
  } *d;

  void listFunctions(QObject* object);
  void addToEnums(const QString& name, const QMetaEnum& enumerator);
  int resolveFunction(const QList<Function>& functions,
                      const QString& name,
                      const QVariantList& params);
  QVariantList paramList(const QStringList& names, const QVariantMap& map);
  QVariant callFunction(const QString& function, QVariantList& params);
  QStringList functionSignatures(const QList<Function>& functions) const;
  QString createChannel();
  void push(const QString& id, PiiHttpDevice* dev, PiiHttpProtocol::TimeLimiter* controller);
  inline Channel* channelById(const QString& id);
  void killChannels();
  void disconnectChannel(Channel* channel);
  QString findSignal(const QString& channelId, const QString& uri);
  QString createNewInstance(const QVariantMap& parameters);
  PiiRemoteObjectServer* findInstance(const QString& instanceId) const;
  void deleteServer(const QString& serverId);
  void deleteServers();

  PII_DISABLE_COPY(PiiRemoteObjectServer);
};

#endif //_PIIREMOTEOBJECTSERVER_H
