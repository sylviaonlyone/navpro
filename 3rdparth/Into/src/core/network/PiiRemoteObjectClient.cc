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

#include "PiiRemoteObjectClient.h"

#include "PiiNetworkException.h"
#include "PiiHttpProtocol.h"
#include "PiiStreamBuffer.h"
#include "PiiMultipartDecoder.h"

#include <PiiUtil.h>
#include <PiiSerializationUtil.h>
#include <PiiGenericTextInputArchive.h>
#include <PiiGenericTextOutputArchive.h>
#include <PiiAsyncCall.h>
#include <PiiDelay.h>
#include <PiiMetaTypeUtil.h>

#include <QUrl>

namespace PiiNetworkPrivate
{
  enum 
  {
    Invalid = 0x00000000,
    Readable = 0x00000001,
    Writable = 0x00000002,
    Resettable = 0x00000004,
    EnumOrFlag = 0x00000008,
    StdCppSet = 0x00000100,
//     Override = 0x00000200,
    Constant = 0x00000400,
    Final = 0x00000800,
    Designable = 0x00001000,
    ResolveDesignable = 0x00002000,
    Scriptable = 0x00004000,
    ResolveScriptable = 0x00008000,
    Stored = 0x00010000,
    ResolveStored = 0x00020000,
    Editable = 0x00040000,
    ResolveEditable = 0x00080000,
    User = 0x00100000,
    ResolveUser = 0x00200000,
    Notify = 0x00400000
  };
}

/*
#include <iostream>
static void printMetaData(const QByteArray& metaData)
{
  for (int i=0; i<metaData.size(); ++i)
    {
      if (metaData[i] != 0)
        std::cout <<  metaData[i];
      else
        std::cout << "\\0";
    }
  std::cout << "\n";
}
*/
               
static int iMethodIndex = 4;
static int iPropertyIndex = 6;
//static int iEnumIndex = 8;
static int iSignalIndex = 13;
static int iMetaHeaderSize = 14;

static int iStringHeaderSize = 43;
static int iEmptyStringIndex = 21;
static int iParameterIndex = 22;

PiiRemoteObjectClient::Data::Data() :
  stringData("PiiRemoteObjectClient\0a,b,c,d,e,f,g,h,i,j\0\0\0", iStringHeaderSize+1),
  pHttpDevice(0),
  iAddToChannelIndex(-1),
  iRemoveFromChannelIndex(-1),
  iCloseChannelIndex(-1),
  pChannelThread(0),
  bChannelRunning(false),
  iRetryCount(2),
  iRetryDelay(1000)
{
  vecMetaData.fill(0, iMetaHeaderSize+1);
  vecMetaData[0] = 5;
  /* Header structure:
    5,    revision
    0,    classname
    0, 0, classinfo
    0, 0, methods
    0, 0, properties
    0, 0, enums/sets
    0, 0, constructors
    0,    flags
    0,    signalCount
    0     end of data
  */

  // enums: name, flags, count, data
  //30, 0x0,    2,   23,

  metaObject.d.superdata = &QObject::staticMetaObject;
  metaObject.d.stringdata = stringData.constData();
  metaObject.d.data = vecMetaData.constData();
  metaObject.d.extradata = 0;
}

PiiRemoteObjectClient::PiiRemoteObjectClient() : d(new Data)
{
  // Unused signal killer
  startTimer(1000);
}

PiiRemoteObjectClient::PiiRemoteObjectClient(const QString& serverUri) : d(new Data)
{
  try { setServerUri(serverUri); } catch (...) { delete d; throw; }
  // Unused signal killer
  startTimer(1000);
}

PiiRemoteObjectClient::~PiiRemoteObjectClient()
{
  synchronized (d->channelMutex)
    {
      try { closeChannel(); }
      catch (PiiNetworkException&) {}
    }
  
  delete d->pHttpDevice;
  delete d;
}

void PiiRemoteObjectClient::timerEvent(QTimerEvent*)
{
  // HACK Remove unconnected signals from channel. This must be done
  // here because Qt doesn't always call disconnectNotify() when a
  // connection is broken.
  for (int i=0; i<d->lstSignals.size(); ++i)
    {
      if (d->lstSignals[i].bConnected)
        {
          QByteArray aSignature(d->lstSignals[i].aSignature);
          aSignature.prepend('2'); // Qt hacks...
          if (receivers(aSignature.constData()) == 0)
            {
              removeFromChannel("signals/" + d->lstSignals[i].aSignature);
              d->lstSignals[i].bConnected = false;
            }
        }
    }
}

