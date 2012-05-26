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

#include "PiiHttpDevice.h"
#include "PiiHttpProtocol.h"
#include "PiiMimeHeader.h"

#include <QUrl>
#include <QBuffer>
#include <QTextCodec>
#include <QAbstractSocket>
#include <QLocalSocket>

#include <PiiDelay.h>
#include <PiiProgressController.h>
#include <PiiSerializationUtil.h>
#include <PiiGenericTextInputArchive.h>
#include <PiiGenericTextOutputArchive.h>
#include <PiiGenericBinaryInputArchive.h>
#include <PiiGenericBinaryOutputArchive.h>

PiiHttpDevice::Data::Data(PiiHttpDevice* owner, const PiiSocketDevice& device, Mode mode) :
  mode(mode),
  pSocket(device),
  pController(0),
  pActiveOutputFilter(owner),
  pTextCodec(0),
  bHeaderRead(false),
  bHeaderSent(false),
  iHeaderSizeLimit(4096),
  iMessageSizeLimit(8*1024*1024),
  iBytesRead(0), iBytesWritten(0),
  bBodyRead(false),
  bFinished(false),
  iBodyLength(-1),
  iHeaderLength(-1)
{
}

PiiHttpDevice::PiiHttpDevice(const PiiSocketDevice& device, Mode mode) :
  PiiStreamFilter(new Data(this, device, mode))
{
  if (mode == Server) setStatus(PiiHttpProtocol::OkStatus);
  open(device->openMode() | QIODevice::Unbuffered);
}

PiiHttpDevice::~PiiHttpDevice()
{
  finish();
}

qint64 PiiHttpDevice::flushFilter()
{
  PII_D;
  qint64 iTotalBytesToWrite = d->pSocket->bytesToWrite();
  while (d->pSocket->bytesToWrite() > 0)
    {
      // Flush the device
      if (!d->pSocket->waitForBytesWritten(1000))
        {
          qint64 iDiff = iTotalBytesToWrite - d->pSocket->bytesToWrite();
          return iDiff > 0 ? iDiff : -1;
        }
    }
  return iTotalBytesToWrite;
}

void PiiHttpDevice::finish()
{
  PII_D;
  if (!d->bFinished && isWritable())
    {
      endOutputFiltering(this);

      // If nothing has been sent, send header.
      if (!d->bHeaderSent)
        {
          if (d->mode == Server || requestMethod() != "GET")
            setHeader("Content-Length", 0);
          sendHeader();
        }
      
      // Flush the device if it is still connected
      if (d->pSocket->bytesToWrite() > 0 && isWritable())
        d->pSocket->waitForBytesWritten(5000);
    }
  else
    {
      // Delete all output filters
      while (d->pActiveOutputFilter != this)
        {
          PiiStreamFilter* tmpFilter = d->pActiveOutputFilter;
          d->pActiveOutputFilter = tmpFilter->outputFilter();
          delete tmpFilter;
        }
    }
  
  d->bFinished = true;
}

bool PiiHttpDevice::isWritable() const
{
  return _d()->pSocket.isWritable();
}

bool PiiHttpDevice::isReadable() const
{
  return _d()->pSocket.isReadable();
}

QHttpRequestHeader PiiHttpDevice::requestHeader() const
{
  return _d()->requestHeader;
}

QHttpResponseHeader PiiHttpDevice::responseHeader() const
{
  return _d()->responseHeader;
}

PiiHttpDevice::ConnectionType PiiHttpDevice::connectionType() const
{
  return (_d()->responseHeader.value("connection").toLower() == "close") ?
    CloseConnection :
    KeepAliveConnection;
}

QString PiiHttpDevice::requestMethod() const
{
  return _d()->requestHeader.method();
}

int PiiHttpDevice::status() const
{
  return _d()->responseHeader.statusCode();
}

void PiiHttpDevice::setStatus(int code)
{
  _d()->responseHeader.setStatusLine(code, PiiHttpProtocol::statusMessage(code));
}

QString PiiHttpDevice::requestUri() const
{
  return _d()->requestHeader.path();
}

QString PiiHttpDevice::queryString() const
{
  return requestUri().remove(QRegExp("^[^?]*\\??"));
}

