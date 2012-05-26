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

#ifndef _PIIIOTHREAD_H
#define _PIIIOTHREAD_H

#include <QMutex>
#include <QThread>
#include <PiiIoChannel.h>
#include <QVector>

class PiiIoThread : public QThread
{

  Q_OBJECT
    
public:
  struct OutputSignal
  {
    bool handled;
    PiiIoChannel *channel;
    bool active;
    int day;
    int msecs;
    int pulseWidth;
  };
  
  PiiIoThread(QObject *parent = 0);
  
  void run();
  void stop() { _bRunning = false; }

  void sendSignal(PiiIoChannel *channel, bool value, int timestamp, int msecs, int pulseWidth);

  void addPollingInput(PiiIoChannel *input);
  void removePollingInput(PiiIoChannel *input);

  /**
   * Handle and remove all outputs from the _lstWaitingOutputSignals
   * depends on given parameter lstChannels.
   */
  void removeOutputList(const QVector<PiiIoChannel*>& lstChannels);

private:
  bool checkTime(const OutputSignal& stru, int day, int msecs);
  void addNewStruct(PiiIoChannel *channel, bool active, int day, int msecs, int width);

  bool _bRunning;
  QMutex _mutex;
  QList<OutputSignal> _lstWaitingOutputSignals;
  QList<PiiIoChannel*> _lstPollingInputs;
};

#endif //_PIIIOTHREAD_H
