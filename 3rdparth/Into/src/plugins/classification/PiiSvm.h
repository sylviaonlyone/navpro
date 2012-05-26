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

#ifndef _PIISVM_H
#define _PIISVM_H

#include "PiiClassifier.h"
#include "PiiLearningAlgorithm.h"
#if 0
/**
 * A Support Vector Machine (SVM).
 *
 * @ingroup PiiNeuralNetPlugin
 */
template <class SampleSet> class PiiSvm :
  public PiiVectorQuantizer<SampleSet>,
  public PiiLearningAlgorithm<SampleSet>
{
public:
	/**
	 * Create a new SVM classifier.
   */ 
  PiiSvm();
	~PiiSvm();

  /**
   * Runs the SVM learning algorithm with the given @a samples and
   * class @a labels. The @a weights will be ignored.
   */
  void learn(const PiiSampleSet<SampleSet>& samples,
             const QVector<double>& labels,
             const QVector<double>& weights = QVector<double>());

  /**
   * Returns classification of given feature vector. If SVM is not
   * trained classify returns -1.
   *
   * @param featureVector the feature vector of sample which
   * classification is not yet know. 
   * Feature vector must be 1 X N matrix.
   *
   * @return Returns class of unknow feature vector, if SVM is not
   * trained classify returns -1.  
   */
  int classify(const PiiMatrix<T>& featureVector) const throw();

  /**
   * Returns found support vectors which are used to classification.
   *
   * @return found support vectors, if training is not done empty
   * matrix will be returned.
   */
  PiiMatrix<double> supportVectors() const;

  void setType(PiiNeuralNet::SvmType type);
  PiiNeuralNet::SvmType type() const;

  void setKernelType(PiiNeuralNet::SvmKernel type);
  PiiNeuralNet::SvmKernel kernelType() const;
  
  void setKernelDegree(int degree);
  int kernelDegree() const;
  
  void setKernelGamma(double gamma);
  double kernelGamma() const;
  
  void setKernelCoef0(double coef0);
  double kernelCoef0() const;

  void setKernelCacheSize(unsigned long bytes);
  unsigned long kernelCacheSize() const;
  
  void setTerminationTolerance(double eps);
  double terminationTolerance() const;

	void setCost(double cost);
	double cost() const;

	void setLabelWeights(const QVector<double>& labelWeights);
	QVector<double> labelWeights() const;

	void setNu(double nu);
	double nu() const;

	void setSvrEps(double eps);
	double svrEps() const;
  
  void setShrinking(bool shrinking);
  bool shrinking() const;
  
  void setEstimateProbabilities(bool estimate);
  int estimateProbabilities() const;
  
private:
	/// @internal
	class Data
	{
	public:
		Data();
		svm_model* pModel;
		svm_problem problem;
		svm_parameter param;
		int iDimension;
	} *d;

	static QString tr(const char* message);
	
  // Serialization declarations
  friend struct PiiSerialization::Accessor;
  template <class Archive> void serialize(Archive& archive, const unsigned int version);
};

#include "PiiSvm-templates.h"

#endif
#endif //_PIISVM_H
