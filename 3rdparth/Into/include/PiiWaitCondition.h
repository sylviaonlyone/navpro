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

#ifndef _PIIWAITCONDITION_H
#define _PIIWAITCONDITION_H

#include <QWaitCondition>
#include <QMutex>
#include "PiiGlobal.h"

/**
 * Provides waiting/waking conditions between two threads. The
 * difference between this class and QWaitCondition is in that this
 * class ensures that the wake signals are not missed. If no thread is
 * waiting, the wakeOne() call sets a flag that causes a subsequent
 * wait() call to terminate immediately.
 *
 * The PiiWaitCondition class works in two modes. By default,
 * wakeOne() calls are queued so that the same number of subsequent
 * wait() calls immediately terminate. In the other mode, only the
 * first wait() call after one or more wakeOne() calls with no threads
 * waiting will terminate immediately.
 *
 * The following example is ripped off and modified from QT docs. In
 * this example, thread 1 may be in doSomething() while thread 2
 * signals more characters read. The signals build up a queue which
 * causes the same number of subsequent wait() calls in thread 1 to
 * return immediately.
 *
 * @code
 * PiiWaitCondition keyPressed;
 *
 * //thread 1
 * for (;;)
 *   {
 *     keyPressed.wait(); // This is a PiiWaitCondition global variable
 *     // Key was pressed, do something interesting
 *     doSomething();
 *   }
 *  
 *
 * //thread 2
 * for (;;)
 *   {
 *     getchar();
 *     // Causes any thread in keyPressed.wait() to return from
 *     // that function and continue processing
 *     keyPressed.wakeOne();
 *   }
 * @endcode
 *
 * @ingroup Ydin
 */
class PII_CORE_EXPORT PiiWaitCondition
{
public:
  /**
   * Signalling modes.
   *
   * @li @p NoQueue - the first wait() call after many wakeOne() calls
   * terminates immediately.
   *
   * @li @p Queue - wakeOne() signals are queued when no thread is
   * waiting so that the same number of subsequent wait() calls
   * immediately terminate
   */
  enum QueueMode { NoQueue, Queue };
  
  /**
   * Construct a new wait condition object.
   *
   * @param mode the signalling mode 
   */
  PiiWaitCondition(QueueMode mode = NoQueue);
  
  /**
   * Wait for a wakeOne() call from another thread. The thread calling
   * this will block if there is no pending wake event until another
   * thread calls wake or time milliseconds has elapsed. If time is
   * ULONG_MAX (the default), then the wait will never timeout (the
   * event must be signalled).
   *
   * @param time the time to wait for a wake event
   *
   * @return true if wake was called and false if the wait timed out
   */
  bool wait(unsigned long time = ULONG_MAX);

  /**
   * Wake one of the threads currently waiting on this condition. If
   * there are no waiting threads, a flag will be set that causes the
   * next wait call to return immediately. If multiple wakeOne calls
   * occur when no thread is waiting, the calls will either build up a
   * queue or set a flag that causes the next wait call to terminate
   * immediately.
   */
  void wakeOne();

  /**
   * Wake all threads currently waiting on this condition. This
   * function ensures that all waiting threads are released, but does
   * not build the signal queue. If there are any pending signals,
   * they will be cleared.
   */
  void wakeAll();

  /**
   * Get the queuing mode.
   */
  QueueMode queueMode() const { return _bQueue ? Queue : NoQueue; }

  /**
   * Get the number of wakeOne() signals currently in queue.
   */
  unsigned int queueLength() const { return _iWakeSignals; }

  /**
   * Get the number of threads currently waiting on the condition.
   */
  unsigned int waiterCount() const { return _iWaiters; }
  
private:
  bool _bQueue;
  //number of threads waiting
  unsigned int _iWaiters;
  //number of wake signals in queue
  unsigned int _iWakeSignals;
  
  QWaitCondition _condition;
  QMutex _mutex;
};

#endif //_PIIWAITCONDITION_H