bool PiiHttpDevice::hasQuery() const
{
  return requestUri().contains('?');
}

void PiiHttpDevice::parseQueryValues(const QString& uri)
{
  int iSplitPos = uri.indexOf('?');
  if (iSplitPos == -1 || iSplitPos == uri.size()-1) return;
  PII_D;
  parseVariables(uri.mid(iSplitPos+1).toUtf8(), d->mapQueryValues, d->lstQueryItems);
}

void PiiHttpDevice::parseVariables(const QByteArray& data,
                                   QVariantMap& valueMap,
                                   QStringList& names)
{
  QList<QByteArray> lstParts(data.split('&'));
  for (int i=0; i<lstParts.size(); ++i)
    {
      if (lstParts[i].isEmpty()) continue;
      int iSplitPos = lstParts[i].indexOf('=');
      QString strName;
      QByteArray aValue;
      if (iSplitPos == -1)
        aValue = lstParts[i];
      else
        {
          strName = QUrl::fromPercentEncoding(lstParts[i].left(iSplitPos));
          aValue = lstParts[i].mid(iSplitPos+1);
        }
      addToMap(valueMap, strName, QUrl::fromPercentEncoding(aValue).toUtf8());
      names << strName;
    }
}


QString PiiHttpDevice::requestPath(const QString& basePath) const
{
  QString path = requestUri().remove(QRegExp("\\?.*$"));
  int iBaseLen = basePath.size();
  if (iBaseLen > 0 && path.startsWith(basePath))
    path = path.right(path.size() - iBaseLen);
  return path;
}

QString PiiHttpDevice::host() const
{
  return _d()->requestHeader.value("host");
}

void PiiHttpDevice::startOutputFiltering(PiiStreamFilter* filter)
{
  // The filter writes its data to the currently active device.
  filter->setOutputFilter(_d()->pActiveOutputFilter);
  // Change the active device.
  _d()->pActiveOutputFilter = filter;
}

qint64 PiiHttpDevice::filterData(const char* data, qint64 maxSize)
{
  // Must ensure that headers are sent first.
  sendHeader();
  return writeToSocket(data, maxSize);
}

PiiStreamFilter* PiiHttpDevice::outputFilter() const
{
  return _d()->pActiveOutputFilter;
}

void PiiHttpDevice::endOutputFiltering(PiiStreamFilter* filter)
{
  PII_D;
  // Collapse the filter stack until filter is found
  while (d->pActiveOutputFilter != this)
    {
      PiiStreamFilter* tmpFilter = d->pActiveOutputFilter;
      // If the last filter has buffered its data, we may know the
      // content length. Note that if a single byte has been written
      // to the socket, the headers have been already sent without the
      // content length. Thus, it makes no harm to set the header
      // here to a possibly wrong value.
      if (tmpFilter->outputFilter() == this && tmpFilter->bufferedSize() >= 0)
        setHeader("Content-Length", tmpFilter->bufferedSize());
      
      tmpFilter->flushFilter();
      d->pActiveOutputFilter = tmpFilter->outputFilter();
      
      delete tmpFilter;
      if (filter == 0 || tmpFilter == filter)
        break;
    }
}

void PiiHttpDevice::setRequest(const QString& method, const QString& uri)
{
  PII_D;
  if (uri.indexOf('?') != -1)
    removeQueryValues();
  
  d->requestHeader.setRequest(method, uri);

  if (d->mode == Client && d->bFinished)
    restart();
}

//void PiiHttpDevice::sendForm(const QString& uri) {}

void PiiHttpDevice::setHeader(const QString& name, const QVariant& value, bool replace)
{
  if (_d()->mode == Server)
    setResponseHeader(name, value.toString(), replace);
  else
    setRequestHeader(name, value.toString(), replace);
  
  checkCodec(name, value.toString());
}

void PiiHttpDevice::removeHeader(const QString& name)
{
  PII_D;
  if (d->mode == Server)
    d->responseHeader.removeValue(name);
  else
    d->requestHeader.removeValue(name);
}

void PiiHttpDevice::setRequestHeader(const QString& name, const QString& value, bool replace)
{
  PII_D;
  if (replace)
    d->requestHeader.setValue(name, value);
  else
    d->requestHeader.addValue(name, value);
}