void PiiRemoteObjectClient::createMetaObject()
{
  d->stringData.resize(iStringHeaderSize);
  d->vecMetaData.resize(iMetaHeaderSize);
  d->vecMetaData << 0; // EOD
  
  collectFunctions(true); // signals
  collectFunctions(false); // other functions

  // Store the number of functions ...
  d->vecMetaData[iMethodIndex] = d->lstFunctions.size() + d->lstSignals.size();
  // Functions start immediately after the header
  d->vecMetaData[iMethodIndex + 1] = iMetaHeaderSize; 
  // The first N are signals
  d->vecMetaData[iSignalIndex] = d->lstSignals.size();

  collectProperties();
  
  d->metaObject.d.stringdata = d->stringData.constData();
  d->metaObject.d.data = d->vecMetaData.constData();
}

PiiHttpDevice& PiiRemoteObjectClient::openConnection()
{
  QIODevice* pSocket = 0;
  for (int iTry=0; iTry<=d->iRetryCount; ++iTry)
    {
      pSocket = d->networkClient.openConnection();
      if (pSocket != 0)
        break;
      else if (iTry != d->iRetryCount)
        PiiDelay::msleep(d->iRetryDelay);
    }
  if (pSocket == 0)
    PII_THROW(PiiNetworkException,
              tr("Connection to the server object at %1 could not be established.").arg(serverUri()));
  if (d->pHttpDevice == 0)
    d->pHttpDevice = new PiiHttpDevice(pSocket, PiiHttpDevice::Client);
  else if (d->pHttpDevice->device() != pSocket)
    d->pHttpDevice->setDevice(pSocket);
  return *d->pHttpDevice;
}

QList<QByteArray> PiiRemoteObjectClient::readDirectoryList(const QString& path)
{
  // Try twice
  for (int iTry=0; iTry<2; ++iTry)
    {
      PiiHttpDevice& dev(openConnection());

      dev.setRequest("GET", d->strPath + path);
      dev.removeHeader("Content-Type");
      dev.finish();

      if (!dev.isReadable())
        continue;

      if (!dev.readHeader())
        PII_THROW(PiiNetworkException, tr("Couldn't receive a response header."));
      if (dev.status() != PiiHttpProtocol::OkStatus)
        PII_THROW(PiiNetworkException, tr("Server responded with status code %1").arg(dev.status()));;
      
      return dev.readBody().split('\n');
    }
  return QList<QByteArray>();
}

void PiiRemoteObjectClient::collectProperties()
{
  QList<QByteArray> lstProperties = readDirectoryList("properties/");
  // Properties are encoded as "type name", e.g. "int value"
  QRegExp propExp("[^ ]+ [^ ]+");

  // Remove EOD marker from vecMetaData
  d->vecMetaData.resize(d->vecMetaData.size()-1);
  int iFirstPropertyIndex = d->vecMetaData.size();
  d->lstProperties.clear();

  for (int i=0; i<lstProperties.size(); ++i)
    {
      // This also catches the special case of no properties (one empty entry in the list).
      if (!propExp.exactMatch(QString(lstProperties[i]))) 
        continue;

      int iSpaceIndex = lstProperties[i].indexOf(' ');
      lstProperties[i][iSpaceIndex] = 0;

      // Check that the variant type is correctly specified
      int type = QMetaType::type(lstProperties[i].constData());
      if (type == 0)
        {
          piiDebug(QString("Unsupported remote property type: ") + QString(lstProperties[i]));
          continue;
        }

      // Add property name to string data and its index to metadata
      d->vecMetaData << d->stringData.size();
      d->stringData.append(lstProperties[i].constData() + iSpaceIndex + 1);
      d->stringData.append('\0');
      // Add property type
      d->vecMetaData << d->stringData.size();
      d->stringData.append(lstProperties[i].constData());
      d->stringData.append('\0');

      using namespace PiiNetworkPrivate;
      // Some magic again. These are the property "flags".
      uint uiFlags =  Readable | Writable | StdCppSet | Designable | Scriptable | Stored;
      if (type < int(QVariant::UserType))
        uiFlags |= type << 24;
      d->vecMetaData << uiFlags;
      
      d->lstProperties << Property(type, lstProperties[i].constData() + iSpaceIndex + 1);
    }

  // Add EOD marker back
  d->vecMetaData << 0;

  // Store the number of properties and the start index
  d->vecMetaData[iPropertyIndex] = d->lstProperties.size();
  d->vecMetaData[iPropertyIndex + 1] = d->lstProperties.isEmpty() ? 0 : iFirstPropertyIndex;
}

