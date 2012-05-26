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

#include "PiiSvm.h"
#include <PiiMath.h>

// PENDING
#if 0

PiiSvm::Data::Data() :
  pModel(0),
  iDimension(0)	
{
}

PiiSvm::PiiSvm() :
  d(new Data)
{
}

PiiSvm::~PiiSvm()
{
	svn_destroy_model(d->pModel);
	delete d;
}

QString PiiSvm::tr(const QString& message)
{
	return QCoreApplication::translate("PiiSvm", message);
}

svm_node* PiiSvm::createSparseVector(const double* data, int len)
{
	int iNonzeroCount = 0;
	for (int i=0; i<len; ++i)
		if (!Pii::almostEqualRel(data[i], 0.0))
			++iNonzeroCount;

	svm_node *pNonzeroNodes = new svm_node[iNonzeroCount+1];

	iNonzeroCount = 0;
	for (int i=0; i<len; ++i)
		if (!Pii::almostEqualRel(data[i], 0.0))
			{
				pNonzeroNodes[iNonzeroCount++].index = i+1;
				pNonzeroNodes[iNonzeroCount++].value = pVector[i];
			}
			
	//libSVM wants that the last element of sparse representation of
	//feature vector is indexed to -1
	pNonzeroNodes[iNonzeroCount].index = -1;
	return pNonzeroNodes;
}

svm_problem* PiiSvm::createProblem(const PiiMatrix<double>& featureVectors,
																	 const PiiMatrix<int>& labels)
{
	svm_problem* pProblem = new svm_problem;
  //"l" is the number of training data,
  pProblem->l = iRows;
  pProblem->y = new double[iRows];         
  pProblem->x = new svm_node*[iRows];
  
  _iDimension = iCols;
  
  for (int r=0; r<iRows; ++r)
    {
			pProblem->x[r] = createSparseVector(featureVectors[r], iCols);
			pProblem->y[r] = double(labels(r));
    }
}

struct PiiSvmProblemPtr
{
	PiiSvmProblemPtr(svm_problem* problem) : pProblem(problem) {}
	~PiiSvmProblemPtr()
	{
		for (int i=0; i<pProblem->l; ++i)
			delete[] pProblem->x[i];
		delete[] pProblem->x;
		delete[] pProblem->y;
		delete pProblem;
	}
	const svm_problem* operator-> () const { return pProblem; }
	svm_problem* operator-> () { return pProblem; }
	operator const svm_problem* () const { return pProblem; }
	operator svm_problem* () { return pProblem; }

	svm_problem* pProblem;
};

void PiiSvm::trainBatch(const PiiMatrix<double>& featureVectors,
												const PiiMatrix<int>& labels)
  throw(PiiClassificationException&)
{
  if (type() != PiiNeuralNet::OneClassSvm &&
			((labels.rows() == 1 && featureVectors.rows() != labels.columns()) ||
			 (labels.columns() == 1 && featureVectors.rows() != labels.rows())))
    PII_THROW(PiiClassificationException, tr("There must be a class label for each training sample."));
        
  //Go through matrix and change data to compatible for libSVM-interface
  int iRows = featureVectors.rows();
  int iCols = featureVectors.columns();

	PiiSvmProblemPtr pProblem(createProblem(featureVectors, labels));
	svm_param param = d->param;
  // If gamma is not set it will be same as 1/k where k is number of
  // attributes in the input data.
  if (param.gamma == 0)
    param.gamma = 1.0/iCols;
    
  //Check that parameters are valid for given problem.
	const char* pMessage = svm_check_parameter(pProblem, &param);
	if (pMessage != 0)
		PII_THROW(PiiClassificationException, tr("Invalid SVM parameters: %1").arg(pMessage));

	//Train support vector machine with given parameters and learning material.
  d->pModel = svm_train(pProblem, &param);
}

int PiiSvm::classify(const PiiMatrix& featureVector) const throw()
{
  if (d->pModel == 0)
    return -1;

	PiiSmartPtr<svm_node[]> pSparseFeatures(createSparseVector(featureVector[0], featureVector.columns()));
  int prediction = int(svm_predict(d->pModel, pSparseFeatures));

  return prediction;
}

PiiMatrix<double> PiiSvm::supportVectors() const
{
  if (d->pModel == 0)
    return PiiMatrix<double>();

  int rows = d->pModel->l;
  int cols = _iDimension;
  PiiMatrix<double> vectors(rows, cols);

  for (int r = 0; r < rows; ++r)
    {
      svm_node* node = d->pModel->SV[r];
      int c = 0;
      while (node[c].index != -1)
        {
          //Index starts from 1 to n and we want to save index 1 to
          //first column 
          
          vectors(r, node[c].index-1) = node[c].value;
          ++c;
        }
    }
  return vectors;
}

void PiiSvm::setType(PiiNeuralNet::SvmType type) { d->param.svm_type = type; }
PiiNeuralNet::SvmType PiiSvm::type() const { return (PiiNeuralNet::SvmType)d->param.svm_type; }
void PiiSvm::setKernelType(PiiNeuralNet::SvmKernel type) { d->param.kernel_type = type; }
PiiNeuralNet::SvmKernel PiiSvm::kernelType() const { return (PiiNeuralNet::SvmKernel)d->param.kernel_type; }
void PiiSvm::setKernelDegree(int degree) { d->param.degree = degree; }
int PiiSvm::kernelDegree() const { return d->param.degree; }
void PiiSvm::setKernelGamma(double gamma) { d->param.gamma = gamma; }
double PiiSvm::kernelGamma() const { return d->param.gamma; }
void PiiSvm::setKernelCoef0(double coef0) { d->param.coef0 = coef0; }
double PiiSvm::kernelCoef0() const { return d->param.coef0; }
void PiiSvm::setKernelCacheSize(unsigned long bytes) { d->param.cache_size = double(bytes) / (1024*1024); }
unsigned long PiiSvm::kernelCacheSize() const { return static_cast<unsigned long>(d->param.cache_size * 1024*1024); }
void PiiSvm::setTerminationTolerance(double eps) { d->param.eps = eps; }
double PiiSvm::terminationTolerance() const { return d->param.eps; }
void PiiSvm::setCost(double cost) { d->param.C = cost; }
double PiiSvm::cost() const { return d->param.C; }
void PiiSvm::setLabelWeights(const QVector<double>& labelWeights)
{
	d->labelWeights = labelWeights;
	d->param.nr_weight = labelWeights.size();
	d->param.weight_label = labelWeights._d();
}
QVector<double> labelWeights() const { return d->labelWeights; }
void setNu(double nu) { d->param.nu = nu; }
void PiiSvm::setNu(double nu) { d->param.nu = nu; }
double PiiSvm::nu() const { return d->param.nu; }
void PiiSvm::setSvrEps(double eps) { d->param.p = eps; }
double PiiSvm::svrEps() const { return d->param.p; }
void PiiSvm::setShrinking(bool shrinking) { d->param.shrinking = shrinking ? 1 : 0;}
bool PiiSvm::shrinking() const { return d->param.shrinking != 0;}
void PiiSvm::setEstimateProbabilities(bool estimate) { d->param.probability = estimate ? 1 : 0; }
int PiiSvm::estimateProbabilities() const { return  d->param.probability != 0; }

#endif
