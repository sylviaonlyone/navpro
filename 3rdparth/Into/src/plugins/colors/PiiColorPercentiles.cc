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

#include "PiiColorPercentiles.h"
#include "PiiColor.h"
#include "PiiImage.h"
#include "PiiHistogram.h"
#include <PiiMatrix.h>
#include <PiiYdinTypes.h>
#include <PiiRoi.h>
#include <QStringList>

template <class T> struct PiiColorPercentiles::GrayPercentiles :
  PiiImage::GrayHistogramHandler<T>
{
  GrayPercentiles(PiiColorPercentiles::Data* data) : d(data) {}
  PiiMatrix<int> calculate();
  PiiColorPercentiles::Data* d;
};

template <class T> struct PiiColorPercentiles::ColorPercentiles :
  PiiImage::ColorHistogramHandler<T>
{
  ColorPercentiles(PiiColorPercentiles::Data* data) : d(data) {}
  PiiMatrix<int> calculate();
  PiiColorPercentiles::Data* d;
};

template <class T> struct PiiColorPercentiles::Percentiles :
  Pii::IfClass<Pii::IsPrimitive<T>,
               GrayPercentiles<T>,
               ColorPercentiles<T> >::Type
{
  typedef typename Pii::IfClass<Pii::IsPrimitive<T>,
                                GrayPercentiles<T>,
                                ColorPercentiles<T> >::Type SuperType;
  Percentiles(PiiColorPercentiles* parent) : SuperType(parent->_d()) {}
};


PiiColorPercentiles::Data::Data() :
  featureSetType(Universal),
  universalFeatureSet(Difference10),
  prebuiltFeatureSet(WoodDefectDetection),
  iLevels(256),
  roiType(PiiImage::AutoRoi),
  pPercentiles(0),
  uiPreviousType(PiiVariant::InvalidType)
{
}

PiiColorPercentiles::PiiColorPercentiles() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  addSocket(new PiiInputSocket("image"));
  addSocket(d->pRoiInput = new PiiInputSocket("roi"));
  d->pRoiInput->setOptional(true);

  addSocket(new PiiOutputSocket("features"));

  setProtectionLevel("percentiles", WriteWhenStoppedOrPaused);
  setProtectionLevel("featureSetType", WriteWhenStoppedOrPaused);
  setProtectionLevel("universalFeatureSet", WriteWhenStoppedOrPaused);
  setProtectionLevel("prebuiltFeatureSet", WriteWhenStoppedOrPaused);
  setProtectionLevel("levels", WriteWhenStoppedOrPaused);
}

PiiColorPercentiles::~PiiColorPercentiles()
{
}

void PiiColorPercentiles::usePercentiles(const QStringList& percentiles)
{
  PII_D;
  
  d->lstPercentileNames = percentiles;
  d->lstPercentiles.clear();
  d->lstDiffs.clear();
  for (int i=0; i<percentiles.size(); i++)
    {
      // Separate a possible difference
      QStringList defs = percentiles[i].split('-');
      if (defs.size() == 1)
        {
          // This is an absolute percentile -> add -1,-1 to diffs
          d->lstPercentiles << createDef(defs[0]);
          d->lstDiffs << qMakePair(-1,-1);
        }
      else if (defs.size() == 2)
        {
          // This is a diff
          d->lstPercentiles << createDef(defs[0]);
          d->lstDiffs << createDef(defs[1]);
        }
      else
        continue;
    }
}

QPair<int,int> PiiColorPercentiles::createDef(QString str)
{
  // Color channel and percentile value are separated by a colon
  QStringList parts = str.split(':');
  if (parts.size() != 2)
    return qMakePair(0,0);

  // We recognize r, g, and b as color channel names
  int channel = 0;
  if (parts[0].toLower() == "r")
    channel = 2;
  else if (parts[0].toLower() == "g")
    channel = 1;
  else if (parts[0].toLower() == "b")
    channel = 0;
  else
    {
      channel = parts[0].toInt();
      if (channel < 0 || channel > 2)
        channel = 0;
    }

  int percentile = parts[1].toInt();
  if (percentile > 100 || percentile < 0)
    percentile = 0;

  // We scale the percentiles to 0-256 so that they can be quickly
  // extracted at run-time (a division is saved)
  percentile = percentile * 256 / 100;

  // Each pair stores the channel and the percentile value
  return qMakePair(channel, percentile);
}
  
