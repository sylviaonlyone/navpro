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

#include "PiiLbpOperation.h"
#include <PiiYdinTypes.h>
#include <PiiColor.h>
#include <PiiMath.h>
#include <PiiTypeTraits.h>

class PiiLbpOperation::AnyLbp
{
public:
  AnyLbp(PiiLbpOperation* parent) :
    pParent(parent),
    d(parent->_d()),
    vecResults(d->lstOperators.size())
  {}
  
  virtual ~AnyLbp() {}

  void send();

  static void addToVariant(PiiVariant& variant, const PiiMatrix<int>& histogram)
  {
    if (!variant.isValid())
      variant = PiiVariant(histogram);
    else
      variant.valueAs<PiiMatrix<int> >() += histogram;
  }
protected:
  void normalize();
  template <class T> PiiMatrix<T> createCompound();

  PiiLbpOperation* pParent;
  PiiLbpOperation::Data* d;
  QVector<PiiVariant> vecResults;
};

template <class T, class LbpType> class PiiLbpOperation::Lbp : public AnyLbp
{
public:
  typedef typename PiiImage::ToGray<T>::Type GrayType;
  
  Lbp(PiiLbpOperation* parent) : AnyLbp(parent) {}

  // Histograms can be summed up, LBP images not.
  static inline bool acceptsManyRegions() { return Pii::IsSame<LbpType,PiiLbp::Histogram>::boolValue; }

  void initialize(const PiiMatrix<GrayType>& image);

  void operator() (const PiiMatrix<T>& image);
  template <class Roi> void operator() (const PiiMatrix<T>& image, const Roi& roi);
  
private:
  template <class Roi> void calculate(const PiiMatrix<GrayType>& image, const Roi& roi);
  // Use at least int for the cumulative sum
  typedef typename Pii::Combine<GrayType,int>::Type SumType;
  PiiMatrix<SumType> matSum;
};

PiiLbpOperation::Data::Data() :
  outputType(HistogramOutput),
  bCompoundConnected(false),
  iStaticOutputCount(0),
  roiType(PiiImage::AutoRoi),
  pLbp(0),
  uiPreviousType(PiiVariant::InvalidType)
{
}

PiiLbpOperation::Data::~Data()
{
  delete pLbp;
}

PiiLbpOperation::PiiLbpOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  
  addSocket(new PiiInputSocket("image"));
  addSocket(d->pRoiInput = new PiiInputSocket("roi"));
  d->pRoiInput->setOptional(true);

  addSocket(new PiiOutputSocket("features"));

  /* NOTE
   * Insert ALL static outputs before the following line!
   */
  d->iStaticOutputCount = outputCount();
  
  createOutput(8, 1, PiiLbp::Standard);
}

PiiLbpOperation::~PiiLbpOperation()
{
  PII_D;
  qDeleteAll(d->lstOperators);
}

PiiLbpOperation::OutputType PiiLbpOperation::outputType() const { return _d()->outputType; }
void PiiLbpOperation::setOutputType(OutputType type) { _d()->outputType = type; }
QStringList PiiLbpOperation::parameters() const { return _d()->lstParameters; }

void PiiLbpOperation::setParameters(const QStringList& parameters)
{
  PII_D;
  
  // Destroy old operators and parameter strings
  qDeleteAll(d->lstOperators);
  d->lstOperators.clear();
  d->lstParameters.clear();
  d->lstSmoothingWindows.clear();
  d->lstThresholds.clear();
  d->bMustSmooth = false;
  
  // ... and create new ones based on the parameters
  for (int i=0; i<parameters.size(); i++)
    {
      QStringList parts = parameters[i].split(',');

      // Default values
      int iSamples = 8;
      double iRadius = 1;
      int iMode = PiiLbp::Standard;
      Pii::Interpolation interpolation = Pii::NearestNeighborInterpolation;
      bool bSmoothed = false;
      double dThreshold = 0;
      
      if (parts.size() > 0)
        {
          int tmp = parts[0].toInt();
          if (tmp > 0 && tmp <= 24)
            iSamples = tmp;
        }
      if (parts.size() > 1)
        {
          double tmp = parts[1].toDouble();
          if (tmp > 0)
            iRadius = tmp;
        }
      for (int p=2; p<parts.size(); p++)
        {
          QString lower = parts[p].toLower();
          // HACK: Uniform | RotationInvariant = UniformRotationInvariant
          if (lower == "uniform")
            iMode |= PiiLbp::Uniform;
          else if (lower == "rotationinvariant")
            iMode |= PiiLbp::RotationInvariant;
          else if (lower == "uniformrotationinvariant")
            iMode = PiiLbp::UniformRotationInvariant;
          else if (lower == "symmetric")
            iMode = PiiLbp::Symmetric;
          else if (lower == "linearinterpolation")
            interpolation = Pii::LinearInterpolation;
          else if (lower == "smoothed")
            bSmoothed = true;
          else
            dThreshold = lower.toDouble();
        }
      createOutput(iSamples, iRadius, (PiiLbp::Mode)iMode, interpolation, bSmoothed, dThreshold);
    }

  // Create default output if everything else fails
  if (d->lstOperators.isEmpty())
    createOutput(8, 1);

  // Delete extra outputs
  while (outputCount() > d->lstOperators.size() + d->iStaticOutputCount)
    delete removeOutput(outputCount() - 1);
}

