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

#include "PiiVectorQuantizerOperation.h"
#include <PiiUtil.h>

PiiVectorQuantizerOperation::Data::Data(PiiClassification::LearnerCapabilities capabilities) :
  PiiClassifierOperation::Data(capabilities),
  distanceCombinationMode(PiiClassification::DistanceSum),
  dRejectThreshold(INFINITY),
  bMultiFeatureMeasure(false),
  bMustConfigureBoundaries(false)
{
}

PiiVectorQuantizerOperation::PiiVectorQuantizerOperation(PiiClassification::LearnerCapabilities capabilities) :
  PiiClassifierOperation(new Data(capabilities))
{
  init();
}

PiiVectorQuantizerOperation::PiiVectorQuantizerOperation(Data* data) :
  PiiClassifierOperation(data)
{
  init();
}

void PiiVectorQuantizerOperation::init()
{
  PII_D;
  addSocket(d->pBoundaryInput = new PiiInputSocket("boundaries"));
  d->pBoundaryInput->setOptional(true);
  //_pBoundaryInput->setDisplayName(tr("Feature boundaries for multi-feature vectors."));

  addSocket(d->pVectorIndexOutput = new PiiOutputSocket("model index"));
  
  addSocket(d->pDistanceOutput = new PiiOutputSocket("distance"));
  d->pDistanceOutput->setProperty("min", -INFINITY);
  d->pDistanceOutput->setProperty("max", INFINITY);
  d->pDistanceOutput->setProperty("resolution", 0);
  d->pDistanceOutput->setProperty("displayName", tr("distance"));
  //_pDistanceOutput->setDisplayName(tr("Distance to closest code
  //vector"));
  setDistanceMeasure("PiiSquaredGeometricDistance");
}

PiiVectorQuantizerOperation::~PiiVectorQuantizerOperation()
{
}

PiiInputSocket* PiiVectorQuantizerOperation::boundaryInput()
{
  return _d()->pBoundaryInput;
}

PiiOutputSocket* PiiVectorQuantizerOperation::vectorIndexOutput()
{
  return _d()->pVectorIndexOutput;
}

PiiOutputSocket* PiiVectorQuantizerOperation::distanceOutput()
{
  return _d()->pDistanceOutput;
}

void PiiVectorQuantizerOperation::setDistanceMeasure(const QString& name)
{
  _d()->lstDistanceMeasures = QStringList() << name;
}
  
QString PiiVectorQuantizerOperation::distanceMeasure() const
{
  const PII_D;
  if (d->lstDistanceMeasures.size() == 1)
    return d->lstDistanceMeasures[0];
  return "PiiMultiFeatureDistance";
}

double PiiVectorQuantizerOperation::labelForIndex(int index) const
{
  const PII_D;
  double dLabel = index;
  if (d->vecClassLabels.size() != 0)
    {
      if (d->vecClassLabels.size() > index && index >= 0)
        dLabel = d->vecClassLabels[index];
      else
        dLabel = NAN;
    }
  else if (index == -1)
    dLabel = NAN;
  return dLabel;
}

void PiiVectorQuantizerOperation::setDistanceMeasures(const QStringList& names) { _d()->lstDistanceMeasures = names; }
QStringList PiiVectorQuantizerOperation::distanceMeasures() const { return _d()->lstDistanceMeasures; }
double PiiVectorQuantizerOperation::rejectThreshold() const { return _d()->dRejectThreshold; }
void PiiVectorQuantizerOperation::setRejectThreshold(double rejectThreshold) { _d()->dRejectThreshold = rejectThreshold; }
void PiiVectorQuantizerOperation::setModels(const PiiVariant& models) { _d()->varModels = models; }
PiiVariant PiiVectorQuantizerOperation::models() const { return _d()->varModels; }
void PiiVectorQuantizerOperation::setDistanceWeights(const QVariantList& distanceWeights) { _d()->lstDistanceWeights = distanceWeights; }
QVariantList PiiVectorQuantizerOperation::distanceWeights() const { return _d()->lstDistanceWeights; }
void PiiVectorQuantizerOperation::setDistanceCombinationMode(PiiClassification::DistanceCombinationMode distanceCombinationMode)
{ _d()->distanceCombinationMode = distanceCombinationMode; }
PiiClassification::DistanceCombinationMode PiiVectorQuantizerOperation::distanceCombinationMode() const { return _d()->distanceCombinationMode; }
void PiiVectorQuantizerOperation::setClassLabels(const QVariantList& labels) { _d()->vecClassLabels = Pii::variantsToVector<double>(labels); }
QVariantList PiiVectorQuantizerOperation::classLabels() const { return Pii::vectorToVariants(_d()->vecClassLabels); }
