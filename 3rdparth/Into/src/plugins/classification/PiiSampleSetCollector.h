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

#ifndef _PIISAMPLESETCOLLECTOR_H
#define _PIISAMPLESETCOLLECTOR_H

#include "PiiLearningAlgorithm.h"

/**
 * A learning algorithm that just collects all incoming data into a
 * sample set.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class SampleSet> class PiiSampleSetCollector :
  public PiiLearningAlgorithm<SampleSet>
{
public:
  typedef typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator ConstFeatureIterator;

  /**
   * Creates a new %PiiSampleSetCollector that creates the sample,
   * label and weight sets automatically and takes care of deleting
   * them.
   */
  PiiSampleSetCollector();

  /**
   * Creates a new sample set collector that appends samples to the
   * given sample set. %PiiSampleSetCollector doesn't take the
   * ownership of the pointers and it must remain valid during the
   * lifetime of this object. The label and weight lists will be
   * created internally.
   */
  PiiSampleSetCollector(SampleSet* sampleSet);

  /**
   * Creates a new sample set collector that appends samples to the
   * given sample set and label list. %PiiSampleSetCollector doesn't
   * take the ownership of the pointers, and they must remain valid
   * during the lifetime of this object. The weight list will be
   * created internally.
   */
  PiiSampleSetCollector(SampleSet* sampleSet, QVector<double>* labels);
  
  /**
   * Creates a new sample set collector that appends samples to the
   * given sample set, label list and weight list. 
   * %PiiSampleSetCollector doesn't take the ownership of the
   * pointers, and they must remain valid during the lifetime of this
   * object.
   */
  PiiSampleSetCollector(SampleSet* sampleSet, QVector<double>* labels, QVector<double>* weights);

  /// Destroys this sample set collector.
  ~PiiSampleSetCollector();

  /**
   * Replaces all collected samples and labels with the given new
   * ones.
   */
  void learn(const SampleSet& samples,
             const QVector<double>& labels,
             const QVector<double>& weights);

  /**
   * Appends @a sample to the sample set. If label collection is
   * enabled, @a label will be added to the label set. If weight
   * collection is enabled, @a weight will be added to the weight set. 
   * If @a length doesn't match the current number of features, the
   * current sample set will be cleared first. Returns the index of
   * the just added sample in the sample set.
   */
  double learnOne(ConstFeatureIterator sample, int length, double label, double weight) throw ();

  /**
   * Returns @p true.
   */
  bool converged() const throw ();

  /**
   * Returns @p NonSupervisedLearner | @p OnlineLearner | @p WeightedLearner;
   */
  PiiClassification::LearnerCapabilities capabilities() const;

  /**
   * Set the sample set. If the samples are set to 0, a new sample set
   * will be automatically created.
   */
  void setSamples(SampleSet* samples);
  /**
   * Returns the sample set.
   */
  SampleSet* samples() const;

  /**
   * Sets the class labels. If the class labels are set to 0, an empty
   * vector will be automatically created.
   */
  void setClassLabels(QVector<double>* labels);
  /**
   * Returns the class labels.
   */
  QVector<double>* classLabels() const;

  /**
   * Sets the sample weights. If the weights are set to 0, an empty
   * vector will be automatically created.
   */
  void setSampleWeights(QVector<double>* weights);
  /**
   * Returns the sample weights.
   */
  QVector<double>* sampleWeights() const;

  /**
   * Returns the number of samples currently in the sample set.
   */
  int sampleCount() const;

  /**
   * Returns the number of features in the stored samples. If the
   * sample set is empty, zero will be returned.
   */
  int featureCount() const;

  /**
   * Enables or disables the collecting of class labels. If class
   * labels are not collected, the class label list will remain empty.
   * The default value is @p true, unless only a sample list is given
   * in the constructor.
   */
  void setCollectLabels(bool collectLabels);
  /**
   * Returns @p true if class labels are being collected, @p false
   * otherwise.
   */
  bool collectLabels() const;

  /**
   * Enables or disables the collecting of sample weights. If sample
   * weights are not collected, the weight list will remain empty. The
   * default value is @p false, unless a weight list is explicitly
   * given in the constructor.
   */
  void setCollectWeights(bool collectWeights);
  /**
   * Returns @p true if sample weights are being collected, @p false
   * otherwise.
   */
  bool collectWeights() const;

  /**
   * Sets the maximum size for the buffer. -1 means no limit.
   */
  void setBatchSize(int batchSize);

  /**
   * Returns the batch size.
   */
  int batchSize() const;

  /**
   * Sets the strategy of handling a full buffer.
   */
  void setFullBufferBehavior(PiiClassification::FullBufferBehavior);

  /**
   * Returns the way full buffer is handled.
   */
  PiiClassification::FullBufferBehavior fullBufferBehavior() const;

  /**
   * Resizes the collected sample set and associated labels and weight
   * so that at most @a samples entries are be retained.
   */
  void resize(int samples);
  
