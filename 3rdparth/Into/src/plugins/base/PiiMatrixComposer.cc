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

#include "PiiMatrixComposer.h"
#include <PiiYdinTypes.h>
#include <PiiUtil.h>

PiiMatrixComposer::Data::Data() :
  direction(Pii::Horizontally),
  pFirstConnectedInput(0),
  uiPreviousType(PiiVariant::InvalidType),
  pMatResult(0)
{
}

PiiMatrixComposer::PiiMatrixComposer() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  addSocket(d->pSyncInput = new PiiInputSocket("sync"));
  d->pSyncInput->setOptional(true);
  setDynamicInputCount(1);
  
  addSocket(new PiiOutputSocket("output"));
  
  setProtectionLevel("dynamicInputCount", WriteWhenStoppedOrPaused);
}

PiiMatrixComposer::~PiiMatrixComposer()
{
  delete _d()->pMatResult;
}

void PiiMatrixComposer::setDynamicInputCount(int cnt)
{
  if (cnt < 1 || cnt > 2048)
    return;
  
  setNumberedInputs(cnt, 1);

  for(int i = 1; i<inputCount(); ++i)
    inputAt(i)->setOptional(true);
}

int PiiMatrixComposer::dynamicInputCount() const
{
  return inputCount()-1;
}

void PiiMatrixComposer::setDefaultValues(const QVariantList& defaultValues)
{
  _d()->lstDefaultValues = Pii::variantsToVector<double>(defaultValues);
}

QVariantList PiiMatrixComposer::defaultValues() const
{
  return Pii::vectorToVariants<double>(_d()->lstDefaultValues);
}

void PiiMatrixComposer::check(bool reset)
{
  PII_D;
  // Check that the number of connected inputs is at least one. If it
  // is not, an exception is thrown.
  d->lstConnectedInputs.resize(inputCount()-1);
  d->pFirstConnectedInput = 0;
  int iGroupId = d->pSyncInput->isConnected() ? 1 : 0;
  for (int i=1; i<inputCount(); ++i)
    {
      inputAt(i)->setGroupId(iGroupId);
      if (inputAt(i)->isConnected())
        {
          if (d->pFirstConnectedInput == 0)
            d->pFirstConnectedInput = inputAt(i);
          d->lstConnectedInputs[i-1] = true;
        }
      else
        d->lstConnectedInputs[i-1] = false;
    }

  if (d->pFirstConnectedInput == 0)
    PII_THROW(PiiExecutionException, tr("At least one input must be connected."));

  while (d->lstDefaultValues.size() < inputCount()-1)
    d->lstDefaultValues << 0.0;

  if (reset)
    {
      delete d->pMatResult;
      d->pMatResult = 0;
      d->uiPreviousType = PiiVariant::InvalidType;
    }

  PiiDefaultOperation::check(reset);
}

void PiiMatrixComposer::process()
{
  PII_D;
  if (!d->pSyncInput->isConnected() ||
      activeInputGroup() == 1)
    {
      switch (d->pFirstConnectedInput->firstObject().type())
        {
          PII_PRIMITIVE_CASES(operate,);
        default:
          PII_THROW_UNKNOWN_TYPE(d->pFirstConnectedInput);
        }
    }
}

void PiiMatrixComposer::syncEvent(SyncEvent* event)
{
  if (event->type() == SyncEvent::EndInput)
    {
      switch (_d()->uiPreviousType)
        {
          PII_PRIMITIVE_CASES(emitMatrix,);
        }
    }
}

template <class T> void PiiMatrixComposer::emitMatrix()
{
  PiiMatrix<T>& matResult = static_cast<PiiMatrix<T>& >(*_d()->pMatResult);
  emitObject(matResult);
  resize(matResult);
}

template <class T> void PiiMatrixComposer::resize(PiiMatrix<T>& matrix)
{
  if (_d()->direction == Pii::Horizontally)
    matrix.resize(0, matrix.columns());
  else
    matrix.resize(matrix.rows(), 0);
}

template <class T> void PiiMatrixComposer::operate()
{
  PII_D;
  const int iCnt = inputCount()-1;

  if (d->uiPreviousType != Pii::typeId<T>())
    {
      delete d->pMatResult;
      if (d->direction == Pii::Horizontally)
        d->pMatResult = new PiiMatrix<T>(0, iCnt);
      else
        d->pMatResult = new PiiMatrix<T>(iCnt, 0);
      d->uiPreviousType = Pii::typeId<T>();
    }
  PiiMatrix<T>& matResult = static_cast<PiiMatrix<T>& >(*d->pMatResult);

  int iItemCount = 0;
  if (d->direction == Pii::Horizontally)
    {
      iItemCount = matResult.rows();
      matResult.appendRow();
    }
  else
    {
      iItemCount = matResult.columns();
      matResult.appendColumn();
    }

  // For each input, it is checked, whether the input is connected or
  // not. If the input is not connected, the default value is used for
  // filling the matrix column/row.
  for (int i=0; i<iCnt; ++i)
    {
      // If the input is not connected. Use the default value for the input.
      T value = d->lstConnectedInputs[i] ?
        PiiYdin::convertPrimitiveTo<T>(inputAt(i+1)) :
        T(d->lstDefaultValues[i]);
      if (d->direction == Pii::Horizontally)
        matResult(iItemCount, i) = value;
      else
        matResult(i, iItemCount) = value;
    }
  
  if (!d->pSyncInput->isConnected())
    {
      emitObject(matResult);
      resize(matResult);
    }
}

void PiiMatrixComposer::setDirection(const Pii::MatrixDirection& direction)
{
  PII_D;
  d->direction = direction;
  delete d->pMatResult;
  d->pMatResult = 0;
  d->uiPreviousType = PiiVariant::InvalidType;
}

Pii::MatrixDirection PiiMatrixComposer::direction() const { return _d()->direction; }
