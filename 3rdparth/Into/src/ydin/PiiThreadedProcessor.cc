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

#include "PiiDefaultOperation.h"
#include "PiiThreadedProcessor.h"

PiiThreadedProcessor::PiiThreadedProcessor(PiiDefaultOperation* parent) :
  PiiOperationProcessor(parent, PiiDefaultOperation::Threaded),
  _inputCondition(PiiWaitCondition::NoQueue), _priority(InheritPriority),
  _pStateMutex(parent->stateLock())
{
  // Set state to stopped once the thread finishes execution
  // DirectConnection ensures that the we don't need to run an event loop.
  connect(this, SIGNAL(finished()), SLOT(setStopped()), Qt::DirectConnection);
}

void PiiThreadedProcessor::setProcessingPriority(QThread::Priority priority)
{
  _priority = priority;
  if (isRunning())
    setPriority(priority);
}

QThread::Priority PiiThreadedProcessor::processingPriority() const
{
  return _priority;
}

void PiiThreadedProcessor::setStopped()
{
  //qDebug("%s: Thread stopped. State is now %s. Signalling.", _pParentOp->metaObject()->className(), PiiOperation::stateName(_pParentOp->state()));
  // The runner has finished. Our state is (or at least should be)
  // Stopping now
  _pStateMutex->lock();
  // Change to Stopped state.
  _pParentOp->setState(PiiOperation::Stopped);
  _pStateMutex->unlock();
}

void PiiThreadedProcessor::check(bool reset)
{
  // If the processor is being reset, we clear any pending signals.
  if (reset)
    _inputCondition.wakeAll();
}

void PiiThreadedProcessor::start()
{
  QMutexLocker lock(_pStateMutex);

  // If the state is Stopped, the thread must be started again. Once
  // it starts, the state will be changed to Running.
  if (_pParentOp->state() == PiiOperation::Stopped)
    {
      _pParentOp->setState(PiiOperation::Starting);
      // Start the thread
      QThread::start(_priority);
    }
  // If the state is Paused, the thread is just suspended and does not
  // need to be restarted.
  else if (_pParentOp->state() == PiiOperation::Paused)
    {
      // Wake it up.
      _inputCondition.wakeOne();
    }
}

void PiiThreadedProcessor::interrupt()
{
  {
    QMutexLocker lock(_pStateMutex);
    switch (_pParentOp->state())
      {
      case PiiOperation::Running:
      case PiiOperation::Paused:
      case PiiOperation::Starting:
      case PiiOperation::Pausing:
      case PiiOperation::Stopping:
        _pParentOp->setState(PiiOperation::Interrupted);
        break;
      case PiiOperation::Stopped:
      case PiiOperation::Interrupted:
        return;
      }
  }

  // This ensures the signal is handled even if runner is waiting for
  // input.
  _inputCondition.wakeOne();
}

void PiiThreadedProcessor::pause()
{
  QMutexLocker lock(_pStateMutex);
  if (_pParentOp->state() != PiiOperation::Running)
    return;

  _pParentOp->setState(PiiOperation::Pausing);
}

void PiiThreadedProcessor::stop()
{
  QMutexLocker lock(_pStateMutex);
  if (_pParentOp->state() != PiiOperation::Running)
    return;

  _pParentOp->setState(PiiOperation::Stopping);
}

bool PiiThreadedProcessor::wait(unsigned long time)
{
  return QThread::wait(time);
}

bool PiiThreadedProcessor::tryToReceive(PiiAbstractInputSocket* sender,
                                        const PiiVariant& object) throw ()
{
  QMutexLocker inputLock(_pStateMutex);
  PiiInputSocket* pInput = static_cast<PiiInputSocket*>(sender);
  if (pInput->canReceive())
    {
      pInput->receive(object);
      // Send a signal to start the next round of processing and
      // return immediately.
      _inputCondition.wakeOne();
      return true;
    }

  return false;
}

