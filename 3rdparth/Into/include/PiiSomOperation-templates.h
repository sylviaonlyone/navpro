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

#ifndef _PIISOMOPERATION_H
# error "Never use <PiiSomOperation-templates.h> directly; include <PiiSomOperation.h> instead."
#endif

#include "PiiSampleSetCollector.h"

/**
 * @internal
 *
 * Implements the design pattern for QObject templates. Implements the
 * SOM classifier for PiiMatrix<T>.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class SampleSet> class PiiSomOperation::Template : public PiiSomOperation
{
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  Template();
  
  int bufferedSampleCount() const { return _d()->collector.sampleCount(); }
  int featureCount() const { return qMax(_d()->collector.featureCount(), _d()->pClassifier->featureCount()); }

  void check(bool reset);

protected:
  PiiClassification::SomTopology topology() const { return _d()->pClassifier->topology(); }
  void setTopology(PiiClassification::SomTopology topology) { _d()->pClassifier->setTopology(topology); }
  PiiClassification::SomRateFunction rateFunction() const { return _d()->pClassifier->rateFunction(); }
  void setRateFunction(PiiClassification::SomRateFunction rateFunction) { _d()->pClassifier->setRateFunction(rateFunction); }
  int iterationNumber() const { return _d()->pClassifier->iterationNumber(); }
  void setIterationNumber(int iterationNumber) { _d()->pClassifier->setIterationNumber(iterationNumber); }
  int learningLength() const { return _d()->pClassifier->learningLength(); }
  void setLearningLength(int learningLength) { _d()->pClassifier->setLearningLength(learningLength); }
  double initialRadius() const { return _d()->pClassifier->initialRadius(); }
  void setInitialRadius(double initialRadius) { _d()->pClassifier->setInitialRadius(initialRadius); }
  double initialLearningRate() const { return _d()->pClassifier->initialLearningRate(); }
  void setInitialLearningRate(double initialLearningRate) { _d()->pClassifier->setInitialLearningRate(initialLearningRate); }
  double currentLearningRate() const { return _d()->pClassifier->currentLearningRate(); }
  double currentRadius() const { return _d()->pClassifier->currentRadius(); }
  PiiClassification::SomInitMode initMode() const { return _d()->pClassifier->initMode(); }
  void setInitMode(PiiClassification::SomInitMode mode) { _d()->pClassifier->setInitMode(mode); }
  PiiClassification::SomLearningAlgorithm learningAlgorithm() const { return _d()->pClassifier->learningAlgorithm(); }
  void setLearningAlgorithm(PiiClassification::SomLearningAlgorithm algorithm) { _d()->pClassifier->setLearningAlgorithm(algorithm); }

  double classify();
  double learnOne(double label, double weight);
  void collectSample(double label, double weight);
  bool learnBatch();
  void replaceClassifier();
  void resizeBatch(int newSize);
  void resetClassifier();
  void finishOnlineLearning();
  
  /// @internal
  class Data : public PiiSomOperation::Data
  {
  public:
    Data();
    ~Data();
    PiiSom<SampleSet>* pClassifier, *pNewClassifier;
    PiiSampleSetCollector<SampleSet> collector;
  };
  PII_D_FUNC;

  /// @internal
  Template(Data* d);

private:
  PiiSom<SampleSet>* createSom();
};

// Codebook is stored as a property nowadays
PII_SERIALIZATION_VERSION_TEMPLATE(PiiSomOperation::Template, 1);

template <class SampleSet> PiiSomOperation::Template<SampleSet>::Data::Data() :
  pClassifier(new PiiSom<SampleSet>(10,10)), pNewClassifier(0)
{
  collector.setCollectLabels(false);
}

template <class SampleSet> PiiSomOperation::Template<SampleSet>::Data::~Data()
{
  delete pClassifier;
  delete pNewClassifier;
}

template <class SampleSet> PiiSomOperation::Template<SampleSet>::Template(Data *dat) :
  PiiSomOperation(dat)
{
}

template <class SampleSet> PiiSomOperation::Template<SampleSet>::Template() :
  PiiSomOperation(new Data)
{
}

template <class SampleSet> void PiiSomOperation::Template<SampleSet>::check(bool reset)
{
  PII_D;

  if (width() <= 0 || height() <= 0)
    PII_THROW(PiiExecutionException, tr("SOM map size is zero."));
  d->pClassifier->setSize(width(), height());

  PiiVectorQuantizerOperation::check(*d->pClassifier, reset);

  d->collector.setBatchSize(learningBatchSize());
  d->collector.setFullBufferBehavior(fullBufferBehavior());
}

template <class SampleSet> double PiiSomOperation::Template<SampleSet>::classify()
{
  PII_D;
  int iCodeBookIndex = -1;
  double dDistance = 0;
  double dLabel = PiiVectorQuantizerOperation::classify(*d->pClassifier, &iCodeBookIndex, &dDistance);
  d->pXOutput->emitObject(iCodeBookIndex % d->pClassifier->width());
  d->pYOutput->emitObject(iCodeBookIndex / d->pClassifier->width());
  classificationOutput()->emitObject(dLabel);
  return dLabel;
}

template <class SampleSet> double PiiSomOperation::Template<SampleSet>::learnOne(double label, double /*weight*/)
{
  PII_D;
  setFeatureBoundaries(*d->pClassifier);
  int iCodeBookIndex = PiiVectorQuantizerOperation::learnOne(*d->pClassifier, label);
  d->pXOutput->emitObject(iCodeBookIndex % d->pClassifier->width());
  d->pYOutput->emitObject(iCodeBookIndex / d->pClassifier->width());
  double dLabel = this->labelForIndex(iCodeBookIndex);
  classificationOutput()->emitObject(dLabel);
  return dLabel;
}

