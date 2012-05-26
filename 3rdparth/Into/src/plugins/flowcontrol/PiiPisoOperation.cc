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

#include "PiiPisoOperation.h"
#include <PiiOneGroupFlowController.h>
#include <PiiYdinTypes.h>

PiiPisoOperation::Data::Data() :
  iGroupSize(1),
  iGroupCount(1),
  defaultValueMode(SameDefaultsForAllGroups),
  operationMode(AsynchronousMode)
{
}

PiiPisoOperation::PiiPisoOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiOutputSocket("index"));
  addSocket(new PiiOutputSocket("output0"));

  setDynamicInputCount(2);
}

void PiiPisoOperation::setDynamicInputCount(int inputCount)
{
  if (inputCount < 1) return;
  setNumberedInputs(inputCount);
  for (int i=0; i<inputCount; ++i)
    PiiDefaultOperation::inputAt(i)->setOptional(true);
}


void PiiPisoOperation::setGroupSize(int groupSize)
{
  if (groupSize < 1) return;
  // Add dynamic outputs after the index output
  setNumberedOutputs(groupSize, 1);
  _d()->iGroupSize = groupSize;
}

PiiInputSocket* PiiPisoOperation::input(const QString& name) const
{
  const PII_D;
  // If the parent knows this input, return it.
  PiiInputSocket* pInput = PiiDefaultOperation::input(name);
  if (pInput != 0)
    return pInput;

  // Try aliased names
  for (int i=0; i<inputCount(); ++i)
    if (name == QString("group%1 input%2").arg(i/d->iGroupSize).arg(i%d->iGroupSize))
      return inputAt(i);

  return 0;
}

PiiOutputSocket* PiiPisoOperation::output(const QString& name) const
{
  // Try aliased name first
  if (name == "output")
    return outputAt(1);
  
  return PiiDefaultOperation::output(name);
}

bool PiiPisoOperation::hasDefaultValue(int inputIndex)
{
  PII_D;
  if (d->defaultValueMode == SameDefaultsForAllGroups)
    inputIndex = inputIndex % d->iGroupSize;
  
  return inputIndex < d->lstDefaultValues.size() &&
    d->lstDefaultValues[inputIndex].userType() == iPiiVariantTypeId;
}

void PiiPisoOperation::check(bool reset)
{
  PII_D;
  const int iCnt = inputCount();
  d->vecConnectedInputs.resize(iCnt);
  d->vecConnectedInputs.fill(false);

  bool bOneConnected = false;
  d->iGroupCount = 0;
  for (int i=0; i<iCnt; i += d->iGroupSize)
    {
      bool bGroupConnected = false, bAllConnected = true;
      ++d->iGroupCount;
      // Check all inputs in a group
      for (int j=i; j<i+d->iGroupSize; ++j)
        {
          // Store connection status
          bool bConnectedInput = false;
          if (j < iCnt)
            {
              d->vecConnectedInputs[j] = bConnectedInput = inputAt(j)->isConnected();
              // In synchronous mode, inputs are all in the same group.
              inputAt(j)->setGroupId(d->operationMode == AsynchronousMode ? i/d->iGroupSize : 0);
            }
          bOneConnected |= bConnectedInput;

          // Someone is connected...
          if (bConnectedInput)
            bGroupConnected = true;
          // This one is not connected and has no default value.
          else if (!hasDefaultValue(j))
            bAllConnected = false;
        }
      if (!bGroupConnected && d->iGroupSize > 1)
        PII_THROW(PiiExecutionException, tr("At least one input in all groups must be connected."));
      // If any input is connected, all must be connected (or have
      // a default value)
      if (bGroupConnected && !bAllConnected)
        PII_THROW(PiiExecutionException, tr("Not all sockets in input group %1 are connected.").arg(i/d->iGroupSize));
    }
  if (!bOneConnected)
    PII_THROW(PiiExecutionException, tr("At least one input must be connected."));

  PiiDefaultOperation::check(reset);
}

void PiiPisoOperation::process()
{
  PII_D;
  if (d->operationMode == AsynchronousMode)
    passObjectsInGroup(activeInputGroup());
  else
    {
      startMany();
      for (int i=0; i<d->iGroupCount; ++i)
        passObjectsInGroup(i);
      endMany();
    }
}

void PiiPisoOperation::passObjectsInGroup(int groupIndex)
{
  PII_D;
  const int iCnt = inputCount();
  // Emit input index
  emitObject(groupIndex);
  if (d->iGroupSize == 1)
    // Emit the object itself
    emitObject(readInput(groupIndex), 1);
  else
    {
      int iFirstInGroup = groupIndex * d->iGroupSize;
      for (int j=0; j<d->iGroupSize; ++j)
        {
          int inputIndex = j + iFirstInGroup;
          // Release all connected inputs and emit their
          // incoming objects.
          if (inputIndex < iCnt && d->vecConnectedInputs[inputIndex])
            emitObject(readInput(inputIndex), j+1);
          // If the input is not connected, output a default value. In
          // check() we made sure there is a default value for all
          // empty inputs. Thus, we won't bother checking if the
          // default value is there.
          else
            emitObject(d->lstDefaultValues[d->defaultValueMode == SameDefaultsForAllGroups ? j : iFirstInGroup + j].value<PiiVariant>(), j+1);
        }
    }
}

