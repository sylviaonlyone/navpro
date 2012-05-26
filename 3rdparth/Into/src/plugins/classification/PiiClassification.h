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

#ifndef _PIICLASSIFICATION_H
#define _PIICLASSIFICATION_H

#include <PiiMatrix.h>
#include <PiiHeap.h>
#include <QPair>
#include <QVector>
#include "PiiClassificationGlobal.h"
#include "PiiDistanceMeasure.h"
#include "PiiSampleSet.h"
#include "PiiClassificationException.h"

/// @file

/**
 * Training algorithms that take a long time to run must occassionally
 * call this macro to check if they are still allowed to continue. 
 * This function calls the @a CONTROLLER's @ref
 * PiiProgressController::canContinue() "canContinue(@a PROGRESS)"
 * function. If @a CONTROLLER non-zero and returns @p false, this
 * macro throws a PiiClassificationException. Otherwise it does
 * nothing.
 *
 * @param CONTROLLER a pointer to a PiiProgressController.
 *
 * @param PROGRESS the current estimated progress as a
 * percentage (0.0 - 1.0, or @p NaN if not known).
 */
#define PII_TRY_CONTINUE(CONTROLLER, PROGRESS) \
  do { \
    PiiProgressController* c = CONTROLLER; \
    if (c != 0 && !c->canContinue(PROGRESS)) \
      PII_THROW(PiiClassificationException, PiiClassificationException::LearningInterrupted); \
  } while (false)


/**
 * Utility functions and type definitions for common classification
 * tasks.
 *
 * @ingroup PiiClassificationPlugin
 */
namespace PiiClassification
{
  /**
   * Calculates classification error. This function returns the
   * ratio of misclassified samples.
   *
   * @param knownLabels the ground truth. N labels.
   *
   * @param hypothesis the classification result. N labels. If a
   * hypothesis is NaN, it will be ignored.
   *
   * @param weights a weight for each sample. The weights should sum
   * up to one. Can be omitted.
   *
   * @return the (weighted) classification error, in [0,1].
   */
  double PII_CLASSIFICATION_EXPORT calculateError(const QVector<double>& knownLabels,
                                                  const QVector<double>& hypothesis,
                                                  const QVector<double>& weights = QVector<double>());

  /**
   * Create a confusion matrix.
   *
   * @param knownLabels the ground truth. N known class indices for
   * the samples.
   *
   * @param hypothesis N class labels produced by a classifier. If any
   * of the hypotheses is -1, an extra "discard" class will be added
   * as the last column of the returned matrix.
   *
   * @return a matrix in which row indices correspond to the ground
   * truth and column indices to the hypotheses. The values are hit
   * counts. For example, if the value at (1,2) is 9, nine samples of
   * class one were incorrectly classified to class two.
   */
  PiiMatrix<int> PII_CLASSIFICATION_EXPORT createConfusionMatrix(const QVector<double>& knownLabels,
                                                                 const QVector<double>& hypothesis);

  /**
   * Go through the row matrix @p labels and replace each -1 with the
   * label of the closest code vector in @p codeBook.
   *
   * @param labels labels for the vectors in @p codeBook. Labels with
   * no associated code vector will not be changed. The label matrix
   * may be either a column or a row matrix.
   *
   * @param codeBook code vectors. The number of rows in this matrix
   * should be greater than or equal to the number of columns in @p
   * labels.
   *
   * @param measure the measure used for distance estimation.
   *
   * @return the new labels
   */
  template <class T, class DistanceMeasure>
	PiiMatrix<int> fillMissingLabels(const PiiMatrix<int>& labels,
																	 const PiiMatrix<T>& codeBook,
																	 DistanceMeasure measure);