void PiiColorPercentiles::check(bool reset)
{
  PII_D;
  
  PiiDefaultOperation::check(reset);

  switch(d->featureSetType)
    {
    case Universal:
      useUniversalFeatureSet(d->universalFeatureSet);
      break;
    case Prebuilt:
      usePrebuiltFeatureSet(d->prebuiltFeatureSet);
      break;
    case Custom:
    default:
      usePercentiles(d->lstPercentileNames);
      break;
    }

  // Ensure we always have at least one feature
  if (d->lstPercentiles.size() == 0)
    PII_THROW(PiiExecutionException, tr("Percentiles have not been set."));
}

void PiiColorPercentiles::aboutToChangeState(State state)
{
  PII_D;
  if (state == Stopped)
    {
      delete d->pPercentiles;
      d->pPercentiles = 0;
      d->uiPreviousType = PiiVariant::InvalidType;
    }
}

void PiiColorPercentiles::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_INT_GRAY_IMAGE_CASES(percentiles, obj);
      PII_INT_COLOR_IMAGE_CASES(percentiles, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }  
}

template <class T> void PiiColorPercentiles::percentiles(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T>& image = obj.valueAs<PiiMatrix<T> >();
  
  if (d->uiPreviousType != Pii::typeId<T>())
    {
      delete d->pPercentiles;
      d->pPercentiles = new Percentiles<T>(this);
      d->uiPreviousType = Pii::typeId<T>();
    }

  Percentiles<T>& percentiles = *static_cast<Percentiles<T>*>(d->pPercentiles);
  percentiles.initialize(d->iLevels, false);
  
  PiiImage::handleRoiInput(d->pRoiInput, d->roiType, image, percentiles);

  emitObject(percentiles.calculate());
}

template <class T> PiiMatrix<int> PiiColorPercentiles::GrayPercentiles<T>::calculate()
{
  // Cumulative distribution of histogram
  PiiMatrix<int> cumulativeHist(PiiImage::cumulative<int>(this->varHistogram.template valueAs<PiiMatrix<int> >()));
  
  PiiMatrix<int> matResult(PiiMatrix<int>::uninitialized(1,d->lstPercentiles.size()));
  int *pResult = matResult[0];
  
  // Extract percentiles from the cumulative histogram
  for (int i=0; i<d->lstPercentiles.size(); ++i)
    {
      // Take first the absolute percentile
      // Color channel is not used here
      // FIXME This overflows with images larger than about 10 Mpx
      int p = PiiImage::percentile(cumulativeHist, (d->lstPercentiles[i].second * this->iPixelCount) >> 8);
      // Subtract the other one, if it is given
      if (d->lstDiffs[i].second >= 0)
        p -= PiiImage::percentile(cumulativeHist, (d->lstDiffs[i].second * this->iPixelCount) >> 8);
      
      pResult[i] = p;
    }
  return matResult;
}

template <class ColorType> PiiMatrix<int> PiiColorPercentiles::ColorPercentiles<ColorType>::calculate()
{
  // Cumulative histograms for each color channel
  PiiMatrix<int> channelHistograms[3];
  for (int i=0; i<3; ++i)
    channelHistograms[i] = PiiImage::cumulative<int>(this->varHistograms[i].template valueAs<PiiMatrix<int> >());

  PiiMatrix<int> matResult(PiiMatrix<int>::uninitialized(1, d->lstPercentiles.size()));
  int *pResult = matResult[0];
  
  for (int i=0; i<d->lstPercentiles.size(); ++i)
    {
      // Take the percentile from the correct channel histogram
      int p = PiiImage::percentile(channelHistograms[d->lstPercentiles[i].first],
                                   (d->lstPercentiles[i].second * this->iPixelCount) >> 8);
      
      // Subtract the other one, if it is given
      if (d->lstDiffs[i].second >= 0)
        p -= PiiImage::percentile(channelHistograms[d->lstDiffs[i].first],
                                  (d->lstDiffs[i].second * this->iPixelCount) >> 8);
      
      pResult[i] = p;
    }
  return matResult;
}