void PiiLbpOperation::createOutput(int samples, double radius,
                                   PiiLbp::Mode mode,
                                   Pii::Interpolation interpolation,
                                   bool smoothed,
                                   double threshold)
{
  PII_D;

  // create a new LBP operation and store it to the operator list
  PiiLbp* lbp = new PiiLbp(samples, radius, mode, interpolation);
  d->lstOperators << lbp;

  if (smoothed)
    {
      d->lstSmoothingWindows << ceil(2*M_PI*radius/samples);
      d->bMustSmooth = true;
    }
  else
    d->lstSmoothingWindows << 1;

  d->lstThresholds << threshold;

  // store parameters
  QString strParams = QString("%1,%2").arg(samples).arg(radius);

  if (threshold != 0)
    strParams.append(QString(",%1").arg(threshold));

  switch (mode)
    {
    case PiiLbp::Uniform: strParams += ",Uniform"; break;
    case PiiLbp::RotationInvariant: strParams += ",RotationInvariant"; break;
    case PiiLbp::UniformRotationInvariant: strParams += ",UniformRotationInvariant"; break;
    case PiiLbp::Symmetric: strParams += ",Symmetric"; break;
    case PiiLbp::Standard:
    default:
      break;
    }

  switch (interpolation)
    {
    case Pii::LinearInterpolation: strParams += ",LinearInterpolation"; break;
    case Pii::NearestNeighborInterpolation:
    default:
      break;
    }

  // create a new socket if needed
  if (outputCount() < d->lstOperators.size() + d->iStaticOutputCount)
    addSocket(new PiiOutputSocket(QString("features%1").arg(outputCount() - d->iStaticOutputCount)));
  d->lstParameters << strParams;
}

void PiiLbpOperation::setRoiType(PiiImage::RoiType roiType) { _d()->roiType = roiType; }
PiiImage::RoiType PiiLbpOperation::roiType() const { return _d()->roiType; }

void PiiLbpOperation::check(bool reset)
{
  PII_D;

  PiiDefaultOperation::check(reset);
  d->vecMustCalculate.resize(outputCount() - d->iStaticOutputCount);

  d->bCompoundConnected = outputAt(0)->isConnected();

  // Cache for fast run-time access
  for (int i=d->iStaticOutputCount; i<outputCount(); ++i)
    {
      d->vecMustCalculate[i - d->iStaticOutputCount] = outputAt(i)->isConnected() ||
        (d->outputType != ImageOutput && d->bCompoundConnected);
    }

  if (d->outputType == ImageOutput)
    d->vecMustCalculate[0] = true;
}

void PiiLbpOperation::aboutToChangeState(State state)
{
  PII_D;
  if (state == Stopped)
    {
      delete d->pLbp;
      d->pLbp = 0;
      d->uiPreviousType = PiiVariant::InvalidType;
    }
}

void PiiLbpOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_GRAY_IMAGE_CASES(calculate, obj);
      PII_COLOR_IMAGE_CASES(calculate, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));      
    }
}

template <class T> void PiiLbpOperation::calculate(const PiiVariant& obj)
{
  if (_d()->outputType != ImageOutput)
    calculate<PiiLbp::Histogram>(obj.valueAs<PiiMatrix<T> >());
  else
    calculate<PiiLbp::Image>(obj.valueAs<PiiMatrix<T> >());
}


template <class LbpType, class T> void PiiLbpOperation::calculate(const PiiMatrix<T>& image)
{
  PII_D;

  // If the LBP class we already have is suitable for this type of
  // input, we won't bother recreating it.
  if (d->uiPreviousType != Pii::typeId<T>())
    {
      delete d->pLbp;
      d->pLbp = new Lbp<T,LbpType>(this);
      d->uiPreviousType = Pii::typeId<T>();
    }

  Lbp<T,LbpType>& lbp = *static_cast<Lbp<T,LbpType>*>(d->pLbp);
  lbp.initialize(image);

  PiiImage::handleRoiInput(d->pRoiInput, d->roiType, image, lbp);

  lbp.send();
}

