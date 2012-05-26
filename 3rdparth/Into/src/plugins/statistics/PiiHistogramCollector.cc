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

#include "PiiHistogramCollector.h"
#include <PiiYdinTypes.h>
#include <PiiMath.h>

PiiHistogramCollector::Data::Data() :
  iBinCount(256),
  bSyncConnected(false),
  outputMode(FixedLengthOutput),
  pSyncObject(0),
  bNormalized(false)
{
}

PiiHistogramCollector::PiiHistogramCollector() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("sync"));
  inputAt(0)->setOptional(true);
  addSocket(new PiiInputSocket("data"));
  inputAt(1)->setGroupId(1);

  addSocket(new PiiOutputSocket("sync"));
  addSocket(new PiiOutputSocket("y"));
  addSocket(new PiiOutputSocket("x"));
}

void PiiHistogramCollector::check(bool reset)
{
  PII_D;
  d->bSyncConnected = inputAt(0)->isConnected();
  inputAt(1)->setGroupId(d->bSyncConnected ? 1 : 0);

  if (d->outputMode == FixedLengthOutput)
    {
      d->matHistogram = PiiMatrix<int>(1, d->iBinCount);
      d->matX = PiiMatrix<int>(1, d->iBinCount);
      for (int i=0; i<d->iBinCount; ++i)
        d->matX(0,i) = i;
    }
  else
    {
      d->matHistogram = PiiMatrix<int>::padded(1,0,d->iBinCount * sizeof(int));
      d->matX = PiiMatrix<int>::padded(1,0,d->iBinCount * sizeof(int));
    }
  
  PiiDefaultOperation::check(reset);
}

void PiiHistogramCollector::emitHistogram()
{
  PII_D;
  if (!d->bNormalized)
    outputAt(1)->emitObject(d->matHistogram);
  else
    outputAt(1)->emitObject(d->matHistogram.mapped(std::multiplies<float>(),
                                                   1.0 / Pii::sumAll<int>(d->matHistogram)));
  outputAt(2)->emitObject(d->matX);
}

void PiiHistogramCollector::syncEvent(SyncEvent* event)
{
  PII_D;
  if (event->type() == SyncEvent::EndInput)
    {
      emitObject(d->pSyncObject);
      emitHistogram();
      outputAt(0)->endDelay();
      outputAt(1)->endDelay();
      outputAt(2)->endDelay();

      //clear the histogram
      for ( int i=0; i<d->matHistogram.columns(); i++)
        d->matHistogram(0,i) = 0;
    }
}

void PiiHistogramCollector::process()
{
  PII_D;
  if (activeInputGroup() == inputAt(1)->groupId())
    {
      PiiVariant obj = inputAt(1)->firstObject();
      
      switch (obj.type())
        {
          PII_PRIMITIVE_CASES(addPrimitive, obj);
          PII_PRIMITIVE_MATRIX_CASES(addMatrix, obj);
        default:
          PII_THROW_UNKNOWN_TYPE(inputAt(1));
        }
    }
  else
    {
      // Just store the sync object now and delay synchronized
      // outputs.
      d->pSyncObject = readInput();
      outputAt(0)->startDelay();
      outputAt(1)->startDelay();
      outputAt(2)->startDelay();
    }
}

void PiiHistogramCollector::addToHistogram(int element)
{
  PII_D;
  if (d->outputMode == FixedLengthOutput)
    {
      if (element >=0 && element < d->matHistogram.columns())
        ++d->matHistogram(0,element);
    }
  else
    {
      if (d->matHistogram.columns() > 0)
        {
          // Look for the element in the list of already seen values.
          int iColumn = qLowerBound(d->matX.rowBegin(0), d->matX.rowEnd(0), element) - d->matX.rowBegin(0);
          // If this value already exists, increase its value
          if (iColumn < d->matX.columns() && d->matX(0, iColumn) == element)
            ++d->matHistogram(0, iColumn);
          else
            {
              // Add the new coordinate to the list of received x coordinates.
              d->matX.insertColumn(iColumn, element);
              // Initialize the histogram entry to one.
              d->matHistogram.insertColumn(iColumn, 1);
            }
        }
    }
}

template <class T> void PiiHistogramCollector::addPrimitive(const PiiVariant& obj)
{
  PII_D;
  addToHistogram(int(obj.valueAs<T>()));
  if (!d->bSyncConnected)
    emitHistogram();
}

template <class T> void PiiHistogramCollector::addMatrix(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> matrix = obj.valueAs<PiiMatrix<T> >();
  T element;
  foreach (element, matrix)
    addToHistogram(int(element));
  if (!d->bSyncConnected)
    emitHistogram();
}

void PiiHistogramCollector::setBinCount(int binCount) { _d()->iBinCount = binCount; }
int PiiHistogramCollector::binCount() const { return _d()->iBinCount; }
void PiiHistogramCollector::setOutputMode(const OutputMode& outputMode) { _d()->outputMode = outputMode; }
PiiHistogramCollector::OutputMode PiiHistogramCollector::outputMode() const { return _d()->outputMode; }

void PiiHistogramCollector::setNormalized(bool normalized) { _d()->bNormalized = normalized; }
bool PiiHistogramCollector::normalized() const { return _d()->bNormalized; }
