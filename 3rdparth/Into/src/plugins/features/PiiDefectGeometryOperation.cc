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

#include "PiiDefectGeometryOperation.h"

#include <PiiYdinTypes.h>
#include <PiiLabeling.h>
#include <PiiObjectProperty.h>
#include <PiiThresholding.h>
#include <PiiImageTraits.h>
#include <PiiImage.h>
#include <functional>

PiiDefectGeometryOperation::Data::Data() :
  defectType(DarkDefect), detectionType(GrayLevelDetection), bMaskConnected(false),
  dGradientThreshold(10.0), dLevelThreshold(0.0), bDiscardExtrema(true)
{
}

PiiDefectGeometryOperation::PiiDefectGeometryOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("features"));
  addSocket(new PiiOutputSocket("mask"));
}

void PiiDefectGeometryOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);
  
  d->bMaskConnected = outputAt(1)->isConnected();
}

void PiiDefectGeometryOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_GRAY_IMAGE_CASES(calculateGeometry, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}


template <class T> void PiiDefectGeometryOperation::calculateGeometry(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> img = obj.valueAs<PiiMatrix<T> >();
  PiiMatrix<int> matDetected;

  switch (d->detectionType)
    {
    case GrayLevelDetection:
      matDetected = detectGrayLevel(img, grayLevelThreshold(img, 0.4));
      break;
    case EdgeDetection:
      matDetected = detectEdge(img);
      break;
    case LevelAndEdgeDetection:
      matDetected = detectLevelAndEdge(img);
      break;
    }

   // Calculate defect properties
  PiiMatrix<int> matAreas, matCentroids, matBoxes;
  PiiImage::calculateProperties(matDetected,
                                 0,
                                 matAreas,
                                 matCentroids,
                                 matBoxes);

  if (matBoxes.rows() < 1)
    emitObject(PiiMatrix<float>(1,4));
  else
    {
      double dWidth = matBoxes(0,2);
      double dHeight = matBoxes(0,3);
      double dArea = matAreas(0,0);
      
      PiiMatrix<float> matFeatures(1,4,
                                   dWidth,
                                   dHeight,
                                   dArea,
                                   dWidth/dHeight);
      emitObject(matFeatures);
    }

  outputAt(1)->emitObject(matDetected);
}  
    
template <class T> PiiMatrix<int> PiiDefectGeometryOperation::detectGrayLevel(const PiiMatrix<T>& img, float threshold)
{
  PII_D;
  if (d->defectType == DarkDefect)
    return img.mapped(PiiImage::InverseThresholdFunction<T,int>(), T(threshold));
  else
    return img.mapped(PiiImage::ThresholdFunction<T,int>(), T(threshold));
}

/*
 * Calculate gray level threshold. If meanRatio is 1, the automatic
 * threshold will be equal to the mean value. If it is 0, the
 * threshold will equal to the minimum/maximum gray level of the
 * image. The calculated mean and extremum value will be stored into
 * the return-value parameters.
 */
template <class T> float PiiDefectGeometryOperation::grayLevelThreshold(const PiiMatrix<T>& img,
                                                                        float meanRatio,
                                                                        float* mean, T* extremum)
{
  PII_D;
  if (d->dLevelThreshold != 0)
    return float(d->dLevelThreshold);

  float fMean = 0;

  if (d->bDiscardExtrema)
    {
      if (d->defectType == DarkDefect)
        // Calculate the mean of all non-maximum pixels
        fMean = Pii::forEachIf(img.begin(),img.end(),
                               std::bind2nd(std::less<T>(), PiiImage::Traits<T>::max()),
                               Pii::Mean<T,float>()).mean();
      else
        // Calculate the mean of all non-minimum pixels
        fMean = Pii::forEachIf(img.begin(),img.end(),
                               std::bind2nd(std::greater<T>(), 0),
                               Pii::Mean<T,float>()).mean();
    }
  else
    // Calculate the mean of all pixels
    fMean = Pii::meanAll<float>(img);

  T extr = d->defectType == DarkDefect ? Pii::minAll(img) : Pii::maxAll(img);
  if (extremum != 0)
    *extremum = extr;
  if (mean != 0)
    *mean = fMean;

  return meanRatio * fMean + (1.0 - meanRatio) * extr;
}

