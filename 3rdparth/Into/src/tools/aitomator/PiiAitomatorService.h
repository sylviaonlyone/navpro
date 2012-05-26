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

#ifndef _PIIAITOMATORSERVICE_H
#define _PIIAITOMATORSERVICE_H

#include <QtServiceBase>
#include <QObject>
#include <QCoreApplication>
#include <QScriptEngine>

#include <PiiHttpServer.h>

/**
 * PiiAitomatorService is a HTTP service/daemon whose main purpose is
 * to expose Into operations (a.k.a Aitomates) to the network.
 *
 * @par Settings
 *
 * Aitomator uses JavaScript as the configuration file format.
 *
@verbatim
importExtension('Kanta')

server =
{
  serverAddress: 'tcp://0.0.0.0:3142',
  minWorkers: 10,
  maxWorkers: 100
}

handlers =
{
  // A "black box" analyzer: image in, results out
  '/pinta/': new KantaRequestHandler(PiiEngine.load('pintaclassifier.cft')),

  // Samples are stored in a database.
  // If modifications to the database are allowed,
  // features and classifier must be specified separately.
  '/matcher/': new KantaRequestHandler
    ({
      features: new PiiShapeContextOperation(),
      classifier: new PiiRigidPlaneMatcher(),
      minAnalyzers: 7,
      database: 'psql://user:passwd@host:port/db'
    }),

  // Remote slaves
  '/distributed/' : new KantaRequestHandler
    ({
      features: new PiiRemoteOperationClient('tcp://localhost:3142/features/'),
      classifier: new PiiRemoteOperationClient('tcp://localhost:3142/classifier/'),
      database: 'psql://user:passwd@host:port/db'
    }),

  // A slave that uses the PiiRemoteOperationServer interface directly.
  '/features/': new PiiRemoteOperationServer(new PiiShapeContextOperation()),

  // A slave that uses the PiiRemoteOperationServer interface for communication
  // and takes the samples from a database.
  '/classifier/': new KantaRequestHandler
    ({
      classifier: new PiiRigidPlaneMatcher(),
      database: 'psql://user:passwd@host:port/db'
      interface: 'into'
    })
}
@endverbatim
 */
class PiiAitomatorService : public QObject, public QtService<QCoreApplication>
{
  Q_OBJECT
  
public:
  PiiAitomatorService(int argc, char** argv);
  ~PiiAitomatorService();

protected:
  void start();
  void stop();
  void pause();
  void resume();
  void processCommand(int code);
  
private:
  void startEngine();
  void stopEngine();
  void pauseEngine();
  void replaceEngine();
  void startServer();
  void stopServer();
  
  QScriptEngine *_pScriptEngine;
  PiiHttpServer *_pHttpServer;
  QList<PiiHttpProtocol::UriHandler*> _lstHandlers;
};

#endif //_PIIAITOMATORSERVICE_H
