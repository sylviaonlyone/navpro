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

#ifndef _PIISAMPLEBALANCER_H
#define _PIISAMPLEBALANCER_H

#include <PiiDefaultOperation.h>

/**
 * An operation that balances training sets by giving more weight to
 * rare samples. The weighting is based on the distribution of
 * individual feature values. The balancer works in two modes: @p
 * ProbabilitySelection and @p WeightCalculation. In the former mode,
 * the operation either passes feature vectors to the @p features
 * output or does nothing, based on the estimated weight of the
 * sample. In the latter mode, all features will be passed, and the
 * weight of the sample will be sent to the @p weight output.
 *
 * The graph below illustrates sample weighting on one-dimensional
 * Gaussian data. The (normalized) distribution of a feature value is
 * shown in blue. Its inverse (green) is used as a weight. The red
 * curve illustrates the effect of setting #emphasis to three.
 *
 * @image html samplebalancer.png
 *
 * For multi-dimensional features, %PiiSampleBalancer uses marginal
 * distributions, based on the assumption that all features are
 * independent. This is often not the case, but gives a reasonable
 * approximation without huge memory requirements.
 *
 * @inputs
 *
 * @in features - feature vector. Each component must be quantized to
 * the number of quantization levels determined by #levels.
 * 
 * @outputs
 *
 * @out features - the features. If #mode is @p ProbabilitySelection,
 * the features will be emitted only if a generated random number is
 * less than @p weight. The @p select output will indicate whether the
 * sample was selected or not. In @p WeightCalculation mode, this
 * output will always pass the incoming features.
 *
 * @out weight - the weight of the sample, 0.0-1.0 (double). 0.0 means
 * not selected and 1.0 means definitely selected.
 *
 * @out select - a boolean value indicating whether the sample was
 * randomly selected or not. In @p WeightCalculation mode, this is
 * output will always emit @p true.
 *
 * @ingroup PiiClassificationPlugin
 */
class PiiSampleBalancer : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Operation mode.
   */
  Q_PROPERTY(Mode mode READ mode WRITE setMode);
  Q_ENUMS(Mode);
  
  /**
   * A list of quantization levels for each feature value. For
   * three-dimensional feature vectors, the default can be changed as
   * follows:
   *
   * @code
   * balancer->setProperty("levels", QVariantList() << 128 << 256 << 64);
   * @endcode
   *
   * The minimum number of quantization levels is one.
   */
  Q_PROPERTY(QVariantList levels READ levels WRITE setLevels);

  /**
   * The default number of quantization levels. This value is used for
   * all features whose quantization levels have not been explicitly
   * set by #levels. The default value is 256.
   */
  Q_PROPERTY(int defaultLevels READ defaultLevels WRITE setDefaultLevels);

  /**
   * By default, the operation tries to flatten out the variations in
   * feature distribution. If the common samples need to be given even
   * less weight, @p emphasis can be set to a larger value. The
   * operation will raise the weight estimate to this power.
   */
  Q_PROPERTY(int emphasis READ emphasis WRITE setEmphasis);
  
  /**
   * The speed of adaptation to changing conditions. The operation
   * initially assumes a uniform feature distribution. The estimate of
   * the distribution is updated once every #learningBatchSize samples. The
   * adaptation ratio tells how much the new measurements affect the
   * learnt model. 0 means that the initial uniform approximation will
   * never be changed. 1 means that the new estimate will fully
   * replace the old one. The default value is 0.1.
   */
  Q_PROPERTY(double adaptationRatio READ adaptationRatio WRITE setAdaptationRatio);

  /**
   * The number of features required for a reliable estimate. The
   * estimate is updated every @p learningBatchSize samples. The default value
   * is 25600 (100 samples / histogram bin).
   */
  Q_PROPERTY(int learningBatchSize READ learningBatchSize WRITE setLearningBatchSize);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Operation modes.
   *
   * @lip ProbabilitySelection - pass those feature vectors that are
   * likely to be important with a higher probability than the others.
   *
   * @lip WeightCalculation - pass every incoming vector accompanied
   * with selection probability.
   */
  enum Mode { ProbabilitySelection, WeightCalculation };
  
  PiiSampleBalancer();
  ~PiiSampleBalancer();

  void setLevels(const QVariantList& levels);
  QVariantList levels() const;
  void setDefaultLevels(int defaultLevels);
  int defaultLevels() const;
  void setEmphasis(int emphasis);
  int emphasis() const;
  void setMode(Mode mode);
  Mode mode() const;
  void setAdaptationRatio(double adaptationRatio);
  double adaptationRatio() const;
  void setLearningBatchSize(int learningBatchSize);
  int learningBatchSize() const;

  void check(bool reset);

protected:
  void process();

private:
  class Histogram;
  inline double weight(int feature, int index);
  void allocateHistograms();
  template <class T> void balance(const PiiVariant& obj);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    Mode mode;
    int iDefaultLevels;
    int iEmphasis;
    int iFeatureCnt;
    Histogram* pHistograms;
    double dAdaptationRatio;
    int iLearningBatchSize;
    QList<int> lstLevels;
  };
  PII_D_FUNC;
};


#endif //_PIISAMPLEBALANCER_H
