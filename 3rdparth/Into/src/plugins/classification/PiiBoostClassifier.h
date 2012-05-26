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

#ifndef _PIIBOOSTCLASSIFIER_H
#define _PIIBOOSTCLASSIFIER_H

#include "PiiLearningAlgorithm.h"
#include "PiiClassifier.h"

/**
 * An generic implementation of a boosted classifier. "Boosting" is
 * actually not a classification method but rather a meta-algorithm
 * that can be used to @i boost the performance of another classifier. 
 * It is required that the classifier to be boosted is able to learn
 * weighted samples.
 *
 * All boosting algorithms work by repeatedly applying a <em>weak
 * classifier</em> to weighted training samples. On each iteration,
 * misclassified samples gain more weight so that subsequently added
 * classifiers will focus on getting them right. The @p FloatBoost
 * algorithm also goes backwards and eliminates already added
 * classifiers if doing so would decrease training error.
 *
 * The boosting algorithm will quit when the @ref setMaxClassifiers()
 * "maximum number of classifiers" has been reached. The @p FloatBoost
 * algorithm will also quit if its exponential loss function goes
 * below @ref setExponentialLossThreshold() "a threshold". The output
 * of the boosted classifier is a weighted sum over the ensemble of
 * weak classifiers:
 *
 * @f[
 * f(\mathbf{x}) = \begin{cases}
 * 1 & \text{if } \sum_{i=1}^N \alpha_i (f_i(\mathbf{x}) - 0.5) > 0 \\
 * 0 & \text{otherwise}
 * \end{cases}
 * @f]
 *
 * Here, @f$f_i@f$ is the ith weak classifier. The weak classifiers
 * are binary classifiers with all but the multi-class @p AdaBoostM1
 * algorithm, which uses weighted voting to find the winning class.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class SampleSet> class PiiBoostClassifier :
  public PiiLearningAlgorithm<SampleSet>,
  public PiiClassifier<SampleSet>
{
public:
  typedef typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator ConstFeatureIterator;

  /**
   * An interface for objects that create weak classifiers for
   * PiiBoostClassifier.
   *
   * @see PiiDefaultClassifierFactory
   */
  class Factory
  {
  public:
    virtual ~Factory() {}
    
    /**
     * Creates a new weak classifier. Caller assumes the ownership of the
     * returned pointer.
     *
     * @param classifier a pointer to the invoking classifier
     *
     * @param samples the samples to train the weak classifier with
     *
     * @param labels class labels for the samples
     *
     * @param weights weights for individual samples
     */
    virtual PiiClassifier<SampleSet>* create(PiiBoostClassifier<SampleSet>* classifier,
                                             const SampleSet& samples,
                                             const QVector<double>& labels,
                                             const QVector<double>& weights) = 0;
  };

  /**
   * Create a new boosting classifier that uses @a factory to create
   * the weak classifiers. The learning algorithm will be set to @a
   * algorithm. This class will not own @a factory; the pointer must
   * be deleted by the caller.
   */
  PiiBoostClassifier(Factory* factory = 0,
                     PiiClassification::BoostingAlgorithm algorithm = PiiClassification::RealBoost);

  ~PiiBoostClassifier();
  
  double classify(ConstFeatureIterator sample) throw();

  /**
   * Runs the selected boosting algorithm on @a samples.
   *
   * @param samples training samples
   *
   * @param labels a class label for each training sample
   *
   * @param weights initial weights for each training sample. This
   * list can be left empty, in which case the initial weights will be
   * automatically calculated.
   */
  void learn(const SampleSet& samples,
             const QVector<double>& labels,
             const QVector<double>& weights = QVector<double>());


  bool converged() const throw ();

  /**
   * Returns @p WeightedLearner.
   */
  PiiClassification::LearnerCapabilities capabilities() const;

  /**
   * Sets the factory used for creating weak learners during batch
   * training. The factory is used in creating a new weak learner on
   * each boosting iteration. The caller owns the factor; this class
   * doesn't delete the pointer.
   */
  void setFactory(Factory* factory);
  /**
   * Returns the factory currently in use.
   */
  Factory* factory() const;

  /**
   * Sets the algorithm used in training and classification. It is
   * possible to change the value between training and classification,
   * if care is taken. For example, it is possible to train the
   * classifier with @p FloatBoost while using the multi-class
   * generalization @p AdaBoostM1 for classification.
   */
  void setAlgorithm(PiiClassification::BoostingAlgorithm algorithm);
  /**
   * Returns the training/classification algorithm currently in use.
   */
  PiiClassification::BoostingAlgorithm algorithm() const;

  /**
   * Sets the maximum number of classifiers #learn() will create. The
   * default value is 100.
   */
  void setMaxClassifiers(int maxClassifiers);
  /**
   * Returns the maximum number of classifiers that will be combined.
   */
  int maxClassifiers() const;

  QList<PiiClassifier<SampleSet>*> classifiers() const;
  QList<float> weights() const;

  /**
   * Returns the number of features, or 0 if the classifier has not
   * been trained. This value will be set before weak learners are
   * being created and can therefore be used by the factory object.
   */
  int featureCount() const;

  /**
   * Returns the number of different classes in training data, or 0 if
   * the classifier has not been trained. This value will be set
   * before weak learners are being created and can therefore be used
   * by the factory object.
   */
  int classCount() const;

  /**
   * Sets the minimum training error. Training will be stopped once
   * the weighted training error goes below this value. The default is
   * zero.
   */
  void setMinError(double minError);
  /**
   * Returns the error limit.
   */
  double minError() const;

