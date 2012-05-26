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

#include "PiiOrientationEstimator.h"
#include <PiiYdinTypes.h>
#include <PiiImage.h>

PiiMatrix<float> PiiOrientationEstimator::_gradientFilterX(3, 3,
                                                           -M_SQRT1_2, 0.0, M_SQRT1_2,
                                                           -1.0      , 0.0, 1.0,
                                                           -M_SQRT1_2, 0.0, M_SQRT1_2);

PiiMatrix<float> PiiOrientationEstimator::_gradientFilterY(3, 3,
                                                           M_SQRT1_2 , 1.0 , M_SQRT1_2,
                                                           0.0       , 0.0 , 0.0,
                                                           -M_SQRT1_2, -1.0, -M_SQRT1_2);
                                                           


PiiOrientationEstimator::Data::Data() :
  iAngles(180),
  estimationType(UnidirectionalGradient),
  bRotateHistogram(false),
  bNormalized(true)
{
}

PiiOrientationEstimator::PiiOrientationEstimator() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  addSocket(d->pImageInput = new PiiInputSocket("image"));
  addSocket(d->pHistogramOutput = new PiiOutputSocket("histogram"));
}

void PiiOrientationEstimator::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if (d->iAngles < 2) d->iAngles = 2;
  else if (d->iAngles > 3600) d->iAngles = 3600;
}