void PiiColorPercentiles::usePrebuiltFeatureSet(PrebuiltFeatureSet set)
{
  switch (set)
    {
    case WoodDefectDetection:
      usePercentiles(QStringList()
                     << "R:3"
                     << "G:80-B:80"
                     << "R:99-G:99"
                     << "G:1"
                     << "R:80-B:5"
                     << "R:90-B:90"
                     << "R:90-G:90"
                     << "R:10"
                     << "G:5-B:5"
                     << "G:90-B:90"
                     << "G:1-B:1"
                     << "R:85"
                     << "G:99-B:99");
      break;
    case WoodDefectRecognition:
      usePercentiles(QStringList()
                     << "R:90-B:90"
                     << "G:1-B:1"
                     << "B:5"
                     << "G:10-B:10"
                     << "G:90-B:90"
                     << "R:50-G:50"
                     << "B:90"
                     << "G:50-B:50"
                     << "R:99-G:99"
                     << "R:1-G:1"
                     << "R:0-G:0"
                     << "G:5-B:5"
                     << "G:20"
                     << "G:80-B:80"
                     << "B:80");
      break;

    default:
      break;
    }
}

void PiiColorPercentiles::useUniversalFeatureSet(UniversalFeatureSet set)
{
  switch(set)
    {
    case Absolute10:
      usePercentiles(QStringList()
                     << "R:10" << "R:20" << "R:30" << "R:40" << "R:50" << "R:60" << "R:70" << "R:80" << "R:90"
                     << "G:10" << "G:20" << "G:30" << "G:40" << "G:50" << "G:60" << "G:70" << "G:80" << "G:90"
                     << "B:10" << "B:20" << "B:30" << "B:40" << "B:50" << "B:60" << "B:70" << "B:80" << "B:90");
      break;

    case Absolute20:
      usePercentiles(QStringList()
                     << "R:20" << "R:40" << "R:60" << "R:80"
                     << "G:20" << "G:40" << "G:60" << "G:80"
                     << "B:20" << "B:40" << "B:60" << "B:80");
      break;

    case Absolute30:
      usePercentiles(QStringList()
                     << "R:30" << "R:60" << "R:90"
                     << "G:30" << "G:60" << "G:90"
                     << "B:30" << "B:60" << "B:90");
      break;

    case Difference10:
      usePercentiles(QStringList()
                     << "R:10-R:1" << "R:20-R:1" << "R:30-R:1" << "R:40-R:1" << "R:50-R:1" << "R:60-R:1" << "R:70-R:1" << "R:80-R:1" << "R:90-R:1"
                     << "G:10-G:1" << "G:20-G:1" << "G:30-G:1" << "G:40-G:1" << "G:50-G:1" << "G:60-G:1" << "G:70-G:1" << "G:80-G:1" << "G:90-G:1"
                     << "B:10-B:1" << "B:20-B:1" << "B:30-B:1" << "B:40-B:1" << "B:50-B:1" << "B:60-B:1" << "B:70-B:1" << "B:80-B:1" << "B:90-B:1");

    case Difference20:
      usePercentiles(QStringList()
                     << "R:20-R:10" << "R:40-R:10" << "R:60-R:10" << "R:80-R:10"
                     << "G:20-G:10" << "G:40-G:10" << "G:60-G:10" << "G:80-G:10"
                     << "B:20-B:10" << "B:40-B:10" << "B:60-B:10" << "B:80-B:10");
      break;

    case Difference30:
      usePercentiles(QStringList()
                     << "R:30-R:10" << "R:60-R:10" << "R:90-R:10"
                     << "G:30-G:10" << "G:60-G:10" << "G:90-G:10"
                     << "B:30-B:10" << "B:60-B:10" << "B:90-B:10");
      break;

    case Absolute10R:
      usePercentiles(QStringList() << "R:10" << "R:20" << "R:30" << "R:40" << "R:50" << "R:60" << "R:70" << "R:80" << "R:90");
      break;

    case Absolute20R:
      usePercentiles(QStringList() << "R:20" << "R:40" << "R:60" << "R:80");
      break;

    case Absolute30R:
      usePercentiles(QStringList() << "R:30" << "R:60" << "R:90");
      break;

    case Difference10R:
      usePercentiles(QStringList() << "R:10-R:1" << "R:20-R:1" << "R:30-R:1" << "R:40-R:1" << "R:50-R:1" << "R:60-R:1" << "R:70-R:1" << "R:80-R:1" << "R:90-R:1");

    case Difference20R:
      usePercentiles(QStringList() << "R:20-R:10" << "R:40-R:10" << "R:60-R:10" << "R:80-R:10");
      break;

    case Difference30R:
      usePercentiles(QStringList() << "R:30-R:10" << "R:60-R:10" << "R:90-R:10");
      break;

    case Absolute10G:
      usePercentiles(QStringList() << "G:10" << "G:20" << "G:30" << "G:40" << "G:50" << "G:60" << "G:70" << "G:80" << "G:90");
      break;

    case Absolute20G:
      usePercentiles(QStringList() << "G:20" << "G:40" << "G:60" << "G:80");
      break;

    case Absolute30G:
      usePercentiles(QStringList() << "G:30" << "G:60" << "G:90");
      break;

    case Difference10G:
      usePercentiles(QStringList() << "G:10-G:1" << "G:20-G:1" << "G:30-G:1" << "G:40-G:1" << "G:50-G:1" << "G:60-G:1" << "G:70-G:1" << "G:80-G:1" << "G:90-G:1");

    case Difference20G:
      usePercentiles(QStringList() << "G:20-G:10" << "G:40-G:10" << "G:60-G:10" << "G:80-G:10");
      break;

    case Difference30G:
      usePercentiles(QStringList() << "G:30-G:10" << "G:60-G:10" << "G:90-G:10");
      break;

    case Absolute10B:
      usePercentiles(QStringList() << "B:10" << "B:20" << "B:30" << "B:40" << "B:50" << "B:60" << "B:70" << "B:80" << "B:90");
      break;

    case Absolute20B:
      usePercentiles(QStringList() << "B:20" << "B:40" << "B:60" << "B:80");
      break;

    case Absolute30B:
      usePercentiles(QStringList() << "B:30" << "B:60" << "B:90");
      break;

    case Difference10B:
      usePercentiles(QStringList() << "B:10-B:1" << "B:20-B:1" << "B:30-B:1" << "B:40-B:1" << "B:50-B:1" << "B:60-B:1" << "B:70-B:1" << "B:80-B:1" << "B:90-B:1");

    case Difference20B:
      usePercentiles(QStringList() << "B:20-B:10" << "B:40-B:10" << "B:60-B:10" << "B:80-B:10");
      break;

    case Difference30B:
      usePercentiles(QStringList() << "B:30-B:10" << "B:60-B:10" << "B:90-B:10");
      break;

    default:
      break;
    }
}