void PiiThreadedProcessor::prepareAndProcess()
{
  // This lock ensures that no input socket is able to take in objects
  // for a while. They are thus unable to signal objectAccepted(),
  // which means that _inputCondition.wakeOne() won't be called. Since
  // we are going to process any object received so far, it is safe to
  // reset the input condition.
  QMutexLocker lock(_pStateMutex);
  while (true)
    {
      _inputCondition.wakeAll();
      //qDebug("%s: calling flow controller", qPrintable(_pParentOp->objectName()));
      PiiFlowController::FlowState state = _pFlowController->prepareProcess(); // may throw
      //qDebug("%s: flow controller returned %d", qPrintable(_pParentOp->objectName()), int(state));
      if (state == PiiFlowController::IncompleteState)
        return;

      lock.unlock();

      _pParentOp->sendSyncEvents(_pFlowController);

      switch (state)
        {
        case PiiFlowController::ProcessableState:
          _pParentOp->processLocked();
        case PiiFlowController::SynchronizedState:
        case PiiFlowController::IncompleteState:
          break;
        case PiiFlowController::PausedState:
          _pParentOp->operationPaused(); // throws
        case PiiFlowController::FinishedState:
          _pParentOp->operationStopped(); // throws
        case PiiFlowController::ResumedState:
          _pParentOp->operationResumed(); // may throw
          break;
        }
      lock.relock();
    }
}

void PiiThreadedProcessor::run()
{
  //qDebug("%s running in thread %d",
  //       _pParentOp->metaObject()->className(),
  //       static_cast<int>(currentThreadId()));

  synchronized (_pStateMutex)
    {
      // State may have changed before we could even start. In such a
      // case we won't turn into Running.
      if (_pParentOp->state() == PiiOperation::Starting)
        _pParentOp->setState(PiiOperation::Running);
    }

  // Run the loop until we get an interrupt signal.
  while (_pParentOp->state() != PiiOperation::Interrupted)
    {
      try
        {
          // If a flow controller exists, we wait until an object is
          // received.
          if (_pFlowController != 0)
            {
              _inputCondition.wait();
              // If the waiting was terminated by interrupt(), kill
              // the thread.
              if (_pParentOp->state() == PiiOperation::Interrupted)
                return;

              prepareAndProcess();
            }
          // If no input is connected, just process without waiting
          // for input.
          else
            {
              _pParentOp->processLocked();

              // If this operation is a producer (not driven by input), it
              // needs to pause spontaneously
              if (_pParentOp->state() == PiiOperation::Pausing)
                _pParentOp->operationPaused(); // throws
              else if (_pParentOp->state() == PiiOperation::Stopping)
                _pParentOp->operationStopped(); // throws
            }
        }
      catch (PiiExecutionException& ex)
        {
          _pStateMutex->lock();
          // If the operation was paused, we change its state to
          // Paused.
          if (ex.code() == PiiExecutionException::Paused)
            {
              _pParentOp->setState(PiiOperation::Paused);
              _pStateMutex->unlock();
              // Suspend the thread.
              _inputCondition.wait();

              // Suspension finished. Now change state back to
              // running. But only if we haven't been interrupted...
              _pStateMutex->lock();
              if (_pParentOp->state() == PiiOperation::Paused)
                {
                  // If there are no connected inputs, we are a
                  // producer and must send resume tags immediately.
                  if (_pFlowController == 0)
                    {
                      try { _pParentOp->operationResumed(); } catch (...) {}
                    }
                  else
                    // Ensure that input queues will be emptied if
                    // something was there before pause.
                    _inputCondition.wakeOne();
                  
                  _pParentOp->setState(PiiOperation::Running);
                }
              _pStateMutex->unlock();
            }
          // Any other reason will cause termination.
          else
            {
              _pParentOp->setState(PiiOperation::Stopping);
              _pStateMutex->unlock();
              //qDebug("%s caused a %s exception. Message: %s", _pParentOp->metaObject()->className(),
              //       PiiExecutionException::errorName(ex.code()), qPrintable(ex.message()));
              // If an error occured, signal the error message
              if (ex.code() == PiiExecutionException::Error)
                emit _pParentOp->errorOccured(_pParentOp, ex.message());
              // Stop the thread.
              return;
            }
        }
    } // while (state != Interrupted)
}
