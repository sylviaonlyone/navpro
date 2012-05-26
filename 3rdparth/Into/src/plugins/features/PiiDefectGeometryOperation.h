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

#ifndef _PIIDEFECTGEOMETRYOPERATION_H
#define _PIIDEFECTGEOMETRYOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>

/**
 * An operation that calculates the geometry of a defect area within a
 * detection window. The defect is first framed according to its gray
 * level or edges. Then, the geometric properties of the detection
 * area are calculated.
 *
 * @inputs
 *
 * @in image - input image. Any gray-level image.
 * 
 * @outputs
 *
 * @out features - a 1-by-4 PiiMatrix<float> that stores the width,
 * height, area, and aspect ratio of the detected defect area, in this
 * order. If no defect is detected, the values will be all zeros.
 *
 * @out mask - a binary mask image (PiiMatrix<int>) in which the
 * defect area is marked with ones. The size of this image equals that
 * of the input image.
 *
 * @ingroup PiiFeaturesPlugin
 */
class PiiDefectGeometryOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The type of defect to be detected. Default is @p DarkDefect.
   */
  Q_PROPERTY(DefectType defectType READ defectType WRITE setDefectType);
  Q_ENUMS(DefectType);

  /**
   * The detection type. Default is @p GrayLevelDetection.
   */
  Q_PROPERTY(DetectionType detectionType READ detectionType WRITE setDetectionType);
  Q_ENUMS(DetectionType);
  
  /**
   * An absolute threshold for the edge-based detection technique. The
   * default value is 10. Set to 0 to make the operation choose an
   * appropriate threshold value automatically.
   */
  Q_PROPERTY(double gradientThreshold READ gradientThreshold WRITE setGradientThreshold);

  /**
   * An absolute threshold for the gray level based detection
   * technique. The default value is zero, which causes the threshold
   * to be automatically selected.
   */
  Q_PROPERTY(double levelThreshold READ levelThreshold WRITE setLevelThreshold);
  
  /**
   * A flag that controls the automatic finding of a suitable gray
   * level threshold. If set to @p true (the default) pixels with the
   * maximum gray level (255 for integer types, 1.0 for floats) will
   * be ignored when looking for dark defects Similarly, pixels with
   * the minimum gray level (0 for all types) will be ignored when
   * looking for bright defects. Set to @p false to disable this
   * behavior.
   */
  Q_PROPERTY(bool discardExtrema READ discardExtrema WRITE setDiscardExtrema);

  /**
   * Feature names (read only).
   */
  Q_PROPERTY(QStringList features READ features);
  
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  /**
   * Defect types. The operation must know whether it is looking for
   * bright or dark defects to be able to reliably frame the defect
   * area.
   *
   * @lip DarkDefect - detected defects are darker than the background
   *
   * @lip BrightDefect - detected defects are brighter than the
   * background
   */
  enum DefectType { DarkDefect, BrightDefect };

  /**
   * Detection types. The defect area can be framed based on its gray
   * level or edges.
   *
   * @lip GrayLevelDetection - detect the defect area by its
   * darkness/brightness relative to the surroundings.
   *
   * @lip EdgeDetection - detect the defect area by its edges. The
   * detection algorithm automatically decides which gradient
   * direction is more reliable and uses either horizontal or vertical
   * gradient to frame the defect area. If neither is a clear winner,
   * both gradient directions will be used.
   *
   * @lip LevelAndEdgeDetection - detect defects using both absolute
   * gray levels and edges. The detection algorithm is based on
   * heuristics that won't stand objective judgement, but here's the
   * shameful truth: 1) Defect @e seeds are fist found by applying
   * both the @p GrayLevelDetection and the @p EdgeDetection schemes
   * and performing a logical @p AND operation on the results. 2)
   * Detections less than two pixel wide or high are removed from the
   * seeds. 3) The detection is repeated with looser thresholds to
   * find a larger set of defect @e candidates. 4) If a @e candidate
   * does not intersect with any of the @e seeds, it will be
   * discarded. The remaining @e candidates are the detections for
   * which the geometry will be calculated.
   */
  enum DetectionType { GrayLevelDetection, EdgeDetection, LevelAndEdgeDetection };
  
  PiiDefectGeometryOperation();

  void setDefectType(DefectType defectType);
  DefectType defectType() const;

  void setDetectionType(DetectionType detectionType);
  DetectionType detectionType() const;

  void setGradientThreshold(double gradientThreshold);
  double gradientThreshold() const;

  void setLevelThreshold(double levelThreshold);
  double levelThreshold() const;

  void setDiscardExtrema(bool discardExtrema);
  bool discardExtrema() const;

  QStringList features() const;
  
  void check(bool reset);
  
protected:
  void process();

private:
  template <class T> struct DiffTraits { typedef T Type; };
  template <class T> void calculateGeometry(const PiiVariant& obj);
  template <class T> PiiMatrix<int> detectGrayLevel(const PiiMatrix<T>& img, float threshold);
  template <class T> float grayLevelThreshold(const PiiMatrix<T>& img,
                                              float meanRatio,
                                              float* mean = 0, T* extremum = 0);
  template <class T> PiiMatrix<int> detectEdge(const PiiMatrix<T>& img);
  template <class T> PiiMatrix<int> detectEdge(const PiiMatrix<T>& vertDiff,
                                               float vertThreshold,
                                               const PiiMatrix<T>& horzDiff,
                                               float horzThreshold);
  template <class T> void edgeThresholds(const PiiMatrix<T>& vertDiff,
                                         const PiiMatrix<T>& horzDiff,
                                         float& vertThreshold,
                                         float& horzThreshold);
  template <class T> PiiMatrix<int> frameDefect(const PiiMatrix<T>& diff,
                                                float threshold,
                                                Pii::MatrixDirection dimension);
  template <class InputIterator, class OutputIterator, class T>
  void frameDefect(InputIterator start,
                   InputIterator end,
                   OutputIterator outputStart,
                   T threshold);
  template <class Iterator> void fillGaps(Iterator start, Iterator end);
  template <class T> PiiMatrix<int> detectLevelAndEdge(const PiiMatrix<T>& img);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    DefectType defectType;
    DetectionType detectionType;
    bool bMaskConnected;
    double dGradientThreshold;
    double dLevelThreshold;
    bool bDiscardExtrema;
  };
  PII_D_FUNC;
};

template <> struct PiiDefectGeometryOperation::DiffTraits<unsigned char> { typedef int Type; };
template <> struct PiiDefectGeometryOperation::DiffTraits<unsigned short> { typedef int Type; };
template <> struct PiiDefectGeometryOperation::DiffTraits<unsigned int> { typedef int Type; };

#endif //_PIIDEFECTGEOMETRYOPERATION_H
