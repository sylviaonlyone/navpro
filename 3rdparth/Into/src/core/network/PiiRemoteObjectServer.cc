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

#include "PiiRemoteObjectServer.h"

#include "PiiHttpDevice.h"
#include "PiiHttpException.h"
#include "PiiStreamBuffer.h"

#include <PiiUtil.h>
#include <PiiSerializationUtil.h>
#include <PiiGenericTextInputArchive.h>
#include <PiiGenericTextOutputArchive.h>
#include <PiiMetaTypeUtil.h>

#include <QThread>
#include <QMetaProperty>
#include <QMetaMethod>
#include <QUrl>
#include <QUuid>

#define PII_THROW_HTTP_ERROR(CODE) PII_THROW(PiiHttpException, PiiHttpProtocol::CODE)
#define PII_THROW_HTTP_ERROR_MSG(CODE, MSG) throw PiiHttpException(PiiHttpProtocol::CODE, MSG, QString(__FILE__ ":%1").arg(__LINE__))

static const int iQObjectFunctionCount = QObject::staticMetaObject.methodCount();

PiiRemoteObjectServer::Data::Data(QObject* object) :
  pObject(object),
  iChannelTimeout(10000),
  iInstanceTimeout(10000),
  iMaxInstances(100),
  iInstanceCount(0)
{}

PiiRemoteObjectServer::Function::Function(QObject* o, const QMetaMethod& m) :
  pObject(o),
  method(m),
  strName(m.signature()),
  bIsVoid(m.typeName()[0] == 0)
{
  QList<QByteArray> lstParams(m.parameterTypes());
  for (int i=0; i<lstParams.size(); ++i)
    lstParamTypes << QMetaType::type(lstParams[i]);
  strName = strName.left(strName.indexOf('('));
}

PiiRemoteObjectServer::PiiRemoteObjectServer(QObject* object) :
  d(new Data(object))
{
  listFunctions(const_cast<PiiRemoteObjectServer*>(this));
  listFunctions(object);
  // Garbage collector
  startTimer(1000);
}

PiiRemoteObjectServer::PiiRemoteObjectServer() :
  d(new Data(0))
{
  // Garbage collector
  startTimer(1000);
}

PiiRemoteObjectServer::~PiiRemoteObjectServer()
{
  killChannels();
  deleteServers();
  delete d;
}

void PiiRemoteObjectServer::killChannels()
{
  QMutexLocker lock(&d->channelMutex);
  for (QHash<QString,Channel*>::iterator i = d->hashChannelsById.begin(); i != d->hashChannelsById.end(); ++i)
    i.value()->quit();
  for (QHash<QString,Channel*>::iterator i = d->hashChannelsById.begin(); i != d->hashChannelsById.end(); ++i)
    i.value()->wait();
  qDeleteAll(d->hashChannelsById);
  d->hashChannelsById.clear();
  d->hashChannelsByUri.clear();
}

QStringList PiiRemoteObjectServer::listFolder(const QString& uri) const
{
  if (uri == "/")
    return QStringList() << "functions/" << "signals/" << "properties/" << "enums/" << "channels/" << "ping";
  else if (uri == "/properties/")
    return propertyDeclarations();
  else if (uri == "/functions/")
    return functionSignatures();
  else if (uri == "/signals/")
    return signalSignatures();
  else if (uri == "/channels/")
    return QStringList() << "new" << "reconnect";
  else if (uri == "/enums/")
    {
      QStringList lstEnums(d->lstEnums);
      return lstEnums.replaceInStrings(QRegExp("$"), "/");
    }
  return QStringList();
}

PiiRemoteObjectServer* PiiRemoteObjectServer::createServer(const QVariantMap& /*parameters*/)
{
  return 0;
}

void PiiRemoteObjectServer::deleteServers()
{
  QMutexLocker lock(&d->instanceMutex);
  for (QHash<QString,Instance>::iterator i = d->hashInstances.begin();
       i != d->hashInstances.end(); ++i)
    delete i->pServer;
}