void PiiRemoteObjectClient::collectFunctions(bool listSignals)
{
  QList<QByteArray> lstSignatures = readDirectoryList(listSignals ? "signals/" : "functions/");
  QRegExp funcExp("([^ ]+ )?([^ (]+)\\(([^ )]*)\\)");

  // Remove EOD marker from vecMetaData
  d->vecMetaData.resize(d->vecMetaData.size()-1);

  if (listSignals)
    d->lstSignals.clear();
  else
    d->lstFunctions.clear();
  
  for (int i=0; i<lstSignatures.size(); ++i)
    {
      if (!funcExp.exactMatch(QString(lstSignatures[i])))
        continue;

      // Check that each parameter is a valid type name
      QStringList lstParams = funcExp.cap(3).split(',', QString::SkipEmptyParts);
      int iSpaceIndex = lstSignatures[i].indexOf(' ');
      int returnType = 0;
      if (iSpaceIndex != -1)
        {
          lstSignatures[i][iSpaceIndex] = 0;
          returnType = QMetaType::type(lstSignatures[i].constData());
        }
      const char* pSignature;
      QList<int> lstParamTypes;
      for (int j=0; j<lstParams.size(); ++j)
        {
          int type = QMetaType::type(qPrintable(lstParams[j]));
          if (type == 9)
            goto nextSignature;
          lstParamTypes << type;
        }
      // Add index of signature to metadata
      d->vecMetaData << d->stringData.size();
      lstSignatures[i].append('\0');
      pSignature = lstSignatures[i].constData() + iSpaceIndex + 1;
      d->stringData.append(pSignature);
      d->stringData.append('\0');

      // Add index of (dummy) parameter names. Qt allows at most 10 parameters.
      d->vecMetaData << iParameterIndex + (10 - lstParamTypes.size()) * 2;
      
      // Not void
      if (iSpaceIndex != -1)
        {
          // Add index to return type to metadata
          d->vecMetaData << d->stringData.size();
          // Add return value to string data
          d->stringData.append(lstSignatures[i].constData());
          d->stringData.append('\0');
        }
      else
        // Void function has null return type
        d->vecMetaData << iEmptyStringIndex;

      // Add "tag" and "flags". No idea what these are for. "5" seems to identify a signal.
      d->vecMetaData << iEmptyStringIndex << (listSignals ? 5 : 2);

      if (listSignals)
        d->lstSignals << Signal(pSignature, returnType, funcExp.cap(2), lstParamTypes);
      else
        {
          d->lstFunctions << Function(pSignature, returnType, funcExp.cap(2), lstParamTypes);
          // Store function indices for fast access.
          if (!strcmp(pSignature, "addToChannel(QString,QString)"))
            d->iAddToChannelIndex = i;
          else if (!strcmp(pSignature, "removeFromChannel(QString,QString)"))
            d->iRemoveFromChannelIndex = i;
          else if (!strcmp(pSignature, "closeChannel(QString)"))
            d->iCloseChannelIndex = i;
        }
    nextSignature:;
    }

  // Add EOD marker back
  d->vecMetaData << 0;
}

const QMetaObject* PiiRemoteObjectClient::metaObject() const
{
  return &d->metaObject;
}

void* PiiRemoteObjectClient::qt_metacast(const char* className)
{
  if (className == 0) return 0;
  if (!strcmp(className, d->stringData.constData()))
    return static_cast<void*>(this);
  return QObject::qt_metacast(className);
}

int PiiRemoteObjectClient::qt_metacall(QMetaObject::Call call, int id, void** args)
{
  id = QObject::qt_metacall(call, id, args);
  if (id < 0)
    return id;

  // Try twice
  for (int iTry=0; iTry<2; ++iTry)
    {
      try
        {
          return metaCall(call, id, args);
        }
      catch (PiiException& ex)
        {
          piiWarning(ex.location("", ": ") + ex.message());
          d->networkClient.closeConnection();
          if (iTry == 0)
            {
              piiDebug(tr("Remote method call failed. Trying to reconnect."));
              PiiDelay::msleep(d->iRetryDelay);
            }
        }
    }
  return id;
}

static const char* pDeviceIsNotConnectedMessage = QT_TR_NOOP("Connection to remote object was broken.");
static const char* pErrorReadingResponse = QT_TR_NOOP("Error receiving server's response.");
#define PII_THROW_IF_NOT_CONNECTED if (!dev.isReadable()) PII_THROW(PiiNetworkException, tr(pDeviceIsNotConnectedMessage))

