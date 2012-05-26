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

#include "PiiEdgeDetector.h"
#include "PiiImage.h"
#include <PiiYdinTypes.h>
#include "PiiThresholding.h"

PiiEdgeDetector::Data::Data() :
  detector(CannyDetector),
  dThreshold(0), dLowThreshold(0),
  bDirectionConnected(false)
{
}

PiiEdgeDetector::PiiEdgeDetector() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("edges"));
  addSocket(new PiiOutputSocket("magnitude"));
  addSocket(new PiiOutputSocket("direction"));
}

void PiiEdgeDetector::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  switch (d->detector)
    {
    case SobelDetector:
    case CannyDetector:
      d->matFilterX = PiiImage::sobelX;
      d->matFilterY = PiiImage::sobelY;
      break;
    case RobertsDetector:
      d->matFilterX = PiiImage::robertsX;
      d->matFilterY = PiiImage::robertsY;
      break;
    case PrewittDetector:
      d->matFilterX = PiiImage::prewittX;
      d->matFilterY = PiiImage::prewittY;
      break;
    }
  
  d->bDirectionConnected = outputAt(2)->isConnected();
}

void PiiEdgeDetector::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_INT_GRAY_IMAGE_CASES(detectIntEdges, obj);
    case PiiYdin::FloatMatrixType:
      detectFloatEdges<float>(obj);
      break;
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiEdgeDetector::detectIntEdges(const PiiVariant& obj)
{
  PII_D;
  PiiMatrix<int> image(obj.valueAs<PiiMatrix<T> >());
  detectEdges(PiiImage::filter<int>(image, d->matFilterX),
              PiiImage::filter<int>(image, d->matFilterY));
}

template <class T> void PiiEdgeDetector::detectFloatEdges(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> image = obj.valueAs<PiiMatrix<T> >();
  detectEdges(PiiImage::filter<T>(image, PiiMatrix<T>(d->matFilterX)),
              PiiImage::filter<T>(image, PiiMatrix<T>(d->matFilterY)));
}

template <class T> void PiiEdgeDetector::detectEdges(const PiiMatrix<T>& gradientX,
                                                     const PiiMatrix<T>& gradientY)
{
  PII_D;
  PiiMatrix<T> matMagnitude = PiiImage::gradientMagnitude(gradientX, gradientY);
  outputAt(1)->emitObject(matMagnitude);
  // Ensure we are the sole owner of the matrix now
  matMagnitude.detach();
  T threshold = T(d->dThreshold);
  if (d->dThreshold == 0)
    {
      float fMean = 0;
      float fStd = Pii::stdAll<float>(matMagnitude, &fMean);
      // Use the famous two-sigma rule (TM) as a threshold.
      threshold = T(fMean + fStd * 2);
    }
  
  if (d->detector == CannyDetector)
    cannyThreshold(gradientX, gradientY, matMagnitude, threshold);
  else
    matMagnitude.map(PiiImage::ThresholdFunction<T>(), threshold);

  // Send detected edges
  emitObject(matMagnitude);

  if (d->bDirectionConnected)
    outputAt(2)->emitObject(PiiImage::gradientDirection(gradientX, gradientY));
}

template <class T> void PiiEdgeDetector::cannyThreshold(const PiiMatrix<T>& gradientX,
                                                        const PiiMatrix<T>& gradientY,
                                                        PiiMatrix<T>& magnitude,
                                                        T highThreshold)
{
  PII_D;
  T lowThreshold = T(d->dLowThreshold == 0 ? 0.4 * highThreshold : d->dLowThreshold);
  /*magnitude = PiiImage::suppressNonMaxima(magnitude,
                                          PiiImage::gradientDirection(gradientX, gradientY),
                                          PiiImage::RadiansToPoints<float>());*/
  magnitude = PiiImage::hysteresisThreshold(PiiImage::suppressNonMaxima(magnitude,
                                                                         PiiImage::gradientDirection(gradientX, gradientY),
                                                                         PiiImage::RadiansToPoints<float>()),
                                             lowThreshold, highThreshold);
}

PiiEdgeDetector::Detector PiiEdgeDetector::detector() const { return _d()->detector; }
void PiiEdgeDetector::setDetector(Detector detector) { _d()->detector = detector; }
void PiiEdgeDetector::setThreshold(double threshold) { _d()->dThreshold = threshold; }
double PiiEdgeDetector::threshold() const { return _d()->dThreshold; }
void PiiEdgeDetector::setLowThreshold(double lowThreshold) { _d()->dLowThreshold = lowThreshold; }
double PiiEdgeDetector::lowThreshold() const { return _d()->dLowThreshold; }
