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

#include "PiiTemplateMatcher.h"

#include <PiiYdinTypes.h>
#include "PiiOpenCv.h"
#include <PiiImageFileReader.h>
#include <PiiThresholding.h>
#include <PiiLabeling.h>
#include <PiiObjectProperty.h>

PiiTemplateMatcher::Data::Data() :
  iMatchingMethod(CV_TM_SQDIFF),
  pTemplate(0),
  pMask(0),
  dThreshold(1.0)
{
}

PiiTemplateMatcher::PiiTemplateMatcher() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("correlation"));
  addSocket(new PiiOutputSocket("mask"));
}

PiiTemplateMatcher::~PiiTemplateMatcher()
{
  PII_D;
  delete d->pTemplate;
  delete d->pMask;
}

void PiiTemplateMatcher::setMatchingMethod(MatchingMethod matchingMethod)
{
  PII_D;
  switch (matchingMethod)
    {
    case SquaredDiff: d->iMatchingMethod = CV_TM_SQDIFF; break;
    case NormalizedSquaredDiff: d->iMatchingMethod = CV_TM_SQDIFF_NORMED; break;
    case CrossCorrelation: d->iMatchingMethod = CV_TM_CCORR; break;
    case NormalizedCrossCorrelation: d->iMatchingMethod = CV_TM_CCORR_NORMED; break;
    case CorrelationCoeff: d->iMatchingMethod = CV_TM_CCOEFF; break;
    case NormalizedCorrelationCoeff: d->iMatchingMethod = CV_TM_CCOEFF_NORMED; break;
    }
}

void PiiTemplateMatcher::setTemplateFile(const QString& fileName)
{
  PII_D;
  delete d->pTemplate;
  d->strTemplateFile = fileName;
  d->pTemplate = PiiImageFileReader::readGrayImage(fileName)->toMatrixPointer();
}

void PiiTemplateMatcher::setMaskFile(const QString& maskFile)
{
  PII_D;
  delete d->pMask;
  d->strMaskFile = maskFile;
  d->pMask = PiiImageFileReader::readGrayImage(maskFile)->toMatrixPointer();
}

void PiiTemplateMatcher::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_GRAY_IMAGE_CASES(match, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiTemplateMatcher::match(const PiiVariant& obj)
{
  // cvMatchTemplate only supports 8-bit ints and 32-bit floats
  if (obj.type() == PiiYdin::UnsignedCharMatrixType ||
      obj.type() == PiiYdin::FloatMatrixType)
    match(obj.valueAs<PiiMatrix<T> >());
  // All other types are converted to floats
  else
    match(PiiMatrix<float>(obj.valueAs<PiiMatrix<T> >()));
}

template <class T> void PiiTemplateMatcher::match(const PiiMatrix<T>& image)
{
  PII_D;
  // Cannot match if template is larger than input image
  if (!d->pTemplate ||
      image.rows() < d->pTemplate->rows() ||
      image.columns() < d->pTemplate->columns())
    {
      emitObject(PiiMatrix<float>());
      outputAt(1)->emitObject(PiiMatrix<bool>());
      return;
    }
  
  PiiMatrix<float> matResult(image.rows() - d->pTemplate->rows() + 1,
                             image.columns() - d->pTemplate->columns() + 1);

  // Cast template image to the correct type
  const PiiMatrix<T> matTmpl(*d->pTemplate); // cast to the correct type

  cvMatchTemplate(PiiOpenCv::iplImage(image),
                  PiiOpenCv::iplImage(matTmpl),
                  PiiOpenCv::iplImage(matResult),
                  d->iMatchingMethod);

  emitObject(matResult);

  if (outputAt(1)->isConnected())
    {
      PiiMatrix<float> peaks(PiiImage::zeroBelow(matResult, (float)d->dThreshold));
      int labelCnt = 0;
      PiiMatrix<int> labels(PiiImage::labelImage(peaks, &labelCnt));
      PiiMatrix<bool> outputMask(image.rows(), image.columns());
      PiiMatrix<int> areas, centroids, bbox;
      PiiImage::calculateProperties(labels, labelCnt, areas, centroids, bbox);
      for (int l = labelCnt; l--; )
        {
          PiiMatrix<float> window(peaks(bbox(l,1), bbox(l,0), bbox(l,3), bbox(l,2)));
          int r = 0, c = 0;
          Pii::maxAll(window, &r, &c);
          r += bbox(l,1);
          c += bbox(l,0);
          if (d->pMask == 0 || d->pMask->rows() != d->pTemplate->rows() || d->pMask->columns() != d->pTemplate->columns())
            outputMask(r,c) = true;
          else
            outputMask(r, c, d->pMask->rows(), d->pMask->columns()) << *((PiiMatrix<bool>*)d->pMask);
        }
      
      //qDebug("Maximum correlation value: %f", Pii::maxAll(*result));
      outputAt(1)->emitObject(outputMask);
    }
}

PiiTemplateMatcher::MatchingMethod PiiTemplateMatcher::matchingMethod() const { return (MatchingMethod)_d()->iMatchingMethod; }
QString PiiTemplateMatcher::templateFile() const { return _d()->strTemplateFile; }
QString PiiTemplateMatcher::maskFile() const { return _d()->strMaskFile; }
void PiiTemplateMatcher::setThreshold(double threshold) { _d()->dThreshold = threshold; }
double PiiTemplateMatcher::threshold() const { return _d()->dThreshold; }