void PiiHttpDevice::setResponseHeader(const QString& name, const QString& value, bool replace)
{
  PII_D;
  if (replace)
    d->responseHeader.setValue(name, value);
  else
    d->responseHeader.addValue(name, value);

  if (name.toLower() == "location" && d->responseHeader.statusCode() / 100 != 3)
    setStatus(PiiHttpProtocol::FoundStatus);
}

void PiiHttpDevice::checkCodec(const QString& key, const QString& value)
{
  if (key.toLower() == "content-encoding")
    _d()->pTextCodec = QTextCodec::codecForName(value.toAscii());
}

QVariant PiiHttpDevice::queryValue(const QString& name) const
{
  return _d()->mapQueryValues[name];
}

QVariantMap PiiHttpDevice::queryValues() const
{
  return _d()->mapQueryValues;
}

QStringList PiiHttpDevice::queryItems() const
{
  return _d()->lstQueryItems;
}

void PiiHttpDevice::addQueryValue(const QString& name, const QVariant& value)
{
  PII_D;
  addToMap(d->mapQueryValues, name, value);
  d->lstQueryItems << name;
  createQueryString();
}

void PiiHttpDevice::removeQueryValue(const QString& name)
{
  PII_D;
  d->lstQueryItems.removeAll(name);
  d->mapQueryValues.remove(name);
  createQueryString();
}

void PiiHttpDevice::removeQueryValues()
{
  PII_D;
  d->lstQueryItems.clear();
  d->mapQueryValues.clear();
  createQueryString();
}

void PiiHttpDevice::readFormValues()
{
  if (isBodyRead())
    return;
  
  PII_D;

  // PENDING Multipart messages
  if (requestMethod() != "POST" ||
      d->requestHeader.contentType() != PiiNetwork::formContentType())
    return;

  QByteArray aBody = readBody();
  parseVariables(aBody, d->mapFormValues, d->lstFormItems);
}

QStringList PiiHttpDevice::formItems() const
{
  const_cast<PiiHttpDevice*>(this)->readFormValues();
  return _d()->lstFormItems;
}

QVariant PiiHttpDevice::formValue(const QString& name) const
{
  const_cast<PiiHttpDevice*>(this)->readFormValues();
  //qDebug("formValue() returning %s", qPrintable(d->mapFormValues[name].toString()));
  return _d()->mapFormValues[name];
}

QVariantMap PiiHttpDevice::formValues() const
{
  const_cast<PiiHttpDevice*>(this)->readFormValues();
  return _d()->mapFormValues;
}

QVariant PiiHttpDevice::requestValue(const QString& name) const
{
  QVariant result = queryValue(name);
  //qDebug("requestValue(): valid %s: %d", qPrintable(name), int(result.isValid()));
  if (result.isValid())
    return result;
  return formValue(name);
}

QVariantMap PiiHttpDevice::requestValues() const
{
  const PII_D;
  const_cast<PiiHttpDevice*>(this)->readFormValues();
  QVariantMap mapResult(d->mapFormValues);
  for (QVariantMap::const_iterator i = d->mapQueryValues.constBegin();
       i != d->mapQueryValues.constEnd(); ++i)
    mapResult.insert(i.key(), i.value());
  return mapResult;
}

void PiiHttpDevice::addToMap(QVariantMap& map, const QString& key, const QByteArray& value)
{
  addToMap(map, key, decodeVariant(value));
}

void PiiHttpDevice::addToMap(QVariantMap& map, const QString& key, const QVariant& value)
{
  // If the map already contains this key, we must append to the
  // list.
  if (map.contains(key))
    {
      QVariant& oldValue = map[key];
      if (oldValue.type() == QVariant::List)
        oldValue = oldValue.toList() << value;
      else
        oldValue = QVariantList() << oldValue << value;
    }
  else
    map.insert(key, value);
}

QVariant PiiHttpDevice::decodeVariant(const QString& data) const
{
  return decodeVariant(data.toUtf8());
}

template <class Archive> QVariant PiiHttpDevice::decodeVariant(const QByteArray& data)
{
  try
    {
      QBuffer bfr(const_cast<QByteArray*>(&data));
      bfr.open(QIODevice::ReadOnly);
      Archive ar(&bfr);
      QVariant result;
      ar >> result;
      return result;
    }
  catch (PiiSerializationException& ex)
    {
      piiWarning(ex.message() + "(" + ex.info() + ")");
      return QVariant();
    }
}