  /**
   * Generate a distance matrix. Let us denote the number of vectors
   * (rows in @p vectors) by N. The size of the distance matrix is
   * N-by-N, and each element (r,c) stores the distance between vector
   * r and vector c, calculated with
   * <tt>measure(vectors[r],vectors[c])</tt>. Note that since distance
   * measures and kernels share the same interface, this function can
   * be used to calculate a kernel matrix as well.
   *
   * @param vectors input vectors. A N-by-M matrix where M is input
   * dimensionality.
   *
   * @param distanceMeasure the measure used to calculate the
   * distances between vectors.
   *
   * @param symmetric if @p true, the upper triangle will be filled by
   * copying the lower triangle.
   *
   * @param calculateDiagonal if @p true, each vector's distance to
   * itself will also be calculated.
   *
   * @return the pairwise distances between input vectors as a matrix.
   */
  template <class SampleSet, class DistanceMeasure>
  PiiMatrix<double> calculateDistanceMatrix(const SampleSet& samples,
                                            const DistanceMeasure& measure,
																						bool symmetric = true,
																						bool calculateDiagonal = false);


  /**
   * Find the closest match for @a sample in @a modelSet.
   *
   * @param sample an iterator to the beginning of feature data. Must
   * be valid through @p modelSet.featureCount() elements.
   *
   * @param modelSet the model samples to compare @p featureVector
   * against.
   *
   * @param measure the distance measure used to calculate the
   * difference between @a sample and each model.
   *
   * @param distance an optional output-value parameter that will
   * store the distance to the closest code book vector.
   *
   * @return the index of the closest code model sample, or -1 if @p
   * modelSet is empty.
   *
   * @code
   * PiiSquaredGeometricDistance<const float*> dist;
   * PiiMatrix<float> matSamples(50,2); // each row is a feature vector
   * PiiMatrix<float> matObserved(1,2); // observed sample
   * int iMatch = PiiClassification::findClosestMatch(matObserved[0],
   *                                                  d->matFeatures,
   *                                                  dist);
   * @endcode
   */
  template <class SampleSet, class DistanceMeasure>
  int findClosestMatch(typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator sample,
                       const SampleSet& modelSet,
                       const DistanceMeasure& measure,
                       double* distance = 0);

  /**
   * The data structure used as a priority queue in k-NN searches.
   * Each element in a match list contains a distance to a sample and
   * the index of the sample in a model sample set.
   */
  typedef PiiHeap<QPair<double,int> > MatchList;
  
  /**
   * Find the @p n closest matches for @a sample in @a modelSet.
   *
   * @param sample an iterator to the beginning of feature data. Must
   * be valid through @p modelSet.featureCount() elements.
   *
   * @param modelSet the model samples to compare @p featureVector
   * against.
   *
   * @param measure the distance measure used to calculate the
   * difference between @a sample and each model.
   *
   * @param n the number of closest matches to return. Each element in
   * the returned list contains the distance to a model sample and its
   * index in the sample set. The list is sorted in ascending order
   * according to the distance. The length of the list is the minimum
   * of @a n and the number of samples in @a modelSet.
   *
   * @code
   * PiiSquaredGeometricDistance<const float*> dist;
   * PiiMatrix<float> matSamples(50,2); // each row is a feature vector
   * PiiMatrix<float> matObserved(1,2); // observed sample
   * PiiClassification::MatchList lstMatces =
   *   PiiClassification::findClosestMatches(matObserved[0],
   *                                         d->matFeatures,
   *                                         dist,
   *                                         5);
   * @endcode
   */
  template <class SampleSet, class DistanceMeasure>
  MatchList findClosestMatches(typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator sample,
                               const SampleSet& modelSet,
                               const DistanceMeasure& measure,
                               int n);

