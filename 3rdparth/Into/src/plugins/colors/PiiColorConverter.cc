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

#include "PiiColorConverter.h"

#include "PiiColors.h"

#include <PiiYdinTypes.h>
#include <PiiMath.h>

PiiColorConverter::Data::Data() :
  colorConversion(GenericConversion),
  dGamma(1.0/2.2)
{
}

PiiColorConverter::PiiColorConverter() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("image"));
}

void PiiColorConverter::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if (d->pConversionMatrix.isValid())
    {
      switch (d->pConversionMatrix.type())
        {
          PII_PRIMITIVE_MATRIX_CASES(d->matGenericConversion = PiiYdin::matrixAs, d->pConversionMatrix);
        }
    }
  if (d->matGenericConversion.rows() != 3 || d->matGenericConversion.columns() != 3)
    d->matGenericConversion = PiiMatrix<float>::identity(3);

  PiiMatrix<float> matWhitePoint;
  if (d->pWhitePoint.isValid())
    {
      switch (d->pWhitePoint.type())
        {
          PII_PRIMITIVE_MATRIX_CASES(matWhitePoint = PiiYdin::matrixAs, d->pWhitePoint);
        }
    }
  if (matWhitePoint.rows() == 1 && matWhitePoint.columns() == 3)
    d->clrWhitePoint = PiiColor<float>(matWhitePoint(0), matWhitePoint(1), matWhitePoint(2));
  else
    d->clrWhitePoint = PiiColor<float>(95.05, 100, 108.88);
}

void PiiColorConverter::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_COLOR_IMAGE_CASES(convertImage, obj);
    default:
      // Gamma correction can be applied to gray-level images
      if (_d()->colorConversion == GammaCorrection)
        {
          switch (obj.type())
            {
              PII_GRAY_IMAGE_CASES(correctGamma, obj);
            default:
              PII_THROW_UNKNOWN_TYPE(inputAt(0));
            }
        }
      else
        PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiColorConverter::correctGamma(const PiiVariant& obj)
{
  emitObject(PiiColors::correctGamma(obj.valueAs<PiiMatrix<T> >(), _d()->dGamma,
                                     PiiImage::Traits<T>::max()));
}

template <class Clr> void PiiColorConverter::convertImage(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<Clr> image = obj.valueAs<PiiMatrix<Clr> >();
  typedef typename SumTraits<typename Clr::Type>::Type SumType;

  switch (d->colorConversion)
    {
    case GenericConversion:
      emitObject(PiiColors::genericConversion(image, d->matGenericConversion));
      break;
    case RgbToGrayMean:
      sumColors(image, std::bind2nd(std::divides<SumType>(), 3));
      break;
    case RgbToGrayMeanFloat:
      sumColors(image, std::bind2nd(std::divides<float>(), 3.0f));
      break;
    case RgbToGraySum:
      sumColors(image, Pii::Identity<SumType>());
      break;
    case RgbToHsv:
      emitObject(PiiColors::rgbToHsv(image));
      break;
    case HsvToRgb:
      emitObject(PiiColors::hsvToRgb(image));
      break;
    case BgrToRgb:
      emitObject(PiiColors::reverseColors(image));
      break;
    case XyzToLab:
      emitObject(PiiColors::xyzToLab(PiiMatrix<PiiColor<float> >(image), d->clrWhitePoint));
      break;
    case LabToXyz:
      emitObject(PiiColors::labToXyz(PiiMatrix<PiiColor<float> >(image), d->clrWhitePoint));
      break;
    case RgbToLab:
      emitObject(PiiColors::xyzToLab(PiiColors::genericConversion(image, d->matGenericConversion),
                                                     d->clrWhitePoint));
      break;
    case RgbToOhtaKanade:
      emitObject(PiiColors::genericConversion(image, PiiColors::ohtaKanadeMatrix));
      break;
    case RgbToY719:
      emitObject(PiiColors::rgbToY709(image));
      break;
    case RgbToYpbpr:
      emitObject(PiiColors::rgbToYpbpr(image));
      break;
    case YpbprToRgb:
      emitObject(PiiColors::ypbprToRgb(image));
      break;
    case RgbToYcbcr:
      emitObject(PiiColors::rgbToYcbcr(image));
      break;
    case YcbcrToRgb:
      emitObject(PiiColors::ycbcrToRgb(image));
      break;
    case GammaCorrection:
      emitObject(PiiColors::correctGamma(image, d->dGamma, PiiImage::Traits<typename Clr::Type>::max()));
    }
}

template <class T, class UnaryFunction>
void PiiColorConverter::sumColors(const PiiMatrix<T>& image, UnaryFunction func)

{
  typedef typename UnaryFunction::result_type U;
  PiiMatrix<U> matResult(PiiMatrix<U>::uninitialized(image.rows(), image.columns()));

  const int iCols = image.columns(), iRows = image.rows();
  for (int r=0; r<iRows; ++r)
    {
      const T* pInputRow = image[r];
      U* pOutputRow = matResult[r];
      for (int c=0; c<iCols; ++c)
        pOutputRow[c] = func(U(pInputRow[c].channels[0]) +
                             U(pInputRow[c].channels[1]) +
                             U(pInputRow[c].channels[2]));
    }
  emitObject(matResult);
}

void PiiColorConverter::setColorConversion(ColorConversion colorConversion) { _d()->colorConversion = colorConversion; }
PiiColorConverter::ColorConversion PiiColorConverter::colorConversion() const { return _d()->colorConversion; }
void PiiColorConverter::setConversionMatrix(const PiiVariant& conversionMatrix) { _d()->pConversionMatrix = conversionMatrix; }
PiiVariant PiiColorConverter::conversionMatrix() const { return _d()->pConversionMatrix; }
void PiiColorConverter::setWhitePoint(const PiiVariant& whitePoint) { _d()->pWhitePoint = whitePoint; }
PiiVariant PiiColorConverter::whitePoint() const { return _d()->pWhitePoint; }
void PiiColorConverter::setGamma(double gamma) { _d()->dGamma = gamma; }
double PiiColorConverter::gamma() const { return _d()->dGamma; }
