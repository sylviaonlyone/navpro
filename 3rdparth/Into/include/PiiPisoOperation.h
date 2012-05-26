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

#ifndef _PIIPISOOPERATION_H
#define _PIIPISOOPERATION_H

#include <PiiDefaultOperation.h>
#include <QVector>

/**
 * A parallel-to-serial converter. In the default non-grouped state,
 * %PiiPisoOperation outputs whatever it receives in any of its inputs
 * to its single output. If #groupSize is one, a PISO works like a
 * multiplexer. A PISO is commonly useful in situations where multiple
 * data sources feed data into one processing pipeline. The operation
 * allows flexible control of input groups to redirect many
 * synchronized input streams at once.
 *
 * If #groupSize is greater than one, inputs are divided into
 * synchronized groups. Input sockets from zero to @p groupSize - 1
 * will form the first group, sockets from @p groupSize to 2 * @p
 * groupSize - 1 the second one and so on. Every socket in a group
 * must contain an object before it is processed. Whenever this
 * happens, all objects will be simultaneously sent to the
 * corresponding outputs. The first input in a group will be sent to
 * @p output0 and so on. If an input socket in a group has not been
 * connected, there must be a @ref defaultValues "default value" for
 * it.
 *
 *
 * @par Configuring default values
 *
 * A list of PiiVariant objects that will be used if a socket in
 * a group is not connected. The first element in the list is the
 * default value for the first input, or the first input in each
 * group, depending on the value of #defaultValueMode. The second
 * element is the value for the second input and so on. An invalid
 * QVariant means no default value. Input sockets with a valid
 * default value will be marked optional. Default values have no
 * effect if #groupSize is one.
 *
 * @code
 * PiiOperation* op = engine.createOperation("PiiPisoOperation");
 * op->setProperty("groupSize", 2);
 * op->setProperty("inputCount", 4);
 * // No default value for first input in each group.
 * // The default value for the second input is int(0)
 * op->setProperty("defaultValues", QVariantList() << QVariant() << Pii::createQVariant(0));
 * @endcode
 *
 * You can configure also the one default value with property map
 * (QVariantMap). Recognized property values are:
 * 
 * @lip index - the index of the input to configure. (int, default
 * value 0)
 *
 * @lip defaultValue - a PiiVariant object that will be used if the
 * corresponding input is not connected.
 *
 * @code
 * // Create a new piso
 * PiiOperation* piso = engine.createOperation("PiiPisoOperation");
 * piso->setProperty("groupSize", 2);
 * piso->setProperty("inputCount", 4);
 *
 * QVariantMap props;
 * props["index"] = 1;
 * props["defaultValue"] = Pii::createQVariant(0);
 * piso->setProperty("defaultValue", props);
 * @endcode
 *
 * The default number of inputs is two.
 *
 * @inputs
 *
 * @in inputX - any number of input sockets that accept any object
 * type. X ranges from 0 to @p inputCount - 1.
 *
 * @in groupX inputY - alias for input number X * #groupSize + Y. If
 * #groupSize is 3, @p group1 @p input1 is an alias for @p input4.
 * 
 * At least one of the inputs must be connected. If any of the inputs
 * in a synchronous input group are connected, then all of them must
 * either be connected or have a default value assigned.
 *
 * @outputs
 *
 * @out index - the index of the input group the object was or objects
 * were received in. If #groupSize is one (the default), the group
 * index equals the index of the input socket.
 *
 * @out output - alias for output0. Emits the object received from any
 * of the inputs. Objects are emitted in the order they are received.
 *
 * @out outputX - synchronous outputs, if #groupSize is greater than
 * one. X ranges from 0 to #groupSize-1. Whenever all sockets in a
 * synchronous group contain an object, the objects will be
 * simultaneously sent to the corresponding outputs. That is,
 * @p groupX @p input0 will be redirected to @p output0, @p groupX @p
 * input1 to @p input 1 etc.
 *
 * @ingroup PiiFlowControlPlugin
 */
class PiiPisoOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The number of input sockets. Default is 2.
   */
  Q_PROPERTY(int dynamicInputCount READ dynamicInputCount WRITE setDynamicInputCount);

  /**
   * The number of sockets in each synchronized group. It is usually a
   * clever idea to make #dynamicInputCount divisible by this value. If it is
   * not, the last group will not have corresponding inputs for all
   * outputs. In such a case there must be a @ref defaultValues
   * "default value" for each orphaned output. The default value is
   * one.
   */
  Q_PROPERTY(int groupSize READ groupSize WRITE setGroupSize);

  /**
   * A list of PiiVariant objects that will be used if a socket in
   * a group is not connected. The first element in the list is the
   * default value for the first input, or the first input in each
   * group, depending on the value of #defaultValueMode. The second
   * element is the value for the second input and so on. An invalid
   * QVariant means no default value. Input sockets with a valid
   * default value will be marked optional. Default values have no
   * effect if #groupSize is one.
   *
   * @code
   * PiiOperation* op = engine.createOperation("PiiPisoOperation");
   * op->setProperty("groupSize", 2);
   * op->setProperty("dynamicInputCount", 4);
   * // No default value for first input in each group.
   * // The default value for the second input is int(0)
   * op->setProperty("defaultValues", QVariantList() << QVariant() << Pii::createQVariant(0));
   * @endcode
   */
  Q_PROPERTY(QVariantList defaultValues READ defaultValues WRITE setDefaultValues);

  /**
   * Configure a single input. The @p index property in the @p
   * defaultValue map specifies the input to configure.
   *
   * @code
   * QVariantMap props;
   * props["defaultValue"] = Pii::createQVariant("name");
   * props["index"] = 0;
   * piso->setProperty("defaultValue", props);
   * @endcode
   */
  Q_PROPERTY(QVariantMap defaultValue WRITE setDefaultValue STORED false);

  /**
   * Treatment of the #defaultValues list. The default value is @p
   * SameDefaultsForAllGroups.
   */
  Q_PROPERTY(DefaultValueMode defaultValueMode READ defaultValueMode WRITE setDefaultValueMode);
  Q_ENUMS(DefaultValueMode);

  /**
   * Operation mode. Default is @p AsynchronousMode.
   */
  Q_PROPERTY(OperationMode operationMode READ operationMode WRITE setOperationMode);
  Q_ENUMS(OperationMode);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Default value handling.
   *
   * @lip SameDefaultsForAllGroups - the #defaultValues list is
   * repeated for each group of synchronized inputs. The same default
   * value applies to the corresponding sockets in each group. If the
   * length of the #defaultValues list is larger than #groupSize, the
   * extra entries will be ignored.
   *
   * @lip IndividualDefaults - each input socket has its own default
   * value. If the length of the #defaultValues list is smaller than
   * the number of input sockets, the rest of the input sockets will
   * not have default values. If the length of the #defaultValues list
   * is larger than #dynamicInputCount, the extra entries will be ignored.
   */
  enum DefaultValueMode { SameDefaultsForAllGroups, IndividualDefaults };

  /**
   * Operation modes.
   *
   * @lip AsynchronousMode - the operation passed data from any input
   * group as soon as the group is full. Some groups may be handled
   * more often than others. Synchronization information will be
   * passed only once it is received in all groups.
   *
   * @lip SynchronousMode - the operation waits until all input groups
   * are full before passing data. Input objects will then be emitted
   * serially starting from group zero. The objects are sent between
   * synchronization tags. (See PiiOutputSocket::startMany() and
   * PiiOutputSocket::endMany().)
   */
  enum OperationMode { AsynchronousMode, SynchronousMode };
  
  PiiPisoOperation();
  
  PiiInputSocket* input(const QString& name) const;
  PiiOutputSocket* output(const QString& name) const;

  void check(bool reset);

  void setDefaultValue(const QVariantMap& defaultValue);

  void setGroupSize(int groupSize);
  int groupSize() const;
  void setDefaultValues(const QVariantList& defaultValues);
  QVariantList defaultValues() const;
  void setDefaultValueMode(const DefaultValueMode& defaultValueMode);
  DefaultValueMode defaultValueMode() const;
  void setDynamicInputCount(int count);
  int dynamicInputCount() const;
  void setOperationMode(const OperationMode& operationMode);
  OperationMode operationMode() const;

protected:
  void process();
  PiiFlowController* createFlowController();

private:
  bool hasDefaultValue(int inputIndex);
  void passObjectsInGroup(int groupId);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    int iGroupSize;
    int iGroupCount;
    QVariantList lstDefaultValues;
    QVector<bool> vecConnectedInputs;
    DefaultValueMode defaultValueMode;
    OperationMode operationMode;
  };
  PII_D_FUNC;
};

#endif //_PIIPISOOPERATION_H
