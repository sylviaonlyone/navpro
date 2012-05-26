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

#ifndef _PIIKNNCLASSIFIEROPERATION_H
#define _PIIKNNCLASSIFIEROPERATION_H

#include "PiiVectorQuantizerOperation.h"
#include "PiiKnnClassifier.h"
#include "PiiSampleSetCollector.h"

/**
 * An operation that classifies samples according to the k nearest
 * neighbors rule. See PiiClassifierOperation and PiiKnnClassifier for
 * more information.
 *
 * @ingroup PiiClassificationPlugin
 */
class PiiKnnClassifierOperation : public PiiVectorQuantizerOperation
{
  Q_OBJECT

  /**
   * The number of nearest neighbors the classifier considers. The
   * default is 5. Setting this value to one makes the classifier use
   * the nearest neighbor rule.
   */
  Q_PROPERTY(int k READ k WRITE setK);

public:
  template <class SampleSet> class Template;

  void setK(int k);
  int k() const;

protected:
  /// Returns @p false.
  virtual bool needsThread() const;
  
  /// @internal
  class Data : public PiiVectorQuantizerOperation::Data
  {
  public:
    Data();
    int k;
  };
  PII_D_FUNC;
  
  /// @internal
  PiiKnnClassifierOperation(Data* data);
};

/// @internal
template <class SampleSet> class PiiKnnClassifierOperation::Template :
  public PiiKnnClassifierOperation
{
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  Template();
  
  int bufferedSampleCount() const;
  int featureCount() const;

protected:
  void check(bool reset);
  double classify();
  double learnOne(double label, double weight);
  void collectSample(double label, double weight);
  void replaceClassifier();
  void resizeBatch(int newSize);
  void resetClassifier();
  
private:
  /// @internal
  class Data : public PiiKnnClassifierOperation::Data
  {
  public:
    Data();
    PiiKnnClassifier<SampleSet> classifier;
    PiiSampleSetCollector<SampleSet> onlineCollector, offlineCollector;
  };
  PII_D_FUNC;
};

template <class SampleSet> void PiiKnnClassifierOperation::Template<SampleSet>::check(bool reset)
{
  PII_D;
  PiiVectorQuantizerOperation::check(d->classifier, reset);
  d->classifier.setClassLabels(d->vecClassLabels);
  if (d->k < 1)
    PII_THROW(PiiExecutionException, tr("k cannot be less than one"));
  d->classifier.setK(d->k);
  d->offlineCollector.setBatchSize(learningBatchSize());
  d->offlineCollector.setFullBufferBehavior(fullBufferBehavior());
}

template <class SampleSet> PiiKnnClassifierOperation::Template<SampleSet>::Data::Data() :
  onlineCollector(&classifier.models(), &classifier.classLabels())
{
}

template <class SampleSet> PiiKnnClassifierOperation::Template<SampleSet>::Template() :
  PiiKnnClassifierOperation(new Data)
{
}

template <class SampleSet> int PiiKnnClassifierOperation::Template<SampleSet>::bufferedSampleCount() const
{
  return _d()->offlineCollector.sampleCount();
}

template <class SampleSet> int PiiKnnClassifierOperation::Template<SampleSet>::featureCount() const
{
  const PII_D;
  return qMax(d->classifier.featureCount(), d->offlineCollector.featureCount());
}

template <class SampleSet> double PiiKnnClassifierOperation::Template<SampleSet>::classify()
{
  double dLabel = PiiVectorQuantizerOperation::classify(_d()->classifier);
  classificationOutput()->emitObject(dLabel);
  return dLabel;
}

template <class SampleSet> double PiiKnnClassifierOperation::Template<SampleSet>::learnOne(double label, double /*weight*/)
{
  PiiVectorQuantizerOperation::learnOne(_d()->onlineCollector, label);
  classificationOutput()->emitObject(label);
  return label;
}

template <class SampleSet>
void PiiKnnClassifierOperation::Template<SampleSet>::collectSample(double label, double /*weight*/)
{
  PiiVectorQuantizerOperation::learnOne(_d()->offlineCollector, label);
}

template <class SampleSet> void PiiKnnClassifierOperation::Template<SampleSet>::replaceClassifier()
{
  PII_D;
  d->classifier.setModels(*d->offlineCollector.samples());
  d->classifier.setClassLabels(*d->offlineCollector.classLabels());
  d->onlineCollector.setSamples(&d->classifier.models());
  d->onlineCollector.setClassLabels(&d->classifier.classLabels());
  d->offlineCollector.setSamples(0);
  d->offlineCollector.setClassLabels(0);

  d->varModels = PiiVariant(d->classifier.models());
  d->vecClassLabels = d->classifier.classLabels();
}

template <class SampleSet> void PiiKnnClassifierOperation::Template<SampleSet>::resetClassifier()
{
  PII_D;
  PiiSampleSet::clear(d->classifier.models());
  d->classifier.classLabels().clear();
}

template <class SampleSet> void PiiKnnClassifierOperation::Template<SampleSet>::resizeBatch(int newSize)
{
  _d()->offlineCollector.resize(newSize);
}

#endif //_PIIKNNCLASSIFIEROPERATION_H
