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

#include "PiiHttpProtocol.h"

#include <QIODevice>
#include <QByteArray>
#include <QHttpRequestHeader>
#include <QtAlgorithms>
#include <QMutexLocker>

#include "PiiHttpDevice.h"
#include "PiiHttpException.h"

PiiHttpProtocol::Data::Data() :
  iMaxConnectionTime(0) // No limit by default
{}

PiiHttpProtocol::UriHandler::~UriHandler() {}

PiiHttpProtocol::PiiHttpProtocol() :
  PiiNetworkProtocol(new Data)
{}

PiiHttpProtocol::~PiiHttpProtocol()
{}

void PiiHttpProtocol::registerUriHandler(const QString& uri, UriHandler* handler)
{
  PII_D;
  if (!uri.startsWith('/'))
    return;
  
  QMutexLocker lock(&d->handlerListLock);
  // Replace old handler if one exists.
  for (int i=0; i<d->lstHandlers.size(); ++i)
    if (d->lstHandlers[i].first == uri)
      {
        d->lstHandlers[i].second = handler;
        return;
      }
  d->lstHandlers << HandlerPair(uri, handler);
}

void PiiHttpProtocol::unregisterUriHandler(const QString& uri)
{
  PII_D;
  QMutexLocker lock(&d->handlerListLock);
  for (int i=0; i<d->lstHandlers.size(); ++i)
    if (d->lstHandlers[i].first == uri)
      {
        d->lstHandlers.removeAt(i);
        return;
      }
}

void PiiHttpProtocol::unregisterUriHandler(UriHandler* handler)
{
  PII_D;
  QMutexLocker lock(&d->handlerListLock);
  for (int i=d->lstHandlers.size(); i--; )
    if (d->lstHandlers[i].second == handler)
      d->lstHandlers.removeAt(i);
}

void PiiHttpProtocol::unregisterAllHandlers(UriHandler* handler)
{
  PII_D;
  QMutexLocker lock(&d->handlerListLock);
  if (handler == 0)
    d->lstHandlers.clear();
  else
    for (int i=d->lstHandlers.size(); i--; )
      if (d->lstHandlers[i].second == handler)
        d->lstHandlers.removeAt(i);
}

void PiiHttpProtocol::communicate(QIODevice* dev, PiiProgressController* controller)
{
  PII_D;
  // This loop handles keep-alive connections.
  while (controller->canContinue())
    {
      //piiDebug("Serving request");
      PiiHttpDevice httpDevice(dev, PiiHttpDevice::Server);
      httpDevice.setController(controller);
      if (!httpDevice.readHeader())
        return;
      
      // Interrupted
      if (!controller->canContinue())
        {
          httpDevice.setStatus(ServiceUnavailableStatus);
          return;
        }

      HandlerPair pair = findHandler(httpDevice.requestUri());
      if (pair.second != 0)
        {
          TimeLimiter limiter(controller, d->iMaxConnectionTime);
          try
            {
              httpDevice.setController(&limiter);
              pair.second->handleRequest(pair.first, &httpDevice, &limiter);
              //piiDebug("PiiHttpProtocol: handleRequest() returned");
            }
          catch (PiiHttpException& ex)
            {
              httpDevice.setStatus(ex.statusCode());
              httpDevice.print(ex.message());
              piiWarning(ex.location("", ": ") +
                         httpDevice.requestMethod() + " " + httpDevice.requestPath() + " " +
                         QString::number(ex.statusCode()) + " " + ex.message());
            }
          catch (PiiException& ex)
            {
              httpDevice.setStatus(InternalServerErrorStatus);
              httpDevice.print(ex.message());
              piiWarning(ex.location("", ": ") + ex.message());
            }
          httpDevice.setController(0);
        }
      else
        httpDevice.setStatus(NotFoundStatus);

      httpDevice.finish();
      
      // HTTP/1.1 behavior: we'll only close the connection if the
      // client specifically asks to do so or just closes connection.
      if (httpDevice.connectionType() == PiiHttpDevice::CloseConnection ||
          !httpDevice.isWritable())
        return;
    }
}

PiiHttpProtocol::UriHandler* PiiHttpProtocol::uriHandler(const QString& uri, bool exactMatch)
{
  HandlerPair pair = findHandler(uri);
  return exactMatch && pair.first != uri ? 0 : pair.second;
}

