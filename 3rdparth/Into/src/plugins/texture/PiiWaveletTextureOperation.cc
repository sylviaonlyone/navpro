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

#include "PiiWaveletTextureOperation.h"
#include <PiiYdinTypes.h>
#include <PiiMatrixUtil.h>

PiiWaveletTextureOperation::Data::Data() :
  iLevels(3),
  iFeaturesPerLevel(3),
  iNorm(1),
  waveletFamily(PiiDsp::Daubechies),
  iWaveletIndex(2)
{
}

PiiWaveletTextureOperation::PiiWaveletTextureOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  d->pImageInput = new PiiInputSocket("image");
  d->pFeatureOutput = new PiiOutputSocket("features");

  addSocket(d->pImageInput);
  addSocket(d->pFeatureOutput);
  
}

QString PiiWaveletTextureOperation::wavelet() const
{
  const PII_D;
  QString name;
  switch (d->waveletFamily)
    {
    case PiiDsp::Haar:
      name = "Haar";
      break;
    case PiiDsp::Daubechies:
      name = "Daubechies" + QString::number(d->iWaveletIndex);
      break;
    }
  return name;
}

void PiiWaveletTextureOperation::setWavelet(const QString& name)
{
  PII_D;
  if (name.startsWith("Daubechies"))
    {
      int number = name.mid(10).toInt();
      // Default wavelet for the Daubechies family is two
      if (number < 1 || number > 10)
        number = 2;
      d->waveletFamily = PiiDsp::Daubechies;
      d->iWaveletIndex = number;
    }
  else //if (name == "Haar")
    {
      d->waveletFamily = PiiDsp::Haar;
      d->iWaveletIndex = 1;
    }
}

void PiiWaveletTextureOperation::process()
{
  PII_D;
  PiiVariant obj = d->pImageInput->firstObject();

  switch (obj.type())
    {
      PII_INT_GRAY_IMAGE_CASES(waveletNormInt, obj);
    case PiiYdin::FloatMatrixType:
      waveletNormFloat<float>(obj);
      break;
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }
}

template <class T> void PiiWaveletTextureOperation::waveletNormInt(const PiiVariant& obj)
{
  waveletNorm(PiiMatrix<float>(obj.valueAs<PiiMatrix<T> >()));
}

template <class T> void PiiWaveletTextureOperation::waveletNormFloat(const PiiVariant& obj)
{
  waveletNorm(obj.valueAs<PiiMatrix<T> >());
}

template <class T> void PiiWaveletTextureOperation::waveletNorm(const PiiMatrix<T>& mat)
{
  PII_D;
  PiiMatrix<float> result(1, d->iLevels * d->iFeaturesPerLevel + 1);
  // Perform a N-level wavelet decomposition
  QList<PiiMatrix<T> > decomposition;
  decomposition << mat;
  int index = 0;
  for (int i=d->iLevels; i--; )
    {
      decomposition = PiiDsp::dwt(decomposition[0], d->waveletFamily, d->iWaveletIndex);
      switch (d->iFeaturesPerLevel)
        {
        case 1: // rotation invariant
          result(0,index++) = Pii::norm(PiiMatrix<T>(decomposition[1] + decomposition[2]), d->iNorm);
          break;
        case 4: // all decomposition results taken
          result(0,index++) = Pii::norm(decomposition[0], d->iNorm);
        case 3: // all but approximation
          result(0,index++) = Pii::norm(decomposition[3], d->iNorm);
        case 2: // only horizontal and vertical details
          result(0,index++) = Pii::norm(decomposition[1], d->iNorm);
          result(0,index++) = Pii::norm(decomposition[2], d->iNorm);
          break;
        }
    }

  // Last level approximation is always included (except for the case
  // where it already was)
  if (d->iFeaturesPerLevel != 4)
    result(0,index) = Pii::norm(decomposition[0], d->iNorm);

  d->pFeatureOutput->emitObject(result);
}

int PiiWaveletTextureOperation::levels() const { return _d()->iLevels; }
void PiiWaveletTextureOperation::setLevels(int levels) { _d()->iLevels = levels; }
int PiiWaveletTextureOperation::featuresPerLevel() const { return _d()->iFeaturesPerLevel; }
void PiiWaveletTextureOperation::setFeaturesPerLevel(int features) { _d()->iFeaturesPerLevel = features; }
int PiiWaveletTextureOperation::norm() const { return _d()->iNorm; }
void PiiWaveletTextureOperation::setNorm(int norm) { _d()->iNorm = norm; }
