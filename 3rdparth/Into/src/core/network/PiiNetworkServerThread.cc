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

#include "PiiNetworkServerThread.h"
#include <QIODevice>
#include <QCoreApplication>

PiiNetworkServerThread::Data::Data(PiiNetworkProtocol* protocol) :
  pProtocol(protocol->clone()),
  bOwnProtocol(protocol != pProtocol),
  pController(0),
  requestCondition(PiiWaitCondition::Queue),
  bRunning(false), bInterrupted(false),
  iMaxIdleTime(10000)
{
}

PiiNetworkServerThread::PiiNetworkServerThread(PiiNetworkProtocol* protocol) :
  d(new Data(protocol))
{
}

PiiNetworkServerThread::~PiiNetworkServerThread()
{
  stop();
  wait();
  
  if (d->bOwnProtocol)
    delete d->pProtocol;
}

void PiiNetworkServerThread::startRequest(PiiGenericSocketDescriptor socketDescriptor)
{
  d->socketDescriptor = socketDescriptor;

  // If the thread is not running, start it.
  if (!d->bRunning)
    {
      d->requestCondition.wakeAll();
      d->bRunning = true;
      d->bInterrupted = false;
      start();
    }

  // Wake it up.
  d->requestCondition.wakeOne();
}

void PiiNetworkServerThread::stop(PiiNetwork::StopMode mode)
{
  d->bRunning = false;
  d->bInterrupted = mode == PiiNetwork::InterruptClients;
  d->requestCondition.wakeOne();
}

bool PiiNetworkServerThread::canContinue(double /*percentage*/)
{
  return !d->bInterrupted;
}

void PiiNetworkServerThread::run()
{
  while (d->bRunning)
    {
      // Wait for a wake-up signal. If we got no new clients within
      // the time limit, kill the thread.
      if (!d->requestCondition.wait(d->iMaxIdleTime))
        break;
      
      if (!d->bRunning)
        break;

      QIODevice* pSocket = d->pController->createSocket(d->socketDescriptor);
      if (pSocket != 0)
        {
          d->pProtocol->communicate(pSocket, this);
          delete pSocket;
        }

      // We are done with the client. Tell mama.
      d->pController->threadAvailable(this);
    }

  d->pController->threadFinished(this);
}

void PiiNetworkServerThread::setController(Controller* controller) { d->pController = controller; }
PiiNetworkServerThread::Controller* PiiNetworkServerThread::controller() const { return d->pController; }
void PiiNetworkServerThread::setMaxIdleTime(int maxIdleTime) { d->iMaxIdleTime = maxIdleTime; }
int PiiNetworkServerThread::maxIdleTime() const { return d->iMaxIdleTime; }