private:
  class Data : public PiiLearningAlgorithm<SampleSet>::Data
  {
  public:
    Data();
    Data(SampleSet* samples);
    Data(SampleSet* samples, QVector<double>* labels);
    Data(SampleSet* samples, QVector<double>* labels, QVector<double>* weights);
    
    SampleSet* pSampleSet;
    QVector<double>* pvecLabels, *pvecWeights;
    bool bOwnSamples, bOwnLabels, bOwnWeights;
    bool bCollectLabels;
    bool bCollectWeights;
    int iBatchSize;
    int iSampleIndex;
    PiiClassification::FullBufferBehavior fullBufferBehavior;
  };
  PII_D_FUNC;
};

template <class SampleSet> PiiSampleSetCollector<SampleSet>::Data::Data() :
  pSampleSet(new SampleSet),
  pvecLabels(new QVector<double>),
  pvecWeights(new QVector<double>),
  bOwnSamples(true), bOwnLabels(true), bOwnWeights(true),
  bCollectLabels(true), bCollectWeights(false),
  iBatchSize(0), iSampleIndex(0),
  fullBufferBehavior(PiiClassification::OverwriteRandomSample)
{
}

template <class SampleSet> PiiSampleSetCollector<SampleSet>::Data::Data(SampleSet* samples) :
  pSampleSet(samples),
  pvecLabels(new QVector<double>),
  pvecWeights(new QVector<double>),
  bOwnSamples(false), bOwnLabels(true), bOwnWeights(true),
  bCollectLabels(false), bCollectWeights(false),
  iBatchSize(0), iSampleIndex(0),
  fullBufferBehavior(PiiClassification::OverwriteRandomSample)
{
}

template <class SampleSet> PiiSampleSetCollector<SampleSet>::Data::Data(SampleSet* samples,
                                                                        QVector<double>* labels) :
  pSampleSet(samples),
  pvecLabels(labels),
  pvecWeights(new QVector<double>),
  bOwnSamples(false), bOwnLabels(false), bOwnWeights(true),
  bCollectLabels(true), bCollectWeights(false),
  iBatchSize(0), iSampleIndex(0),
  fullBufferBehavior(PiiClassification::OverwriteRandomSample)
{
}

template <class SampleSet> PiiSampleSetCollector<SampleSet>::Data::Data(SampleSet* samples,
                                                                        QVector<double>* labels,
                                                                        QVector<double>* weights) :
  pSampleSet(samples),
  pvecLabels(labels),
  pvecWeights(weights),
  bOwnSamples(false), bOwnLabels(false), bOwnWeights(false),
  bCollectLabels(true), bCollectWeights(true),
  iBatchSize(0), iSampleIndex(0),
  fullBufferBehavior(PiiClassification::OverwriteRandomSample)
{
}

template <class SampleSet> PiiSampleSetCollector<SampleSet>::PiiSampleSetCollector() :
  PiiLearningAlgorithm<SampleSet>(new Data)
{}

