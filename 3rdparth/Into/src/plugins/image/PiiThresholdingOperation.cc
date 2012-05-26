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

#include <PiiThresholdingOperation.h>
#include <PiiThresholding.h>
#include <PiiMatrix.h>
#include <PiiHistogram.h>
#include <PiiYdinTypes.h>


PiiThresholdingOperation::Data::Data() :
  dAbsoluteThreshold(0.0),
  dRelativeThreshold(1.0),
  thresholdType(StaticThreshold),
  bThresholdConnected(false),
  bInverse(false),
  windowSize(15,15)
{
}

PiiThresholdingOperation::PiiThresholdingOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  d->pImageInput = new PiiInputSocket("image");
  d->pThresholdInput = new PiiInputSocket("threshold");
  d->pBinaryImageOutput = new PiiOutputSocket("image");
  d->pThresholdOutput = new PiiOutputSocket("threshold");
  d->pThresholdInput->setOptional(true);

  addSocket(d->pImageInput);
  addSocket(d->pThresholdInput);
  addSocket(d->pBinaryImageOutput);
  addSocket(d->pThresholdOutput);
}
  
void PiiThresholdingOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if ((d->thresholdType == RelativeToMeanAdaptiveThreshold ||
       d->thresholdType == MeanStdAdaptiveThreshold) &&
      (d->windowSize.width() < 1 ||
       d->windowSize.height() < 1))
    PII_THROW(PiiExecutionException, tr("Window size is too small for adaptive thresholding."));
  
  d->bThresholdConnected = d->pThresholdInput->isConnected();
}

void PiiThresholdingOperation::process()
{
  PII_D;
  PiiVariant obj = d->pImageInput->firstObject();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(thresholdGray, obj);
      PII_COLOR_IMAGE_CASES(thresholdColor, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }  
}

template <class T> void PiiThresholdingOperation::thresholdColor(const PiiVariant& obj)
{
  threshold(PiiMatrix<typename T::Type>(obj.valueAs<PiiMatrix<T> >()));
}

template <class T> void PiiThresholdingOperation::thresholdGray(const PiiVariant& obj)
{
  threshold(obj.valueAs<PiiMatrix<T> >());
}