QStringList PiiColorPercentiles::percentiles() const { return _d()->lstPercentileNames; }
void PiiColorPercentiles::setPercentiles(const QStringList& percentiles) { _d()->lstPercentileNames = percentiles; }

PiiColorPercentiles::FeatureSetType PiiColorPercentiles::featureSetType() const { return _d()->featureSetType; }
void PiiColorPercentiles::setFeatureSetType(FeatureSetType featureSetType) { _d()->featureSetType = featureSetType; }

PiiColorPercentiles::UniversalFeatureSet PiiColorPercentiles::universalFeatureSet() const { return _d()->universalFeatureSet; }
void PiiColorPercentiles::setUniversalFeatureSet(UniversalFeatureSet universalFeatureSet) { _d()->universalFeatureSet = universalFeatureSet; }

PiiColorPercentiles::PrebuiltFeatureSet PiiColorPercentiles::prebuiltFeatureSet() const { return _d()->prebuiltFeatureSet; }
void PiiColorPercentiles::setPrebuiltFeatureSet(PrebuiltFeatureSet prebuiltFeatureSet) { _d()->prebuiltFeatureSet = prebuiltFeatureSet; }

int PiiColorPercentiles::levels() const { return _d()->iLevels; }
void PiiColorPercentiles::setLevels(int levels) { _d()->iLevels = levels; }

void PiiColorPercentiles::setRoiType(PiiImage::RoiType roiType) { _d()->roiType = roiType; }
PiiImage::RoiType PiiColorPercentiles::roiType() const { return _d()->roiType; }