QVariant PiiHttpDevice::decodeVariant(const QByteArray& data) const
{
  QVariant result;
  if (data.size() == 0)
    return result;
  // These two capture serialized objects
  if (data.startsWith(PII_TEXT_ARCHIVE_ID))
    return decodeVariant<PiiGenericTextInputArchive>(data);
  else if (data.startsWith(PII_BINARY_ARCHIVE_ID))
    return decodeVariant<PiiGenericBinaryInputArchive>(data);
  // Numbers
  else if (strchr("0123456789-.", data[0]) != 0)
    {
      bool bOk = false;
      int iValue = data.toInt(&bOk);
      if (bOk)
        return iValue;
      double dValue = data.toDouble(&bOk);
      if (bOk)
        return dValue;
      return QString::fromUtf8(data.data(), data.size());
    }
  // Boolean values
  else if (data == "true")
    return true;
  else if (data == "false")
    return false;
  // The default is a string
  else
    {
      int iSize = data.size();
      if (data.size() >= 2 && data[0] == '"' && data[iSize-1] == '"')
        return QString(data.mid(1, iSize-2));
      return QString::fromUtf8(data.data(), data.size());
    }
}

template <class Archive> QByteArray PiiHttpDevice::encode(const QVariant& variant)
{
  try
    {
      return PiiSerialization::toByteArray<Archive>(variant);
    }
  catch (PiiSerializationException& ex)
    {
      piiWarning(ex.message());
      return QByteArray();
    }
}

QByteArray PiiHttpDevice::encode(const QVariant& variant, EncodingFormat format) const
{
  switch (variant.type())
    {
    case QVariant::Int:
    case QVariant::Double:
    case QVariant::UInt:
    case QVariant::ULongLong:
    case QVariant::Bool:
      return encode(variant.toString());
    case QVariant::String:
      return encode('"' + variant.toString() + '"');
    default:
      if (format == BinaryFormat)
        return encode<PiiGenericBinaryOutputArchive>(variant);
      else
        return encode<PiiGenericTextOutputArchive>(variant);
    }
}

QByteArray PiiHttpDevice::encode(const QString& msg) const
{
  const PII_D;
  if (d->pTextCodec != 0)
    return d->pTextCodec->fromUnicode(msg);
  return msg.toUtf8();
}

qint64 PiiHttpDevice::print(const QString& msg)
{
  return write(encode(msg));
}

qint64 PiiHttpDevice::writeData(const char* bytes, qint64 maxSize)
{
  // Write data to the active output filter
  return _d()->pActiveOutputFilter->filterData(bytes, maxSize);
}

qint64 PiiHttpDevice::writeToSocket(const char* bytes, qint64 maxSize)
{
  PII_D;
  return d->pSocket.writeWaited(bytes, maxSize, 20000, d->pController);
}

qint64 PiiHttpDevice::readData(char* bytes, qint64 maxSize)
{
  PII_D;
  // Once headers have been read, manually reading from the socket
  // means that we cannot decode POST bodies any more.
  if (d->bHeaderRead)
    d->bBodyRead = true;

  // We know how much there is to come...
  if (d->iHeaderLength != -1 && d->iBodyLength != -1)
    {
      qint64 iBytesLeft = d->iHeaderLength + d->iBodyLength - d->iBytesRead;
      
      /*qDebug("PiiHttpDevice::readData(%ld). d->pSocket->bytesAvailable() = %ld, QIODevice::bytesAvailable() = %ld",
             maxSize, d->pSocket->bytesAvailable(), QIODevice::bytesAvailable());
      qDebug("  %ld bytes read so far. Total number of bytes: %ld. Remaining: %ld",
      d->iBytesRead, d->iHeaderLength + d->iBodyLength, iBytesLeft);*/

      // The whole message has been read.
      if (iBytesLeft <= 0) return 0;

      maxSize = qMin(iBytesLeft, maxSize);
    }
  
  qint64 iRead = d->pSocket.readWaited(bytes, maxSize, 20000, d->pController);

  //qDebug("  read %ld bytes", iRead);
  
  // Count the total number of bytes read.
  if (iRead >= 0)
    {  
      d->iBytesRead += iRead;
      if (d->iMessageSizeLimit > 0 && d->iBytesRead > d->iMessageSizeLimit)
        return -1;
    }

  return iRead;
}