void PiiRemoteObjectServer::deleteServer(const QString& serverId)
{
  QMutexLocker lock(&d->instanceMutex);
  QHash<QString,Instance>::iterator i = d->hashInstances.find(serverId);
  if (i != d->hashInstances.end())
    {
      delete i->pServer;
      d->hashInstances.erase(i);
    }
}

QString PiiRemoteObjectServer::createNewInstance(const QVariantMap& parameters)
{
  QMutexLocker lock(&d->instanceMutex);
  if (d->iInstanceCount >= d->iMaxInstances)
    PII_THROW_HTTP_ERROR_MSG(ServiceUnavailableStatus, tr("Maximum number of remote object instances reached."));
  PiiRemoteObjectServer* pNewServer = createServer(parameters);
  if (pNewServer == 0)
    PII_THROW_HTTP_ERROR_MSG(InternalServerErrorStatus, tr("Unable to create remote object instance."));
  ++d->iInstanceCount;
  // Remove curly braces around the uuid
  QString strId = QUuid::createUuid().toString().mid(1);
  strId.chop(1);
  d->hashInstances.insert(strId, Instance(pNewServer));
  return strId;
}

PiiRemoteObjectServer* PiiRemoteObjectServer::findInstance(const QString& instanceId) const
{
  QHash<QString,Instance>::iterator i = d->hashInstances.find(instanceId);
  if (i == d->hashInstances.end())
    PII_THROW_HTTP_ERROR(NotFoundStatus);
  i->idleTime.restart();
  return i->pServer;
}

