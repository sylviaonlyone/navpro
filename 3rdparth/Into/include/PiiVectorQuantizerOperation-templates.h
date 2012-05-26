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

#ifndef _PIIVECTORQUANTIZEROPERATION_H
# error "Never use <PiiVectorQuantizerOperation-templates.h> directly; include <PiiVectorQuantizerOperation.h> instead."
#endif

#include <PiiYdinTypes.h>
#include "PiiMultiFeatureDistance.h"
#include <QFile>

#include <PiiYdinResources.h>

template <class Measure>
Measure* PiiVectorQuantizerOperation::createDistanceMeasure(const QString& name)

{
  Measure* pMeasure = PiiYdin::createResource<Measure>(name);
  if (pMeasure == 0)
    PII_THROW(PiiExecutionException, tr("Cannot create %1.").arg(name));
  return pMeasure;
}

template <class SampleSet>
void PiiVectorQuantizerOperation::setDistanceMeasure(PiiVectorQuantizer<SampleSet>& classifier)

{
  typedef typename PiiSampleSet::Traits<SampleSet>::FeatureType T;
  typedef typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator ConstFeatureIterator;
  PII_D;
  if (d->lstDistanceMeasures.size() > 1)
    {
      QString strMultiName = QString("PiiMultiFeatureDistance<%1>").arg(PiiYdin::resourceName<T>());
      typedef PII_POLYMORPHIC_MEASURE(PiiMultiFeatureDistance) MeasureType;
      PiiSmartPtr<MeasureType> pMeasureList(new MeasureType);
      for (int i=0; i<d->lstDistanceMeasures.size(); ++i)
        {
          QString strName = QString("%1<%2>")
            .arg(d->lstDistanceMeasures[i])
            .arg(PiiYdin::resourceName<T>());
          pMeasureList->append(createDistanceMeasure<PiiDistanceMeasure<ConstFeatureIterator> >(strName));
        }
      pMeasureList->setWeights(Pii::variantsToVector<double>(d->lstDistanceWeights));
      pMeasureList->setCombinationMode(d->distanceCombinationMode);
      classifier.setDistanceMeasure(pMeasureList.release());
      d->bMultiFeatureMeasure = true;
    }
  else if (d->lstDistanceMeasures.size() == 1)
    {
      QString strName = QString("%1<%2>")
        .arg(d->lstDistanceMeasures[0])
        .arg(PiiYdin::resourceName<T>());
      classifier.setDistanceMeasure(createDistanceMeasure<PiiDistanceMeasure<ConstFeatureIterator> >(strName));
      d->bMultiFeatureMeasure = false;
    }
  else
    PII_THROW(PiiExecutionException, tr("No distance measures specified."));
}

template <class SampleSet>
void PiiVectorQuantizerOperation::check(PiiVectorQuantizer<SampleSet>& classifier, bool reset)

{
  PII_D;
  PiiClassifierOperation::check(reset);
  setModels(classifier);
  if (d->vecClassLabels.size() != 0 &&
      d->vecClassLabels.size() != classifier.modelCount())
    PII_THROW(PiiExecutionException, tr("If class labels are given, their number must match that of the model samples."));
  setDistanceMeasure(classifier);

  if (reset)
    d->bMustConfigureBoundaries = d->bMultiFeatureMeasure;
}

template <class SampleSet>
void PiiVectorQuantizerOperation::setFeatureBoundaries(PiiVectorQuantizer<SampleSet>& classifier)

{
  PII_D;
  if (d->bMustConfigureBoundaries)
    {
      typedef typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator ConstFeatureIterator;
      PiiVariant obj = d->pBoundaryInput->firstObject();
      // We know distance measure is a multi-feature distance.
      // Configure the distance with the newly read feature vector boundaries.
      if (obj.type() == PiiYdin::IntMatrixType)
        {
          typedef PII_POLYMORPHIC_MEASURE(PiiMultiFeatureDistance) MeasureType;
          static_cast<MeasureType*>(classifier.distanceMeasure())->setBoundaries(obj.valueAs<PiiMatrix<int> >());
        }
      else
        PII_THROW_UNKNOWN_TYPE(d->pBoundaryInput);
      d->bMustConfigureBoundaries = false;
    }
}

template <class SampleSet>
double PiiVectorQuantizerOperation::classify(PiiVectorQuantizer<SampleSet>& classifier,
                                             int* vectorIndex,
                                             double* distance)
{
  PII_D;
  setFeatureBoundaries(classifier);

  double dDistance = INFINITY;
  PiiClassification::FeatureReader<SampleSet> readFeatures;
  int iFeatures = classifier.featureCount();
  int iVectorIndex = classifier.findClosestMatch(readFeatures(featureInput(), &iFeatures), &dDistance);
  d->pVectorIndexOutput->emitObject(iVectorIndex);
  d->pDistanceOutput->emitObject(dDistance);

  if (distance != 0)
    *distance = dDistance;
  if (vectorIndex != 0)
    *vectorIndex = iVectorIndex;
  
  return labelForIndex(iVectorIndex);
}

template <class SampleSet>
void PiiVectorQuantizerOperation::setModels(PiiVectorQuantizer<SampleSet>& classifier)
{
  PII_D;
  if (d->varModels.isValid())
    {
      if (d->varModels.type() != Pii::typeId<SampleSet>())
        PII_THROW(PiiExecutionException,
                  tr("Model set is of incorrect type (0x%1). Should be 0x%2.")
                  .arg(d->varModels.type(), 0, 16)
                  .arg(Pii::typeId<SampleSet>(), 0, 16));
      classifier.setModels(d->varModels.valueAs<SampleSet>());
    }
  else
    classifier.setModels(PiiSampleSet::create<SampleSet>(0, 0));
}
