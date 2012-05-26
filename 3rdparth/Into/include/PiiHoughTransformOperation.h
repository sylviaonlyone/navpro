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

#ifndef _PIIHOUGHTRANSFORMOPERATION_H
#define _PIIHOUGHTRANSFORMOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiYdinTypes.h>
#include <PiiVariant.h>
#include "PiiTransformsGlobal.h"

/** 
 * Operation interface for Hough transform. Hough transform is a 
 * technique which can be used to find imperfect instances of
 * objects within a certain class of shapes by a voting procedure.
 *
 * This voting procedure is carried out in a parameter space,
 * from which object candidates are obtained as local maxima in a
 * so-called accumulator space that is explicitly constructed by the
 * choosed algorithm (for computing the Hough transform).
 *
 * Classical Hough transform is concerned with the identification of
 * lines in the image, but Hough transform has been extended
 * to identifying positions of arbitrary shapes, most commonly circles
 * or ellipses.
 *
 * @note At the moment only linear Hough transform is implemented.
 * 
 * @inputs
 *
 * @in image - an image to be transformed. Typically, the input
 * image is binary, but any gray-level image can be used. All non-zero
 * values in the input image will add to the transform. Higher values
 * have higher weight.
 *
 * @outputs
 *
 * @out accumulator - the accumulator array. See @ref
 * PiiTransforms::linearHough() for detailed information.
 *
 * @out peaks - detected peaks in the transformation domain. The
 * output value is a PiiMatrix<double> with at most #peakCount rows. 
 * Each row stores the location of a peak @f$(d, \theta)@f$, where @e
 * d is the distance to the center of the image and @f$\theta@f$ is
 * the slope of the line. If there are less than #peakCount peaks, the
 * number of rows in the matrix matches the number of peaks found. The
 * rows of the matrix are ordered so that the highest peak comes first.
 * 
 * @out coordinates - a PiiMatrix<double> containing the locations of
 * located geometric objects in the input image. #peakCount highest
 * peaks in the transformation domain are considered. With the linear
 * Hough transform, the matric has 4 columns containing the begin and
 * end points of a detected line (x1, y1, x2, y2). This ouput can be
 * directly connected to the @p property input of @ref
 * PiiImageAnnotator.
 *
 * @ingroup PiiTransformsPlugin
 */
class PII_TRANSFORMS_EXPORT PiiHoughTransformOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The transformType defines which kind Hough transform is done to input
   * image. Default type is Linear.
   */
  Q_PROPERTY(TransformType type READ transform WRITE setTransform);
  Q_ENUMS(TransformType);
    
  /**
   * The angleResolution defines number of degrees each column
   * represents. The default value is 1.0 which produces 180 columns
   * in the result matrix.
   */
  Q_PROPERTY(double angleResolution READ angleResolution WRITE setAngleResolution);
  
  /**
   * The distanceResolution defines distance (in number of pixels)
   * what each row represents in the result matrix. Default value is 1.0
   */
  Q_PROPERTY(double distanceResolution READ distanceResolution WRITE setDistanceResolution);
  
  /**
   * The number of highest peaks detected in the accumulator.
   */
  Q_PROPERTY(int peakCount READ peakCount WRITE setPeakCount);

  /**
   * startAngle description
   */
  Q_PROPERTY(int startAngle READ startAngle WRITE setStartAngle);

  /**
   * endAngle description
   */
  Q_PROPERTY(int endAngle READ endAngle WRITE setEndAngle);

  /**
   * startDistance description
   */
  Q_PROPERTY(int startDistance READ startDistance WRITE setStartDistance);

  /**
   * endDistance description
   */
  Q_PROPERTY(int endDistance READ endDistance WRITE setEndDistance);

  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Supported Hough transform types. 
   *
   * @lip Linear - a linear Hough transform (searches lines from
   * in input image).
   *
   * @lib Circle - a circle Hough transform (searches circles in input
   * image).
   *
   * @note Circle is not yet implemented.
   */
  enum TransformType { Linear, Circle};

  PiiHoughTransformOperation();
  virtual ~PiiHoughTransformOperation();

  void check(bool reset);

  void setTransform(TransformType type);
  TransformType transform() const;

    
  void setAngleResolution(double resolution);
  double angleResolution() const;


  void setDistanceResolution(double resolution);
  double distanceResolution() const;
  
  void setPeakCount(int value);
  int peakCount() const;
  void setStartAngle(int startAngle);
  int startAngle() const;
  void setEndAngle(int endAngle);
  int endAngle() const;
  void setStartDistance(int startDistance);
  int startDistance() const;
  void setEndDistance(int endDistance);
  int endDistance() const;
  
protected:
  void process();


private:
  template <class T> void doHough(const PiiVariant& obj);
  template <class T> void findPeaks(const PiiMatrix<T>& objects);

  /**
   * Finds from line begin and end point and saves them into param
   * coordinates.
   *
   * @param coordinates is matrix where line begin and end point are
   * saved.
   *
   * @param d is distance from image center.
   *
   * @param angle is line angle.
   */
  void savePeak(PiiMatrix<double>& peaks, PiiMatrix<double>& coordinates, int d, int angle);

  template <class T> struct TransformTraits;

  template <class T> struct LineObject
  {
    LineObject(int r=0, int c=0, T val=0) : value(val), row(r), col(c) {}
    
    T value;
    int row;
    int col;
    
    bool operator<(const LineObject& ob) const { return value < ob.value; }
    bool operator>(const LineObject& ob) const { return value > ob.value; }
  };
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    TransformType type;
    
    double dAngleResolution;
    double dDistanceResolution;
    int iPeakCount;
    bool bPeaksConnected;
    
    int iRows;
    int iCols;
    int iStartAngle;
    int iEndAngle;
    int iStartDistance;
    int iEndDistance;
  };
  PII_D_FUNC;
};

template <class T> struct PiiHoughTransformOperation::TransformTraits { typedef int Type; };
template <> struct PiiHoughTransformOperation::TransformTraits<float> { typedef float Type; };

#endif //_PIIHOUGHTRANSFORMOPERATION_H