template <class T> void PiiDefectGeometryOperation::edgeThresholds(const PiiMatrix<T>& vertDiff,
                                                                   const PiiMatrix<T>& horzDiff,
                                                                   float& vertThreshold,
                                                                   float& horzThreshold)
{
  PII_D;
  float fVertStd = Pii::stdAll<float>(vertDiff);
  float fHorzStd = Pii::stdAll<float>(horzDiff);

  if (fVertStd == fHorzStd || // traps divzero
      qMin(fVertStd, fHorzStd) / qMax(fVertStd, fHorzStd) > 0.5) // ratio is less than 1/2
    {
      if (d->dGradientThreshold != 0)
        vertThreshold = horzThreshold = float(d->dGradientThreshold);
      else
        {
          vertThreshold = qMin(6*fVertStd, 0.35f*Pii::maxAll(vertDiff));
          horzThreshold = qMin(6*fHorzStd, 0.35f*Pii::maxAll(horzDiff));
        }
    }
  else if (fVertStd < fHorzStd)
    {
      if (d->dGradientThreshold != 0)
        vertThreshold = float(d->dGradientThreshold);
      else
        vertThreshold = qMin(6*fVertStd, 0.35f*Pii::maxAll(vertDiff));
      horzThreshold = 0;
    }
  else
    {
      vertThreshold = 0;
      if (d->dGradientThreshold != 0)
        horzThreshold = d->dGradientThreshold;
      else
        horzThreshold = qMin(6*fHorzStd, 0.35f*Pii::maxAll(horzDiff));
    }
}

template <class T> PiiMatrix<int> PiiDefectGeometryOperation::detectEdge(const PiiMatrix<T>& img)
{
  typedef typename DiffTraits<T>::Type DiffType;
  PiiMatrix<DiffType> matVertDiff(Pii::diff(PiiMatrix<DiffType>(img),1,1,Pii::Vertically));
  PiiMatrix<DiffType> matHorzDiff(Pii::diff(PiiMatrix<DiffType>(img)));
  float fVertThreshold, fHorzThreshold;

  edgeThresholds(matVertDiff, matHorzDiff, fVertThreshold, fHorzThreshold);

  return detectEdge(matVertDiff, fVertThreshold, matHorzDiff, fHorzThreshold);
}

template <class T> PiiMatrix<int> PiiDefectGeometryOperation::detectEdge(const PiiMatrix<T>& vertDiff,
                                                                         float vertThreshold,
                                                                         const PiiMatrix<T>& horzDiff,
                                                                         float horzThreshold)
{
  // There is no clear difference in horizontal and vertical
  // gradients -> use both
  if (vertThreshold != 0 && horzThreshold != 0)
    {
      // Defect area is framed in both directions
      return
        PiiMatrix<int>(frameDefect(vertDiff,
                                   vertThreshold,
                                   Pii::Vertically) ||
                       frameDefect(horzDiff,
                                   horzThreshold,
                                   Pii::Horizontally));
    }

  // The image is smoother vertically -> easier to detect
  // horizontal defect boundaries (vertical gradient)
  else if (vertThreshold != 0)
    return frameDefect(vertDiff,
                       vertThreshold,
                       Pii::Vertically);
  // Image is smoother horizontally -> look for vertical edges
  else
    return frameDefect(horzDiff,
                       horzThreshold,
                       Pii::Horizontally);
}


// Creates a mask matrix that has ones between defect edges found in diff.
template <class T> PiiMatrix<int> PiiDefectGeometryOperation::frameDefect(const PiiMatrix<T>& diff,
                                                                          float threshold,
                                                                          Pii::MatrixDirection dimension)
{
  PiiMatrix<int> mask;
  if (dimension == Pii::Vertically)
    mask.resize(diff.rows()+1, diff.columns());
  else
    mask.resize(diff.rows(), diff.columns()+1);
  
  mask = 0;

  //qDebug("Detection threshold: %f", float(threshold));

  // Detect horizontal changes on each row
  if (dimension == Pii::Horizontally)
    {
      for (int r=0; r<diff.rows(); ++r)
        frameDefect(diff.rowBegin(r), diff.rowEnd(r), mask.rowBegin(r), threshold);

      // Fill in one-pixel gaps vertically
      if (mask.rows() > 2)
        for (int c=0; c<mask.columns(); ++c)
          fillGaps(mask.columnBegin(c), mask.columnEnd(c));
    }
  // Detect vertical changes on each column
  else
    {
      for (int c=0; c<diff.columns(); ++c)
        frameDefect(diff.columnBegin(c), diff.columnEnd(c), mask.columnBegin(c), threshold);

      // Fill in one-pixel gaps horizontally
      if (mask.columns() > 2)
        for (int r=0; r<mask.rows(); ++r)
          fillGaps(mask.rowBegin(r), mask.rowEnd(r));
    }
  
  return mask;
}

