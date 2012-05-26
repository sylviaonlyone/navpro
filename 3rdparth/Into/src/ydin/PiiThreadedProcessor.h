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

#ifndef _PIITHREADEDPROCESSOR_H
#define _PIITHREADEDPROCESSOR_H

#include "PiiOperationProcessor.h"
#include <QThread>

class QMutex;

/**
 * An implementation of a processor that runs in a separate thread. 
 * PiiThreadedProcessor creates a new thread when its start() function
 * is called. The thread sleeps when no data is available in inputs
 * and is awakened by each incoming object.
 *
 * @internal
 * @ingroup Ydin
 */
class PiiThreadedProcessor : public QThread, public PiiOperationProcessor
{
  Q_OBJECT
  
public:
  /**
   * Construct a new PiiThreadedProcessor.
   */
  PiiThreadedProcessor(PiiDefaultOperation* parent);
  
  void check(bool reset);
  
  /**
   * Execute the processor until interruption. This method starts a
   * new thread that sleeps until new objects appear on connected
   * inputs (if any). The process() method is called whenever all
   * connected inputs are ready to be read.
   *
   * If the processor is a producer that has no connected inputs, the
   * default implementation causes it to output objects as quickly as
   * the synchronous inputs at the receiving ends allow.
   */
  void start();

  /**
   * Sets the state to @p Interrupted. The state changes to @p Stopped
   * once the processor stops running.
   */
  void interrupt();

  /**
   * Sets the state to @p Pausing.
   */
  void pause();

  /**
   * Sets the state to @p Stopping.
   */
  void stop();

  /**
   * Waits until the thread is finished.
   */
  bool wait(unsigned long time = ULONG_MAX);

  /**
   * Invoked when a new object appears on any input socket. This
   * function just signals the runner thread that new data is
   * available.
   */
  bool tryToReceive(PiiAbstractInputSocket* sender, const PiiVariant& object) throw ();

  void setProcessingPriority(QThread::Priority priority);
  QThread::Priority processingPriority() const;

protected:
  void run();

private slots:
  void setStopped();

private:
  inline void prepareAndProcess();

  PiiWaitCondition _inputCondition;
  Priority _priority;
  QMutex *_pStateMutex;
};

#endif // _PIITHREADEDPROCESSOR_H