template <class T, class LbpType> void PiiLbpOperation::Lbp<T,LbpType>::operator() (const PiiMatrix<T>& image)
{
  calculate(PiiImage::toGray(image), PiiImage::DefaultRoi());
}

template <class T, class LbpType>
template <class Roi>
void PiiLbpOperation::Lbp<T,LbpType>::operator() (const PiiMatrix<T>& image,
                                                  const Roi& roi)
{
  calculate(PiiImage::toGray(image), roi);
}

template <class T, class LbpType>
template <class Roi>
void PiiLbpOperation::Lbp<T,LbpType>::calculate(const PiiMatrix<GrayType>& image,
                                                const Roi& roi)
{
#define PII_LBP_SMOOTH(image) d->lstSmoothingWindows[i] <= 1 ? image : Pii::fastMovingAverage<GrayType>(matSum, d->lstSmoothingWindows[i])

  // Thresholded LBP cannot be calculated with unsigned types due to
  // the possibility of an under/overflow.
  typedef typename Pii::Combine<GrayType,int>::Type TT;

  for (int i=0; i<d->lstOperators.size(); ++i)
    {
      // Calculate features only if needed.
      if (d->vecMustCalculate[i])
        {
          if (d->lstThresholds[i] == 0)
            addToVariant(vecResults[i],
                         d->lstOperators[i]->genericLbp<LbpType>(PII_LBP_SMOOTH(image), roi));
          else
            addToVariant(vecResults[i],
                         d->lstOperators[i]->genericLbp<LbpType>(PII_LBP_SMOOTH(image), roi,
                                                                 std::bind2nd(std::plus<TT>(),
                                                                              TT(d->lstThresholds[i]))));
        }
    }
#undef PII_LBP_SMOOTH
}

template <class T, class LbpType>
void PiiLbpOperation::Lbp<T,LbpType>::initialize(const PiiMatrix<GrayType>& image)
{
  if (d->bMustSmooth)
    matSum = Pii::cumulativeSum<SumType,PiiMatrix<GrayType> >(image, Pii::ZeroBorderCumulativeSum);
  for (int i=0; i<vecResults.size(); ++i)
    vecResults[i] = PiiVariant();
}

void PiiLbpOperation::AnyLbp::normalize()
{
  for (int i=0; i<vecResults.size(); ++i)
    if (vecResults[i].isValid())
      {
        const PiiMatrix<int>& matHistogram = vecResults[i].valueAs<PiiMatrix<int> >();
        float fPixelCount = Pii::sumAll<int>(matHistogram);
        if (fPixelCount > 0)
          vecResults[i] = PiiVariant(matHistogram.mapped(std::bind2nd(std::multiplies<float>(),
                                                                      1.0f / fPixelCount)));
      }
}

void PiiLbpOperation::AnyLbp::send()
{
  if (d->outputType == NormalizedHistogramOutput)
    normalize();
  
  for (int i=0; i<d->lstOperators.size(); ++i)
    if (vecResults[i].isValid())
      pParent->emitObject(vecResults[i], i + d->iStaticOutputCount);

  if (d->bCompoundConnected)
    {
      // If outputType is ImageOutput, send the first feature image to
      // the compound output.
      if (d->outputType == ImageOutput)
        pParent->emitObject(vecResults[0]);
      else if (d->outputType == HistogramOutput)
        pParent->emitObject(createCompound<int>());
      else
        {
          // Create and send normalized compound feature vector
          PiiMatrix<float> matCompound(createCompound<float>());
          matCompound *= 1.0f / d->lstOperators.size();
          pParent->emitObject(matCompound);
        }
    }
}

template <class T> PiiMatrix<T> PiiLbpOperation::AnyLbp::createCompound()
{
  int iColumns = 0;
  for (int i=0; i<vecResults.size(); ++i)
    iColumns += vecResults[i].valueAs<PiiMatrix<T> >().columns();
  PiiMatrix<T> matCompound(1, iColumns);
  T* pData = matCompound[0];
  for (int i=0; i<vecResults.size(); ++i)
    {
      const PiiMatrix<T>& matVector = vecResults[i].valueAs<PiiMatrix<T> >();
      memcpy(pData, matVector[0], matVector.columns()*sizeof(T));
      pData += matVector.columns();
    }

  return matCompound;
}
