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

#ifndef _PIIDEFAULTOPERATION_H
#define _PIIDEFAULTOPERATION_H

#include <QList>
#include <QStringList>
#include <PiiReadWriteLock.h>
#include "PiiBasicOperation.h"
#include "PiiFlowController.h"

class PiiOperationProcessor;

/**
 * A functional implementation of the PiiOperation interface. This
 * class provides facilities for synchronizing and processing incoming
 * objects adding and an execution mechanism for serial and parallel
 * processing.
 *
 * PiiDefaultOperation delegates the task of handling synchronization
 * issues to PiiFlowController. It uses different flow controllers for
 * different operations to optimize the performance of passing
 * objects. The processing of accepted objects is controlled by
 * PiiOperationProcessor, which uses the flow controller to check
 * processing preconditions and invokes the @ref process() function of
 * PiiDefaultOperation whenever needed. PiiOperationProcessor
 * subclasses determine the actual execution mechanism (threaded or
 * not).
 *
 * It is possible to change the execution mechanism at run time, but
 * only if the operation is in @p Stopped state.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiDefaultOperation :
  public PiiBasicOperation,
  protected PiiFlowController::SyncListener
{
  Q_OBJECT

  /**
   * Processing mode. PiiDefaultOperation can be run in either its own
   * thread (@p Threaded) or in the thread of the operation that sends
   * data to its inputs (@p NonThreaded). Note that the processing
   * mode can only be changed when the operation is either stopped or
   * paused, and only before #check(). Setting the value in other
   * situations has no effect. Furthermore, some derived operations
   * may disable processing mode changes altogether.
   */
  Q_PROPERTY(ProcessingMode processingMode READ processingMode WRITE setProcessingMode);
  Q_ENUMS(ProcessingMode);

  /**
   * The priority of the operation in @p Threaded processing mode. 
   * Threaded operations with a high priority are more likely to be
   * scheduled for execution than those with a low priority. Use
   * QThread::Priority as the value for this property. The default
   * value is QThread::NormalPriority. Note that this value has no
   * effect on non-threaded operations.
   *
   * Within a single processing pipeline, priority is meaningful only
   * for asynchronous operations that may discard some of their input
   * objects. Setting priority to a lower value increases the
   * probability of discarding an input object. Priority is useful for
   * synchronous operations if a configuration contains multiple
   * independent processing pipelines.
   *
   * @note Thread priority cannot be changed on Linux.
   */
  Q_PROPERTY(int priority READ priority WRITE setPriority);

public:
  typedef PiiFlowController::SyncEvent SyncEvent;
  
  /**
   * Processing modes.
   *
   * @lip NonThreaded - objects are processed immediately when received in
   * the context of the sending thread.
   *
   * @lip Threaded - processing happens in a separate thread that is
   * awakened when new objects appear.
   */
  enum ProcessingMode { NonThreaded, Threaded };
  
  ~PiiDefaultOperation();
  
  /**
   * Ensures that no property will be set while process() or
   * syncEvent() is being called by acquiring #processLock() for
   * writing. Then sets the property.
   */
  bool setProperty(const char* name, const QVariant& value);

  /**
   * Acquires #processLock() for reading and returns the property.
   */
  QVariant property(const char* name);

  /**
   * Checks the operation for execution. This function creates a
   * suitable flow controller by calling #createFlowController(). It
   * then sets the flow controller to the active @ref
   * PiiOperationProcessor "processor" and sets the processor as the
   * @ref PiiInputController "input controller" for all inputs.
   *
   * If you change socket groupings in your overridden implementation,
   * please call PiiDefaultOperation::check() @b after that. 
   * Otherwise, your new groupings will not be in effect.
   */
  void check(bool reset);

  /**
   * Starts the processor. If #check() has not been called, this
   * function writes out a warning and returns.
   */
  void start();

  /**
   * Interrupts the processor. Does not alter the state of the
   * operation unless the processor does so.
   */
  void interrupt();

  /**
   * Prepares the operation for pausing. The functioning is dependent
   * on the type of the processor. In threaded mode, the operation
   * will be turned into @p Pausing state, and processing will pause
   * once the thread has finished its current processing round. In
   * simple mode, the processor will check if the operation has
   * connected inputs. If it does, the operation will turn into @p
   * Pausing state and wait until it receives pause signals from
   * previous operations in the pipeline. If there are no connected
   * inputs, the operation will immediately turn into @p Paused state.
   */
  void pause();

  /**
   * Prepares the operation for stopping. Works analogously to
   * #pause().
   */
  void stop();
  
  /**
   * Waits until the processor is finished.
   */
  bool wait(unsigned long time = ULONG_MAX);

