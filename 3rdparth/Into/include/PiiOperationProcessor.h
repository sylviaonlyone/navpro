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

#ifndef _PIIOPERATIONPROCESSOR_H
#define _PIIOPERATIONPROCESSOR_H

#include "PiiYdinTypes.h"
#include "PiiDefaultOperation.h"
#include "PiiInputController.h"
#include <QThread>

class PiiInputSocket;
class PiiFlowController;

/**
 * A helper class used by PiiDefaultOperation to implement threaded
 * and non-threaded processing modes. It implements most of the
 * PiiOperation interface and adds a few functions of its own. The
 * input processor uses a @ref PiiFlowController "flow controller" to
 * synchronize objects read from the input sockets.
 *
 * %PiiOperationProcessor is intimately bound to PiiDefaultOperation
 * and not of much use elsewhere. Its interface is not guaranteed to
 * remain binary compatible. You have been warned.
 *
 * @internal
 * @ingroup Ydin
 */
class PiiOperationProcessor : public PiiInputController
{
public:
  virtual ~PiiOperationProcessor();

  virtual void check(bool reset) = 0;
  virtual void start() = 0;
  virtual void interrupt() = 0;
  virtual void pause() = 0;
  virtual void stop() = 0;

  virtual bool wait(unsigned long time = ULONG_MAX) = 0;

  /**
   * Set the processing priority of this processor to @p priority. 
   */
  virtual void setProcessingPriority(QThread::Priority priority) = 0;

  /**
   * Get the current processing priority of this processor.
   */
  virtual QThread::Priority processingPriority() const = 0;

  /**
   * Returns either @p Threaded or @p NonThreaded, depending on the
   * implementation.
   */
  PiiDefaultOperation::ProcessingMode processingMode() const { return _processingMode; }

  /**
   * Set the flow controller that prepares the operation for
   * processing. This function is called by
   * PiiDefaultOperation::check().
   */
  void setFlowController(PiiFlowController* flowController) { _pFlowController = flowController; }
  /**
   * Get the current flow controller.
   */
  PiiFlowController* flowController() const { return _pFlowController; }
  
protected:
  /**
   * Create a new %PiiOperationProcessor.
   *
   * @param parent the operation to be executed
   *
   * @param mode processing mode. The value depends on the
   * implementation. PiiThreadedProcessor sets @p mode to @p Threaded,
   * and PiiSimpleProcessor to @p NonThreaded.
   */
  PiiOperationProcessor(PiiDefaultOperation* parent,
                        PiiDefaultOperation::ProcessingMode mode) :
    _pParentOp(parent),
    _processingMode(mode)
  {}

  /**
   * A pointer to the parent operation.
   */
  PiiDefaultOperation* _pParentOp;
  /**
   * A pointer to the currently installed flow controller.
   */
  PiiFlowController* _pFlowController;

private:
  PiiDefaultOperation::ProcessingMode _processingMode;
};

#endif //_PIIOPERATIONPROCESSOR_H
