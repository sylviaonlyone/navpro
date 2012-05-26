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

#ifndef _PIIRANSAC_H
#define _PIIRANSAC_H

#include "PiiOptimizationGlobal.h"
#include <QVector>
#include <PiiMatrix.h>

/**
 * A generic implementation of the Randomized Sample Consensus
 * (RANSAC) algorithm. RANSAC is an iterative method to estimate
 * parameters of a mathematical model from a set of observed data
 * containing outliers.
 *
 * The algorithm randomly selects a subset of observed data and finds
 * the models that are consistent with it. Among all possible subsets,
 * it selects the one that results in a model that matches the most
 * samples. RANSAC evaluates the selected model by measuring how well
 * it matches all observed data. @e Inlying data are those that match
 * the model with an error less than a given threshold.
 *
 * RANSAC is a generic algorithm that has many uses in computer vision
 * applications. It is most commonly used in solving the
 * correspondence problem: finding the geometric transform that best
 * describes the arrangement of points with respect to each other.
 *
 * This algorithm assumes that the mathematical model can be described
 * by N @p doubles. Therefore, models are represented as row matrices
 * with N columns.
 *
 * @ingroup PiiOptimization
 */
class PII_OPTIMIZATION_EXPORT PiiRansac
{
public:
  virtual ~PiiRansac();

  /**
   * Finds the model that best matches observed data. This function
   * implements the RANSAC algorithm in an abstract way. Subclasses
   * define the actual model that is matched to the data, and the type
   * of data actually used.
   *
   * @return @p true on success, @p false if no suitable model could
   * be found.
   */
  bool findBestModel();

  /**
   * Returns the best model found by the last #findBestModel() call.
   * The returned value is a 1-by-N matrix that contains the
   * parameters of the estimated mathematical model.
   *
   * Note that the best model found by the RANSAC algorithm is based
   * on the minimum number of points necessary to solve the model
   * parameters (#minSamples()). As a result, the model that is
   * returned by this function by default may be a rather rough
   * estimate. Subclasses may override this function in order to
   * refine the model.
   */
  virtual PiiMatrix<double> bestModel() const;

  /**
   * Returns the indices of inlying samples found by the last
   * #findBestModel() call.
   */
  QVector<int> inlyingPoints() const;

  /**
   * Returns the number of inlying points.
   */
  int inlierCount() const;

  /**
   * Sets the maximum number of iterations the algorithm will run if
   * it doesn't find a good solution earlier. The default value is
   * 1000.
   */
  void setMaxIterations(int maxIterations);
  /**
   * Returns the maximum number of iterations.
   */
  int maxIterations() const;
  /**
   * Sets the maximum number of random samplings the algorithm will
   * try while finding model candidates. The algorithm will fail if no
   * valid model candidates are found after this many trials. The
   * default value is 100.
   */
  void setMaxSamplings(int maxSamplings);
  /**
   * Returns the maximum number of random samplings.
   */
  int maxSamplings() const;
  /**
   * Sets the minimum number of inliers that are required for an
   * accepted match. A candidate model will be rejected if the number
   * of inliers is less than this value. The default value is zero.
   */
  void setMinInliers(int minInliers);
  /**
   * Returns the minimum number of inliers required for an accepted
   * match.
   */
  int minInliers() const;
  /**
   * Sets the threshold for accepting a sample as an inlier. Usually,
   * the fit is measured as a squared geometric distance between a
   * model and a query point, but the measure may also be different
   * depending on the type of the model. Default value is 16. See
   * #fitToModel().
   */
  void setFittingThreshold(double fittingThreshold);
  /**
   * Returns the current fitting threshold.
   */
  double fittingThreshold() const;
  /**
   * Set the probability of choosing a model that fits the data well
   * enough. Provided that the model can be fitted to the data, the
   * RANSAC algorithm can estimate the number or rounds required to
   * find a good solution. Due to the probabilistic nature of the
   * algorithm, one can never be sure that the solution is actually
   * found. This parameter sets the confidence level for finding a
   * solution. The default is 0.99.
   */
  void setSelectionProbability(double selectionProbability);
  /**
   * Returns the selection probability.
   */
  double selectionProbability() const;

protected:
  /// @internal
  class PII_OPTIMIZATION_EXPORT Data
  {
  public:
    Data();
    
    int iMaxIterations;
    int iMaxSamplings;
    int iMinInliers;
    double dFittingThreshold;
    double dSelectionProbability;
    QVector<int> vecBestInliers;
    PiiMatrix<double> matBestModel;
  } *d;
  
  /// @internal
  PiiRansac(Data* d);

  /// Constructs a new RANSAC algorithm.
  PiiRansac();

  /**
   * Returns the total number of samples in observed data.
   */
  virtual int totalSampleCount() const = 0;
  
  /**
   * Returns the minimum number of samples required to construct a
   * model. For example, a plane needs at least three points. This
   * function may return a number that is less than the amount of
   * samples required to uniquely determine the model, if
   * #findPossibleModels() is written so that it can find all possible
   * models.
   */
  virtual int minSamples() const = 0;
  
  /**
   * Returns all models that can describe the given point
   * configuration. If #minSamples() returns a value that is less than
   * the the amount of samples required to uniquely determine the
   * model, this function returns all models consistent with the given
   * points. For example, up to three fundamental matrices can be
   * fitted to seven points. Typically, however, the function only
   * returns a single model.
   *
   * If no model can fit the given points (degenerate case), an empty
   * matrix must be returned. For example, three co-linear points
   * don't determine a plane even though they would otherwise be
   * enough for finding the parameters.
   *
   * @param dataIndices an array of indices that refer to the samples
   * that should be used in constructing the model. The number of
   * indices is always #minSamples(). For example, if the samples at
   * indices 0, 5, and 9 should be used to form the model, @p
   * dataIndices would contain 0, 5, and 9. It is up to the model
   * implementation to handle the data associated with the given
   * indices. Possible indices range from 0 to #totalSampleCount() -
   * 1.
   *
   * @return a matrix in which each row represents a model that
   * matches the given configuration of samples, of an empty matrix if
   * the samples are in a degenerate configuration.
   */
  virtual PiiMatrix<double> findPossibleModels(const int* dataIndices) = 0;

  /**
   * Fit the sample at @a dataIndex to the given @a model. The
   * function should return zero when the sample at @a dataIndex is in
   * perfect agreement with @a model. The value should grow as the
   * agreement becomes worse.
   *
   * @param dataIndex the index of a sample to be tested in the
   * observed data
   *
   * @param model a pointer to the beginning of the model to be
   * matched
   *
   * @return the fit of the sample with respect to the model. Usually
   * a squared geometric distance, but other measures can also be
   * used. Remember to set @p fittingThreshold correspondingly.
   *
   * @see setFittingThreshold()
   */
  virtual double fitToModel(int dataIndex, const double* model) = 0;

  PII_DISABLE_COPY(PiiRansac);
};

#endif //_PIIRANSAC_H