protected:
  static QString tr(const char* s) { return QCoreApplication::translate("PiiBoostClassifier", s); }
  
private:
  double updateWeights(const SampleSet& samples,
                       const QVector<double>& labels,
                       const QVector<double>& hypotheses,
                       QVector<double>& weights,
                       double correctWeight,
                       double incorrectWeight);
  double excludeOne(const SampleSet& samples,
                    const QVector<double>& weights,
                    const QVector<double>& labels,
                    QVector<double>& hypotheses,
                    double* minError);
  void formHypothesis(PiiClassifier<SampleSet>* classifier,
                      const SampleSet& samples,
                      QVector<double>& hypothesis);
  void formHypothesisExcluding(const SampleSet& samples,
                               QVector<double>& hypothesis,
                               int excludedIndex);
  double classifyExcluding(ConstFeatureIterator sample,
                           int excludedIndex);

  class Data : public PiiLearningAlgorithm<SampleSet>::Data
  {
  public:
    Data(Factory* factory, PiiClassification::BoostingAlgorithm algorithm);
    Factory* pFactory;
    PiiClassification::BoostingAlgorithm algorithm;
    int iClassCount;
    int iFeatureCount;
    int iMaxClassifiers;
    QList<double> lstClassifierWeights;
    QList<PiiClassifier<SampleSet>*> lstClassifiers;
    double dMinError;
  };
  PII_D_FUNC;
  PII_DISABLE_COPY(PiiBoostClassifier);

  friend struct PiiSerialization::Accessor;
  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION;
  template <class Archive> void serialize(Archive& archive, const unsigned int)
  {
    PII_D;
    if (Archive::InputArchive)
      qDeleteAll(d->lstClassifiers);
    archive & PII_NVP("algorithm", PII_ENUM(d->algorithm));
    archive & PII_NVP("classes", d->iClassCount);
    archive & PII_NVP("features", d->iFeatureCount);
    archive & PII_NVP("maxClassifiers", d->iMaxClassifiers);
    archive & PII_NVP("weights", d->lstClassifierWeights);
    archive & PII_NVP("classifiers", d->lstClassifiers);
  }
};

PII_CLASSIFICATION_NAME_ALIAS_AS_T(PiiBoostClassifier, PiiMatrix<T>);
PII_DEFINE_VIRTUAL_METAOBJECT_FUNCTION_TEMPLATE(PiiBoostClassifier);

#include "PiiBoostClassifier-templates.h"

#endif //_PIIBOOSTCLASSIFIER_H
