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

//#ifndef _PIISVMOPERATION_H
#if 0
#define _PIISVMOPERATION_H

#include "PiiSvm.h"
#include "PiiNeuralNetGlobal.h"
#include <PiiBatchClassifierOperation.h>

/**
 * An Ydin-compatible SVM classifier operation. SVM is a supervised
 * learning machine. Thus, it does have a class label input. See the
 * description of input and output sockets in PiiClassifierOperation.
 * 
 * To instantiate the operation from an object registry, one must
 * specify the data type, e.g "PiiSvmOperation<double>". 
 *
 * @see PiiClassifierOperation
 *
 * @ingroup PiiNeuralNetPlugin
 */
class PII_NEURALNET_EXPORT PiiSvmOperation : public PiiBatchClassifierOperation
{
  Q_OBJECT

  /**
   * The type of the SVM. Property defines which kind SVM user wants
   * to use. Default value for property is PiiNeuralNet::CSvc.
   *
   * @see PiiNeuralNet for possible types.
   *
   * @note Because Qt does not support well enum properties, enums
   * here are used as integers so that value zero corresponds first enum value etc.
   * 
   */
  Q_PROPERTY(int svmType READ getSvmType WRITE setSvmType);
  
  /**
   * The type of kernel in the SVM. Property defines which kind
   * kernel function is used in SVM operation. Default value for
   * property is PiiNeuralNet::RBF. 
   *
   * @see PiiNeuralNet for possible kernel types.
   *
   */
  Q_PROPERTY(int kernelType READ getKernelType WRITE setKernelType);
  
  /**
   * Property which defines size of the kernel cache, specified in megabytes.
   * Value is used as internally to optimization purposes in libsvm.
   * Default value for property is 100.0 Mb. 
   *
   */
  Q_PROPERTY(double memoryCache READ getMemoryCache WRITE setMemoryCache);

  /**
   * Stopping criterion for SVM. Property defines value which is used
   * to define a moment when SVM training algorithm has converged
   * to solution. Usually user should use value which is near 0.00001
   * for SVM's which type is NuSvc, and 0.001 for others. Default
   * value for property is 0.001.    
   * 
   */
  Q_PROPERTY(double  stoppingCriteria READ getStoppingCriteria WRITE setStoppingCriteria);

  /**
   * A flag which defines is shrinking conducted. Value 1 (one)
   * means shrinking is conducted 0 (zero) otherwise. Default value
   * for property is one. 
   */
  Q_PROPERTY(int shrinking READ getShrinking WRITE setShrinking);

  /**
   * A flag which defines is model with probability
   * information obtained. Value 1 (one) means probabily model is used
   * zero otherwise. Default value for property is zero. 
   */
  Q_PROPERTY(int probability READ getProbability WRITE setProbability);

  /**
   * Set the degree of kernel function. This property is used only for
   * polynomial kernel functions. Default value for property is 3.
   *
   * @see PiiNeuralNet for futher use. 
   *
   */
  Q_PROPERTY(int degree READ getDegree WRITE setDegree);

  /**
   * Set the gamma of kernel function. Property value is used only for
   * polynomial, radial basis and sigmoid kernel functions. Default
   * value for property is 0.
   *
   * @see PiiNeuralNet for futher use.
   *
   */
  Q_PROPERTY(double gamma READ getGamma WRITE setGamma);

  /**
   * Set the coefficient term of kernel function. Property value is
   * used only for polynomial and sigmoid kernel functions. Default
   * value for property is 0.0
   *
   * @see PiiNeuralNet for futher use.
   */
  Q_PROPERTY(double coef0 READ getCoef0 WRITE setCoef0);

 
  /**
   * Set cost of constraints violation. Default value for property is 1.0. 
   * 
   */
  Q_PROPERTY(double constraintsViolation READ getConstraintsViolation WRITE setConstraintsViolation);

  /**
   * Size of nu. Property value is used in SVM which type is 
   * nu-SVM, nu-SVR, or one-class-SVM. Default value for property is 0.5
   *
   * @see PiiNeuralNet for possible SVM types.
   *
   */
  Q_PROPERTY(double nu READ getNu WRITE setNu);

  /**
   * Size of epsilon in epsilon-insensitive loss function
   * of epsilon-SVM regression. Property is used only in SVM which type is
   * PiiNeuralNet::EpsilonSvm. Default value is 0.1
   *
   * 
   */
  Q_PROPERTY(double epsilon READ getEpsilon WRITE setEpsilon);

  /**
   * A flag which indicates that are parameters check used. If flag is
   * set off operation does not try to validate given parameters
   * (values of properties). If validation is "on" and parameters are not
   * valid, operation will throw exception in training phase. That
   * exception is catched in parent class and emited forward as
   * signal #trainingError(). Default value is false.
   *
   * @see PiiBatchClassifierOperation
   * @see PiiSvm
   */
  Q_PROPERTY(bool paramCheck READ getParamCheck WRITE setParamCheck);
  
public:
  PiiSvmOperation();

  virtual void setKernelType(int kernelType) = 0;
  virtual int getKernelType() const = 0;

  virtual void setMemoryCache(double memoryCache) = 0;
  virtual double getMemoryCache() const = 0;

  virtual void setStoppingCriteria(double  stoppingCriteria) = 0;
  virtual double  getStoppingCriteria() const = 0;

  virtual void setShrinking(int shrinking) = 0;
  virtual int getShrinking() const = 0;

  virtual void setProbability(int probability) = 0;
  virtual int getProbability() const = 0;

  virtual void setDegree(int degree) = 0;
  virtual int getDegree() const = 0;

  virtual void setGamma(double gamma) = 0;
  virtual double getGamma() const = 0;

  virtual void setCoef0(double coef0) = 0;
  virtual double getCoef0() const = 0;

  virtual void setSvmType(int svmType) = 0;
  virtual int getSvmType() const = 0;

  virtual void setConstraintsViolation(double constraintsViolation) = 0;
  virtual double getConstraintsViolation() const = 0;

  virtual void setNu(double nu) = 0;
  virtual double getNu() const = 0;

  virtual void setEpsilon(double epsilon) = 0;
  virtual double getEpsilon() const = 0;

  virtual void setLabelWeight(int label, double weight) = 0;

  virtual void setParamCheck(bool enable) = 0;
  virtual bool getParamCheck() const = 0; 

  template <class T> class Template;

protected:
  /// @internal
  virtual void startLearning() = 0;
};

#include "PiiSvmOperation-templates.h"

PII_SERIALIZATION_ABSTRACT(PiiSvmOperation);

PII_DECLARE_SPECIALIZED_VIRTUAL_METAOBJECT_FUNCTION(PiiSvmOperation, float, PII_NEURALNET_EXPORT);
PII_DECLARE_SPECIALIZED_VIRTUAL_METAOBJECT_FUNCTION(PiiSvmOperation, double, PII_NEURALNET_EXPORT);

PII_SHLIB_EXPORT_CLASS_INSTANCE(class, PII_NEURALNET_EXPORT) PiiSvmOperation::Template<float>;
PII_SHLIB_EXPORT_CLASS_INSTANCE(class, PII_NEURALNET_EXPORT) PiiSvmOperation::Template<double>;

#endif //_PIISVMOPERATION_H
