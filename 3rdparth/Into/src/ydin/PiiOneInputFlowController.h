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

#ifndef _PIIONEINPUTFLOWCONTROLLER_H
#define _PIIONEINPUTFLOWCONTROLLER_H

#include "PiiFlowController.h"
#include <QVector>
#include <QList>

/**
 * A flow controller that does not handle synchronization between
 * sibling sockets. %PiiOneInputFlowController can be used with
 * operations that have only one connected input.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiOneInputFlowController : public PiiFlowController
{
public:
  /**
   * Create a new flow controller.
   *
   * @param inputs the single synchronized input to be controlled.
   *
   * @param outputs the a list of outputs to be controlled. The class
   * will only consider outputs whose @ref PiiSocket::setGroupId()
   * "group ID" matches that of the input.
   */
  PiiOneInputFlowController(PiiInputSocket* input,
                            const QList<PiiOutputSocket*>& outputs);
  
  PiiFlowController::FlowState prepareProcess();

protected:
  /// @internal
  class Data : public PiiFlowController::Data
  {
  public:
    Data(PiiInputSocket* input, const QList<PiiOutputSocket*>& outputs);

    PiiInputSocket* pInput;
    QVector<PiiOutputSocket*> vecOutputs;
    int iOutputCount; //optimization
  };
  PII_D_FUNC;
};

#endif //_PIIONEINPUTFLOWCONTROLLER_H