// Frame a defect in one row/column of a matrix
template <class InputIterator, class OutputIterator, class T>
void PiiDefectGeometryOperation::frameDefect(InputIterator start,
                                             InputIterator end,
                                             OutputIterator outputStart,
                                             T threshold)
{
  PII_D;
  OutputIterator defectStart = outputStart;
  OutputIterator currentOut = outputStart;
  enum { OnBackground, OnDefect, OnBoundary } state = OnBackground;
      
  if (d->defectType == DarkDefect)
    {
      for (InputIterator current = start; current != end; ++current, ++currentOut)
        {
          switch (state)
            {
            case OnBackground:
              // Rapid change from bright to dark
              if (*current < -threshold)
                {
                  // We are on a defect now
                  state = OnDefect;
                  // Mark the defect start point
                  defectStart = currentOut;
                }
              break;
            case OnDefect:
              // Rapid change back to bright
              if (*current > threshold)
                {
                  // Defect boundary starts, but may continue
                  state = OnBoundary;
                  // Mark the defect area so far
                  for (; defectStart != currentOut; ++defectStart) *defectStart = 1;
                }
              break;
            case OnBoundary:
              // Mark the boundary as a defect
              *currentOut = 1;
              // The change is not steep enough any more...
              if (*current < threshold) state = OnBackground;
            }
        }
    }
  else
    {
      // Almost cut 'n' pasted from above.
      for (InputIterator current = start; current != end; ++current, ++currentOut)
        {
          switch (state)
            {
            case OnBackground:
              // Rapid change from dark to bright
              if (*current > threshold)
                {
                  // We are on a defect now
                  state = OnDefect;
                  // Mark the defect start point
                  defectStart = currentOut;
                }
              break;
            case OnDefect:
              // Rapid change back to dark
              if (*current < -threshold)
                {
                  // Defect boundary starts, but may continue
                  state = OnBoundary;
                  // Mark the defect area so far
                  for (; defectStart != currentOut; ++defectStart) *defectStart = 1;
                }
              break;
            case OnBoundary:
              // Mark the boundary as a defect
              *currentOut = 1;
              // The change is not steep enough any more...
              if (*current > -threshold) state = OnBackground;
            }
        }
    }
}

template <class Iterator>
void PiiDefectGeometryOperation::fillGaps(Iterator start,
                                          Iterator end)
{
  Iterator prev = start;
  Iterator next = start+2;
  --end;

  for (; next != end; ++prev, ++next)
    // If next and previous are ones, the pixel between them is also
    // one.
    if (*prev == 1 && *next == 1)
      *(prev+1) = 1;
}

/*
 * First frame defect by both gradient and gray level using strict
 * thresholds. Combine the results (logical and), find connected
 * components and clean too small detections. Reapply the detection
 * with slightly looser thresholds and detect connected components. 
 * Accept only those connected components in which there exists at
 * least one pixel that meets the stricter rule.
 */