#define PII_CHECK_SERVER_RESPONSE                                     \
  PII_THROW_IF_NOT_CONNECTED;                                         \
  if (!dev.readHeader() || dev.status() != PiiHttpProtocol::OkStatus) \
    PII_THROW(PiiNetworkException, tr(pErrorReadingResponse))


int PiiRemoteObjectClient::metaCall(QMetaObject::Call call, int id, void** args)
{
  PiiHttpDevice& dev(openConnection());
  
  if (call == QMetaObject::InvokeMetaMethod)
    {
      // The fist ids are signals, which we stored in another list.
      id -= d->lstSignals.size();
      
      dev.startOutputFiltering(new PiiStreamBuffer);
      const QList<int>& lstTypes = d->lstFunctions[id].lstParamTypes;
      if (lstTypes.size() > 0)
        {
          dev.setRequest("POST", d->strPath + "functions/" + d->lstFunctions[id].strName);
          dev.removeHeader("Content-Type");
          dev.write(PiiSerialization::toByteArray<PiiGenericTextOutputArchive>(Pii::argsToList(lstTypes, args+1)));
        }
      else
        {
          dev.setRequest("GET", d->strPath + "functions/" + d->lstFunctions[id].strName);
          dev.removeHeader("Content-Type");
        }

      dev.finish();

      PII_CHECK_SERVER_RESPONSE;
      
      if (args[0] != 0 && d->lstFunctions[id].returnType != 0)
        {
          QVariant varReply = dev.decodeVariant(dev.readBody());
          if (!Pii::copyMetaType(varReply, d->lstFunctions[id].returnType, args))
            PII_THROW(PiiNetworkException, tr("Unexpected return data."));
        }
      else
        dev.discardBody();
      
      id -= d->lstFunctions.size();
    }
  else if (call == QMetaObject::ReadProperty)
    {
      dev.setRequest("GET", d->strPath + "properties/" + d->lstProperties[id].strName);
      dev.removeHeader("Content-Type");
      dev.finish();

      PII_CHECK_SERVER_RESPONSE;

      QByteArray aBody(dev.readBody());
      QVariant varReply = dev.decodeVariant(aBody);
      //piiDebug(QString("%1 = %2 (%3)").arg(d->lstProperties[id].strName).arg(QString(aBody)).arg(varReply.toString()));
      if (!Pii::copyMetaType(varReply, d->lstProperties[id].type, args))
        {
          piiDebug(QString(aBody));
          PII_THROW(PiiNetworkException, tr("Server returned a QVariant with type id %1, but %2 was expected for %3.")
                    .arg(varReply.type()).arg(d->lstProperties[id].type).arg(d->lstProperties[id].strName));
        }
      id -= d->lstProperties.size();
    }
  else if (call == QMetaObject::WriteProperty)
    {
      dev.startOutputFiltering(new PiiStreamBuffer);
      dev.setRequest("POST", d->strPath + "properties/" + d->lstProperties[id].strName);
      dev.removeHeader("Content-Type");
      dev.write(dev.encode(*reinterpret_cast<QVariant*>(args[1])));
      dev.finish();

      PII_CHECK_SERVER_RESPONSE;

      dev.discardBody();
      
      id -= d->lstProperties.size();
    }
  else
    id -= d->lstProperties.size();

  return id;
}

void PiiRemoteObjectClient::connectNotify(const char* signal)
{
  for (int i=0; i<d->lstSignals.size(); ++i)
    if (!strcmp(d->lstSignals[i].aSignature, signal+1))
      {
        if (addToChannel("signals/" + d->lstSignals[i].aSignature))
          d->lstSignals[i].bConnected = true;
        break;
      }
}

void PiiRemoteObjectClient::disconnectNotify(const char* signal)
{
  for (int i=0; i<d->lstSignals.size(); ++i)
    if (!strcmp(d->lstSignals[i].aSignature, signal+1))
      {
        removeFromChannel("signals/" + d->lstSignals[i].aSignature);
        break;
      }
}

bool PiiRemoteObjectClient::addToChannel(const QString& uri)
{
  return manageChannel(d->iAddToChannelIndex, uri);
}

bool PiiRemoteObjectClient::removeFromChannel(const QString& uri)
{
  return manageChannel(d->iRemoveFromChannelIndex, uri);
}

