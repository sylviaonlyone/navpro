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

#include "PiiImageFilterOperation.h"

#include <PiiYdinTypes.h>
#include <PiiMath.h>
#include "PiiImage.h"

PiiImageFilterOperation::Data::Data() :
  filterType(Prebuilt), iFilterSize(3),
  borderHandling(Pii::ExtendZeros),
  matPrebuiltFilter(3, 3),
  bSeparableFilter(false)
{
}

PiiImageFilterOperation::PiiImageFilterOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  setFilterName("uniform");

  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("image"));
}

void PiiImageFilterOperation::setFilterName(const QString& filterName)
{
  PII_D;
  static const char* filterNames[] =
    {
      "sobelx", "sobely",
      "prewittx", "prewitty",
      "robertsx", "robertsy",
      "uniform", "gaussian",
      "log"
    };
  
  QString n = filterName.toLower();
  if (n == "custom")
    d->filterType = Custom;
  else if (n == "median")
    d->filterType = Median;
  else
    {
      int iFilterIndex = 0;
      for (iFilterIndex = sizeof(filterNames)/sizeof(filterNames[0]); iFilterIndex--; )
        if (n == filterNames[iFilterIndex])
          break;

      // Name not found
      if (iFilterIndex < 0)
        {
          n = "uniform"; // default
          iFilterIndex = PiiImage::UniformFilter;
        }
      d->matPrebuiltFilter = PiiImage::makeFilter<double>(static_cast<PiiImage::PrebuiltFilterType>(iFilterIndex),
                                                          d->iFilterSize);
      d->filterType = Prebuilt;
    }
  d->strFilterName = n;
}

void PiiImageFilterOperation::setFilter(const PiiVariant& filter)
{
  PII_D;
  if (!filter.isValid())
    d->matCustomFilter.resize(0,0);
  else
    {
      switch (filter.type())
        {
          PII_INTEGER_MATRIX_CASES(setCustomFilter, filter);
        default:
          return;
        }
    }
  d->pCustomFilter = filter;
}

template <class T> void PiiImageFilterOperation::setCustomFilter(const PiiVariant& obj)
{
  PII_D;
  d->matCustomFilter = obj.valueAs<PiiMatrix<T> >();
}

void PiiImageFilterOperation::setFilterSize(int filterSize)
{
  PII_D;
  d->iFilterSize = filterSize;
  if (d->filterType == Prebuilt) // rescale prebuilt filters
    setFilterName(d->strFilterName);
}

void PiiImageFilterOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);
  d->matActiveFilter = d->filterType == Prebuilt ? d->matPrebuiltFilter : d->matCustomFilter;

  // Decompose the filter if possible
  d->bSeparableFilter = PiiImage::separateFilter(d->matActiveFilter, d->matHorzFilter, d->matVertFilter);
  /*qDebug("separable: %d (%d x %d) (%d x %d)", d->bSeparableFilter,
         d->matHorzFilter.rows(), d->matHorzFilter.columns(),
         d->matVertFilter.rows(), d->matVertFilter.columns());
  */
}

void PiiImageFilterOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_INT_GRAY_IMAGE_CASES(intGrayFilter, obj);
      PII_INT_COLOR_IMAGE_CASES(intColorFilter, obj);
    case PiiYdin::FloatMatrixType:
      floatGrayFilter<float>(obj);
      break;
    case PiiYdin::FloatColorMatrixType:
      floatColorFilter<PiiColor<float> >(obj);
      break;
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiImageFilterOperation::intGrayFilter(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> img = obj.valueAs<PiiMatrix<T> >();
  switch (d->filterType)
    {
    case Prebuilt:
    case Custom:
      if (d->bSeparableFilter)
        emitObject(PiiImage::intFilter(img, d->matHorzFilter, d->matVertFilter, d->borderHandling));
      else
        emitObject(PiiImage::intFilter(img, d->matActiveFilter, d->borderHandling));
      break;
    case Median:
      emitObject(PiiImage::medianFilter(img, d->iFilterSize, d->iFilterSize, d->borderHandling));
      break;
    }
}

template <class T> void PiiImageFilterOperation::floatGrayFilter(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> img = obj.valueAs<PiiMatrix<T> >();
  switch (d->filterType)
    {
    case Prebuilt:
    case Custom:
      if (d->bSeparableFilter)
        emitObject(PiiImage::filter<T>(img, d->matHorzFilter, d->matVertFilter, d->borderHandling));
      else
        emitObject(PiiImage::filter<T>(img, d->matActiveFilter, d->borderHandling));
      break;
    case Median:
      emitObject(PiiImage::medianFilter(img, d->iFilterSize, d->iFilterSize, d->borderHandling));
      break;
    }
}