QByteArray PiiHttpDevice::readBody()
{
  PII_D;
  if (d->iBodyLength >= 0)
    return read(d->iBodyLength);
  return readAll();
}

void PiiHttpDevice::discardBody()
{
  PII_D;
  char bfr[128];
  if (d->iBodyLength >= 0)
    {
      qint64 iBytesLeft = d->iBodyLength;
      while (iBytesLeft > 0)
        {
          qint64 iBytesRead = read(bfr, qMin(iBytesLeft, qint64(128)));
          if (iBytesRead <= 0)
            return;
          iBytesLeft -= iBytesRead;
        }
    }
  else
    while (read(bfr, 128) == 128);
}

bool PiiHttpDevice::readHeader()
{
  PII_D;
  if (d->mode == Server && d->bFinished)
    restart();
  
  if (d->bHeaderRead) return true;
    
  bool bResult = d->mode == Client ? decodeResponseHeader() : decodeRequestHeader();
  d->bHeaderRead = true;
  // GET request cannot have a message body
  if (d->mode == Server && requestMethod() == "GET")
    {
      d->bBodyRead = true;
      d->iBodyLength = 0;
    }
  return bResult;
}

bool PiiHttpDevice::sendHeader()
{
  PII_D;
  if (d->bHeaderSent) return true;

  bool result = d->mode == Server ? sendResponseHeader() : sendRequestHeader();
  d->bHeaderSent = true;
  return result;
}

qint64 PiiHttpDevice::bytesAvailable() const
{
  const PII_D;
  //qDebug("PiiHttpDevice::bytesAvailable(): socket: %ld, QIODevice: %ld", d->pSocket->bytesAvailable(), QIODevice::bytesAvailable());
  return d->pSocket->bytesAvailable() + QIODevice::bytesAvailable();
}

bool PiiHttpDevice::isSequential() const
{
  return _d()->pSocket->isSequential();
}


bool PiiHttpDevice::sendResponseHeader()
{
  PII_D;
  // If the response header has no Content-Length, the end of the
  // transfer must be indicated by closing the connection.
  if (!d->responseHeader.hasContentLength() && !d->responseHeader.hasKey("Connection"))
    setHeader("Connection", "close");

  QByteArray aHeader(d->responseHeader.toString().toUtf8());
  return writeToSocket(aHeader.constData(), aHeader.size()) == aHeader.size();
}


bool PiiHttpDevice::decodeResponseHeader()
{
  PII_D;
  try
    {
      // Try to construct a response header
      QByteArray aHeader(PiiMimeHeader::readHeaderData(this, headerSizeLimit(), &d->iHeaderLength));
      if (aHeader.isEmpty())
        return false;
      QString strHeader(aHeader); // put this directly into constructor and see g++ fail
      QHttpResponseHeader header(strHeader);
      if (!header.isValid())
        return false;

      if (header.hasContentLength())
        {
          d->iBodyLength = header.contentLength();
          // If request body is too long
          if (d->iMessageSizeLimit > 0 && d->iBodyLength + d->iHeaderLength > d->iMessageSizeLimit)
            return false;
        }

      d->responseHeader = header;
    }
  catch (PiiException& ex)
    {
      piiWarning(ex.message());
      return false;
    }
  
  return true;
}

void PiiHttpDevice::createQueryString()
{
  PII_D;
  QString strQuery(requestPath());
  bool bFirst = true;
  QStringList lstUniqueItems(d->lstQueryItems);
  lstUniqueItems.removeDuplicates();
  for (int i=0; i<lstUniqueItems.size(); ++i)
    {
      QByteArray aKey = QUrl::toPercentEncoding(lstUniqueItems[i]);
      // Variant lists are encoded by repeating the same key many times
      QVariant varValue = d->mapQueryValues[lstUniqueItems[i]];
      if (varValue.type() == QVariant::List)
        {
          QVariantList lstValues = varValue.toList();
          for (int j=0; j<lstValues.size(); ++j)
            {
              strQuery.append(bFirst ? '?' : '&');
              bFirst = false;
              if (!aKey.isEmpty())
                {
                  strQuery.append(aKey);
                  strQuery.append('=');
                }
              strQuery.append(QUrl::toPercentEncoding(encode(lstValues[j])));
            }
        }
      else
        {
          strQuery.append(bFirst ? '?' : '&');
          bFirst = false;
          if (!aKey.isEmpty())
            {
              strQuery.append(aKey);
              strQuery.append('=');
            }
          strQuery.append(QUrl::toPercentEncoding(encode(varValue)));
        }
    }
  d->requestHeader.setRequest(d->requestHeader.method(), strQuery);
}