template <class T> void PiiThresholdingOperation::threshold(const PiiMatrix<T>& image)
{
  PII_D;
  double threshold = 0;

  if (d->bThresholdConnected)
    {
      threshold = d->dRelativeThreshold * PiiYdin::primitiveAs<double>(d->pThresholdInput) + d->dAbsoluteThreshold;
    }
  else
    {
      switch (d->thresholdType)
        {
        case StaticThreshold:
          threshold = d->dAbsoluteThreshold;
          break;
        case OtsuThreshold:
          {
            PiiMatrix<double> normalized(PiiImage::normalize<double>(PiiImage::histogram(image)));
            if ( normalized.columns() < 2 )
              threshold = d->dAbsoluteThreshold;
            else
              threshold = d->dRelativeThreshold * PiiImage::otsuThreshold(normalized) + d->dAbsoluteThreshold;
          }
          break;
        case RelativeToMeanThreshold:
          threshold = Pii::meanAll<double>(image) * d->dRelativeThreshold + d->dAbsoluteThreshold;
          break;
        case RelativeToMinThreshold:
          threshold = d->dRelativeThreshold * Pii::minAll(image) + d->dAbsoluteThreshold;
          break;
        case RelativeToMaxThreshold:
          threshold = d->dRelativeThreshold * Pii::maxAll(image) + d->dAbsoluteThreshold;
          break;
        case MeanStdThreshold:
          {
            if (d->dRelativeThreshold != 0)
              {
                double mean;
                double stdev = Pii::stdAll<double>(image, &mean);
                threshold = mean + d->dRelativeThreshold * stdev + d->dAbsoluteThreshold;
              }
            else
              threshold = Pii::meanAll<double>(image) + d->dAbsoluteThreshold;
          }
          break;
        case PercentageThreshold:
          {
            PiiMatrix<int> matCumulative(PiiImage::cumulative(PiiImage::histogram(image)));
            // Relative threshold times the number of pixels in image
            int iLimit = int(d->dRelativeThreshold * matCumulative(0, matCumulative.columns()-1));
            // Binary search for the first bin in cumulative
            // distribution that exceeds iLimit.
            PiiMatrix<int>::row_iterator i = qLowerBound(matCumulative.rowBegin(0), matCumulative.rowEnd(0), iLimit);
            // The position i can be outside of matrix if
            // d->dRelativeThreshold > 1, but we don't care.
            threshold = int(i - matCumulative.rowBegin(0)) + d->dAbsoluteThreshold;
          }
          break;

          // The rest of the techniques are special cases which must
          // be handled differently.
        case TwoLevelThreshold:
          {
            double otherThreshold = d->dAbsoluteThreshold + d->dRelativeThreshold;
            if (!d->bInverse)
              d->pBinaryImageOutput->emitObject(image.mapped(PiiImage::TwoLevelThresholdFunction<T>(T(qMin(d->dAbsoluteThreshold, otherThreshold)),
                                                                                                    T(qMax(d->dAbsoluteThreshold, otherThreshold)))));
            else
              d->pBinaryImageOutput->emitObject(image.mapped(PiiImage::InverseTwoLevelThresholdFunction<T>(T(qMin(d->dAbsoluteThreshold, otherThreshold)),
                                                                                                           T(qMax(d->dAbsoluteThreshold, otherThreshold)))));
          }
          d->pThresholdOutput->emitObject(d->dAbsoluteThreshold);
          return;
        case HysteresisThreshold:
          if (!d->bInverse)
            d->pBinaryImageOutput->emitObject(PiiImage::hysteresisThreshold(image,
                                                                            T(d->dAbsoluteThreshold - d->dRelativeThreshold),
                                                                            T(d->dAbsoluteThreshold)));
          else
            d->pBinaryImageOutput->emitObject(PiiImage::inverseHysteresisThreshold(image,
                                                                                   T(d->dAbsoluteThreshold - d->dRelativeThreshold),
                                                                                   T(d->dAbsoluteThreshold)));
          d->pThresholdOutput->emitObject(d->dAbsoluteThreshold);
          return;
        case RelativeToMeanAdaptiveThreshold:
          if (!d->bInverse)
            d->pBinaryImageOutput->emitObject(PiiImage::adaptiveThreshold(image,
                                                                          PiiImage::ThresholdFunction<T>(),
                                                                          float(d->dRelativeThreshold),
                                                                          float(d->dAbsoluteThreshold),
                                                                          d->windowSize.height(), d->windowSize.width()));
          else
            d->pBinaryImageOutput->emitObject(PiiImage::adaptiveThreshold(image,
                                                                          PiiImage::InverseThresholdFunction<T>(),
                                                                          float(d->dRelativeThreshold),
                                                                          float(d->dAbsoluteThreshold),
                                                                          d->windowSize.height(), d->windowSize.width()));
          d->pThresholdOutput->emitObject(d->dAbsoluteThreshold);
          return;
        case MeanStdAdaptiveThreshold:
          if (!d->bInverse)
            d->pBinaryImageOutput->emitObject(PiiImage::adaptiveThresholdVar(image,
                                                                             PiiImage::meanStdThresholdFunction(PiiImage::ThresholdFunction<double,T>(),
                                                                                                                std::bind2nd(std::minus<double>(),
                                                                                                                             d->dAbsoluteThreshold),
                                                                                                                d->dRelativeThreshold),
                                                                             d->windowSize.height(), d->windowSize.width()));
          else
            d->pBinaryImageOutput->emitObject(PiiImage::adaptiveThresholdVar(image,
                                                                             PiiImage::meanStdThresholdFunction(PiiImage::InverseThresholdFunction<double,T>(),
                                                                                                                std::bind2nd(std::minus<double>(),
                                                                                                                             d->dAbsoluteThreshold),
                                                                                                                d->dRelativeThreshold),
                                                                             d->windowSize.height(), d->windowSize.width()));
          d->pThresholdOutput->emitObject(d->dAbsoluteThreshold);
          return;
        case SauvolaAdaptiveThreshold:
          if (!d->bInverse)
            d->pBinaryImageOutput->emitObject(PiiImage::adaptiveThresholdVar(image,
                                                                             PiiImage::sauvolaThresholdFunction(PiiImage::ThresholdFunction<double,T>(),
                                                                                                                std::bind2nd(std::minus<double>(),
                                                                                                                             d->dAbsoluteThreshold),
                                                                                                                d->dRelativeThreshold),
                                                                             d->windowSize.height(), d->windowSize.width()));
          else
            d->pBinaryImageOutput->emitObject(PiiImage::adaptiveThresholdVar(image,
                                                                             PiiImage::sauvolaThresholdFunction(PiiImage::InverseThresholdFunction<double,T>(),
                                                                                                                std::bind2nd(std::minus<double>(),
                                                                                                                             d->dAbsoluteThreshold),
                                                                                                                d->dRelativeThreshold),
                                                                             d->windowSize.height(), d->windowSize.width()));
          d->pThresholdOutput->emitObject(d->dAbsoluteThreshold);
          return;
        }
      
    }

  if (!d->bInverse)
    d->pBinaryImageOutput->emitObject(PiiImage::threshold(image, T(threshold)));
  else
    d->pBinaryImageOutput->emitObject(PiiImage::inverseThreshold(image, T(threshold)));
    
  d->pThresholdOutput->emitObject(threshold);
}

double PiiThresholdingOperation::absoluteThreshold() const { return _d()->dAbsoluteThreshold; }
void PiiThresholdingOperation::setAbsoluteThreshold(double absoluteThreshold) { _d()->dAbsoluteThreshold = absoluteThreshold; }
void PiiThresholdingOperation::setRelativeThreshold(double relativeThreshold) { _d()->dRelativeThreshold = relativeThreshold; }
double PiiThresholdingOperation::relativeThreshold() const { return _d()->dRelativeThreshold; }
PiiThresholdingOperation::ThresholdType PiiThresholdingOperation::thresholdType() const { return _d()->thresholdType; }
void PiiThresholdingOperation::setThresholdType(ThresholdType thresholdType) { _d()->thresholdType = thresholdType; }
void PiiThresholdingOperation::setInverse(bool inverse) { _d()->bInverse = inverse; }
bool PiiThresholdingOperation::isInverse() const { return _d()->bInverse; }
void PiiThresholdingOperation::setWindowSize(const QSize& windowSize) { _d()->windowSize = windowSize; }
QSize PiiThresholdingOperation::windowSize() const { return _d()->windowSize; }