template <class T> void PiiImageFilterOperation::intColorFilter(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> img = obj.valueAs<PiiMatrix<T> >();
  typedef typename T::Type PrimitiveType;
  switch (d->filterType)
    {
    case Custom:
    case Prebuilt:
      {
        PiiMatrix<PrimitiveType> ch2 = d->bSeparableFilter ?
          PiiImage::intFilter(PiiImage::colorChannel(img,2),
                              d->matHorzFilter, d->matVertFilter, d->borderHandling) :
          PiiImage::intFilter(PiiImage::colorChannel(img,2),
                              d->matActiveFilter, d->borderHandling);
          
        PiiMatrix<T> matResult(ch2.rows(), ch2.columns());
        PiiImage::setColorChannel(matResult, 2, ch2);
        for (int i=0; i <= 1; ++i)
          PiiImage::setColorChannel(matResult, i, d->bSeparableFilter ?
                                    PiiImage::intFilter(PiiImage::colorChannel(img,i),
                                                        d->matHorzFilter, d->matVertFilter, d->borderHandling) :
                                    PiiImage::intFilter(PiiImage::colorChannel(img,i),
                                                        d->matActiveFilter, d->borderHandling));
        emitObject(matResult);
      }
      break;
    case Median:
      {
        PiiMatrix<PrimitiveType> ch2 = PiiImage::medianFilter(PiiImage::colorChannel(img,2),
                                                               d->iFilterSize, d->iFilterSize, d->borderHandling);
        PiiMatrix<T> matResult(ch2.rows(), ch2.columns());
        PiiImage::setColorChannel(matResult, 2, ch2);
        PiiImage::setColorChannel(matResult, 1, PiiImage::medianFilter(PiiImage::colorChannel(img,1),
                                                                     d->iFilterSize, d->iFilterSize, d->borderHandling));
        PiiImage::setColorChannel(matResult, 0, PiiImage::medianFilter(PiiImage::colorChannel(img,0),
                                                                     d->iFilterSize, d->iFilterSize, d->borderHandling));
        emitObject(matResult);
      }
      break;
    }
}

template <class T> void PiiImageFilterOperation::floatColorFilter(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> img = obj.valueAs<PiiMatrix<T> >();
  typedef typename T::Type PrimitiveType;
  switch (d->filterType)
    {
    case Prebuilt:
    case Custom:
      if (d->bSeparableFilter)
        emitObject(PiiImage::filter<T>(img,
                                                       PiiMatrix<PrimitiveType>(d->matHorzFilter),
                                                       PiiMatrix<PrimitiveType>(d->matVertFilter),
                                                       d->borderHandling));
      else
        emitObject(PiiImage::filter<T>(img,
                                                       PiiMatrix<PrimitiveType>(d->matActiveFilter),
                                                       d->borderHandling));
      break;
    case Median:
      PiiMatrix<PrimitiveType> ch2 = PiiImage::medianFilter(PiiImage::colorChannel(img,2), d->iFilterSize, d->iFilterSize, d->borderHandling);
      PiiMatrix<T> result(ch2.rows(), ch2.columns());
      PiiImage::setColorChannel(result, 2, ch2);
      PiiImage::setColorChannel(result, 1, PiiImage::medianFilter(PiiImage::colorChannel(img,1), d->iFilterSize, d->iFilterSize, d->borderHandling));
      PiiImage::setColorChannel(result, 0, PiiImage::medianFilter(PiiImage::colorChannel(img,0), d->iFilterSize, d->iFilterSize, d->borderHandling));
      emitObject(result);
      break;
    }
}

QString PiiImageFilterOperation::filterName() const { return _d()->strFilterName; }
PiiVariant PiiImageFilterOperation::filter() const { return _d()->pCustomFilter; }
int PiiImageFilterOperation::filterSize() const { return _d()->iFilterSize; }
void PiiImageFilterOperation::setBorderHandling(ExtendMode borderHandling) { _d()->borderHandling = static_cast<Pii::ExtendMode>(borderHandling); }
PiiImageFilterOperation::ExtendMode PiiImageFilterOperation::borderHandling() const { return static_cast<ExtendMode>(_d()->borderHandling); }