void PiiPisoOperation::setDefaultValue(const QVariantMap& map)
{
  PII_D;
  int index = map.value("index",0).toInt();
  QVariant defaultObject = map.value("defaultValue", QVariant());

  if (index < d->lstDefaultValues.size())
    d->lstDefaultValues[index] = defaultObject;
  else
    {
      //add some empty values
      for ( int i=d->lstDefaultValues.size(); i<index; i++)
        d->lstDefaultValues.append(QVariant());

      //add the given value
      d->lstDefaultValues.append(defaultObject);
    }
}

class PiiPisoController : public PiiFlowController
{
public:
  PiiPisoController(const QList<PiiInputSocket*> inputs, const QList<PiiOutputSocket*> outputs);

  FlowState prepareProcess();

private:
  class InputGroup : public QVector<PiiInputSocket*>
  {
  public:
    InputGroup(int groupId) : _iGroupId(groupId) {}
    
    /*void setTagged(bool tagged) { _bTagged = tagged; }
      bool isTagged() const { return _bTagged; }*/

    int groupId() const { return _iGroupId; }
    
    void release();
    
  private:
    int _iGroupId;
  };
  
  void passTag();
  
  QList<PiiInputSocket*> _lstInputs;
  QList<PiiOutputSocket*> _lstOutputs;
  QList<InputGroup> _lstGroups;
  QList<InputGroup> _lstActiveGroups;
  int _iTagMask;
};

void PiiPisoController::InputGroup::release()
{
  for (int i=0; i<size(); ++i)
    at(i)->release();
}

PiiFlowController* PiiPisoOperation::createFlowController()
{
  PII_D;
  if (d->operationMode == AsynchronousMode)
    return new PiiPisoController(inputSockets(), outputSockets());
  else
    return new PiiOneGroupFlowController(inputSockets(), outputSockets());
}

PiiPisoController::PiiPisoController(const QList<PiiInputSocket*> inputs, const QList<PiiOutputSocket*> outputs) :
  _lstInputs(inputs),
  _lstOutputs(outputs),
  _iTagMask(0)
{
  int previousGroupId = -1;
  for (int i=0; i<inputs.size(); ++i)
    {
      if (inputs[i]->isConnected())
        {
          if (previousGroupId != inputs[i]->groupId())
            {
              _lstGroups << InputGroup(inputs[i]->groupId());
              previousGroupId = inputs[i]->groupId();
            }
          _lstGroups.last() << inputs[i];
        }
    }
  _lstActiveGroups = _lstGroups;
}

void PiiPisoController::passTag()
{
  PiiVariant tag = _lstGroups[0][0]->firstObject();
  for (int i=0; i<_lstOutputs.size(); ++i)
    _lstOutputs[i]->emitObject(tag);
}

PiiFlowController::FlowState PiiPisoController::prepareProcess()
{
  for (int g=_lstActiveGroups.size(); g--; )
    {
      switch (int iTypeMask = inputGroupTypeMask(_lstActiveGroups[g].begin(), _lstActiveGroups[g].end()))
        {
        case NoObject:
          break;
        case NormalObject:
          setActiveInputGroup(_lstActiveGroups[g].groupId());
          _lstActiveGroups[g].release();
          return ProcessableState;
        case StartTag:
        case EndTag:
        case ResumeTag:
        case PauseTag:
        case StopTag:
          //qDebug("PiiPisoOperation %p: Type mask = 0x%x", this, iTypeMask);
          // We need to wait for tags in all groups before proceeding.
          _lstActiveGroups[g].release();
          _lstActiveGroups.removeAt(g);
          _iTagMask |= iTypeMask;
          break;
        default:
          PII_THROW(PiiExecutionException, tr("Synchronization error at input group %1.\n%2")
                    .arg(_lstActiveGroups[g].groupId())
                    .arg(dumpInputObjects(_lstActiveGroups[g].begin(), _lstActiveGroups[g].end())));
        }
    }
  // Tags in all groups
  if (_lstActiveGroups.size() == 0)
    {
      _lstActiveGroups = _lstGroups;
      //qDebug("PiiPisoOperation %p: All groups tagged. Type mask = 0x%x", this, _iTagMask);
      switch (_iTagMask)
        {
        case StartTag:
        case EndTag:
          passTag();
          _iTagMask = 0;
          return SynchronizedState;
        case PauseTag:
          return PausedState;
        case StopTag:
          return FinishedState;
        case ResumeTag:
          _iTagMask = 0;
          return ResumedState;
        default:
          PII_THROW(PiiExecutionException, tr("Synchronization error. Got mixed control objects.\n%2")
                    .arg(dumpInputObjects(_lstInputs.begin(), _lstInputs.end(), 0)));
        }
    }
  return IncompleteState;
}

int PiiPisoOperation::groupSize() const { return _d()->iGroupSize; }
void PiiPisoOperation::setDefaultValues(const QVariantList& defaultValues) { _d()->lstDefaultValues = defaultValues; }
QVariantList PiiPisoOperation::defaultValues() const { return _d()->lstDefaultValues; }
void PiiPisoOperation::setDefaultValueMode(const DefaultValueMode& defaultValueMode) { _d()->defaultValueMode = defaultValueMode; }
PiiPisoOperation::DefaultValueMode PiiPisoOperation::defaultValueMode() const { return _d()->defaultValueMode; }
int PiiPisoOperation::dynamicInputCount() const { return _d()->lstInputs.size(); }
void PiiPisoOperation::setOperationMode(const OperationMode& operationMode) { _d()->operationMode = operationMode; }
PiiPisoOperation::OperationMode PiiPisoOperation::operationMode() const { return _d()->operationMode; }
