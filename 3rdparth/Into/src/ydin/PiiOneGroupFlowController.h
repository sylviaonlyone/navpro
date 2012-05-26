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

#ifndef _PIIONEGROUPFLOWCONTROLLER_H
#define _PIIONEGROUPFLOWCONTROLLER_H

#include "PiiFlowController.h"
#include <QVector>
#include <QList>

/**
 * A flow controller that synchronizes a group of sockets with the
 * same @ref PiiSocket::setGroupId() "group ID".
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiOneGroupFlowController : public PiiFlowController
{
public:
  /**
   * Create a new flow controller.
   *
   * @param inputs a list of inputs to be controlled. All should be
   * synchronous and in the same synchronization group.
   *
   * @param outputs the a list of outputs to be controlled. This class
   * only considers outputs whose @ref PiiSocket::setGroupId() "group
   * ID" matches that of the input.
   */
  PiiOneGroupFlowController(const QList<PiiInputSocket*>& inputs,
                            const QList<PiiOutputSocket*>& outputs);

  PiiFlowController::FlowState prepareProcess();

private:
  void releaseInputs();

  class Data : public PiiFlowController::Data
  {
  public:
    Data(const QList<PiiInputSocket*>& inputs,
         const QList<PiiOutputSocket*>& outputs);
    QVector<PiiInputSocket*> vecInputs;
    QVector<PiiOutputSocket*> vecOutputs;
  };
  
  PII_D_FUNC;
};

#endif //_PIIONEGROUPFLOWCONTROLLER_H
