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

#include "PiiNetworkOutputOperation.h"

#include <PiiNetworkClient.h>
#include <PiiHttpDevice.h>
#include <PiiStreamBuffer.h>
#include <PiiYdinTypes.h>
#include <PiiGenericTextOutputArchive.h>

#include <QUrl>

PiiNetworkOutputOperation::Data::Data() :
  pNetworkClient(0),
  requestMethod(PostRequest)
{
}

PiiNetworkOutputOperation::PiiNetworkOutputOperation() :
  PiiNetworkOperation(new Data)
{
}

PiiNetworkOutputOperation::~PiiNetworkOutputOperation()
{
  PII_D;
  delete d->pNetworkClient;
}

void PiiNetworkOutputOperation::check(bool reset)
{
  PII_D;
  PiiNetworkOperation::check(reset);

  if (d->requestMethod == GetRequest && d->bBodyConnected)
    PII_THROW(PiiExecutionException, tr("Cannot send GET requests with a message body."));

  QUrl url(d->strServerUri);
  if (!url.isValid())
    PII_THROW(PiiExecutionException, tr("The supplied server URI is not valid."));
    
  d->strHost = url.host();
  d->strUri = url.path();
    
  delete d->pNetworkClient;
  d->pNetworkClient = new PiiNetworkClient(d->strServerUri);
  d->pNetworkClient->setConnectionTimeout(d->iResponseTimeout);
}

void PiiNetworkOutputOperation::process()
{
  PII_D;
  try
    {
      if (d->requestMethod == PostRequest)
        sendPostRequest();
      else
        sendGetRequest();
    }
  catch (PiiSerializationException& ex)
    {
      PII_THROW(PiiExecutionException, ex.message());
    }
}

void PiiNetworkOutputOperation::sendGetRequest()
{
  PII_D;
  //qDebug("PiiNetworkOutputOperation::sendGetRequest()");
  QIODevice *pSocket = d->pNetworkClient->openConnection();
  if (pSocket == 0)
    {
      if (!d->bIgnoreErrors)
        PII_THROW(PiiExecutionException, tr("Could not open connection to %1.").arg(d->strServerUri));
      return;
    }
  PiiHttpDevice h(pSocket, PiiHttpDevice::Client);
  h.setRequest("GET", d->strUri);
  for (int i=0; i<d->lstInputNames.size(); ++i)
    h.addQueryValue(d->lstInputNames[i], PiiYdin::convertToQString(inputAt(i+d->iStaticInputCount)));
  if (!d->strHost.isEmpty())
    h.setHeader("Host", d->strHost);
  h.finish();

  readResponse(h);

  emitOutputValues();
}

void PiiNetworkOutputOperation::sendPostRequest()
{
  PII_D;
  if (d->bBodyConnected)
    {
      QIODevice *pSocket = d->pNetworkClient->openConnection();
      if (pSocket == 0)
        {
          if (!d->bIgnoreErrors)
            PII_THROW(PiiExecutionException, tr("Could not open connection to %1.").arg(d->strServerUri));
          return;
        }
      PiiHttpDevice h(pSocket, PiiHttpDevice::Client);
      h.setRequest("POST", d->strUri);
      if (!d->strHost.isEmpty())
        h.setHeader("Host", d->strHost);
      h.setHeader("Content-Type",
                  d->bTypeConnected ?
                  PiiYdin::convertToQString(d->pTypeInput) :
                  d->strContentType);

      h.print(PiiYdin::convertToQString(d->pBodyInput));
      
      h.finish();

      readResponse(h);
    }
  else
    {
      for (int i=0; i<d->lstInputNames.size(); ++i)
        {
          QIODevice *pSocket = d->pNetworkClient->openConnection();
          if (pSocket == 0)
            {
              if (!d->bIgnoreErrors)
                PII_THROW(PiiExecutionException, tr("Could not open connection to %1.").arg(d->strServerUri));
              return;
            }
          PiiHttpDevice h(pSocket, PiiHttpDevice::Client);
          h.setRequest("POST", d->strUri);
          if (!d->strHost.isEmpty())
            h.setHeader("Host", d->strHost);
          h.setHeader(PII_CONTENT_NAME_HEADER, d->lstInputNames[i]);
          h.startOutputFiltering(new PiiStreamBuffer);

          PiiVariant obj = inputAt(i+d->iStaticInputCount)->firstObject();
          // Everything but QStrings are serializer
          if (obj.type() != PiiYdin::QStringType)
            {
              h.setHeader("Content-Type", PII_TEXT_ARCHIVE_MIMETYPE);

              PiiGenericTextOutputArchive outputArchive(&h);
              outputArchive << inputAt(i+d->iStaticInputCount)->firstObject();

              // This if block ensures the archive flushes itself to the
              // device before finish().
            }
          // QStrings are just printed
          else
            {
              h.setHeader("Content-Type", "text/plain");
              h.print(obj.valueAs<QString>());
            }
          h.finish();
          
          readResponse(h);
        }
    }

  emitOutputValues();
}

void PiiNetworkOutputOperation::readResponse(PiiHttpDevice& h)
{
  PII_D;
  if (!h.readHeader())
    {
      // Cannot ignore errors if we need outputs
      if (d->bIgnoreErrors && d->lstOutputNames.size() == 0)
        return;
      PII_THROW(PiiExecutionException, tr("Error in reading HTTP response headers."));
    }
  
  if (h.status() != 200)
    {
      // Cannot ignore errors if we need outputs
      if (d->bIgnoreErrors && d->lstOutputNames.size() == 0)
        return;
      PII_THROW(PiiExecutionException, tr("Server responded with status code %1.").arg(h.status()));
    }

  // If we need a response but don't know how to decode it, must
  // signal an error.
  if (d->lstOutputNames.size() > 0 && !decodeObjects(h, h.responseHeader()))
    PII_THROW(PiiExecutionException, tr("Could not decode server response."));
}

void PiiNetworkOutputOperation::setServerUri(const QString& serverUri) { _d()->strServerUri = serverUri; }
QString PiiNetworkOutputOperation::serverUri() const { return _d()->strServerUri; }
void PiiNetworkOutputOperation::setRequestMethod(const RequestMethod& requestMethod) { _d()->requestMethod = requestMethod; }
PiiNetworkOutputOperation::RequestMethod PiiNetworkOutputOperation::requestMethod() const { return _d()->requestMethod; }