void PiiRemoteObjectServer::handleRequest(const QString& uri, PiiHttpDevice* dev,
                                          PiiHttpProtocol::TimeLimiter* controller)
{
  bool bPostRequest = dev->requestMethod() == "POST";

  if (!bPostRequest && dev->requestMethod() != "GET")
    PII_THROW_HTTP_ERROR(MethodNotAllowedStatus);

  dev->startOutputFiltering(new PiiStreamBuffer);

  QString strRequestPath = dev->requestPath(uri);
  piiDebug(dev->requestMethod() + " " + strRequestPath);

  // If there is no object to be served, provide only "/new" and
  // "/delete" functionality and pass other requests to secondary
  // servers.
  if (d->pObject == 0)
    {
      if (strRequestPath.isEmpty())
        dev->print("new\ndelete");
      else if (strRequestPath == "new")
        dev->print(createNewInstance(dev->requestValues()));
      else if (strRequestPath == "delete")
        deleteServer(dev->queryString());
      // Find the server that handles request to the given object
      // instance.
      else
        {
          int iSlashIndex = strRequestPath.indexOf('/');
          if (iSlashIndex == -1)
            PII_THROW_HTTP_ERROR(NotFoundStatus);
          QString strServerId = strRequestPath.left(iSlashIndex);
          synchronized (d->instanceMutex)
            {
              PiiRemoteObjectServer* pServer = findInstance(strServerId);
              // Pass the request to the selected server
              pServer->handleRequest(uri + strServerId + "/", dev, controller);
            }
        }
      return;
    }

  if (strRequestPath.isEmpty())
    {
      dev->print(listFolder("/").join("\n"));
      return;
    }

  int iSlashIndex = strRequestPath.indexOf('/');
  if (iSlashIndex == -1)
    {
      if (strRequestPath == "ping" || strRequestPath == "new")
        // Do nothing. "new" allows the client to always request a new
        // object even if the server provides just a single instance.
        return;
      PII_THROW_HTTP_ERROR(NotFoundStatus);
    }

  QString strSubDir = strRequestPath.left(iSlashIndex);

  // Property list/get/set
  if (strSubDir == "properties")
    {
      QString strPropName = strRequestPath.mid(iSlashIndex+1);
      if (strPropName.isEmpty())
        {
          // Set many properties at once
          if (dev->hasQuery() || bPostRequest)
            Pii::setProperties(*d->pObject, dev->requestValues());
          // List all properties
          else
            dev->print(listFolder("/properties/").join("\n"));
        }
      // Get property
      else if (!dev->hasQuery() && !bPostRequest)
        dev->write(dev->encode(d->pObject->property(qPrintable(strPropName))));
      // Set property
      else
        {
          if (bPostRequest)
            d->pObject->setProperty(qPrintable(strPropName),
                                    dev->decodeVariant(dev->readBody()));
          else
            d->pObject->setProperty(qPrintable(strPropName),
                                    dev->decodeVariant(QUrl::fromPercentEncoding(dev->queryString().toUtf8())));
        }
    }
  // Function calls
  else if (strSubDir == "functions")
    {
      QString strFunction = strRequestPath.mid(iSlashIndex+1);
      // Empty function name -> list all functions
      if (strFunction.isEmpty())
        dev->print(listFolder("/functions/").join("\n"));
      else
        {
          QVariant varReturn;
          QVariantList lstParams;
          // GET request -> parameters in URL
          if (!bPostRequest)
            lstParams = paramList(dev->formItems(), dev->formValues());
          // POST form -> parameters in body
          else if (dev->requestHeader().contentType() == PiiNetwork::formContentType())
            lstParams = paramList(dev->queryItems(), dev->queryValues());
          // Other POST -> decode a QVariantList
          else
            {
              try { PiiSerialization::fromByteArray<PiiGenericTextInputArchive>(dev->readBody(), lstParams); }
              catch (PiiSerializationException& ex)
                {
                  PII_THROW_HTTP_ERROR_MSG(BadRequestStatus, ex.message() + "(" + ex.info() + ")");
                }
            }
          varReturn = callFunction(strFunction, lstParams);
          
          if (varReturn.isValid())
            dev->write(dev->encode(varReturn));
        }
    }
  else if (strSubDir == "signals")
    {
      QString strFunction = strRequestPath.mid(iSlashIndex+1);
      // Empty function name -> list all signals
      if (strFunction.isEmpty())
        dev->print(listFolder("/signals/").join("\n"));
      else
        // Signals cannot be connected here
        PII_THROW_HTTP_ERROR(NotFoundStatus);
    }
  else if (strSubDir == "channels")
    {
      QString strFunction = strRequestPath.mid(iSlashIndex+1);
      if (strFunction.isEmpty())
        dev->print(listFolder("/channels/").join("\n"));
      else
        {
          // new and reconnect behave differently from other
          // "functions". They hang the calling thread.
          if (strFunction == "new")
            {
              QString strId = createChannel();
              dev->print(strId);
              dev->putChar('\n');
              push(strId, dev, controller);
            }
          else if (strFunction == "reconnect")
            {
              QString strId = dev->decodeVariant(dev->queryString()).toString();
              push(strId, dev, controller);
            }
          else
            PII_THROW_HTTP_ERROR(NotFoundStatus);
        }
    }
  else if (strSubDir == "enums")
    {
      QString strEnum = strRequestPath.mid(iSlashIndex+1);
      if (strEnum.isEmpty())
        dev->print(listFolder("/enums/").join("\n"));
      else
        {
          if (!d->hashEnums.contains(strEnum))
            PII_THROW_HTTP_ERROR(NotFoundStatus);
          QStringList lstEnums = d->hashEnums[strEnum];
          for (int i=0; i<lstEnums.size(); ++i)
            {
              dev->print(lstEnums[i]);
              dev->putChar(' ');
              dev->print(QString::number(d->hashEnumValues[lstEnums[i]]));
              if (i < lstEnums.size()-1)
                dev->putChar('\n');
            }
        }
    }
  else
    PII_THROW_HTTP_ERROR(NotFoundStatus);
}

void PiiRemoteObjectServer::push(const QString& id, PiiHttpDevice* dev, PiiHttpProtocol::TimeLimiter* controller)
{
  Channel* pChannel = channelById(id);
  if (pChannel != 0)
    pChannel->push(dev, controller);
  else
    PII_THROW_HTTP_ERROR(GoneStatus);
}

PiiRemoteObjectServer::Channel* PiiRemoteObjectServer::channelById(const QString& id)
{
  QMutexLocker lock(&d->channelMutex);
  return d->hashChannelsById[id];
}