template <class SampleSet> PiiSampleSetCollector<SampleSet>::PiiSampleSetCollector(SampleSet* samples,
                                                                                   QVector<double>* labels) :
  PiiLearningAlgorithm<SampleSet>(new Data(samples, labels))
{}

template <class SampleSet> PiiSampleSetCollector<SampleSet>::PiiSampleSetCollector(SampleSet* samples,
                                                                                   QVector<double>* labels,
                                                                                   QVector<double>* weights) :
  PiiLearningAlgorithm<SampleSet>(new Data(samples, labels, weights))
{}

template <class SampleSet> PiiSampleSetCollector<SampleSet>::~PiiSampleSetCollector()
{
  PII_D;
  if (d->bOwnSamples) delete d->pSampleSet;
  if (d->bOwnLabels) delete d->pvecLabels;
  if (d->bOwnWeights) delete d->pvecWeights;
}

template <class SampleSet> void PiiSampleSetCollector<SampleSet>::setSamples(SampleSet* samples)
{
  PII_D;
  if (d->bOwnSamples)
    delete d->pSampleSet;

  if (samples != 0)
    {
      d->pSampleSet = samples;
      d->bOwnSamples = false;
    }
  else
    {
      d->pSampleSet = new SampleSet;
      d->bOwnSamples = true;
    }
}

template <class SampleSet> SampleSet* PiiSampleSetCollector<SampleSet>::samples() const
{
  return _d()->pSampleSet;
}

template <class SampleSet> int PiiSampleSetCollector<SampleSet>::sampleCount() const
{
  return PiiSampleSet::sampleCount(*_d()->pSampleSet);
}

template <class SampleSet> int PiiSampleSetCollector<SampleSet>::featureCount() const
{
  return PiiSampleSet::featureCount(*_d()->pSampleSet);
}

template <class SampleSet> QVector<double>* PiiSampleSetCollector<SampleSet>::classLabels() const
{
  return _d()->pvecLabels;
}

template <class SampleSet> void PiiSampleSetCollector<SampleSet>::setClassLabels(QVector<double>* labels)
{
  PII_D;
  if (d->bOwnLabels)
    delete d->pvecLabels;

  if (labels != 0)
    {
      d->pvecLabels = labels;
      d->bOwnLabels = false;
    }
  else
    {
      d->pvecLabels = new QVector<double>;
      d->bOwnLabels = true;
    }
}

template <class SampleSet> QVector<double>* PiiSampleSetCollector<SampleSet>::sampleWeights() const
{
  return _d()->pvecWeights;
}

template <class SampleSet> void PiiSampleSetCollector<SampleSet>::setSampleWeights(QVector<double>* weights)
{
  PII_D;
  if (d->bOwnWeights)
    delete d->pvecWeights;

  if (weights != 0)
    {
      d->pvecWeights = weights;
      d->bOwnWeights = false;
    }
  else
    {
      d->pvecWeights = new QVector<double>;
      d->bOwnWeights = true;
    }
}

template <class SampleSet>
void PiiSampleSetCollector<SampleSet>::learn(const SampleSet& samples,
                                             const QVector<double>& labels,
                                             const QVector<double>& weights)

{
  PII_D;
  *d->pSampleSet = samples;
  if (d->bCollectLabels)
    *d->pvecLabels = labels;
  if (d->bCollectWeights)
    *d->pvecWeights = weights;
}

