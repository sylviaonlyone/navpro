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

#ifndef _PIISVMOPERATION_H
# error "Never use <PiiSvmOperation-templates.h> directly; include <PiiSvmOperation.h> instead."
#endif


/**
 * Implements the design pattern for QObject templates. Instantiated
 * with a PiiSvm of type @p T.
 *
 * @ingroup PiiNeuralNetPlugin
 */
template <class T> class PiiSvmOperation::Template : public PiiSvmOperation
{
  friend struct PiiSerialization::Accessor;
  PII_SEPARATE_SAVE_LOAD_MEMBERS
  PII_DECLARE_SAVE_LOAD_MEMBERS
  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION;
  
public:


  Template() : _bParamsChanged(true),
               _pParams(new PiiSvmParameters),
               _iKernelType(PiiNeuralNet::RBF),
               _dMemoryCache(100.0),
               _dStoppingCriteria(1e-3),
               _iShrinking(1),
               _iProbability(0),
               _iDegree(3),
               _dGamma(0.0),
               _dCoef0(0.0),
               _iSvmType(PiiNeuralNet::CSvc),
               _dConstraintsViolation(1.0),
               _dNu(0.5),
               _dEpsilon(0.1),
               _bParamCheck(false)
  {
    collector.setCodeBook(new PiiMatrix<T>);
    collector.setClassLabels(new PiiMatrix<int>(0,1));
    classifier.setParameters(_pParams);
  }
               

  ~Template()
  {
    delete collector.getCodeBook();
    delete collector.getClassLabels();
    _pParams = 0;
  }

  void setKernelType(int kernelType) { _bParamsChanged = true, _iKernelType = kernelType; }
  int getKernelType() const { return _iKernelType; }

  void setMemoryCache(double memoryCache) {_bParamsChanged = true, _dMemoryCache = memoryCache; }
  double getMemoryCache() const { return _dMemoryCache; }

  void setStoppingCriteria(double  stoppingCriteria) {_bParamsChanged = true,  _dStoppingCriteria = stoppingCriteria; }
  double  getStoppingCriteria() const { return _dStoppingCriteria; }

  void setShrinking(int shrinking) {_bParamsChanged = true,  _iShrinking = shrinking; }
  int getShrinking() const { return _iShrinking; }

  void setProbability(int probability) {_bParamsChanged = true, _iProbability = probability; }
  int getProbability() const { return _iProbability; }

  void setDegree(int degree) {_bParamsChanged = true;  _iDegree = degree; }
  int getDegree() const { return _iDegree; }

  void setGamma(double gamma) {_bParamsChanged = true;  _dGamma = gamma; }
  double getGamma() const { return _dGamma; }

  void setCoef0(double coef0) { _bParamsChanged = true, _dCoef0 = coef0; }
  double getCoef0() const { return _dCoef0; }

  void setSvmType(int svmType) {_bParamsChanged = true,  _iSvmType = svmType; }
  int getSvmType() const { return _iSvmType; }

  void setConstraintsViolation(double constraintsViolation) {_bParamsChanged = true;
    _dConstraintsViolation = constraintsViolation; }

  double getConstraintsViolation() const { return _dConstraintsViolation; }

  void setNu(double nu) {_bParamsChanged = true, _dNu = nu; }
  double getNu() const { return _dNu; }

  void setEpsilon(double epsilon) {_bParamsChanged = true,  _dEpsilon = epsilon; }
  double getEpsilon() const { return _dEpsilon; }

  void setParamCheck(bool enable) {_bParamsChanged = true, _bParamCheck = enable; }
  bool getParamCheck() const { return _bParamCheck; }

  
  void setLabelWeight(int label, double weight)
  {
    if(_pParams != 0 && _pParams.type() == PiiNeuralNet::CSvc)
      {
        _bParamsChanged = true;
        PiiCSvcParameters* params = static_cast<PiiCSvcParameters*>(_pParams);
        params->setLabelWeight(label,weight);
      }
  }

protected:

  void process()
  {
    if(_bParamsChanged)
      createParams();
    
    PiiBatchClassifierOperation::process(classifier, collector);
    
  }
  /**
   * Starts to train Support Vector Machine with collected data
   * samples. Calls directly super class implementation.
   *
   * @see PiiBatchClassifierOperation
   */
  void startLearning()
  {
    PiiBatchClassifierOperation::startLearning(&classifier, collector.getCodeBook(), collector.getClassLabels());
  }
  
  void createParams()
  {
   
    PiiNeuralNet::SvmType type = static_cast<PiiNeuralNet::SvmType>(_iSvmType);
    PiiNeuralNet::SvmKernel kernel = static_cast<PiiNeuralNet::SvmKernel>(_iKernelType);
    
    switch(type)
      {
        
      case PiiNeuralNet::CSvc:
        {
          
          _pParams = new PiiCSvcParameters(type,
                                           kernel,
                                           _dConstraintsViolation,
                                           _dMemoryCache,
                                           _dStoppingCriteria,
                                           _iShrinking,
                                           _iProbability,
                                           _iDegree,
                                           _dGamma,
                                           _dCoef0,
                                           QMap<int,double>());  
          
          break;
        }
      case PiiNeuralNet::NuSvc:
        {
          _pParams = new PiiNuSvcParameters(type,
                                            kernel,
                                            _dNu,
                                            _dMemoryCache,
                                            _dStoppingCriteria,
                                            _iShrinking,
                                            _iProbability,
                                            _iDegree,
                                            _dGamma,
                                            _dCoef0);
          
          
          break;
        }
      case PiiNeuralNet::OneClassSvm:
        {
          _pParams = new PiiOneClassSvmParameters(type,
                                                  kernel,
                                                  _dNu,
                                                  _dMemoryCache,
                                                  _dStoppingCriteria,
                                                  _iShrinking,
                                                  _iProbability,
                                                  _iDegree,
                                                  _dGamma,
                                                  _dCoef0);
          
          break;
        }
      case PiiNeuralNet::EpsilonSvm:
        {
          _pParams = new PiiEpsilonSvmParameters(type,
                                                 kernel,
                                                 _dConstraintsViolation,
                                                 _dEpsilon,
                                                 _dMemoryCache,
                                                 _dStoppingCriteria,
                                                 _iShrinking,
                                                 _iProbability,
                                                 _iDegree,
                                                 _dGamma,
                                                 _dCoef0);
          break;
        }
      case PiiNeuralNet::NuSvr:
        {
          _pParams = new PiiNuSvrParameters(type,
                                            kernel,
                                            _dConstraintsViolation,
                                            _dNu,
                                            _dMemoryCache,
                                            _dStoppingCriteria,
                                            _iShrinking,
                                            _iProbability,
                                            _iDegree,
                                            _dGamma,
                                            _dCoef0);
          break;
          
        }
      default:
        break;
      }

    _bParamsChanged = false;
    classifier.setParameters(_pParams);
    classifier.setParameterCheck(_bParamCheck);
   
  }
  
  PiiSvm<T> classifier;
  PiiCodeBookCollector<T> collector;

 
private:
  
  bool _bParamsChanged;
  PiiSvmParameters* _pParams;
  
  int _iKernelType;
  double _dMemoryCache;
  double  _dStoppingCriteria;
  int _iShrinking;
  int _iProbability;
  int _iDegree;
  double _dGamma;
  double _dCoef0;
  int _iSvmType;
  double _dConstraintsViolation;
  double _dNu;
  double _dEpsilon;
  bool _bParamCheck;
    
};

template <class T>
template <class Archive> void PiiSvmOperation::Template<T>::save(Archive& archive, const unsigned int /*version*/)
{
  PII_SERIALIZE_BASE(archive, PiiOperation);
  PiiSerialization::saveProperties(archive, *this);
  archive << PII_NVP("trained", getTrainedStatus());
  archive << PII_NVP("PiiSvm", classifier);
}

template <class T>
template <class Archive> void PiiSvmOperation::Template<T>::load(Archive& archive, const unsigned int /*version*/)
{
  PII_SERIALIZE_BASE(archive, PiiOperation);
  PiiSerialization::loadProperties(archive, *this);
  bool status = false;
  archive >> PII_NVP("trained", status);
  setTrainedStatus(status);
  archive >> PII_NVP("PiiSvm", classifier);
  //Create parameters again
  createParams();
}
