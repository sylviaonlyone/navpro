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

#include "PiiNetworkInputOperation.h"

#include <QMutexLocker>
#include <PiiHttpDevice.h>
#include <PiiHttpServer.h>
#include <PiiStreamBuffer.h>
#include <PiiMultipartStreamBuffer.h>
#include <PiiYdinTypes.h>
#include <PiiOneGroupFlowController.h>
#include <PiiGenericTextOutputArchive.h>
#include <PiiLog.h>

PiiNetworkInputOperation::Data::Data() :
  pServer(0),
  bOwnServer(false),
  bNeedToWaitResponse(false),
  bStatusConnected(false),
  iStatusCode(200),
  strInterruptedResponse("The operation was interrupted."),
  strTimeoutResponse("Timed out while waiting for response")
{
}

PiiNetworkInputOperation::PiiNetworkInputOperation() :
  PiiNetworkOperation(new Data)
{
  PII_D;
  addSocket(d->pStatusInput = new PiiInputSocket("status"));
  d->pStatusInput->setOptional(true);

  ++d->iStaticInputCount;
  
  setProcessingMode(NonThreaded);
}

PiiNetworkInputOperation::~PiiNetworkInputOperation()
{
  destroyServer();
}

void PiiNetworkInputOperation::destroyServer()
{
  PII_D;
  // Unregister old location, if any.
  if (d->pServer != 0)
    d->pServer->protocol()->unregisterUriHandler(this);
  if (d->bOwnServer)
    {
      d->pServer->stop(PiiNetwork::InterruptClients);
      delete d->pServer;
      d->pServer = 0;
    }
}

void PiiNetworkInputOperation::stop()
{
  PII_D;
  QMutexLocker lock(&d->requestLock);
  if (d->state == Running)
    {
      setState(d->bNeedToWaitResponse ? Stopping : Stopped);
      sendTag(PiiYdin::createStopTag());
    }  
}

void PiiNetworkInputOperation::pause()
{
  PII_D;
  QMutexLocker lock(&d->requestLock);
  if (d->state == Running)
    {
      // If response is connected, we pause after the feedback loop is
      // done.
      setState(d->bNeedToWaitResponse ? Pausing : Paused);
      sendTag(PiiYdin::createPauseTag());
    }
}

void PiiNetworkInputOperation::check(bool reset)
{
  PII_D;
  PiiNetworkOperation::check(reset);

  d->bNeedToWaitResponse = d->bBodyConnected || d->lstInputNames.size() > 0;
  
  if (d->lstOutputNames.size() == 0)
    PII_THROW(PiiExecutionException, tr("At least one output must be configured."));
  
  QString strUri = d->strUri.isEmpty() ? "/" + objectName() : d->strUri;
  if (strUri.isEmpty())
    PII_THROW(PiiExecutionException, tr("The URI cannot be empty."));

  destroyServer();
  d->pServer = PiiHttpServer::server(d->strHttpServer);
  if (d->pServer == 0)
    {
      d->pServer = PiiHttpServer::createServer(d->strHttpServer);
      d->bOwnServer = true;
      if (d->pServer == 0)
        PII_THROW(PiiExecutionException, tr("There is no HTTP server called \"%1\" and cannot create one.").arg(d->strHttpServer));
      if (!d->pServer->start())
        PII_THROW(PiiExecutionException, tr("Cannot start server at \"%2\".").arg(d->strHttpServer));
    }
  else
    d->bOwnServer = false;

  d->pServer->protocol()->registerUriHandler(strUri, this);

  d->strCurrentContentType = d->strContentType;
  d->strResponseData = "";
  d->iStatusCode = 200;
  d->bStatusConnected = d->pStatusInput->isConnected();
}

void PiiNetworkInputOperation::process()
{
  PII_D;
  if (d->bBodyConnected)
    {
      d->strResponseData = PiiYdin::convertToQString(d->pBodyInput);
      if (d->bTypeConnected)
        d->strCurrentContentType = PiiYdin::convertToQString(d->pTypeInput);
    }
  else
    {
      d->lstResponseValues.clear();
      for (int i=0; i<d->lstInputNames.size(); ++i)
        d->lstResponseValues << inputAt(i+d->iStaticInputCount)->firstObject();
    }

  if (d->bStatusConnected)
    d->iStatusCode = PiiYdin::primitiveAs<int>(d->pStatusInput);

  d->responseCondition.wakeOne();
}