QString PiiRemoteObjectServer::findSignal(const QString& channelId, const QString& uri)
{
  if (!d->hashChannelsById.contains(channelId))
    PII_THROW_HTTP_ERROR_MSG(BadRequestStatus, tr("There is no such channel."));
  
  int iSlashIndex = uri.indexOf('/');
  if (uri.left(iSlashIndex) == "signals")
    return uri.mid(iSlashIndex + 1);

  PII_THROW_HTTP_ERROR_MSG(BadRequestStatus, tr("%1 is not a pushable resource.").arg(uri));
}

bool PiiRemoteObjectServer::removeFromChannel(const QString& channelId, const QString& uri)
{
  synchronized (d->channelMutex)
    {
      QString strSignal = findSignal(channelId, uri);
      Channel* pChannel = d->hashChannelsById[channelId];
      d->hashChannelsByUri.remove(uri, pChannel);
      pChannel->removeAll(uri);
      return pChannel->dynamicDisconnect(d->pObject, qPrintable(strSignal));
    }
  return false; // suppresses a compiler warning
}

bool PiiRemoteObjectServer::addToChannel(const QString& channelId, const QString& uri)
{
  synchronized (d->channelMutex)
    {
      // No multiple registrations
      if (d->hashChannelsByUri.contains(uri))
        return true;
      
      QString strSignal = findSignal(channelId, uri);
      Channel* pChannel = d->hashChannelsById[channelId];
      if (strSignal.isEmpty() || pChannel->dynamicConnect(d->pObject, qPrintable(strSignal), 0, Qt::DirectConnection) == -1)
        PII_THROW_HTTP_ERROR_MSG(BadRequestStatus, tr("There is no signal called \"%1\"").arg(strSignal));
      d->hashChannelsByUri.insert(uri, pChannel);
    }
  return true;
}

void PiiRemoteObjectServer::closeChannel(const QString& channelId)
{
  synchronized (d->channelMutex)
    {
      // If there is no such channel ... well, it is closed.
      if (!d->hashChannelsById.contains(channelId))
        return;
      // Quit the thread and remove from maps. The channel will be
      // deleted by timerEvent().
      Channel* pChannel = d->hashChannelsById[channelId];
      pChannel->quit();
      disconnectChannel(pChannel);
      d->hashChannelsById.remove(channelId);
    }
}

QVariantList PiiRemoteObjectServer::paramList(const QStringList& names, const QVariantMap& map)
{
  QVariantList lstResult;
  QStringList lstUniqueNames(names);
  lstUniqueNames.removeDuplicates();
  //qDebug() << names << map;
  for (int i=0; i<lstUniqueNames.size(); ++i)
    {
      QVariant varValue = map[lstUniqueNames[i]];
      if (varValue.type() != QVariant::List)
        lstResult << varValue;
      else
        lstResult << varValue.toList();
    }
  return lstResult;
}

void PiiRemoteObjectServer::listFunctions(QObject* object)
{
  const QMetaObject* pMetaObject = (object == this ? &staticMetaObject : object->metaObject());
  for (int i = iQObjectFunctionCount; i < pMetaObject->methodCount(); ++i)
    {
      QMetaMethod method = pMetaObject->method(i);
      if (method.methodType() == QMetaMethod::Signal)
        d->lstSignals << Function(object, method);
      else if (method.methodType() member_of (QMetaMethod::Method, QMetaMethod::Slot) &&
               method.access() == QMetaMethod::Public)
        d->lstFunctions << Function(object, method);
    }
}

QStringList PiiRemoteObjectServer::functionSignatures() const
{
  return functionSignatures(d->lstFunctions);
}

QStringList PiiRemoteObjectServer::signalSignatures() const
{
  return functionSignatures(d->lstSignals);
}

QStringList PiiRemoteObjectServer::functionSignatures(const QList<Function>& functions) const
{
  QStringList lstResult;
  for (int i=0; i<functions.size(); ++i)
    {
      const char* pReturnType = functions[i].method.typeName();
      if (*pReturnType != 0)
        lstResult << QString::fromLatin1(pReturnType) + " " +
          QString::fromLatin1(functions[i].method.signature());
      else
        lstResult << QString::fromLatin1(functions[i].method.signature());
    }

  return lstResult;
}