PiiHttpProtocol::HandlerPair PiiHttpProtocol::findHandler(const QString& path)
{
  PII_D;
  QMutexLocker lock(&d->handlerListLock);
  
  int iBestMatchIndex = -1, iBestMatchLength = 0;
  for (int i=0; i<d->lstHandlers.size(); ++i)
    {
      // If the handler's URI matches the beginning of the request,
      // and the match is longer than anything before, this one is the
      // best so far.
      if (path.startsWith(d->lstHandlers[i].first) &&
          d->lstHandlers[i].first.size() > iBestMatchLength)
        {
          iBestMatchIndex = i;
          iBestMatchLength = d->lstHandlers[i].first.size();
        }
    }
  if (iBestMatchIndex != -1)
    return d->lstHandlers[iBestMatchIndex];

  return HandlerPair(path, 0);
}

// Stores HTTP status codes and their descriptions.
struct PiiHttpProtocol::StatusCode
{
  // Numerical status code.
  int code;
  // A human-representable description of the code.
  const char* description;
  bool operator< (const StatusCode& other) const { return code < other.code; }
};

QString PiiHttpProtocol::statusMessage(int code)
{
  static const StatusCode httpStatusCodes[] =
    { {100, "Continue" },
      {101, "Switching Protocols" },
      {102, "Processing" },
      {200, "OK" },
      {201, "Created" },
      {202, "Accepted" },
      {203, "Non-Authoritative Information" },
      {204, "No Content" },
      {205, "Reset Content" },
      {206, "Partial Content" },
      {207, "Multi-Status" },
      {226, "IM Used" },
      {300, "Multiple Choices" },
      {301, "Moved Permanently" },
      {302, "Found" },
      {303, "See Other" },
      {304, "Not Modified" },
      {305, "Use Proxy" },
      {306, "Reserved" },
      {307, "Temporary Redirect" },
      {400, "Bad Request" },
      {401, "Unauthorized" },
      {402, "Payment Required" },
      {403, "Forbidden" },
      {404, "Not Found" },
      {405, "Method Not Allowed" },
      {406, "Not Acceptable" },
      {407, "Proxy Authentication Required" },
      {408, "Request Timeout" },
      {409, "Conflict" },
      {410, "Gone" },
      {411, "Length Required" },
      {412, "Precondition Failed" },
      {413, "Request Entity Too Large" },
      {414, "Request-URI Too Long" },
      {415, "Unsupported Media Type" },
      {416, "Requested Range Not Satisfiable" },
      {417, "Expectation Failed" },
      {422, "Unprocessable Entity" },
      {423, "Locked" },
      {424, "Failed Dependency" },
      {426, "Upgrade Required" },
      {500, "Internal Server Error" },
      {501, "Not Implemented" },
      {502, "Bad Gateway" },
      {503, "Service Unavailable" },
      {504, "Gateway Timeout" },
      {505, "HTTP Version Not Supported" },
      {506, "Variant Also Negotiates" },
      {507, "Insufficient Storage" },
      {510, "Not Extended" }
    };

  int iCodes = sizeof(httpStatusCodes) / sizeof(StatusCode);
  const StatusCode searchCode = { code, 0 };
  const StatusCode* pEnd = httpStatusCodes + iCodes;
  const StatusCode* pMatch = qBinaryFind(httpStatusCodes, pEnd, searchCode);

  if (pMatch != pEnd)
    return pMatch->description;
  
  return QString();
}


PiiHttpProtocol::TimeLimiter::TimeLimiter(PiiProgressController* controller, int maxTime) :
  _pController(controller), _iMaxTime(maxTime)
{
  _time.start();
}

bool PiiHttpProtocol::TimeLimiter::canContinue(double progressPercentage)
{
  return _pController->canContinue(progressPercentage) &&
    (_iMaxTime <= 0 || _time.elapsed() < _iMaxTime);
}

void PiiHttpProtocol::TimeLimiter::setMaxTime(int maxTime) { _iMaxTime = maxTime; }
int PiiHttpProtocol::TimeLimiter::maxTime() const { return _iMaxTime; }
