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

#include "PiiConfusionMatrixBuilder.h"
#include <PiiYdinTypes.h>
#include <QTextStream>

PiiConfusionMatrixBuilder::Data::Data() :
  confusionMatrix(0)
{
}

PiiConfusionMatrixBuilder::PiiConfusionMatrixBuilder() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("real class"));
  addSocket(new PiiInputSocket("classification"));
  addSocket(new PiiInputSocket("distance"));
  inputAt(2)->setOptional(true);
  addSocket(new PiiOutputSocket("confusion matrix"));
  addSocket(new PiiOutputSocket("distance matrix"));
}

void PiiConfusionMatrixBuilder::setDistanceMatrix(const PiiVariant& distanceMatrix)
{
  PII_D;
  if (distanceMatrix.type() == PiiYdin::DoubleMatrixType)
    d->distanceMatrix = distanceMatrix.valueAs<PiiMatrix<double> >();
  else
    d->distanceMatrix.resize(0,0);
}

void PiiConfusionMatrixBuilder::setConfusionMatrix(const PiiVariant& confusionMatrix)
{
  PII_D;
  if (confusionMatrix.type() == PiiYdin::IntMatrixType)
    d->confusionMatrix = confusionMatrix.valueAs<PiiMatrix<int> >();
  else
    d->confusionMatrix = PiiConfusionMatrix(0);
}

void PiiConfusionMatrixBuilder::check(int reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if (reset)
    {
      d->confusionMatrix.resize(0,0);
      d->distanceMatrix.resize(0,0);
    }
}


void PiiConfusionMatrixBuilder::process()
{
  PII_D;
  int realClass = PiiYdin::primitiveAs<int>(inputAt(0)),
    classification = PiiYdin::primitiveAs<int>(inputAt(1));

  d->confusionMatrix.addEntry(realClass, classification);

  emitObject(d->confusionMatrix);

  if (inputAt(2)->isConnected())
    {
      double distance = PiiYdin::primitiveAs<double>(inputAt(2));
      addDistance(realClass, classification, distance);
      outputAt(1)->emitObject(d->distanceMatrix);
    }
}

void PiiConfusionMatrixBuilder::addDistance(int correctClass, int classification, double distance)
{
  PII_D;
  if (correctClass < 0 || classification < 0)
    return;
  
  int max = qMax(correctClass, classification) + 1;
  if (max > d->distanceMatrix.rows())
    d->distanceMatrix.resize(max,max);

  double mu = 1.0 / d->confusionMatrix(correctClass, classification);
  // Calculate mean distance iteratively
  d->distanceMatrix(correctClass,classification) = (1.0 - mu) * d->distanceMatrix(correctClass,classification) +
    mu * distance;
}

PiiVariant PiiConfusionMatrixBuilder::distanceMatrix() const { return PiiVariant(_d()->distanceMatrix); }
PiiVariant PiiConfusionMatrixBuilder::confusionMatrix() const { return PiiVariant(static_cast<const PiiMatrix<int>&>(_d()->confusionMatrix)); }

QString PiiConfusionMatrixBuilder::matrixAsText(const QStringList& classNames, int space) const
{
  const PII_D;
  QString strResult;
  QTextStream stream(&strResult, QIODevice::WriteOnly);
  d->confusionMatrix.print(stream, classNames, space);
  return strResult;
}