void PiiRemoteObjectServer::addToEnums(const QString& name, const QMetaEnum& enumerator)
{
  if (d->hashEnums.contains(name))
    return;
  d->lstEnums << name;
  QStringList lstKeys;
  for (int i=0; i<enumerator.keyCount(); ++i)
    {
      lstKeys << enumerator.key(i);
      d->hashEnumValues.insert(name, enumerator.value(i));
    }
}

QStringList PiiRemoteObjectServer::propertyDeclarations() const
{
  const QMetaObject* pMetaObject = d->pObject->metaObject();
  QStringList lstResult;
  
  // We won't show objectName to the outer world
  for (int i=1; i<pMetaObject->propertyCount(); ++i)
    {
      QMetaProperty prop = pMetaObject->property(i);
      if (prop.isEnumType())
        lstResult << QString("int %1").arg(prop.name());
        /*  {
          strPrefix = prop.isFlagType() ? "flag " : "enum ";
          addToEnums(prop.typeName(), prop.enumerator());
        }
      */
      else
        lstResult << QString("%1 %2").arg(prop.typeName()).arg(prop.name());
    }

  QList<QByteArray> lstDynamicPropertyNames = d->pObject->dynamicPropertyNames();
  for (int i=0; i<lstDynamicPropertyNames.size(); ++i)
    lstResult << lstDynamicPropertyNames[i];

  return lstResult;
}

int PiiRemoteObjectServer::resolveFunction(const QList<Function>& functions,
                                           const QString& name,
                                           const QVariantList& params)
{
  int iBestMatchIndex = -1;
  int iBestMatchCount = -1;
  bool bFunctionNameFound = false;
  for (int i=0; i<functions.size(); ++i)
    {
      // Function name must match
      if (functions[i].strName != name)
        continue;

      bFunctionNameFound = true;
      int iMatchCount = Pii::scoreOverload(params, functions[i].lstParamTypes);

      // The best overload is the one with the most exact matches on
      // parameter types.
      if (iMatchCount > iBestMatchCount)
        {
          iBestMatchIndex = i;
          iBestMatchCount = iMatchCount;
        }
    }
  if (!bFunctionNameFound)
    PII_THROW_HTTP_ERROR(NotFoundStatus);
  else if (iBestMatchIndex == -1)
    PII_THROW_HTTP_ERROR_MSG(BadRequestStatus, tr("Function parameter types don't match."));
  return iBestMatchIndex;
}

QVariant PiiRemoteObjectServer::callFunction(const QString& function, QVariantList& params)
{
  int iFunctionIndex = resolveFunction(d->lstFunctions, function, params);
  
  QGenericArgument aArguments[10];
  const QList<int>& lstParamTypes = d->lstFunctions[iFunctionIndex].lstParamTypes;

  // Convert parameter list to generic arguments.
  for (int j=0; j<params.size(); ++j)
    {
      if (lstParamTypes[j] < int(QVariant::UserType))
        params[j].convert(QVariant::Type(lstParamTypes[j]));
      aArguments[j] = QGenericArgument(QMetaType::typeName(lstParamTypes[j]), params[j].constData());
    }

  // If the function is void, no return argument can be formed
  if (d->lstFunctions[iFunctionIndex].bIsVoid)
    {
      if (!d->lstFunctions[iFunctionIndex].method.invoke(d->lstFunctions[iFunctionIndex].pObject,
                                                         Qt::DirectConnection,
                                                         aArguments[0], aArguments[1], aArguments[2],
                                                         aArguments[3], aArguments[4], aArguments[5],
                                                         aArguments[6], aArguments[7], aArguments[8],
                                                         aArguments[9]))
        PII_THROW_HTTP_ERROR(InternalServerErrorStatus);
      return QVariant();
    }
  else
    {
      const char* pReturnType = d->lstFunctions[iFunctionIndex].method.typeName();
      QVariant varReturn(QMetaType::type(pReturnType), (void*)0);
      if (!d->lstFunctions[iFunctionIndex].method.invoke(d->lstFunctions[iFunctionIndex].pObject,
                                                         Qt::DirectConnection,
                                                         QGenericReturnArgument(pReturnType, varReturn.data()),
                                                         aArguments[0], aArguments[1], aArguments[2],
                                                         aArguments[3], aArguments[4], aArguments[5],
                                                         aArguments[6], aArguments[7], aArguments[8],
                                                         aArguments[9]))
        PII_THROW_HTTP_ERROR(InternalServerErrorStatus);
      return varReturn;
    }
}