bool PiiHttpDevice::sendRequestHeader()
{
  PII_D;
  QByteArray aHeader(d->requestHeader.toString().toUtf8());
  return writeToSocket(aHeader.constData(), aHeader.size()) == aHeader.size();
}

bool PiiHttpDevice::decodeRequestHeader()
{
  PII_D;
  try
    {
      // Try to construct a request header
      QByteArray aHeader(PiiMimeHeader::readHeaderData(this, headerSizeLimit(), &d->iHeaderLength));
      if (aHeader.isEmpty())
        return false;
      QHttpRequestHeader header(aHeader);
      if (!header.isValid())
        {
          setStatus(PiiHttpProtocol::BadRequestStatus);
          return false;
        }

      // If request body is too long
      if (header.hasContentLength())
        {
          d->iBodyLength = header.contentLength();
          if (d->iMessageSizeLimit > 0 && d->iBodyLength + d->iHeaderLength > d->iMessageSizeLimit)
            {
              setStatus(PiiHttpProtocol::RequestEntityTooLargeStatus);
              return false;
            }
        }

      d->requestHeader = header;
      parseQueryValues(header.path());

      // If the client wants to close the connection, we'll do it for her.
      if (d->requestHeader.value("Connection").toLower() == "close")
        setHeader("Connection", "close");
    }
  catch (PiiMimeException& ex)
    {
      switch (ex.code())
        {
        case PiiMimeException::HeaderTooLarge:
          setStatus(PiiHttpProtocol::RequestEntityTooLargeStatus);
          return false;
        case PiiMimeException::InvalidFormat:
          setStatus(PiiHttpProtocol::BadRequestStatus);
          return false;
        }
    }
  
  return true;
}

bool PiiHttpDevice::isBodyRead() const { return _d()->bBodyRead; }
qint64 PiiHttpDevice::bodyLength() const { return _d()->iBodyLength; }
qint64 PiiHttpDevice::headerLength() const { return _d()->iHeaderLength; }
bool PiiHttpDevice::headerRead() const { return _d()->bHeaderRead; }
void PiiHttpDevice::setHeaderSizeLimit(qint64 headerSizeLimit) { _d()->iHeaderSizeLimit = headerSizeLimit; }
qint64 PiiHttpDevice::headerSizeLimit() const { return _d()->iHeaderSizeLimit; }
void PiiHttpDevice::setMessageSizeLimit(qint64 messageSizeLimit) { _d()->iMessageSizeLimit = messageSizeLimit; }
qint64 PiiHttpDevice::messageSizeLimit() const { return _d()->iMessageSizeLimit; }
void PiiHttpDevice::setController(PiiProgressController* controller) { _d()->pController = controller; }
PiiProgressController* PiiHttpDevice::controller() const { return _d()->pController; }

void PiiHttpDevice::setDevice(const PiiSocketDevice& device)
{
  if (device == 0)
    PII_THROW(PiiInvalidArgumentException, tr("Cannot set the communication device to null."));
  PII_D;
  d->pSocket = device;
}

PiiSocketDevice PiiHttpDevice::device() const { return _d()->pSocket; }

void PiiHttpDevice::restart()
{
  PII_D;
  d->bBodyRead = false;
  d->bHeaderRead = d->bHeaderSent = false;
  d->iBytesRead = d->iBytesWritten = 0;
  d->iBodyLength = d->iHeaderLength = -1;
  d->bFinished = false;
  d->mapFormValues.clear();
  d->lstFormItems.clear();
  d->mapQueryValues.clear();
  d->lstQueryItems.clear();
}
