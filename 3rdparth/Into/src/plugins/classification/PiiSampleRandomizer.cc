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

#include "PiiSampleRandomizer.h"

#include <PiiYdinTypes.h>
#include <QtAlgorithms>
#include <cstdlib>
#include <PiiUtil.h>

PiiSampleRandomizer::Data::Data() :
  iClassIndex(0),
  iMaxSamples(0),
  iCurrentSampleIndex(0),
  bRandomSampling(false)
{
}

PiiSampleRandomizer::PiiSampleRandomizer() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("trigger"));
  addSocket(new PiiOutputSocket("name"));
  addSocket(new PiiOutputSocket("label"));
  inputAt(0)->setOptional(true);
}

void PiiSampleRandomizer::setSampleNames(const QVariantList& sampleNames)
{
  PII_D;
  d->lstSampleNames = Pii::variantsToList<QStringList>(sampleNames);
  d->lstSampleIndices.resize(d->lstSampleNames.size());
  qFill(d->lstSampleIndices.begin(), d->lstSampleIndices.end(), 0);
}

QVariantList PiiSampleRandomizer::sampleNames() const
{
  const PII_D;
  return Pii::listToVariants(d->lstSampleNames);
}

void PiiSampleRandomizer::setClassWeights(const QVariantList& classWeights)
{
  PII_D;
  d->lstClassWeights = Pii::variantsToList<double>(classWeights);
  int len = d->lstClassWeights.size();
  d->lstCumulativeWeights.resize(len);
  if (len > 0)
    {
      double sum = 0;
      for (int i=len; i--; )
        sum += d->lstClassWeights[i];

      d->lstCumulativeWeights[0] = d->lstClassWeights[0] / sum;
      for (int i=1; i<len; ++i)
        d->lstCumulativeWeights[i] = d->lstCumulativeWeights[i-1] + d->lstClassWeights[i] / sum;
      // Get rid of rounding errors
      d->lstCumulativeWeights.last() = 1.0;
    }
}

QVariantList PiiSampleRandomizer::classWeights() const
{
  const PII_D;
  return Pii::listToVariants(d->lstClassWeights);
}


void PiiSampleRandomizer::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if (d->lstSampleNames.size() == 0)
    PII_THROW(PiiExecutionException, tr("Sample names have not been set."));
  for (int i = d->lstSampleNames.size(); i--; )
    if (d->lstSampleNames[i].size() == 0)
      PII_THROW(PiiExecutionException, tr("The sample name list for class %1 is empty.").arg(i));
  // Weights have not been set -> balance all equally
  if (d->lstCumulativeWeights.size() != 0 &&
      d->lstCumulativeWeights.size() != d->lstSampleNames.size())
    PII_THROW(PiiExecutionException, tr("There must be an equal number of class names and class weights."));

  if (reset)
    {
      d->iCurrentSampleIndex = 0;
      d->iClassIndex = 0;
    }
}

void PiiSampleRandomizer::emitFromClass(int classIndex)
{
  PII_D;
  QStringList& names = d->lstSampleNames[classIndex];
  // Select a sample randomly from the class names
  if (d->bRandomSampling)
    emitObject(names[::rand() % names.size()]);
  else
    {
      emitObject(names[d->lstSampleIndices[classIndex]]);
      // Advance sample index and roll around if necessary
      d->lstSampleIndices[classIndex] = (d->lstSampleIndices[classIndex] + 1) % names.size();
    }
  // Emit class index
  outputAt(1)->emitObject(classIndex);
}


void PiiSampleRandomizer::process()
{
  PII_D;
  // Weights are set -> select randomly
  if (d->lstCumulativeWeights.size() > 0)
    {
      double p = double(::rand())/RAND_MAX; // p is in [0,1]

      for (int i=0; i<d->lstCumulativeWeights.size(); ++i)
        if (p <= d->lstCumulativeWeights[i])
          {
            emitFromClass(i);
            break;
          }
    }
  else
    {
      // Advance counter by one and roll back
      emitFromClass(d->iClassIndex);
      d->iClassIndex = (d->iClassIndex + 1) % d->lstSampleNames.size();
    }

  d->iCurrentSampleIndex++;
  if (d->iMaxSamples > 0 && !inputAt(0)->isConnected() &&
      d->iCurrentSampleIndex >= d->iMaxSamples)
    operationStopped();
}

void PiiSampleRandomizer::setRandomSampling(bool randomSampling) { _d()->bRandomSampling = randomSampling; }
bool PiiSampleRandomizer::randomSampling() const { return _d()->bRandomSampling; }
void PiiSampleRandomizer::setMaxSamples(int maxSamples) { _d()->iMaxSamples = maxSamples; }
int PiiSampleRandomizer::maxSamples() const { return _d()->iMaxSamples; }
int PiiSampleRandomizer::currentSampleIndex() const { return _d()->iCurrentSampleIndex; }
