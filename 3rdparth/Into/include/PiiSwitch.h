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

#ifndef _PIISWITCH_H
#define _PIISWITCH_H

#include <PiiDefaultOperation.h>

/**
 * An operation that passes objects conditionally depending on a
 * trigger input.
 *
 * @inputs
 *
 * @in trigger - trigger input. Type is ignored in @p
 * AsynchronousMode, and the input can even be left unconnected. In @p
 * SynchronousMode, the trigger must be a boolean value or a number.
 *
 * @in inputX - reads in objects of any type. The object read from @p
 * inputX will be emitted to @p outputX when triggered. X ranges from
 * 0 to @ref PiiVariableInputOperation::inputCount "inputCount" - 1.
 * @p input0 can also be accessed as @p input.
 *
 * @outputs
 *
 * @out outputX - emits the last input object whenever a trigger is
 * received. @p output0 can also be accessed as @p output.
 *
 * The default number of input-output pairs is 1.
 *
 * @ingroup PiiFlowControlPlugin
 */
class PiiSwitch : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The number of dynamic inputs (in addition to @p trigger). The
   * default is one.
   */
  Q_PROPERTY(int dynamicInputCount READ dynamicInputCount WRITE setDynamicInputCount);
 
  /**
   * Operation mode. The default mode is @p SynchronousMode.
   */
  Q_PROPERTY(OperationMode operationMode READ operationMode WRITE setOperationMode);
  Q_ENUMS(OperationMode);

  /**
   * Turns the trigger input to "always on". This property only
   * affects AsynchronousMode. Setting its value to @p true makes the
   * operation pass all incoming objects directly and ignore the @p
   * trigger input. If the @p trigger input is not connected (in
   * @p AsynchronousMode), this property controls can be used to block
   * and release the object flow through this operation. The default
   * value is @p false.
   */
  Q_PROPERTY(bool passThrough READ passThrough WRITE setPassThrough);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Operation modes.
   *
   * @lip AsynchronousMode - trigger and the other inputs are not
   * synchronized to each other. The trigger can come from any
   * unrelated source. If #passThrough is @p false, exactly one set of
   * input objects will be passed for each object received from the
   * trigger input. If there are no incoming objects when the trigger
   * arrives, the next incoming objects will be emitted. If
   * #passThrough is @p true, the trigger input will have no effect
   * and all input objects will be passed. Note that the same objects
   * may be sent many times if the trigger input has a higher data
   * rate than the other inputs.
   *
   * @lip SynchronousMode - the trigger signal is used as a "filter". 
   * All inputs work in sync: there must be a trigger signal for each
   * set of input objects. If a non-zero number or @p true is received,
   * the input objects will be passed. Otherwise, the input objects
   * will be ignored.
   */
  enum OperationMode { SynchronousMode, AsynchronousMode };
  
  PiiSwitch();

  /**
   * Aliases @p input to @p input0.
   */
  PiiInputSocket*	input(const QString &name) const;
  /**
   * Aliases @p output to @p output0.
   */
  PiiOutputSocket* output(const QString &name) const;
  
  void check(bool reset);
  
protected:
  PiiInputSocket* createInput(int index);
  
  void process();
  void aboutToChangeState(State state);

  void setOperationMode(OperationMode operationMode);
  OperationMode operationMode() const;
  void setDynamicInputCount(int inputCount);
  int dynamicInputCount() const;
  void setPassThrough(bool passThrough);
  bool passThrough() const;
  
private:
  void emitInputObjects();
  void emitObjectList();
  void storeInputObjects();

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    OperationMode operationMode;
    int iTriggerCount;
    QList<PiiVariant> lstObjects;
    int iStaticInputCount;
    bool bPassThrough;
  };
  PII_D_FUNC;
};
  
#endif //_PIISWITCH_H