protected:
  /// @internal
  class PII_YDIN_EXPORT Data : public PiiBasicOperation::Data
  {
  public:
    Data();
    ~Data();
    
  private:
    friend class PiiDefaultOperation;
    friend class PiiSimpleProcessor;
    friend class PiiThreadedProcessor;
    
    // Handles object flow. Synchronizes inputs etc.
    PiiFlowController* pFlowController;

    // Executes process() when needed.
    PiiOperationProcessor* pProcessor;

    // The group id of the input group being processed.
    int iActiveInputGroup;

    bool bChecked;
    
    PiiReadWriteLock processLock;
  };
  PII_D_FUNC;

  /**
   * Creates a new %PiiDefaultOperation with the given default
   * processing mode.
   */
  PiiDefaultOperation(ProcessingMode mode);

  /// @internal
  PiiDefaultOperation(Data* data, ProcessingMode mode);

  void setProcessingMode(ProcessingMode processingMode);
  ProcessingMode processingMode() const;

  void setPriority(int priority);
  int priority() const;

  /**
   * Executes one round of processing. This function is invoked by the
   * processor if the necessary preconditions for a new processing
   * round are met. This function does all the necessary calculations
   * to create output objects and sends them to output sockets.
   *
   * Calls to process(), syncEvent(), and setProperty() are
   * synchronized and cannot occur simultaneously. 
   * %PiiDefaultOperation ensures this by locking #processLock() for
   * reading before calling process().
   *
   * @b Note: With time-consuming operations, one should occasionally
   * check that the operation hasn't been interrupted, i.e. that
   * state() returns @p Running.
   *
   * @exception PiiExecutionException whenever an unrecoverable error
   * occurs during a processing round, the operation is interrupted,
   * or finishes execution due to end of input data.
   */
  virtual void process() = 0;

  /**
   * Returns the id of the synchronized socket group being processed. 
   * If all input sockets work in parallel, or there are no input
   * sockets, this value can be safely ignored. Otherwise, one can use
   * the value to decide which sockets need to be read and processed
   * in #process().
   */
  int activeInputGroup() const;

  /**
   * Informs the operation about synchronization events. The most
   * typical use of this function is to see when all objects in an
   * input group and all of its child groups have been received. For
   * example, if the operation reads large images from one input and a
   * number of sub-images for each large image from another input, a
   * sync event is sent whenever all the small images that correspond
   * to one large image have been processed. Your implementation may
   * then either just record the synchronized state or to send any
   * buffered data. The default implementation does nothing.
   *
   * Calls to process(), syncEvent(), and setProperty() are
   * synchronized and cannot occur simultaneously. 
   * %PiiDefaultOperation ensures this by locking #processLock() for
   * reading before calling syncEvent().
   *
   * @code
   * void MyOperation::syncEvent(SyncEvent* event)
   * {
   *   if (event->type() == SyncEvent::EndInput &&
   *       event->groupId() == _pLargeImageInput->groupId())
   *     doWhateverNeededNow();
   * }
   * @endcode
   */
  void syncEvent(SyncEvent* event);

  /**
   * Creates a flow controller for this operation. This function is
   * called by the default implementation of #check(). A new flow
   * controller will be always be created when the #check() function
   * is called. The old controller will be deleted.
   *
   * The default implementation tries to find an optimal flow
   * controller for the active input configuration:
   *
   * @li If there are no connected inputs, a null pointer will be
   * returned.
   *
   * @li If the operation has only one connected input,
   * PiiOneInputFlowController will be used.
   *
   * @li If there are many inputs, but all are in the same group,
   * PiiOneGroupFlowController will be used.
   *
   * @li Otherwise, PiiDefaultFlowController will be used. The flow
   * controller will be configured with @e loose parent-child
   * relationships between groups with a non-negative group id and at
   * least one connected socket. The relationships will be assigned in
   * the order of increasing magnitude. For example, if there are
   * sockets in groups 0, 1, and -1, 0 will be set as the (loose)
   * parent of 1. Group -1 will be independent of the others.
   *
   * Subclasses may override the default behavior by installing a
   * custom flow controller. You need to do this if your operation
   * has, for example, sibling groups that share a common parent
   * group, or if you need to assign @e strict relationships between
   * input groups. If you override this function, make sure the flow
   * controller takes the control of all connected inputs.
   *
   * @return a pointer to a flow controller. PiiDefaultOperation will
   * take the ownership of the pointer. If you don't want inputs to be
   * controlled, return 0.
   *
   * @see PiiDefaultFlowController
   */
  virtual PiiFlowController* createFlowController();

  /**
   * Returns @p true if the operation has been checked for execution
   * (#check()) but not started (#start()) yet.
   */
  bool isChecked() const;

  /**
   * Returns a pointer to a lock that %PiiDefaultOperation uses to
   * synchronize calls to #property(), #setProperty(), #process(), and
   * #syncEvent(). This lock can be used if a subclass needs to
   * protect stuff from being accessed concurrently. Acquiring the
   * lock for writing blocks concurrent calls to all of the mentioned
   * functions. Acquiring the lock for reading blocks only
   * #setProperty() and allows simultaneous execution of the other
   * (read-locked) functions.
   */
  PiiReadWriteLock* processLock();
  
private:
  void init(ProcessingMode mode);
  void createProcessor(ProcessingMode processingMode);
  
  friend class PiiSimpleProcessor;
  friend class PiiThreadedProcessor;
  
  inline void processLocked()
  {
    PiiReadLocker lock(&_d()->processLock);
    process();
  }

  inline void sendSyncEvents(PiiFlowController* controller)
  {
    PiiReadLocker lock(&_d()->processLock);
    controller->sendSyncEvents(this);
  }
};

#endif //_PIIDEFAULTOPERATION_H