	/**
	 * Classify a sample using the <em>k nearest neighbors</em> rule. 
	 * This function compares @a sample to each model in @a modelSet, to
	 * find the @a k closest ones. Then, it uses @a labels to find out
	 * the class label that has the most occurrences within the @a k
	 * closest models. In the case of a tie, the class with the closest
	 * neighbor wins.
	 *
   * @param sample an iterator to the beginning of feature data. Must
   * be valid through @p modelSet.featureCount() elements.
   *
   * @param modelSet the model samples to compare @p featureVector
   * against.
   *
	 * @param labels a label for each sample in @a modelSet. The length
	 * of this list must match the number of samples in @a modelSet.
   *
   * @param measure the distance measure used to calculate the
   * difference between @a sample and each model.
	 *
   * @param k the number of nearest neighbors to consider.
	 *
	 * @param distance an optional output value that, if non-zero, will
	 * store the distance to the closest sample representing the winning
	 * class.
   *
   * @param closestIndex an optional output value that, if non-zero,
   * will store the index of the closest model sample of the winning
   * class. Note the closest sample in the winning class may not be
   * the closest of all samples.
	 *
	 * @return the class label with the most representatives among the k
	 * nearest neighbors of @a sample.
	 */
  template <class SampleSet, class DistanceMeasure>
  double knnClassify(typename PiiSampleSet::Traits<SampleSet>::ConstFeatureIterator sample,
                     const SampleSet& modelSet,
                     const QVector<double>& labels,
                     const DistanceMeasure& measure,
                     int k,
                     double* distance = 0,
                     int* closestIndex = 0);

  /**
   * Adapt a @a code vector towards @a sample with the given strength
   * @a alpha. The code vector will be modified in place. The function
   * will calculate the weighted average of code vector C and sample S
   * as @f$C \gets \alpha S + (1-\alpha) C@f$.
   *
   * @param code an iterator to the beginning of the code vector. 
   *
   * @param sample an iterator to the beginning of the vector to tune
   * the code vector towards.
   *
   * @param alpha the strength of the tuning. 0 means no change, 1
   * means that code vector will be replaced with @p sample.
   */
  template <class FeatureIterator, class ConstFeatureIterator>
  void adaptVector(FeatureIterator code,
                   ConstFeatureIterator sample,
                   int length,
                   double alpha);
  
  /**
   * K-means clustering algorithm.
   *
   * The k-means algorithm is an algorithm to cluster @e n objects
   * based on attributes into @e k partitions, @e k < @e n. It is
   * similar to the expectation-maximization algorithm for mixtures of
   * Gaussians in that they both attempt to find the centers of
   * natural clusters in the data. It assumes that the object
   * attributes form a vector space. The objective it tries to achieve
   * is to minimize total intra-cluster variance, or, the squared
   * error function
   *
   * @f[
   * V = \sum_{i=1}^{k} \sum_{x_j \in S_i} (x_j - \mu_i)^2, 
   * @f]
   *
   * where there are @e k clusters @f$S_i@f$, @f$i=1,2,\ldots,k@f$,
   * and @f$\mu_i@f$ is the centroid or mean point of all the points
   * @f$x_j \in S_i@f$. This implementation uses an iterative
   * refinement heuristic known as Lloyd's algorithm to solve the
   * optimization problem.
   *
   * @param samples a set of feature vectors to run the algorithm on. 
   * Each row of this matrix represents a feature vector. The number
   * of samples must be greater than @p k.
   *
   * @param k the number of centroids
   *
   * @param measure a measure used to calculate the distance between
   * samples and centroids.
   *
   * @param maxIterations if this value is non-positive, the algorithm
   * will be run until convergence. If you want to quit earlier, set
   * this to a positive value
   *
   * @return the centroids
   */
  template <class SampleSet, class DistanceMeasure>
  SampleSet kMeans(const SampleSet& samples,
                   unsigned int k,
                   const DistanceMeasure& measure,
                   unsigned int maxIterations = 0);

