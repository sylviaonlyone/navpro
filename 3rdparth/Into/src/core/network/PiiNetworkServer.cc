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

#include "PiiNetworkServer.h"
#include "PiiNetworkServerThread.h"
#include <QIODevice>

PiiNetworkServer::Data::Data(PiiNetworkProtocol* protocol) :
  iMinWorkers(0), iMaxWorkers(10),
  iWorkerMaxIdleTime(20),
  iMaxPendingConnections(0),
  aBusyMessage("Server busy\n"),
  pProtocol(protocol),
  bRunning(false)
{
}

PiiNetworkServer::Data::~Data()
{
}

PiiNetworkServer::PiiNetworkServer(PiiNetworkProtocol* protocol) :
  d(new Data(protocol))
{
  // Deletes finished threads.
  startTimer(1000);
}

PiiNetworkServer::PiiNetworkServer(Data* data) :
  d(data)
{
}

PiiNetworkServer::~PiiNetworkServer()
{
  stop(PiiNetwork::InterruptClients);
  delete d;
}

bool PiiNetworkServer::start()
{
  QMutexLocker lock(&d->threadListLock);

  if (d->bRunning)
    return true;

  d->lstPendingConnections.clear();
  
  // Add threads to the pool.
  while (d->lstFreeThreads.size() < d->iMinWorkers)
    {
      PiiNetworkServerThread *newWorker = createWorker(d->pProtocol);
      newWorker->setController(this);
      d->lstFreeThreads << newWorker;
    }

  d->bRunning = true;

  return true;
}

void PiiNetworkServer::stop(PiiNetwork::StopMode mode)
{
  d->threadListLock.lock();

  d->bRunning = false;
  // Copy the thread list
  QList<PiiNetworkServerThread*> lstThreads = d->lstAllThreads;
  // Send stop signal to each
  for (int i=0; i<lstThreads.size(); ++i)
    lstThreads[i]->stop(mode);

  d->threadListLock.unlock();
    

  // Wait until all threads are done. We can't use d->lstAllThreads here
  // because it is modified by threadFinished().
  for (int i=0; i<lstThreads.size(); ++i)
    lstThreads[i]->wait();
  deleteFinishedThreads();
}

void PiiNetworkServer::deleteFinishedThreads()
{
  synchronized (d->threadListLock)
    {
      for (int i=0; i<d->lstFinishedThreads.size(); ++i)
        {
          d->lstFinishedThreads[i]->wait();
          delete d->lstFinishedThreads[i];
        }
      d->lstFinishedThreads.clear();
    }
}

void PiiNetworkServer::timerEvent(QTimerEvent*)
{
  deleteFinishedThreads();
}

// Add thread to the list of idle threads.
void PiiNetworkServer::threadAvailable(PiiNetworkServerThread* worker)
{
  QMutexLocker lock(&d->threadListLock);

  if (!d->bRunning) return;

  // If there is a pending connection, start serving it immediately.
  if (d->lstPendingConnections.size() > 0)
    worker->startRequest(d->lstPendingConnections.dequeue());
  else
    // If there are no pending connections, add the thread to the list
    // of free threads.
    d->lstFreeThreads << worker;
}

void PiiNetworkServer::threadFinished(PiiNetworkServerThread* worker)
{
  QMutexLocker lock(&d->threadListLock);

  if (d->lstAllThreads.size() > d->iMinWorkers)
    {
      d->lstFinishedThreads << worker;
      // Take it out of our thread lists.
      d->lstFreeThreads.removeOne(worker);
      d->lstAllThreads.removeOne(worker);
    }
}

void PiiNetworkServer::incomingConnection(PiiGenericSocketDescriptor socketDescriptor)
{
  QMutexLocker lock(&d->threadListLock);

  //qDebug("Connection attempt.");

  if (!d->bRunning) return;

  // If at least one thread is available, use it.
  if (d->lstFreeThreads.size() > 0)
    {
      //qDebug("Picked an idle thread");
      PiiNetworkServerThread *worker = d->lstFreeThreads.takeLast();
      worker->startRequest(socketDescriptor);
    }
  // If no free threads are available, and we are still allowed to
  // create a new one, do it.
  else if (d->lstAllThreads.size() < d->iMaxWorkers)
    {
      //qDebug("Created a new thread");
      // Stateful protocol needs to be cloned to maintain state
      // separately for each client.
      PiiNetworkServerThread *newWorker = createWorker(d->pProtocol);
      newWorker->setController(this);
      newWorker->startRequest(socketDescriptor);
      d->lstAllThreads << newWorker;
    }
  // No more threads, please. Are we allowed to queue pending connections?
  else if (d->lstPendingConnections.size() < d->iMaxPendingConnections)
    {
      //qDebug("Connection pending");
      /* It may happen that a connection is first accepted and placed
         to the list of pending connections. If the client then
         happens to get tired of waiting, it may go away. If the
         server is extremely busy, it may happen that the same socket
         descriptor is allocated to another client. We must thus check
         that the descriptor isn't already in our list.
      */
      if (!d->lstPendingConnections.contains(socketDescriptor))
        d->lstPendingConnections.enqueue(socketDescriptor);
    }
  // No luck this time.
  else
    serverBusy(socketDescriptor);
}

PiiNetworkServerThread* PiiNetworkServer::createWorker(PiiNetworkProtocol* protocol)
{
  return new PiiNetworkServerThread(protocol);
}

void PiiNetworkServer::serverBusy(PiiGenericSocketDescriptor socketDescriptor)
{
  QIODevice* dev = createSocket(socketDescriptor);
  dev->write(d->aBusyMessage);
  delete dev;
}

void PiiNetworkServer::setMinWorkers(int minWorkers) { if (minWorkers >= 0 && minWorkers < 1000) d->iMinWorkers = minWorkers; }
int PiiNetworkServer::minWorkers() const { return d->iMinWorkers; }
void PiiNetworkServer::setMaxWorkers(int maxWorkers) { if (maxWorkers > 0 && maxWorkers < 1000) d->iMaxWorkers = maxWorkers; }
int PiiNetworkServer::maxWorkers() const { return d->iMaxWorkers; }
void PiiNetworkServer::setWorkerMaxIdleTime(int workerMaxIdleTime) { d->iWorkerMaxIdleTime = workerMaxIdleTime; }
int PiiNetworkServer::workerMaxIdleTime() const { return d->iWorkerMaxIdleTime; }
void PiiNetworkServer::setMaxPendingConnections(int maxPendingConnections) { d->iMaxPendingConnections = maxPendingConnections; }
int PiiNetworkServer::maxPendingConnections() const { return d->iMaxPendingConnections; }
void PiiNetworkServer::setBusyMessage(const QString& busyMessage) { d->aBusyMessage = busyMessage.toUtf8(); }
QString PiiNetworkServer::busyMessage() const { return QString::fromUtf8(d->aBusyMessage.constData(), d->aBusyMessage.size()); }
PiiNetworkProtocol* PiiNetworkServer::protocol() const { return d->pProtocol; }