void PiiNetworkInputOperation::handleRequest(const QString& /*uri*/,
                                             PiiHttpDevice* h,
                                             PiiHttpProtocol::TimeLimiter* controller)
{
  PII_D;
  if (state() != Running)
    {
      h->setStatus(503); // service unavailable
      return;
    }

  QMutexLocker lock(&d->requestLock);

  try
    {
      //qDebug(qPrintable(h->requestHeader().toString()));
      // Parse request body
      if (h->requestMethod() == "POST" &&
          !decodeObjects(*h, h->requestHeader()))
        {
          if (!d->bIgnoreErrors)
            PII_THROW(PiiExecutionException, tr("Client sent an invalid request."));
          h->setStatus(422); // Unprocessable entity
          return;
        }
      // Add query values (GET parameters)
      addToOutputMap(h->queryValues());

      //qDebug() << d->mapOutputValues;

      // All objects are here
      if (d->mapOutputValues.size() >= d->lstOutputNames.size())
        {
          d->responseCondition.wakeAll();
          emitOutputValues();

          bool bTimedOut = false;
          
          // If there are connected inputs, must wait for response now
          if (d->bNeedToWaitResponse)
            {
              QTime t;
              t.start();
              while ((d->state == Running || d->state == Pausing) &&
                     h->isWritable() && controller->canContinue())
                {
                  if (t.elapsed() > d->iResponseTimeout)
                    {
                      bTimedOut = true;
                      break;
                    }
                  if (d->responseCondition.wait(100))
                    {
                      replyToClient(h);
                      return;
                    }
                }
            }

          if (h->isWritable())
            {
              // The operation is being stopped, but client is connected
              if (d->state == Stopping || d->state == Stopped || d->state == Interrupted)
                {
                  h->setStatus(500); // internal server error
                  h->print(d->strInterruptedResponse);
                }
              else if (bTimedOut)
                {
                  h->setStatus(500);
                  h->print(d->strTimeoutResponse);
                }
            }
        }
    }
  catch (PiiSerializationException& ex)
    {
      h->setStatus(422); // Unprocessable entity
      h->print(ex.message());
      piiWarning(ex.message());
      piiWarning(ex.info());
      return;
    }
  catch (PiiException& ex)
    {
      h->setStatus(422); // Unprocessable entity
      h->print(ex.message());
      piiWarning(ex.message());
      return;
    }
}

void PiiNetworkInputOperation::replyToClient(PiiHttpDevice* h)
{
  PII_D;
  h->setStatus(d->iStatusCode);
  
  if (d->bBodyConnected)
    {
      h->startOutputFiltering(new PiiStreamBuffer);
      h->setHeader("Content-Type", d->strCurrentContentType);
      h->print(d->strResponseData);
    }
  // Only one input -> serialize a single object
  else if (d->lstInputNames.size() == 1)
    {
      h->startOutputFiltering(new PiiStreamBuffer);
      h->setHeader(PII_CONTENT_NAME_HEADER, d->lstInputNames[0]);
      
      // Everything but QStrings are marshalled with the standard
      // serialization mechanism.
      if (d->lstResponseValues[0].type() != PiiYdin::QStringType)
        {
          h->setHeader("Content-Type", PII_TEXT_ARCHIVE_MIMETYPE);
          
          PiiGenericTextOutputArchive outputArchive(h);
          outputArchive << d->lstResponseValues[0];
        }
      // QStrings are just printed as such.
      else
        {
          h->setHeader("Content-Type", "text/plain");
          h->print(d->lstResponseValues[0].valueAs<QString>());
        }
    }
  else
    {
      QString strBoundary("243F6A8885A308D31319");
      h->setHeader("Content-Type", "multipart/mixed; boundary=\"" + strBoundary + "\"");
      
      for (int i=0; i<d->lstResponseValues.size(); ++i)
        {
          PiiMultipartStreamBuffer* bfr = new PiiMultipartStreamBuffer(strBoundary);
          bfr->setHeader(PII_CONTENT_NAME_HEADER, d->lstInputNames[i]);
          bfr->setHeader("Content-Type", PII_TEXT_ARCHIVE_MIMETYPE);
          h->startOutputFiltering(bfr);
          PiiGenericTextOutputArchive outputArchive(h);
          outputArchive << d->lstResponseValues[i];
          h->endOutputFiltering();
          if (!h->isWritable())
            {
              if (!d->bIgnoreErrors)
                PII_THROW(PiiExecutionException, tr("Replying to a client failed."));
              return;
            }
        }
      h->print("\r\n--" + strBoundary + "--\r\n");
    }
  d->lstResponseValues.clear();
}

void PiiNetworkInputOperation::setHttpServer(const QString& httpServer) { _d()->strHttpServer = httpServer; }
QString PiiNetworkInputOperation::httpServer() const { return _d()->strHttpServer; }
void PiiNetworkInputOperation::setUri(const QString& uri) { _d()->strUri = uri; }
QString PiiNetworkInputOperation::uri() const { return _d()->strUri; }
void PiiNetworkInputOperation::setInterruptedResponse(const QString& interruptedResponse) { _d()->strInterruptedResponse = interruptedResponse; }
QString PiiNetworkInputOperation::interruptedResponse() const { return _d()->strInterruptedResponse; }
void PiiNetworkInputOperation::setTimeoutResponse(const QString& timeoutResponse) { _d()->strTimeoutResponse = timeoutResponse; }
QString PiiNetworkInputOperation::timeoutResponse() const { return _d()->strTimeoutResponse; }
