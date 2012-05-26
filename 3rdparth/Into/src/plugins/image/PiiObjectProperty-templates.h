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
# error "Never use <PiiObjectProperty-templates.h> directly; include <PiiObjectProperty.h> instead."
#endif

#include <PiiPrincipalComponents.h>

namespace PiiImage
{
  template <class T> void calculateProperties(const PiiMatrix<T>& mat, int labels, PiiMatrix<int>& areas,
                                              PiiMatrix<int>& centroids, PiiMatrix<int>& bbox)
  {
    const int iRows = mat.rows();
    const int iCols = mat.columns();

    if (labels == 0)
      labels = Pii::maxAll(mat);

    areas = PiiMatrix<int>(labels,1);
    centroids = PiiMatrix<int>(labels,2);
    PiiMatrix<double> matTmpCentroids(labels,2);
    bbox = PiiMatrix<int>(labels,4);
    for (int i=0; i<labels; i++)
      {
        bbox(i,0) = iCols-1;
        bbox(i,1) = iRows-1;
      }
  
    //calculate areas, centroids and bounding box-values
    const T* data;
    for (int r=0; r<iRows; ++r)
      {
        data = mat.row(r);
        for (int c=0; c<iCols; ++c)
          {
            int label = data[(int)c];
            if (data[c] > 0)
              {
                if (label >= labels)
                  {
                    areas.resize(label, 1);
                    centroids.resize(label, 2);
                    bbox.resize(label,4);
                    matTmpCentroids.resize(label,2);
                    for (int i=labels; i<label; ++i)
                      {
                        bbox(i,0) = iCols-1;
                        bbox(i,1) = iRows-1;
                      }
                    labels = label;
                  }
                label--;
                if (c < bbox(label,0)) //left
                  bbox(label,0) = c;
                if (r < bbox(label,1)) //top
                  bbox(label,1) = r;
                if (c > bbox(label,2)) //right
                  bbox(label,2) = c;
                if (r > bbox(label,3)) //bottom
                  bbox(label,3) = r;
              
                areas(label,0)++;
                matTmpCentroids(label,0) += c;
                matTmpCentroids(label,1) += r;
              }
          }
      }

    // Convert coordinates to width, height
    for (int i=0; i<bbox.rows(); i++)
      {
        bbox(i,2) = bbox(i,2) - bbox(i,0) + 1;
        bbox(i,3) = bbox(i,3) - bbox(i,1) + 1;
      }
  
    for (int i=0; i<labels; i++)
      {
        centroids(i,0) = (int)(matTmpCentroids(i,0) / (double)areas(i,0) + 0.5);
        centroids(i,1) = (int)(matTmpCentroids(i,1) / (double)areas(i,0) + 0.5);
      }
  }

  template <class T> PiiMatrix<double> calculateDirection(const PiiMatrix<T>& mat,
                                                          T label,
                                                          double* length,
                                                          double* width,
                                                          int* pixels,
                                                          typename Pii::OnlyNumeric<T>::Type)
  {
    return calculateDirection(mat, std::bind2nd(std::equal_to<T>(), label), length, width, pixels);
  }
  
  template <class T, class UnaryFunction>
  PiiMatrix<double> calculateDirection(const PiiMatrix<T>& mat,
                                       UnaryFunction decisionRule,
                                       double* length,
                                       double* width,
                                       int* pixels)
  {
    const int iRows = mat.rows();
    const int iCols = mat.columns();
    
    // Collect matching points
    PiiMatrix<double> matXY(0, 2);
    matXY.reserve(128);
    
    double dWeightr = 0.0, dWeightc = 0.0;
    for (int r=0; r<iRows; ++r)
      {
        const T* pRow = mat[r];
        for (int c=0; c<iCols; ++c)
          if (decisionRule(pRow[c]))
            {
              dWeightr += r;
              dWeightc += c;
              matXY.appendRow(double(c), double(r));
            }
      }

    int iArea = matXY.rows();
    
    if (pixels != 0)
      *pixels = iArea;
    
    if (iArea < 2)
      {
        if (length != 0) *length = 0;
        if (width != 0) *width = 0;
        return PiiMatrix<double>::identity(2);
      }
      
    double aCenter[] = { dWeightc / iArea, dWeightr / iArea };
    Pii::transformRows(matXY, aCenter, std::minus<double>());

    PiiMatrix<double> matSingularValues;
    PiiMatrix<double> matBase(Pii::principalComponents(matXY, &matSingularValues));

    if (length != 0) *length = matSingularValues(0,0) + 1;
    if (width != 0) *width = matSingularValues(0,1) + 1;

    return matBase;
  }
  
  template <class ImageType, class SweepFunction>
  SweepFunction sweepLine(const PiiMatrix<ImageType>& image,
                          const PiiMatrix<double>& coordinates,
                          SweepFunction function,
                          int radius)
  { 
    double xBegin = coordinates(0,0), yBegin = coordinates(0,1), xEnd = coordinates(0,2), yEnd = coordinates(0,3);
    
    // Calculate directional unit vector
    double diffX = xEnd - xBegin;
    double diffY = yEnd - yBegin;
    
    double length = sqrt(diffX*diffX + diffY*diffY);
    diffX /= length, diffY /= length;

    //qDebug()<<" Direct vec : x: "<<diffX<<" y: "<<diffY;
    double curPosX = xBegin, curPosY = yBegin;
    
    
    //.. and normal unit vector.
    double dXnormal = diffY;
    double dYnormal = -diffX;

    //qDebug()<<" curPosX : "<<curPosX<<" curPosY : "<<curPosY<<" dXnormal: "<<dXnormal<<" dYnormal: "<<dYnormal;
       
    // Now x,y goes in line coordinates and sampleX and sampleY goes
    // in image coordinates!! 
    int x = 0;
    
    do
      {
        for (int y = -radius; y<=radius; ++y)
          {
            double sampleX = curPosX + dXnormal * y;
            double sampleY = curPosY + dYnormal * y;
            //qDebug()<<" sampleX : "<<sampleX<<" sampleY : "<<sampleY;
            if (sampleX > 0 && sampleX < image.columns()-1 &&
                sampleY > 0 && sampleY < image.rows()-1)
              {
                function(image, x, y, sampleX, sampleY);
              }
          }

        curPosX +=diffX;
        curPosY +=diffY;
        ++x;
        //qDebug()<<" now curX :"<<curPosX <<" curY : "<<curPosY<<" rows: "<<image.rows()<<" cols: "<<image.columns();
      } while (curPosX >= 0
               && curPosY >= 0
               && curPosX < image.columns()
               && curPosY < image.rows());
        
    return function; 
  }
  
  
}


