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

#ifndef _PIISOM_H
#define _PIISOM_H

#include "PiiVectorQuantizer.h"
#include "PiiClassificationGlobal.h"
#include "PiiLearningAlgorithm.h"

#include <QVector>

/**
 * An implementation of the self-organizing map (Kohonen map). 
 * Typically, SOM is trained in two phases. First, a "rough"
 * training is made with a relatively large learning rate (~0.05), a
 * relatively large radius (e.g. 10, depending on the size of the
 * map), and a relatively low number of iterations (depending on the
 * number of training samples). Then, fine-tuning is performed with
 * a smaller learning rate (~0.02), a smaller radius (e.g. 3), and a
 * larger number of iterations.
 *
 * Each training sample is fed to the SOM sequentially. The same
 * samples can be (and usually are) used multiple times until a
 * sufficient number of iterations (e.g. 10000) has been performed. 
 * Each sample tunes the code book according to the then current
 * learning parameters. The class modifies a code book that is given
 * upon construction of the class.
 *
 * In classification, the SOM works as a vector quantizer.
 *
 * @ingroup PiiClassificationPlugin
 */
template <class SampleSet> class PiiSom :
  public PiiVectorQuantizer<SampleSet>,
  public PiiLearningAlgorithm<SampleSet>
{
public:
  typedef typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator ConstFeatureIterator;
  
  /**
   * Create a new SOM with the number of nodes in horizontal and
   * vertical directions.
   *
   * @param width the number of nodes in horizontal direction
   *
   * @param height the number of nodes in vertical direction. If set
   * to a value less than one, a @a width-by-@a width map will be
   * created.
   */
  PiiSom(int width = 10, int height = 0);

  /**
   * Train the SOM with a batch of samples.
   *
   * @param samples a batch of training samples
   *
   * @param labels ignored. The SOM is a supervised algorithm.
   *
   * @param weights ignored.
   *
   * @return the index of the closest code vector
   */
  void learn(const SampleSet& samples,
             const QVector<double>& labels,
             const QVector<double>& weights = QVector<double>());
  
  /**
   * Train the SOM with the given feature vector.
   *
   * @param features the feature vector of a training sample
   *
   * @param length the number of features to consider. Must match the
   * current dimensionality of stored feature vectors.
   *
   * @param label ignored. The SOM is a supervised algorithm.
   *
   * @param weight ignored.
   *
   * @return the index of the closest code vector
   */
  double learnOne(ConstFeatureIterator features, int length, double label=NAN, double weight=1.0) throw();

  bool converged() const throw ();

  PiiClassification::LearnerCapabilities capabilities() const;

  /**
   * Get the initial learning radius.
   */
  double initialRadius() const;
  /**
   * Set the initial learning radius. The radius affects the number of
   * code vectors that are adapted in the neighborhood of the winning
   * node. The default value is 10. The radius decreases depending on
   * the learning rate function on each training iteration. The
   * current radius can be retrieved with currentRadius(). The
   * initial size of the adapted neighborhood should always be smaller
   * than or equal to the width and height of the map. In other words,
   * if you set the map size to, say, 6x6, set the radius to at most
   * 3.
   */
  void setInitialRadius(double radius);
  /**
   * Get the current learning radius.
   */
  double currentRadius() const;

  /**
   * Get the topology of the network.
   */
  PiiClassification::SomTopology topology() const;
  /**
   * Set the topology of the network.
   */
  void setTopology(PiiClassification::SomTopology topology);

  /**
   * Get the initial learning rate.
   */
  double initialLearningRate() const;
  /**
   * Set the initial learning rate. The default value is 0.05. The
   * learning rate decreases on each training iteration depending on
   * the learning rate function. The current value is retrieved with
   * currentLearningRate().
   */
  void setInitialLearningRate(double rate);
  /**
   * Get the current learning rate.
   */
  double currentLearningRate() const;

  /**
   * Get the estimated number of iterations in training.
   */
  int learningLength() const;
  /**
   * Set the estimated number of iterations in training. The SOM does
   * not know the number of training iterations in advance. However,
   * an estimate needs to be given because of the linearly changing
   * learning parameters (radius and learning rate). The default value
   * is 10000. If more iterations are taken, the learning parameters
   * will be fixed. With a linearly decreasing learning rate this
   * effectively stops the learning process after the estimated number
   * of iterations have been pefrormed. Thus, one needs to make sure
   * the value is large enough.
   */
  void setLearningLength(int length);
  
  /**
   * Get the type of learning rate change in training.
   */
  PiiClassification::SomRateFunction rateFunction() const;
  /**
   * Set the type of learning rate change in training.
   */
  void setRateFunction(PiiClassification::SomRateFunction func);

  /**
   * Set the current iteration number. This number affects the
   * learning rate and the size of the neighborhood. Set this value to
   * zero to restart learning.
   *
   * @b Note. If you want to restart learning at an index other than
   * zero, set the iteration number to zero first to ensure all
   * necessary initialization is done.
   */
  void setIterationNumber(int iteration);
  /**
   * Get the current iteration number.
   */
  int iterationNumber() const;
  
  /**
   * Get the width of the SOM.
   */
  int width() const;
  /**
   * Get the height of the SOM.
   */
  int height() const;

  /**
   * Set the size of the SOM. When the size is changed, the old code
   * book may not be usable. One should ensure that the number of code
   * vectors always matches the size of the map (width*height).
   */
  void setSize(int width, int height);
  
  /**
   * Set the width of the som. This will invalidate the internal code
   * book.
   */
  void setWidth(int width);
  
  /**
   * Set the height of the som. This will invalidate the internal code
   * book.
   */
  void setHeight(int height);

  /**
   * Set the initialization mode to @p mode. The default
   * initialization mode is SomSampleInit.
   */
  void setInitMode(PiiClassification::SomInitMode mode);

  /**
   * Get the initialization mode.
   */
  PiiClassification::SomInitMode initMode() const;

  /**
   * Get the learning algorithm.
   */
  PiiClassification::SomLearningAlgorithm learningAlgorithm() const;

  /**
   * Set the learning algorithm
   */
  void setLearningAlgorithm(PiiClassification::SomLearningAlgorithm algorithm);

  int codeBookCollectionIndex();

  QVector<double> findMostDistantNeighbors(int* vector1Index = 0, int* vector2Index = 0) const;
  double findMostDistantNeighbor(int row, int *vector2Index = 0 ) const;
  
private:
  /// @internal
  class Data : public PiiVectorQuantizer<SampleSet>::Data
  {
  public:
    Data(int width,
         int height);
    int iSizeX, iSizeY;
    int iIterationNumber;
    int iLearningLength;
    double dRadius, dLearningRate;
    int iCodeBookCollectionIndex;
    PiiClassification::SomInitMode initMode;
    PiiClassification::SomTopology topology;
    PiiClassification::SomRateFunction rateFunction;
    PiiClassification::SomNeighborhood neighborhood;
    PiiClassification::SomLearningAlgorithm algorithm;
    double dMinQErr, dMaxQErr, dQErrRange; // storage for the Qerr algorithm
    SampleSet previousSample; // storage for the balanced SOM algorithm
    SampleSet meanSample;
    double dMeanDist;
  };
  inline Data* _d() { return static_cast<Data*>(PiiVectorQuantizer<SampleSet>::d); }
  inline const Data* _d() const { return static_cast<const Data*>(PiiVectorQuantizer<SampleSet>::d); }

  double linearAlpha(int iter) const
  {
    const PII_D;
    return (d->dLearningRate * (double) (d->iLearningLength - iter) / (double) d->iLearningLength);
  }
		
  double inverseAlpha(int iter) const
  {
    const PII_D;
    double c = (double)d->iLearningLength / 100.0;
    return (d->dLearningRate * c / (c + iter));
  }

  int adaptTo(ConstFeatureIterator vector);
  void adaptNeighborhood(int hitX, int hitY, ConstFeatureIterator vector, double distance);
};

namespace PiiClassification
{
  /**
   * Calculate the squared distance between two nodes in a SOM with a
   * hexagonal topology.
   */
  PII_CLASSIFICATION_EXPORT double somHexagonalDistance(int bx, int by, int tx, int ty);
  /**
   * Calculate the squared distance between two nodes in a SOM with a
   * square topology.
   */
  PII_CLASSIFICATION_EXPORT double somSquareDistance(int bx, int by, int tx, int ty);
}


#include "PiiSom-templates.h"

#endif //_PIISOM_H
