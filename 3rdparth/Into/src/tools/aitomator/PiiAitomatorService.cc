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

#include "PiiAitomatorService.h"
#include <PiiException.h>
#include <PiiLog.h>

#include <QCoreApplication>
#include <QScriptValueIterator>
#include <QDir>
#include <PiiYdinUtil.h>

PiiAitomatorService::PiiAitomatorService(int argc, char** argv) :
  QtService<QCoreApplication>(argc, argv, "PiiAitomatorService"),
  _pScriptEngine(0),
  _pHttpServer(0)
{
  setServiceDescription("Exposes Aitomates to the net.");
  setStartupType(QtServiceController::ManualStartup);
}

PiiAitomatorService::~PiiAitomatorService()
{
  stop();
  delete _pScriptEngine;
}

void PiiAitomatorService::start()
{
  if (_pScriptEngine == 0)
    {
      _pScriptEngine = new QScriptEngine;
      QScriptValue result = _pScriptEngine->importExtension("Into");
      if (!result.isUndefined())
        piiFatal("Couldn't import Into script extension.");
    }
  
  QStringList lstArgs = QCoreApplication::arguments();
  QString strConfigFile("aitomator.js"), strLogFile;
  bool bConsole = false;
  
  for (int i=1; i<lstArgs.size(); ++i)
    {
      if (lstArgs[i] == "-e")
        bConsole = true;
      else if (lstArgs[i] == "-f" && i<lstArgs.size()-1)
        {
          strConfigFile = lstArgs[++i];
          continue;
        }
      else if (lstArgs[i] == "-l" && i<lstArgs.size()-1)
        {
          strLogFile = lstArgs[++i];
          continue;
        }
    }

  if (strLogFile.isEmpty() && !bConsole)
    {
      strLogFile = QCoreApplication::applicationDirPath();
      strLogFile.replace(QRegExp("[/\\\\](debug|release)$"),"");
      strLogFile += "/aitomator.log";
    }

  if (strLogFile.isEmpty())
    qInstallMsgHandler(0);
  else
    {
      qInstallMsgHandler(PiiLog::writeToFile);
      PiiLog::setLogFile(strLogFile);
    }

  QFile file(strConfigFile);
  if (!file.open(QIODevice::ReadOnly))
    piiFatal("Cannot open %s for reading.", piiPrintable(strConfigFile));

  stop();

  QScriptValue result = _pScriptEngine->evaluate(file.readAll(), strConfigFile);

  if (result.isError())
    piiFatal(QString("%1 at line %2.").arg(result.toString()).arg(_pScriptEngine->uncaughtExceptionLineNumber()));

  result = _pScriptEngine->globalObject();
  
  QScriptValue server = result.property("server");
  if (!server.isObject())
    piiFatal("No server configuration found in the configuration file.");

  QString strAddress = server.property("serverAddress").toString();
  _pHttpServer = PiiHttpServer::addServer("Aitomator", strAddress);
  if (_pHttpServer == 0)
    piiFatal(QString("Unable to create a HTTP server at %1.").arg(strAddress));

  QScriptValue handlers = result.property("handlers");
  if (!handlers.isObject())
    piiFatal("The 'handlers' configuration item must be an object.");
  QScriptValueIterator it(handlers);
  while (it.hasNext())
    {
      it.next();
      QScriptValue handler = it.value();
      QString strUri(it.name());
      PiiHttpProtocol::UriHandler* pHandler = qobject_cast<PiiHttpProtocol::UriHandler*>(handler.toQObject());
      if (pHandler == 0)
        piiFatal(QString("Cannot add %1: invalid handler.").arg(strUri));
      if (strUri.size() < 2 || strUri[0] != '/')
        piiWarning(QString("Invalid URI: %1").arg(strUri));
      _pHttpServer->protocol()->registerUriHandler(it.name(), pHandler);
      _lstHandlers << pHandler;
    }
  
  _pHttpServer->start();
}

void PiiAitomatorService::stop()
{
  if (_pHttpServer != 0)
    {
      _pHttpServer->stop(PiiNetwork::InterruptClients);
      PiiHttpServer::removeServer("Aitomator");
      _pHttpServer = 0;
    }
  qDeleteAll(_lstHandlers);
  _lstHandlers.clear();
}

void PiiAitomatorService::pause()
{
}

void PiiAitomatorService::resume()
{
}

void PiiAitomatorService::processCommand(int /*code*/)
{
}