void PiiRemoteObjectServer::disconnectChannel(Channel* channel)
{
  // Remove all URI mappings to channel
  QMultiHash<QString,Channel*>::iterator i = d->hashChannelsByUri.begin();
  while (i != d->hashChannelsByUri.end())
    {
      if (i.value() == channel)
        i = d->hashChannelsByUri.erase(i);
      else
        ++i;
    }
}

void PiiRemoteObjectServer::timerEvent(QTimerEvent*)
{
  synchronized (d->channelMutex)
    {
      // Kill all inactive channels
      QHash<QString,Channel*>::iterator i = d->hashChannelsById.begin();
      while (i != d->hashChannelsById.end())
        {
          if (!i.value()->isAlive(d->iChannelTimeout))
            {
              disconnectChannel(i.value());
              delete i.value();
              i = d->hashChannelsById.erase(i);
            }
          else
            ++i;
        }
    }
  synchronized (d->instanceMutex)
    {
      // Kill all inactive instances
      QHash<QString,Instance>::iterator i = d->hashInstances.begin();
      while (i != d->hashInstances.end())
        {
          if (!i.value().idleTime.elapsed() > d->iInstanceTimeout)
            {
              delete i.value().pServer;
              --d->iInstanceCount;
              i = d->hashInstances.erase(i);
            }
          else
            ++i;
        }
    }
}

QString PiiRemoteObjectServer::createChannel()
{
  QMutexLocker lock(&d->channelMutex);
  // Remove curly braces around the uuid
  QString strId = QUuid::createUuid().toString().mid(1);
  strId.chop(1);
  d->hashChannelsById.insert(strId, new Channel);
  return strId;
}

void PiiRemoteObjectServer::enqueuePushData(const QString& uri, const QByteArray& data)
{
  synchronized (d->channelMutex)
    {
      // Send the data to all channels the given uri is connected to.
      for (QMultiHash<QString,Channel*>::iterator i = d->hashChannelsById.find(uri);
           i != d->hashChannelsById.end();
           ++i)
        if (i.key() == uri)
          i.value()->enqueuePushData(uri, data);
    }
}


PiiRemoteObjectServer::Channel::Channel() :
  _bPushing(false),
  _bKilled(false),
  _pThread(0)
{
  _idleTimer.stop();
}

bool PiiRemoteObjectServer::Channel::invokeSlot(int id, void** args)
{
  try
    {
      QByteArray aBody;
      if (argumentCount(id) > 0)
        aBody = PiiSerialization::toByteArray<PiiGenericTextOutputArchive>(argsToList(id, args));
      //piiDebug("Sending %d bytes to %s.", aBody.size(), signatureOf(id).constData());
      enqueuePushData("signals/" + signatureOf(id), aBody);
    }
  catch (PiiSerializationException& ex)
    {
      piiWarning(tr("Cannot invoke slot %1: %2").arg(id).arg(ex.message()));
      return false;
    }
  return true;
}

void PiiRemoteObjectServer::Channel::enqueuePushData(const QString& uri, const QByteArray& data)
{
  QMutexLocker lock(&_queueMutex);
  if (_dataQueue.size() > 10)
    {
      //piiWarning("Maximum size of channel buffer reached.");
      return;
    }
  _dataQueue.enqueue(qMakePair(uri, data));
  //if (_dataQueue.size() > 1)
  //  piiDebug("%d objects in queue", _dataQueue.size());
  _queueCondition.wakeOne();
}

