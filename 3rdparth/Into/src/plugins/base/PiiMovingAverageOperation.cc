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

#include "PiiMovingAverageOperation.h"
#include <PiiYdinTypes.h>
#include <PiiTypeTraits.h>
#include <complex>
#include <PiiMath.h>

PiiMovingAverageOperation::Data::Data() :
  iWindowSize(2),
  dRangeMin(0),
  dRangeMax(0),
  dRange(0),
  uiType(PiiVariant::InvalidType),
  bForceInputType(false)
{
}

PiiMovingAverageOperation::PiiMovingAverageOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("average"));
}

void PiiMovingAverageOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);
  if (reset)
    {
      d->uiType = PiiVariant::InvalidType;
      d->lstBuffer.clear();
    }
}

void PiiMovingAverageOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_CASES(average, obj);
      PII_COMPLEX_CASES(average, obj);
      PII_NUMERIC_MATRIX_CASES(matrixAverage, obj);
      PII_COMPLEX_MATRIX_CASES(matrixAverage, obj);

    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

// Type conversion "table"
template <class T> struct AverageTraits { typedef float Type; }; // everything is converted to float, except ...
//template <> struct AverageTraits<long double> { typedef long double Type; };
template <> struct AverageTraits<double> { typedef double Type; };
template <> struct AverageTraits<long long int> { typedef double Type; };
template <> struct AverageTraits<long int> { typedef double Type; };
template <> struct AverageTraits<unsigned long long int> { typedef double Type; };
template <> struct AverageTraits<unsigned long int> { typedef double Type; };
// Complex and PiiMatrix reuse the above
template <class T> struct AverageTraits<std::complex<T> > { typedef std::complex<typename AverageTraits<T>::Type > Type; };
template <class T> struct AverageTraits<PiiMatrix<T> > { typedef PiiMatrix<typename AverageTraits<T>::Type > Type; };

template <class T> void PiiMovingAverageOperation::average(const PiiVariant& obj)
{
  averageTemplate<T, typename AverageTraits<T>::Type>(obj);
}
template <class T> void PiiMovingAverageOperation::matrixAverage(const PiiVariant& obj)
{
  averageTemplate<PiiMatrix<T>, typename AverageTraits<PiiMatrix<T> >::Type>(obj);
}

template <class T, class ResultType> void PiiMovingAverageOperation::averageTemplate(const PiiVariant& obj)
{
  PII_D;
  if (d->lstBuffer.isEmpty())
    {
      d->lstBuffer << obj;
      if ( d->bForceInputType )
        d->lstOutputs[0]->emitObject(obj.valueAs<T>());
      else
        {
          ResultType result(obj.valueAs<T>());
          d->lstOutputs[0]->emitObject(result);
        }
      d->uiType = Pii::typeId<T>();
      return;
    }
  
  if (obj.type() != d->uiType)
    PII_THROW(PiiExecutionException, tr("Cannot average objects of different type."));

  // Add the object to the buffer, and remove the first one if window
  // size is exceeded.
  d->lstBuffer << obj;
  while (d->lstBuffer.size() > d->iWindowSize)
    d->lstBuffer.removeFirst();

  // Calculate average
  int index = 0;
  QLinkedList<PiiVariant>::iterator i = d->lstBuffer.begin();
  ResultType result((*i).valueAs<T>());
  i++, index++;
  try
    {
      while (i != d->lstBuffer.end())
        {
          // Same type -> no conversion
          if (Pii::typeId<T>() == Pii::typeId<ResultType>())
            add(result, (*i).valueAs<ResultType>(), index);
          // Different type -> conversion needed
          else
            add(result, ResultType((*i).valueAs<T>()), index);
          i++, index++;;
        }
    }
  catch (PiiMathException&)
    {
      PII_THROW(PiiExecutionException, tr("Cannot average matrices of different size."));
    }

  scale(result, d->lstBuffer.size());

  if ( d->bForceInputType )
    emitObject((T)result);
  else
    emitObject(result);
}

template <class T> void PiiMovingAverageOperation::addImpl(T& op1, T op2, int index)
{
  PII_D;
  if (d->dRange == 0)
    op1 += op2;
  else
    {
      T diff = Pii::abs(op2 - op1);
      // Can we reduce difference by adding "range" to op2?
      if (op2 < op1 && Pii::abs(op2 + (T)d->dRange - op1) < diff)
        op2 += (T)d->dRange;
      // Can we reduce difference by subtracting "range" from op2?
      else if (op2 > op1 && Pii::abs(op2 - (T)d->dRange - op1) < diff)
        op2 -= (T)d->dRange;

      op1 = (op1 * index + op2) / (index+1);
    }
}

template <class T> void PiiMovingAverageOperation::scaleImpl(T& result, int cnt)
{
  PII_D;
  if (d->dRange == 0)
    result /= cnt;
  else
    {
      // Rotate result back to the allowed range
      while (result > d->dRangeMax)
        result -= d->dRange;
      while (result < d->dRangeMin)
        result += d->dRange;
    }
}

// Sums up and scales back all aggregate types.
struct AggregateHandler
{
  template <class T> static inline void addImpl(T& op1, const T& op2, int /*index*/)
  {
    op1 += op2;
  }

  template <class T> static inline void scaleImpl(T& result, int cnt)
  {
    result /= cnt;
  }
};


// Template code to split primitive types into separate functions
template <class T> void PiiMovingAverageOperation::add(T& op1, const T& op2, int index)
{
  Pii::IfClass<Pii::IsPrimitive<T>, PiiMovingAverageOperation, AggregateHandler>::Type::addImpl(op1, op2, index);
}

template <class T> void PiiMovingAverageOperation::scale(T& result, int cnt)
{
  Pii::IfClass<Pii::IsPrimitive<T>, PiiMovingAverageOperation, AggregateHandler>::Type::scaleImpl(result, cnt);
}

void PiiMovingAverageOperation::setWindowSize(int windowSize) { _d()->iWindowSize = windowSize; }
int PiiMovingAverageOperation::windowSize() const { return _d()->iWindowSize; }
void PiiMovingAverageOperation::setRangeMin(double rangeMin) { PII_D; d->dRangeMin = rangeMin; d->dRange = d->dRangeMax-d->dRangeMin; }
double PiiMovingAverageOperation::rangeMin() const { return _d()->dRangeMin; }
void PiiMovingAverageOperation::setRangeMax(double rangeMax) { PII_D; d->dRangeMax = rangeMax; d->dRange = d->dRangeMax-d->dRangeMin; }
double PiiMovingAverageOperation::rangeMax() const { return _d()->dRangeMax; }
void PiiMovingAverageOperation::setForceInputType(bool forceInputType) { _d()->bForceInputType = forceInputType; }
bool PiiMovingAverageOperation::forceInputType() const { return _d()->bForceInputType; }