template <class T> PiiMatrix<int> PiiDefectGeometryOperation::detectLevelAndEdge(const PiiMatrix<T>& img)
{
  // Calculate horizontal and vertical difference images
  typedef typename DiffTraits<T>::Type DiffType;
  PiiMatrix<DiffType> matVertDiff(Pii::diff(PiiMatrix<DiffType>(img),1,1,Pii::Vertically));
  PiiMatrix<DiffType> matHorzDiff(Pii::diff(PiiMatrix<DiffType>(img)));

  // Calculate edge thresholds (strict and loose)
  float fVertThreshold1 = 0, fHorzThreshold1 = 0;
  edgeThresholds(matVertDiff, matHorzDiff, fVertThreshold1, fHorzThreshold1);
  float fVertThreshold2 = 0.7 * fVertThreshold1, fHorzThreshold2 = 0.7 * fHorzThreshold1;

  // Calculate gray level thresholds (strict and loose)
  float fGrayMean = 0;
  T grayExtremum = 0;  
  float grayThreshold1 = grayLevelThreshold(img, 0.3, &fGrayMean, &grayExtremum);
  float grayThreshold2 = 0.5 * fGrayMean + 0.5 * grayExtremum;

  int iDefectSeeds = 0, iDefectCandidates = 0;

  // Find possible defects with strict and loose parameters
  PiiMatrix<int> strictDefects = PiiImage::labelImage(detectGrayLevel(img, grayThreshold1) &&
                                                      detectEdge(matVertDiff, fVertThreshold1, matHorzDiff, fHorzThreshold1),
                                                      &iDefectSeeds);
  PiiMatrix<int> looseDefects = PiiImage::labelImage(detectGrayLevel(img, grayThreshold2) &&
                                                     detectEdge(matVertDiff, fVertThreshold2, matHorzDiff, fHorzThreshold2),
                                                     &iDefectCandidates);

  QVector<bool> vecLargeEnough(iDefectSeeds,false);
  QVector<bool> vecRetainedLabels(iDefectCandidates+1, 0);

  // Calculate properties for the defects detected with strict
  // parameters.
  PiiMatrix<int> areas, centroids, bbox;
  PiiImage::calculateProperties(strictDefects, iDefectSeeds,
                                 areas,centroids,bbox);
  // Retain only those detections that are large enough (wider or
  // higher than 1 pixel)
  for (int i=0; i<iDefectSeeds; ++i)
    if (bbox(i,2) > 1 && bbox(i,3) > 1)
      vecLargeEnough[i] = true;

  // Retain only those labels in looseDefects in which there exists at
  // least one pixel in a large enough connected component in
  // looseDefects.
  for (int r=looseDefects.rows(); r--; )
    {
      const int* pRow = strictDefects[r];
      for (int c=looseDefects.columns(); c--; )
        if (pRow[c] > 0 && vecLargeEnough[pRow[c]-1])
          vecRetainedLabels[looseDefects(r,c)] = true;
    }

  // Convert all retained labels to ones and discarded ones to zero.
  for (int r=looseDefects.rows(); r--; )
    {
      int* pRow = looseDefects[r];
      for (int c=looseDefects.columns(); c--; )
        if (pRow[c] > 0)
          pRow[c] = vecRetainedLabels[pRow[c]];
    }

  // Phew...
  return looseDefects;
}

void PiiDefectGeometryOperation::setDefectType(DefectType defectType) { _d()->defectType = defectType; }
PiiDefectGeometryOperation::DefectType PiiDefectGeometryOperation::defectType() const { return _d()->defectType; }
void PiiDefectGeometryOperation::setDetectionType(DetectionType detectionType) { _d()->detectionType = detectionType; }
PiiDefectGeometryOperation::DetectionType PiiDefectGeometryOperation::detectionType() const { return _d()->detectionType; }
void PiiDefectGeometryOperation::setGradientThreshold(double gradientThreshold) { _d()->dGradientThreshold = gradientThreshold; }
double PiiDefectGeometryOperation::gradientThreshold() const { return _d()->dGradientThreshold; }
void PiiDefectGeometryOperation::setLevelThreshold(double levelThreshold) { _d()->dLevelThreshold = levelThreshold; }
double PiiDefectGeometryOperation::levelThreshold() const { return _d()->dLevelThreshold; }
void PiiDefectGeometryOperation::setDiscardExtrema(bool discardExtrema) { _d()->bDiscardExtrema = discardExtrema; }
bool PiiDefectGeometryOperation::discardExtrema() const { return _d()->bDiscardExtrema; }
QStringList PiiDefectGeometryOperation::features() const
{
  return QStringList()
    << "Width"
    << "Height"
    << "Area"
    << "AspectRatio";
}