template <class SampleSet> void PiiSomOperation::Template<SampleSet>::finishOnlineLearning()
{
  // Store the code book to the operation's property.
  setModels(PiiVariant(_d()->pClassifier->models()));
}

template <class SampleSet>
void PiiSomOperation::Template<SampleSet>::collectSample(double label, double weight)
{
  PiiVectorQuantizerOperation::learnOne(_d()->collector, label, weight);
}

template <class SampleSet> void PiiSomOperation::Template<SampleSet>::resetClassifier()
{
  PII_D;
  PiiSom<SampleSet> *pSom = createSom();
  pSom->setIterationNumber(0);
  
  delete d->pClassifier;
  d->pClassifier = pSom;
  setModels(PiiVariant());
}

template <class SampleSet> bool PiiSomOperation::Template<SampleSet>::learnBatch()
{
  PII_D;
  d->pNewClassifier = createSom();
  bool bSuccess = PiiClassifierOperation::learnBatch(*d->pNewClassifier,
                                                     *d->collector.samples(),
                                                     *d->collector.classLabels());
  if (!bSuccess)
    {
      delete d->pNewClassifier;
      d->pNewClassifier = 0;
    }
  
  return bSuccess;
}

template <class SampleSet> void PiiSomOperation::Template<SampleSet>::replaceClassifier()
{
  PII_D;
  delete d->pClassifier;
  d->pClassifier = d->pNewClassifier;
  d->pNewClassifier = 0;
  d->varModels = PiiVariant(d->pClassifier->models());
}

template <class SampleSet> PiiSom<SampleSet>* PiiSomOperation::Template<SampleSet>::createSom()
{
  PII_D;
  // Configure the SOM with currently active parameters.
  PiiSom<SampleSet>* pSom = new PiiSom<SampleSet>(this->width(), this->height());
  pSom->setInitialRadius(d->pClassifier->initialRadius());
  pSom->setTopology(d->pClassifier->topology());
  pSom->setInitialLearningRate(d->pClassifier->initialLearningRate());
  pSom->setLearningLength(d->pClassifier->learningLength());
  pSom->setRateFunction(d->pClassifier->rateFunction());
  pSom->setIterationNumber(d->pClassifier->iterationNumber());
  pSom->setInitMode(d->pClassifier->initMode());
  pSom->setLearningAlgorithm(d->pClassifier->learningAlgorithm());
  return pSom;
}

template <class SampleSet> void PiiSomOperation::Template<SampleSet>::resizeBatch(int newSize)
{
  _d()->collector.resize(newSize);
}