void PiiRemoteObjectServer::Channel::removeAll(const QString& uri)
{
  QMutexLocker lock(&_queueMutex);
  for (int i=_dataQueue.size(); i--; )
    if (_dataQueue[i].first == uri)
      _dataQueue.removeAt(i);
}

bool PiiRemoteObjectServer::Channel::isAlive(int timeout) const
{
  QMutexLocker lock(const_cast<QMutex*>(&_queueMutex));
  return _bPushing || _idleTimer.milliseconds() < timeout;
}

void PiiRemoteObjectServer::Channel::quit()
{
  QMutexLocker lock(&_queueMutex);
  _bKilled = true;
  _queueCondition.wakeOne();
}

void PiiRemoteObjectServer::Channel::wait()
{
  _queueMutex.lock();
  bool bPushing = _bPushing;
  _queueMutex.unlock();
  if (bPushing)
    _pThread->wait();
}

void PiiRemoteObjectServer::Channel::push(PiiHttpDevice* dev, PiiHttpProtocol::TimeLimiter* controller)
{
  static const char* pBoundary = "--243F6A8885A308D3";

  // Disallow reuse of the same channel. This may happen if someone
  // somehow steals the channel ID.
  synchronized (_queueMutex)
    {
      if (_bPushing)
        PII_THROW_HTTP_ERROR(ReservedStatus);
      _bPushing = true;
      _idleTimer.stop();
      // Store the calling thread.
      _pThread = QThread::currentThread();
    }
  
  controller->setMaxTime(-1);
  dev->setHeader("Content-Type", QString("multipart/mixed-replace; boundary=\"%1\"").arg(pBoundary+2));
  dev->write(pBoundary);
  dev->write("\r\n");
  // Flush all buffered data to socket and pass all remaining data
  // unfiltered.
  dev->outputFilter()->flushFilter();
  dev->endOutputFiltering();
  // Flush the socket too.
  dev->flushFilter();
  
  _queueMutex.lock();

  forever
    {
      while (!_bKilled && !_dataQueue.isEmpty() && dev->isWritable() && controller->canContinue())
        {
          QPair<QString,QByteArray> pair(_dataQueue.dequeue());

          // Writing to the device may take time. Let new data appear
          // meanwhile.
          _queueMutex.unlock();
          //piiDebug("PiiRemoteObjectServer::push() writing");
          dev->print(QString("X-URI: %1\r\nContent-Length: %2\r\n\r\n")
                     .arg(pair.first).arg(pair.second.size()));
          qint64 iBytesWritten = dev->write(pair.second);
          // Couldn't write all data -> warn and put the data back to
          // the queue.
          if (iBytesWritten != pair.second.size())
            {
              piiWarning("Failed to push data to channel. Only %d bytes written out of %d.",
                         int(iBytesWritten), pair.second.size());
              synchronized (_queueMutex) _dataQueue.prepend(pair);
              continue;
            }
          dev->write("\r\n");
          dev->write(pBoundary);
          dev->write("\r\n");
          dev->flushFilter();
          //piiDebug("Wrote %d of %d bytes to %s", (int)iBytesWritten, pair.second.size(), piiPrintable(pair.first));
          _queueMutex.lock();
        }
      if (_bKilled || !dev->isWritable() || !controller->canContinue())
        break;
      
      _queueCondition.wait(&_queueMutex, 50);
    }
  
  _bKilled = false;
  _bPushing = false;
  _pThread = 0;
  _idleTimer.restart();
  
  _queueMutex.unlock();
}

void PiiRemoteObjectServer::setChannelTimeout(int channelTimeout) { d->iChannelTimeout = channelTimeout; }
int PiiRemoteObjectServer::channelTimeout() const { return d->iChannelTimeout; }
void PiiRemoteObjectServer::setInstanceTimeout(int instanceTimeout) { d->iInstanceTimeout = instanceTimeout; }
int PiiRemoteObjectServer::instanceTimeout() const { return d->iInstanceTimeout; }

void PiiRemoteObjectServer::setMaxInstances(int maxInstances) { d->iMaxInstances = maxInstances; }
int PiiRemoteObjectServer::maxInstances() const { return d->iMaxInstances; }