bool PiiRemoteObjectClient::manageChannel(int functionIndex, const QString& uri)
{
  try
    {
      if (functionIndex == -1) return false;
      if (!openChannel()) return false;
      
      bool bReturn = false;
      void* args[] = { &bReturn, (void*)&d->strChannelId, (void*)&uri };
      return metaCall(QMetaObject::InvokeMetaMethod, d->lstSignals.size() + functionIndex, args) && bReturn;
    }
  catch (PiiException& ex)
    {
      piiWarning(tr("Cannot configure return channel: %1").arg(ex.message()) + ex.location(" (", ")"));
      return false;
    }
}

bool PiiRemoteObjectClient::openChannel()
{
  QMutexLocker lock(&d->channelMutex);

  if (d->bChannelRunning)
    return true;
  
  closeChannel();
  d->strChannelId.clear();
  
  d->pChannelThread = Pii::createAsyncCall(this, &PiiRemoteObjectClient::readChannel);
  d->pChannelThread->start();
  d->channelUpCondition.wait(&d->channelMutex);
  if (!d->bChannelRunning)
    {
      piiWarning(tr("Failed to request a return channel from %1.").arg(d->networkClient.serverAddress()));
      lock.unlock();
      d->pChannelThread->wait();
      lock.relock();
      delete d->pChannelThread;
      d->pChannelThread = 0;
    }
  return d->bChannelRunning;
}

// NOTE channelMutex must be locked when entering this function!
void PiiRemoteObjectClient::closeChannel()
{
  d->bChannelRunning = false;
  if (d->pChannelThread != 0)
    {
      d->channelMutex.unlock();
      d->pChannelThread->wait();
      d->channelMutex.lock();
      delete d->pChannelThread;
      d->pChannelThread = 0;
    }

  // Call the server-side function.
  void* args[] = { 0, &d->strChannelId };
  metaCall(QMetaObject::InvokeMetaMethod, d->lstSignals.size() + d->iCloseChannelIndex, args);
}

bool PiiRemoteObjectClient::canContinue(double)
{
  return d->bChannelRunning;
}

void PiiRemoteObjectClient::readChannel()
{
  QMutexLocker lock(&d->channelMutex);

  PiiNetworkClient networkClient(d->networkClient.serverAddress());
  PiiSocketDevice pSocket = networkClient.openConnection();
  
  if (pSocket == 0)
    {
      d->channelUpCondition.wakeOne();
      return;
    }
  
  PiiHttpDevice dev(pSocket, PiiHttpDevice::Client);
  // We are going to accept any number of bytes
  dev.setMessageSizeLimit(0);
  dev.setRequest("GET", d->strPath + "channels/new");
  dev.finish();
  dev.readHeader();
  dev.setController(this);

  if (!checkChannelResponse(dev))
    {
      d->channelUpCondition.wakeOne();
      return;
    }
    
  // Read preamble (should contain the channel ID)
  QByteArray aChannelId = dev.readLine();
  while (aChannelId.size() > 0 && char(aChannelId[aChannelId.size()-1]) member_of ('\n', '\r'))
    aChannelId.chop(1);

  if (aChannelId.isEmpty())
    {
      piiWarning(tr("Could not read channel ID from input stream."));
      d->channelUpCondition.wakeOne();
      return;
    }

  d->strChannelId = aChannelId;

  // Now we are finally up and running
  d->bChannelRunning = true;
  d->channelUpCondition.wakeOne();
  lock.unlock();
  
  try
    {
      forever
        {
          PiiMultipartDecoder decoder(&dev, dev.responseHeader());
          while (d->bChannelRunning && dev.isReadable())
            {
              // Fetch next message
              if (!decoder.nextMessage()) continue;
              
              // Read and decode body contents
              QByteArray aBody = decoder.readAll();
              //piiDebug("Received message (%d bytes), read %d to %s",
              //  decoder.header().contentLength(),
              //  aBody.size(), piiPrintable(decoder.header().value("X-URI")));
              
              if (!decodePushData(decoder.header().value("X-URI"), aBody))
                {
                  // Oops, couldn't decode server's response. Let's
                  // try to restore the situation.
                  networkClient.closeConnection();
                  PiiDelay::msleep(200);
                  break;
                }
            }
          if (!d->bChannelRunning)
            break;
          if (!dev.isReadable())
            {
              // Try to reconnect
              for (int iTry=0; iTry<=d->iRetryCount; ++iTry)
                {
                  QMutexLocker channelLock(&d->channelMutex);
                  networkClient.setServerAddress(d->networkClient.serverAddress());
                  pSocket = networkClient.openConnection();
                  
                  if (pSocket != 0)
                    break;
                  if (iTry != d->iRetryCount)
                    PiiDelay::msleep(d->iRetryDelay);
                }
              if (pSocket == 0)
                {
                  piiWarning(tr("Broken return channel %1 from %2 could not be re-established.")
                             .arg(d->strChannelId)
                             .arg(networkClient.serverAddress()));
                  break;
                }
              dev.setDevice(pSocket);
              
              dev.setRequest("GET", d->strPath + "channels/reconnect?" + d->strChannelId);
              dev.finish();
              dev.readHeader();

              if (!checkChannelResponse(dev))
                break;
            }
        }
    }
  catch (PiiException& ex)
    {
      piiWarning(ex.location("", ": ") + ex.message());
    }

  synchronized (d->channelMutex) d->bChannelRunning = false;
}