void PiiOrientationEstimator::process()
{
  PII_D;
  PiiVariant obj = d->pImageInput->firstObject();

  switch (obj.type())
    {
      PII_GRAY_IMAGE_CASES(grayOrientation, obj);
      PII_COLOR_IMAGE_CASES(colorOrientation, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }
}


template <class ColorType> void PiiOrientationEstimator::colorOrientation(const PiiVariant& obj)
{
  const PiiMatrix<ColorType> img = obj.valueAs<PiiMatrix<ColorType> >();
  estimateOrientation(PiiMatrix<typename ColorType::Type>(img));
}

template <class T> void PiiOrientationEstimator::grayOrientation(const PiiVariant& obj)
{
  estimateOrientation(obj.valueAs<PiiMatrix<T> >());
}

template <class T> void PiiOrientationEstimator::estimateOrientation(const PiiMatrix<T>& img)
{
  PII_D;
  switch (d->estimationType)
    {
    case Gradient:
    case UnidirectionalGradient:
      estimateGradient(img);
      break;
    case Fourier:
      estimateFourier(img);
      break;
    }
}

template <class T> void PiiOrientationEstimator::estimateFourier(const PiiMatrix<T>& img)
{
  // Convert the image to floating point
  PiiMatrix<float> floatImage(img);
  estimateFourierFloat(floatImage);
}

void PiiOrientationEstimator::estimateFourierFloat(PiiMatrix<float>& img)
{
  PII_D;
  // Reduce aperture effect
  img -= Pii::meanAll<float>(img);
  PiiMatrix<std::complex<float> > transformed(d->fft.forwardFft(img));
  // Power spectrum is symmetric for real signals. We thus take only
  // the upper half.
  PiiMatrix<float> powerSpectrum(Pii::abs(transformed(0,0,transformed.rows()/2,-1)));

  int halfCols = powerSpectrum.columns() / 2;
  float scale = float(d->iAngles) / M_PI;
  int halfAngles = d->iAngles >> 1;

  PiiMatrix<float> matResult(1, d->iAngles);
  float* resultRow = matResult[0];
  const float* spectrumRow = 0;

  float fAspectRatio = float(transformed.columns()) / transformed.rows();

  for (int r = 1; r < powerSpectrum.rows(); ++r)
    {
      spectrumRow = powerSpectrum.row(r);

      // Only need to calculate half of the angles. The other half is
      // just 180-angle.
      // We didn't do fftshift and must index the spectrum like this:
      // 0 1 2 3 -4 -3 -2 -1
      // (Assuming halfCols == 4)
      // Angle to x axis at 0 is 90 degrees. If the angle at 1 is a,
      // the angle at -1 is 180 - a.
      // If the number of columns is odd, we neglect the highest
      // frequency component at the center.

      // The first element is at 90 degrees.
      resultRow[halfAngles] += spectrumRow[0];

      /* PENDING Would it be clever to just count on a
       * ellipse/disc-shaped area? Now diagonals are emphasized by
       * sqrt(2) with square images.
       */
      for (int c = 1; c < halfCols; ++c)
        {
          // Calculate angle to x axis. The value will always be in
          // [0,pi]. Note that this is a negation of the real value as
          // the y axis points down. Quantize at the same time.
          int angle1 = Pii::round<int>(atanf(fAspectRatio*r/c) * scale);
          // Angle is now in [0,d->iAngles/2]. Since we cannot
          // separate orientations pointing to opposite directions,
          // -angle is equal to 180-angle.
          int angle2 = d->iAngles-angle1;
          if (angle2 >= d->iAngles) angle2 -= d->iAngles;
          resultRow[angle2] += spectrumRow[c];
          resultRow[angle1] += spectrumRow[powerSpectrum.columns()-c];
        }
    }
  // Add horizontal component
  spectrumRow = powerSpectrum.row(0);
  for (int c = 1; c < halfCols; ++c)
    resultRow[0] += spectrumRow[c];

  emitHistogram(matResult);
}

template <class T> void PiiOrientationEstimator::estimateGradient(const PiiMatrix<T>& img)
{
  // Estimate gradient with a custom gradient mask
  PiiMatrix<float> gradientX = PiiImage::filter<float>(img, _gradientFilterX, Pii::ExtendNot);
  PiiMatrix<float> gradientY = PiiImage::filter<float>(img, _gradientFilterY, Pii::ExtendNot);

  estimateGradient(gradientX, gradientY);
}

void PiiOrientationEstimator::estimateGradient(const PiiMatrix<float>& gradientX,
                                               const PiiMatrix<float>& gradientY)

{
  PII_D;
  // Calculate gradient strength and direction
  PiiMatrix<float> magnitude = gradientX.mapped(Pii::Hypotenuse<float>(), gradientY);
  PiiMatrix<float> direction = d->iAngles < 90 ?
    gradientY.mapped(Pii::FastestAtan2<float>(), gradientX) : // if 4 dgr accuracy is enough, use a crude approximation
    gradientY.mapped(Pii::Atan2<float>(), gradientX); // else use pretty good approximation

  // Initialize orientation histogram
  PiiMatrix<float> matResult(1, d->iAngles);
  float* resultRow = matResult[0];

  float scale;
  if (d->estimationType == Gradient)
    {
      scale = float(d->iAngles) / (2*M_PI);
    }
  else
    {
      scale = float(d->iAngles) / M_PI;
    }

  // Increase orientation histogram by the value of gradient magnitude
  for (int r=magnitude.rows(); r--; )
    {
      const float* dirRow = direction.row(r);
      const float* magRow = magnitude.row(r);
      for (int c=magnitude.columns(); c--; )
        {
          // Zero magnitude means that there is no gradient vector.
          // The angle cannot thus be calculated.
          if (magRow[c] != 0)
            {
              int dirIndex = Pii::round<int>(dirRow[c] * scale);
              if (dirIndex < 0) dirIndex += d->iAngles;
              else if (dirIndex >= d->iAngles) dirIndex -= d->iAngles;
              resultRow[dirIndex] += magRow[c];
            }
        }
    }
  emitHistogram(matResult);
}

void PiiOrientationEstimator::emitHistogram(PiiMatrix<float>& histogram)
{
  PII_D;
  if (d->bRotateHistogram)
    {
      // In Gradient estimation, the full circle is used. Thus, 90 = 360/4.
      int shift = d->estimationType == Gradient ? d->iAngles / 4 : d->iAngles / 2;
      // Take a deep copy of the first "shift" elements
      PiiMatrix<float> firstPiece(histogram(0,0,1,shift));
      // Move the last elements in the histogram to the beginning.
      memmove(histogram[0], histogram[0] + shift, (d->iAngles - shift) * sizeof(float));
      // Copy the original data back to the end.
      memcpy(histogram[0] + shift, firstPiece[0], shift * sizeof(float));
    }
  if (d->bNormalized)
    histogram /= Pii::sumAll<float>(histogram);
  
  d->pHistogramOutput->emitObject(histogram);
}

int PiiOrientationEstimator::angles() const { return _d()->iAngles; }
void PiiOrientationEstimator::setAngles(int angles) { if (angles > 0 && angles <= 3600) _d()->iAngles = angles; }
PiiOrientationEstimator::EstimationType PiiOrientationEstimator::estimationType() const { return _d()->estimationType; }
void PiiOrientationEstimator::setEstimationType(EstimationType estimationType) { _d()->estimationType = estimationType; }
void PiiOrientationEstimator::setRotateHistogram(bool rotateHistogram) { _d()->bRotateHistogram = rotateHistogram; }
bool PiiOrientationEstimator::rotateHistogram() const { return _d()->bRotateHistogram; }
void PiiOrientationEstimator::setNormalized(bool normalize) { _d()->bNormalized = normalize; }
bool PiiOrientationEstimator::normalized() const { return _d()->bNormalized; }