template <class SampleSet>
double PiiSampleSetCollector<SampleSet>::learnOne(ConstFeatureIterator featureVector,
                                                  int length,
                                                  double label,
                                                  double weight) throw ()
{
  PII_D;
  SampleSet& sampleSet(*d->pSampleSet);
  if (PiiSampleSet::featureCount(sampleSet) != length)
    {
      PiiSampleSet::resize(sampleSet, 0, length);
      d->pvecLabels->clear();
      d->pvecWeights->clear();
      d->iSampleIndex = 0;
      
      // If batch size is known, reserve enough memory now to
      // avoid reallocations altogether.
      if (d->iBatchSize != -1)
        PiiSampleSet::reserve(sampleSet, d->iBatchSize);
    }

  int iSampleCount = PiiSampleSet::sampleCount(sampleSet);
  // There is still room in the batch -> append a new row
  if (d->iBatchSize < 0 || iSampleCount < d->iBatchSize)
    {
      // Allocate room for at most 64 samples each time.
      if (PiiSampleSet::capacity(sampleSet) == iSampleCount)
        PiiSampleSet::reserve(sampleSet, qBound(1, iSampleCount*2, iSampleCount + 64));

      PiiSampleSet::append(sampleSet, featureVector);

      if (d->bCollectLabels)
        d->pvecLabels->append(label);
      if (d->bCollectWeights)
        d->pvecWeights->append(weight);
    }
  // No room -> overwrite one of the old ones if needed
  else if (d->fullBufferBehavior != PiiClassification::DiscardNewSample)
    {
      int iOverwriteIndex = ((d->fullBufferBehavior == PiiClassification::OverwriteRandomSample) ?
                             rand() : d->iSampleIndex) % PiiSampleSet::sampleCount(sampleSet);

      PiiSampleSet::setSampleAt(sampleSet, iOverwriteIndex, featureVector);
      if (d->bCollectLabels)
        d->pvecLabels->replace(iOverwriteIndex, label);
      if (d->bCollectWeights)
        d->pvecWeights->replace(iOverwriteIndex, weight);
    }
  ++d->iSampleIndex;

  return PiiSampleSet::sampleCount(sampleSet)-1;
}

template <class SampleSet> bool PiiSampleSetCollector<SampleSet>::converged() const throw ()
{
  return true;
}

template <class SampleSet> PiiClassification::LearnerCapabilities PiiSampleSetCollector<SampleSet>::capabilities() const
{
  return
    PiiClassification::NonSupervisedLearner |
    PiiClassification::OnlineLearner |
    PiiClassification::WeightedLearner;
}

template <class SampleSet> void PiiSampleSetCollector<SampleSet>::setCollectLabels(bool collectLabels) { _d()->bCollectLabels = collectLabels; }
template <class SampleSet> bool PiiSampleSetCollector<SampleSet>::collectLabels() const { return _d()->bCollectLabels; }
template <class SampleSet> void PiiSampleSetCollector<SampleSet>::setCollectWeights(bool collectWeights) { _d()->bCollectWeights = collectWeights; }
template <class SampleSet> bool PiiSampleSetCollector<SampleSet>::collectWeights() const { return _d()->bCollectWeights; }

template <class SampleSet> void PiiSampleSetCollector<SampleSet>::setBatchSize(int batchSize) { _d()->iBatchSize = batchSize; }
template <class SampleSet> int PiiSampleSetCollector<SampleSet>::batchSize() const { return _d()->iBatchSize; }

template <class SampleSet> void PiiSampleSetCollector<SampleSet>::setFullBufferBehavior(PiiClassification::FullBufferBehavior fullBufferBehavior)
{
  _d()->fullBufferBehavior = fullBufferBehavior;
}
template <class SampleSet> PiiClassification::FullBufferBehavior PiiSampleSetCollector<SampleSet>::fullBufferBehavior() const
{
  return _d()->fullBufferBehavior;
}

template <class SampleSet> void PiiSampleSetCollector<SampleSet>::resize(int samples)
{
  PII_D;
  if (samples < 0 || samples >= sampleCount())
    return;

  if (samples == 0)
    {
      setSamples(0);
      setClassLabels(0);
      setSampleWeights(0);
      return;
    }

  PiiSampleSet::resize(*d->pSampleSet, samples);
  if (d->pvecLabels != 0)
    d->pvecLabels->resize(samples);
  if (d->pvecWeights != 0)
    d->pvecLabels->resize(samples);
}

#endif //_PIISAMPLESETCOLLECTOR_H
