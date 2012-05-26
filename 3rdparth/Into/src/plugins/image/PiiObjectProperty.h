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

#ifndef _PIIOBJECTPROPERTY_H
#define _PIIOBJECTPROPERTY_H

#include <PiiMatrix.h>
#include <PiiMath.h>

#include <PiiMatrixUtil.h>
#include <QDebug>

/**
 * @file
 * @ingroup PiiImagePlugin
 *
 * Functions for calculating the properties (features) of objects in
 * binary images.
 */

namespace PiiImage
{
  /**
   * Calculate areas, centroids and bounding boxes for labeled objects.
   *
   * @param mat labeled matrix
   *
   * @param labels number of labeled objects. Must equal to the
   * maximum value in @p mat. Set to zero if unknown.
   *
   * @param areas the number of pixels in each labeled object. A
   * N-by-1 matrix, where N is the number of labels.
   *
   * @param centroids center of mass for each labeled object. A N-by-2
   * matrix in which each row stores the column and row coordinates of
   * the object's centroid, in this order.
   *
   * @param bbox the bounding boxes of found objects. A N-by-4 matrix
   * in which each row represent the x (column), y (row), width and
   * height of a labeled object, in this order.
   */
  template <class T> void calculateProperties(const PiiMatrix<T>& mat, int labels, PiiMatrix<int>& areas,
                                              PiiMatrix<int>& centroids, PiiMatrix<int>& bbox);
  

  /**
   * Calculate the "direction" of an object in @p mat. This function
   * uses PCA to find the most prominent orientation of the object
   * marked with @p label in image.
   *
   * @param mat labeled image
   *
   * @param label the object to be inspected
   *
   * @param length an output parameter that will store the relative
   * length of the object
   *
   * @param width an output parameter that will store the relative
   * width of the object
   *
   * @param pixels an output parameter that will store the number of
   * pixels used for estimation
   *
   * @return a 2-by-2 matrix consisting of orthonormal row vectors. 
   * The first one is aligned to the most prominent direction. For
   * example (1,0) means right and (0,1) up. If there are less than
   * two pixels that match the label, the matrix will be [1 0; 0 1],
   * and length/width will be set to zero.
   */
  template <class T> PiiMatrix<double> calculateDirection(const PiiMatrix<T>& mat,
                                                          T label,
                                                          double* length = 0,
                                                          double* width = 0,
                                                          int* pixels = 0,
                                                          typename Pii::OnlyNumeric<T>::Type = 0);

  /**
   * @overload
   *
   * This function takes a decision function as a parameter.
   *
   * @code
   * PiiMatrix<int> matInput;
   * // All pixels with a gray level higher than 3 are foreground
   * PiiMatrix<double> matDir(Pii::calculateDirection(matInput,
   *                                                  std::bind2nd(std::greater<int>(), 3));
   * @endcode
   */
  template <class T, class UnaryFunction>
  PiiMatrix<double> calculateDirection(const PiiMatrix<T>& mat,
                                       UnaryFunction decisionRule,
                                       double* length = 0,
                                       double* width = 0,
                                       int* pixels = 0);

  /**
   * A Default struct which is used in line sweeper. Which does
   * nothing. Provided for convience.
   */
  template <class ResultType, class ImageType> struct DefaultSweepFunction
  {
    virtual ~DefaultSweepFunction() {}

    /**
     * Operator() is called in sweepLine -function for each sweeped
     * point.
     *
     * @param image is gray-level image.
     *
     * @param x is a x-point on line coordinate system.
     *
     * @param y is a y-point on line coordinate system.
     *
     * @param pixelX is a corresponding x-point on image coordinate
     * system.
     * 
     * @param pixelY is a corresponding y-point on image coordinate
     * system.
     *
     *
     */
    virtual void operator() (const PiiMatrix<ImageType>& image,
                             int x,
                             int y,
                             double pixelX,
                             double pixelY)
    {
      Q_UNUSED(image);
      Q_UNUSED(x);
      Q_UNUSED(y);
      Q_UNUSED(pixelX);
      Q_UNUSED(pixelY);
    }
  };

  /**
   * A Functional that collects points which intensity is
   * higher then given threshold.   
   *
   */
  
  template <class ResultType, class ImageType> struct SweepCollectorFunction
  {
    SweepCollectorFunction(double  thres = 0) : threshold(thres) { matCoordinates.resize(0,5);}

    virtual ~SweepCollectorFunction() {}

    /**
     * Operator()-which is called for each sweeped pixel. Operator
     * saves pixel coordinates if intensity value at pixel is higher
     * then given threshold. 
     *
     * @param image is a gray-level image
     *
     * @param x is a x-point in line coordinate system.
     *
     * @param y is a y-point in line coordinate system.
     *
     * @param pixelX is a x-point in image coordinate system.
     *
     * @param pixelY is a y-point in image coordinate system. 
     */
    virtual void operator()(const PiiMatrix<ImageType>& image,
                            int x,
                            int y,
                            double pixelX,
                            double pixelY)
    {
      if (Pii::valueAt(image,pixelY, pixelX) > threshold)
        {
          /*
          if(Pii::almostEqualRel(threshold,0.0))
            if(Pii::almostEqualRel(Pii::valueAt(image, pixelY, pixelX), threshold,1e-3))
              return;
         */   
         ResultType* row = matCoordinates.insertRow(-1);
          row[0] = x, row[1] = y, row[2] = pixelX, row[3] = pixelY, row[4] = Pii::valueAt(image, pixelY, pixelX);
        }
    }

    // Contains sweeped coordinates
    PiiMatrix<ResultType> matCoordinates;

    double threshold;
  };
  
  
  /**
   * A Line sweeper. Function goes through given segment of a line
   * and sweeps line from both side going through line perpendicular
   * vector. For each point which is inside of given radius, function
   * calls type (note this is done also for line point) object
   * operator(), with params point coordinates (x,y) and intensity. 
   *
   * @param image a gray-level image.
   *
   * @param coordinates is a row matrix which contains line segment begin and end
   * point (x1,y1,x2,y2).  
   *
   * @param radius is length of perpendicular vector. 
   * 
   * @param function is a object which operator() is called with point
   * coordinates (x,y) and intensity and which returns value which is
   * saved in result matrix.
   */
  template <class ImageType, class SweepFunction>
  SweepFunction sweepLine(const PiiMatrix<ImageType>& image,
                          const PiiMatrix<double>& coordinates,
                          SweepFunction function,
                          int radius = 0);              
}

#include "PiiObjectProperty-templates.h"

#endif //_PIIOBJECTPROPERTY_H