  /**
   * Create a random sample set. Each element in the returned sample
   * set is a random number uniformly distributed in the range
   * [minimum, maximum].
   *
   * @param samples the number of samples to create
   *
   * @param features the number of columns (i.e. the length of the feature vector)
   *
   * @param minimum smallest possible feature value
   *
   * @param maximum largest possible feature value
   *
   * @code
   * using namespace PiiClassification;
   * PiiMatrix<double> matSamples = createRandomSampleSet<PiiMatrix<double> >(10, 16, -1, 1);
   * @endcode
   */
  template <class SampleSet> SampleSet createRandomSampleSet(int samples,
                                                             int features,
                                                             double minimum,
                                                             double maximum);

  /**
   * Counts the number of distinct labels in @a labels. Returns the
   * found labels as a list of pairs storing the class label
   * (pair.first) and the number of entries (pair.second).
   *
   * @code
   * QVector<double> labels = QVector<double>() << 0 << 1 << 2 << 1 << 4 << 0;
   * QList<QPair<double,int> > counts = PiiClassification::countLabels(labels);
   * // counts = ((0.0, 2), (1.0, 2), (2.0, 1), (4.0, 1))
   * @endcode
   *
   * @note The label list may not contain NANs.
   */
  QList<QPair<double,int> > PII_CLASSIFICATION_EXPORT countLabels(const QVector<double>& labels);
  /**
   * Counts the number of distinct integer labels in @a labels. This
   * function ignores the decimal part of the class labels. The nth
   * element in the returned list contains the number of labels whose
   * value (truncated to an integer) equals n. All negative labels
   * will be collected to the zero bin in the returned histogram.
   *
   * @code
   * QVector<double> labels = QVector<double>() << 0.9 << 1.1 << 2.5 << 1.3 << 4.05 << 0.01;
   * QVector<int> counts = PiiClassification::countLabelsInt(labels);
   * // counts = (2, 2, 1, 0, 1)
   * @endcode
   *
   * @note The label list may not contain NANs.
   */
  QVector<int> PII_CLASSIFICATION_EXPORT countLabelsInt(const QVector<double>& labels);
  
  /**
   * Creates a non-linearly separable sample set in which two classes
   * spiral around each other on a plane. In the picture below, @a
   * samplesPerSet = 2000 and @a rounds = 3.0. Samples with label 0
   * are shown in red, and the samples with label 1 in blue.
   *
   * @image html double_spiral.png
   *
   * @param samplesPerSet the number of samples in each of the two
   * sets
   *
   * @param rounds the number of times the spirals will turn around
   * the origin.
   *
   * @param samples this matrix will be filled with the
   * two-dimensional feature vectors. The first @a samplesPerSet rows
   * will represent class 0 and the rest class 1. The size of the
   * matrix will be 2 * @a samplesPerSet by 2.
   *
   * @param labels this vector will be filled with the corresponding
   * class labels (0 for the first half, 1 for the rest).
   */
  void PII_CLASSIFICATION_EXPORT createDoubleSpiral(int samplesPerSet,
                                                    double rounds,
                                                    PiiMatrix<double>& samples,
                                                    QVector<double>& labels);
  /**
   * Creates a non-linearly separable binary sample set so that the
   * samples in class one are surrounded by those in the other one. In
   * the picture below, @a samples1 = @a samples2 = 200. Samples with
   * label 0 are shown in red, and the samples with label 1 in blue.
   *
   * @image html dartboard.png
   *
   * @param samples1 the number of samples in the set at the center.
   *
   * @param samples2 the number of samples in the surrounding set.
   *
   * @param samples this matrix will be filled with the
   * two-dimensional feature vectors. The first @a samples1 rows
   * will represent class 0 and the rest class 1. The size of the
   * matrix will be @a samples1 + @a samples2 by 2.
   *
   * @param labels this vector will be filled with the corresponding
   * class labels (0 for the @a samples1 entries, 1 for the rest).
   */
  void PII_CLASSIFICATION_EXPORT createDartBoard(int samples1, int samples2,
                                                 PiiMatrix<double>& samples,
                                                 QVector<double>& labels);
}

#include "PiiClassification-templates.h"

#endif //_PIICLASSIFICATION_H