bool PiiRemoteObjectClient::checkChannelResponse(PiiHttpDevice& dev)
{
  if (dev.status() != PiiHttpProtocol::OkStatus)
    {
      piiWarning(tr("Cannot set up a channel. Remote server responded with status code %1.").arg(dev.status()));
      return false;
    }
  else if (!dev.responseHeader().contentType().startsWith("multipart/"))
    {
      piiWarning(tr("Cannot set up a channel. Unrecognized content type \"%1\".").arg(dev.responseHeader().contentType()));
      return false;
    }
  
  return true;
}


bool PiiRemoteObjectClient::emitSignal(int id, const QByteArray& data)
{
  try
    {
      QVariantList lstArgs;
      if (!data.isEmpty())
        PiiSerialization::fromByteArray<PiiGenericTextInputArchive>(data, lstArgs);
      if (Pii::scoreOverload(lstArgs, d->lstSignals[id].lstParamTypes) == -1)
        {
          piiWarning(tr("Cannot emit %1: deserialized parameter types do not match.")
                     .arg(QString::fromLatin1(d->lstSignals[id].aSignature)));
          return false;
        }
      void* args[11];
      args[0] = 0;
      for (int i=0; i<lstArgs.size(); ++i)
        {
          if (d->lstSignals[id].lstParamTypes[i] < int(QVariant::UserType))
            lstArgs[i].convert(QVariant::Type(d->lstSignals[id].lstParamTypes[i]));
          args[i+1] = const_cast<void*>(lstArgs[i].constData());
        }
      QMetaObject::activate(this, &d->metaObject, id, args);
    }
  catch (PiiSerializationException& ex)
    {
      piiWarning(tr("Cannot emit %1: %2").arg(QString::fromLatin1(d->lstSignals[id].aSignature)).arg(ex.message()));
      return false;
    }
  return true;
}

bool PiiRemoteObjectClient::decodePushData(const QString& uri, const QByteArray& data)
{
  //piiDebug(QString("Received %1 bytes to %2.").arg(data.size()).arg(uri));
  if (uri.startsWith("signals/"))
    {
      QByteArray aSignature = uri.mid(8).toLatin1();
      for (int i=0; i<d->lstSignals.size(); ++i)
        if (d->lstSignals[i].aSignature == aSignature)
          return emitSignal(i, data);
    }
  return true;
}

void PiiRemoteObjectClient::setServerUri(const QString& uri)
{
  QMutexLocker lock(&d->channelMutex);
  bool bFirstTime = d->networkClient.serverAddress().isEmpty();
  QRegExp uriExp("([^:]+://[^/]+)(/[^ ]*)");
  if (!uriExp.exactMatch(uri))
    PII_THROW(PiiInvalidArgumentException, tr("The provided server URI (%1) is invalid.").arg(uri));

  d->networkClient.setServerAddress(uriExp.cap(1));
  d->strPath = uriExp.cap(2);
  if (d->strPath[d->strPath.size()-1] != '/')
    d->strPath.append('/');

  if (bFirstTime)
    createMetaObject();
}

QString PiiRemoteObjectClient::serverUri() const
{
  return d->networkClient.serverAddress() + d->strPath;
}

void PiiRemoteObjectClient::setRetryCount(int retryCount) { d->iRetryCount = qBound(0,retryCount,5); }
int PiiRemoteObjectClient::retryCount() const { return d->iRetryCount; }
void PiiRemoteObjectClient::setRetryDelay(int retryDelay) { d->iRetryDelay = qBound(0,retryDelay,2000); }
int PiiRemoteObjectClient::retryDelay() const { return d->iRetryDelay; }
