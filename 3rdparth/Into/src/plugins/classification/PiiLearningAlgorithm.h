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

#ifndef _PIILEARNINGALGORITHM_H
#define _PIILEARNINGALGORITHM_H

#include <PiiMatrix.h>
#include <PiiProgressController.h>
#include "PiiClassification.h"
#include "PiiClassificationException.h"
#include "PiiSampleSet.h"

/**
 * An interface for learning algorithms. Learning algorithms work by
 * optimizing a learning criterion based on a batch of training
 * samples. The criterion to be optimized may be represented in a
 * number of very different ways, including maximizing the margin of
 * separation between classes (PiiSvm, PiiKernelAdatron), just
 * separating classes (PiiPerceptron), or clustering data (PiiSom).
 *
 * Learning algorithms can be categorized in a number of ways. This
 * interface makes a distinction between on-line and batch algorithms. 
 * Algorithms that don't require all training samples to be present at
 * once as a batch (so-called on-line algorithms such as sequential
 * SOM) only need to implement the #learnOne() function. If the
 * algorithm is not capable of on-line training, the #learn() function
 * must be overridden instead.
 *
 * The learning functions take two optional parameters: label and
 * weight. Not all learning algorithms need these values. Class labels
 * are used in supervised training, and sample weights in weighted
 * learning. Some algorithms such as PiiDecisionStump use both the
 * labels and the weights.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class SampleSet> class PiiLearningAlgorithm
{
public:
  typedef SampleSet SampleSetType;
  
  /**
   * Create a new learning algorithm instance.
   */
  PiiLearningAlgorithm();

  /// Destroys the learning algorithm.
  virtual ~PiiLearningAlgorithm();

  /**
   * Train the learning algorithm with a batch of samples. The default
   * implementation sequentially sends each sample in @a samples to
   * #learnOne() until #converged() returns @p true.
   *
   * @param samples a set of feature vectors to train the algorithm
   * with.
   *
   * @param labels the labels of the samples. This value is not used
   * by non-supervised classifiers and can be set to an empty list. If
   * the value is given, the length of the list must be equal to the
   * number of samples in @a samples.
   *
   * @param weights weights for individual samples. This value is used
   * only if the learning algorithm is capable of weighted learning. 
   * If this value is given, the length of the list must be equal to
   * the number of samples in @a samples.
   *
   * @exception PiiClassificationException& if something goes wrong.
   * An exception is thrown, for example, if the training is cancelled
   * by the progress controller.
   */
  virtual void learn(const SampleSet& samples,
                     const QVector<double>& labels = QVector<double>(),
                     const QVector<double>& weights = QVector<double>());

  
  /**
   * Feeds a feature vector to the learning algorithm. The learning
   * algorithm adapts its internal state according to the feature
   * vector and a known label (supervised learning). In non-supervised
   * learning, the label is ignored.
   *
   * @param featureVector a feature vector
   *
   * @param length the length of the feature vector. This must be the
   * same for all samples. Otherwise, the behavior is undefined.
   *
   * @param label an optional class label for supervised learning.
   *
   * @param weight an optional weight for the training sample
   *
   * @return classification for the sample. Some classifiers are able
   * to classify samples during the learning process with no
   * additional computational effort. They return the current
   * classification of the feture vector. Others should return @p NaN. 
   * The default implementation returns @p NaN.
   */
  virtual double learnOne(typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator featureVector,
                          int length,
                          double label = NAN,
                          double weight = 1.0) throw ();

  /**
   * Returns @p true if the algorithm has converged (found an optimal
   * solution) and @p false otherwise.
   */
  virtual bool converged() const throw () = 0;

  /**
   * Returns the capabilities of the learning algorithm as a
   * combination of LearnerCapability flags.
   */
  virtual PiiClassification::LearnerCapabilities capabilities() const = 0;

  /**
   * Set the progress controller. Set to zero to disable the
   * controller.
   */
  void setController(PiiProgressController* controller);

  /**
   * Returns the currently installed controller.
   *
   * @return the current controller or zero if no controller is
   * installed
   */
  PiiProgressController* controller() const;

protected:
  /// @internal
  class Data
  {
  public:
    Data();
    virtual ~Data();
    PiiProgressController* pController;
  } *d;

  /// @internal
  PiiLearningAlgorithm(Data* data);
};

template <class SampleSet> PiiLearningAlgorithm<SampleSet>::Data::Data() :
  pController(0)
{
}

template <class SampleSet> PiiLearningAlgorithm<SampleSet>::Data::~Data()
{
}

template <class SampleSet> PiiLearningAlgorithm<SampleSet>::PiiLearningAlgorithm() :
  d(new Data)
{
}

template <class SampleSet> PiiLearningAlgorithm<SampleSet>::PiiLearningAlgorithm(Data* data) :
  d(data)
{
}

template <class SampleSet> PiiLearningAlgorithm<SampleSet>::~PiiLearningAlgorithm()
{
  delete d;
}

template <class SampleSet> void PiiLearningAlgorithm<SampleSet>::learn(const SampleSet& samples,
                                                                       const QVector<double>& labels,
                                                                       const QVector<double>& weights)

{
  const int iSamples = PiiSampleSet::sampleCount(samples),
    iFeatures = PiiSampleSet::featureCount(samples),
    iLabels = labels.size(),
    iWeights = weights.size();
  int i=0;
  while (!converged())
    {
      learnOne(PiiSampleSet::sampleAt(samples,i), iFeatures,
               iLabels > i ? labels[i] : NAN,
               iWeights > i ? weights[i] : 1.0);
      PII_TRY_CONTINUE(d->pController, NAN);
      // Restart from beginning if the learning algorithm didn't
      // converge in one round.
      if (++i >= iSamples)
        i = 0;
    }
}

template <class SampleSet>
double PiiLearningAlgorithm<SampleSet>::learnOne(typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator featureVector,
                                                 int length,
                                                 double label,
                                                 double weight) throw ()
{
  Q_UNUSED(featureVector);
  Q_UNUSED(length);
  Q_UNUSED(label);
  Q_UNUSED(weight);
  return NAN;
}

template <class SampleSet> void PiiLearningAlgorithm<SampleSet>::setController(PiiProgressController* controller)
{
  d->pController = controller;
}

template <class SampleSet> PiiProgressController* PiiLearningAlgorithm<SampleSet>::controller() const
{
  return d->pController;
}

#endif //_PIILEARNINGALGORITHM_H
