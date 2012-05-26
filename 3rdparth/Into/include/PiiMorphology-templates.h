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

#ifndef _PIIMORPHOLOGY_TEMPLATES_H
#define _PIIMORPHOLOGY_TEMPLATES_H

#include <PiiMatrix.h>
#include <PiiMatrixUtil.h>
#include <PiiFunctional.h>

namespace PiiImage
{
  template <class T> struct TophatFunction : public Pii::BinaryFunction<T>
  {
    T operator() (T firstValue, T secondValue) { return bool(secondValue) - bool(firstValue); }
  };

  template <class T> struct BottomhatFunction : public Pii::BinaryFunction<T>
  {
    T operator() (T firstValue, T secondValue) { return bool(firstValue) - bool(secondValue); }
  };


  template <class T, class U> PiiMatrix<T> morphology(const PiiMatrix<T>& image, const PiiMatrix<U>& mask, MorphologyOperation type, bool handleBorders )
   {
     switch(type)
       {
       case Erode:
         return erode(image, mask, handleBorders);
       case Dilate:
         return dilate(image, mask);
       case Open:
         return open(image,mask);
       case Close:
         return close(image,mask);
       case TopHat:
         return topHat(image, mask);
       case BottomHat:
         return bottomHat(image, mask);
       default:
         return image;
       }
   }  

  template <class T, class U> PiiMatrix<T> topHat(const PiiMatrix<T>& image, const PiiMatrix<U>& mask )
  {
    PiiMatrix<T> opened(open(image,mask));
    opened.map(TophatFunction<T>(), image);
    return opened;
  }

  template <class T, class U> PiiMatrix<T> bottomHat(const PiiMatrix<T>& image, const PiiMatrix<U>& mask )
	{
		int maskRows = mask.rows(), maskCols = mask.columns();
		int rOrig = maskRows / 2, cOrig = maskCols/2; //Origin
    
    //first do close-operation
    PiiMatrix<T> closed = erode(dilate(image, mask), mask);

    //remove edges from closed and original matrix and minus them each other
    static_cast<PiiMatrix<T>&>(closed(rOrig,cOrig,-(maskRows-rOrig),-(maskCols-cOrig)))
      .map(BottomhatFunction<T>(), image(rOrig,cOrig,-(maskRows-rOrig),-(maskCols-cOrig)));

    return closed;
  }
  
  template <class T, class U> PiiMatrix<T> erode(const PiiMatrix<T>& image, const PiiMatrix<U>& mask, bool handleBorders)
	{
		int maskRows = mask.rows(), maskCols = mask.columns();
		int rOrig = maskRows / 2, cOrig = maskCols/2; //Origin
    const U* maskData;
    const T* imageData;

    int rows,cols;
    PiiMatrix<T> img(handleBorders ?
                     Pii::extend(image,
                                           rOrig, maskRows-rOrig-1, cOrig, maskCols-cOrig-1,
                                           Pii::ExtendReplicate) :
                     image);

    rows = img.rows();
    cols = img.columns();
    
    if (maskRows > rows || maskCols > cols)
      {
        qWarning("PiiMorphology::erode(image, mask): Mask cannot be larger than image.");
        return img;
      }
    
		PiiMatrix<T> result(rows,cols);
    int rDiff = rows-maskRows;
    int cDiff = cols-maskCols;
    for (int r=0; r<=rDiff; ++r)
      {
        for (int c=0; c<=cDiff; ++c)
          {
            for (int mr=0; mr<maskRows; ++mr)
              {
                maskData = mask.row(mr);
                imageData = img.row(r+mr) + c;
                for (int mc=maskCols; mc--; )
                  {
                    if (int(maskData[mc]) & ~int(imageData[mc]))
                      goto out;
                  }
              }
            result(r+rOrig, c+cOrig) = T(1);
          out:;
          }
      }

    if ( handleBorders )
      return result(rOrig,cOrig,image.rows(), image.columns());
     
    return result;
	}
  
  template <class T, class U> PiiMatrix<T> dilate(const PiiMatrix<T>& image, const PiiMatrix<U>& mask )
		{
		int maskRows = mask.rows(), maskCols = mask.columns();
		int rows = image.rows(), cols = image.columns();
		int rOrig = maskRows/2, cOrig = maskCols/2;
    
		if (maskRows > rows || maskCols > cols)
      qWarning("BinaryMorphology::dilate(image, mask): Mask cannot be larger than image.");
    
		PiiMatrix<T> result(rows,cols);
    T* ptr;
    const U* maskData;
    int rDiff = rows - maskRows;
    int cDiff = cols - maskCols;
    for (int r=0; r<=rDiff; ++r)
      {
        for (int c=0; c<=cDiff; ++c)
          {
            
            if (!image(r+rOrig,c+cOrig))
              continue;
            for (int mr=0; mr<maskRows; ++mr)
              {
                maskData = mask.row(mr);
                ptr = result.row(r + mr) + c;
                for (register int mc=maskCols; mc--;)
                  ptr[mc] = T(int(ptr[mc]) | int(maskData[mc]));
              }
          }
      }

    // left edge
  int edge = cOrig + 1;
  for(int c=0; c<cOrig; c++)
    {
      edge--;
      for (int r=0; r<=rDiff; ++r)
        {
          
          if ( !image(r+rOrig,c) )
            continue;

          for (int mr=0; mr<maskRows; ++mr)
            {
              maskData = mask.row(mr) + edge;
              ptr = result.row(r+mr);
              for ( register int mc = maskCols-edge; mc--; )
                ptr[mc] = T(int(ptr[mc]) | int(maskData[mc]));
            }
        }
    }

   //right edge
  edge = maskCols;
  int maxCols = cols - (maskCols-cOrig);
  for(int c=cols-maskCols+1; c<=maxCols; ++c)
    {
      edge--;
      if ( (c + cOrig) > (cols-1) )
        break;
      
      for (int r=0; r<=rDiff; r++)
        {
          
          if ( !image(r+rOrig,c+cOrig) )
            continue;
          
          for (int mr=0; mr<maskRows; ++mr)
            {
              maskData = mask.row(mr);
              ptr = result.row(r+mr) + c;
              for(register int mc=edge; mc--;)
                ptr[mc] = T(int(ptr[mc]) | int(maskData[mc]));
              
            }
        }
    }

  //top edge
  edge = rOrig+1;
  for (int r=0; r<rOrig; ++r)
    {
      edge--;
      for(int c=0; c<=cDiff; ++c)
        {

          if ( !image(r,c+cOrig) )
            continue;
          for (int mr=edge, ir=0; mr<maskRows; ++mr,++ir)
            {
              maskData = mask.row(mr);
              ptr = result.row(ir) + c;
              for (register int mc=maskCols; mc--;)
                ptr[mc] = T(int(ptr[mc]) | int(maskData[mc]));
            }
        }
    }


  
  //bottom edge
  edge = maskRows;
  int maxRows = rows - (maskRows-rOrig);
  for(int r=rows-maskRows+1; r<=maxRows; ++r)
    {

      if ( (r + rOrig) > (rows-1) )
        break;
      
      edge--;
      for (int c=0; c<=cDiff; c++)
        {

          if ( !image(r+rOrig,c+cOrig) )
            continue;

          for (int mr=0; mr<edge; ++mr)
            {
              maskData = mask.row(mr);
              ptr = result.row(r+mr) + c;
              for(register int mc=maskCols; mc--;)
                ptr[mc] = T(int(ptr[mc]) | int(maskData[mc]));
              
            }
        }
    }

  
  //left top
  int rEdge = rOrig + 1;
  int cEdge;
  for(int r=0; r<rOrig; ++r)
    {
      --rEdge;
      cEdge = cOrig +1;
    for(int c=0; c<cOrig; ++c)
      {
        --cEdge;
        if ( !image(r,c) )
            continue;
        for (int mr=rEdge, ir=0; mr<maskRows; ++mr,++ir)
          {
            maskData = mask.row(mr) + cEdge;
            ptr = result.row(ir);
            for (register int mc=maskCols-cEdge; mc--;)
              ptr[mc] = T(int(ptr[mc]) | int(maskData[mc]));
          }
      }
    }

  
  //right top
  rEdge = rOrig + 1;
  for(int r=0; r<rOrig; ++r)
    {
      --rEdge;
      cEdge = maskCols;
      for(int c=cols-maskCols+cOrig+1; c<cols; ++c)
        {
          --cEdge;
          if ( !image(r,c) )
            continue;
          for (int mr=rEdge, ir=0; mr<maskRows; ++mr,++ir)
            {
              maskData = mask.row(mr);
              ptr = result.row(ir) + c - cOrig;
              for (register int mc=cEdge; mc--;)
                ptr[mc] = T(int(ptr[mc]) | int(maskData[mc]));
            }
        }
    }
     
      
  //left bottom
  rEdge = maskRows;
  for(int r=rows-maskRows+rOrig+1; r<rows; ++r)
    {
      --rEdge;
      cEdge = cOrig +1;
      for(int c=0; c<cOrig; ++c)
        {
          --cEdge;
          if ( !image(r,c) )
            continue;
          for (int mr=0; mr<rEdge; ++mr)
            {
              maskData = mask.row(mr) + cEdge;
              ptr = result.row(r+mr-rOrig);
              for (register int mc=maskCols-cEdge; mc--;)
                ptr[mc] = T(int(ptr[mc]) | int(maskData[mc]));
            }
        }
    }


  //right bottom
  rEdge = maskRows;
  for(int r=rows-maskRows+rOrig+1; r<rows; ++r)
    {
     --rEdge;
     cEdge = maskCols;
     for(int c=cols-maskCols+cOrig+1; c<cols; ++c)
       {
         --cEdge;
         if ( !image(r,c) )
           continue;
         for (int mr=0; mr<rEdge; mr++)
           {
             maskData = mask.row(mr);
             ptr = result.row(r+mr-rOrig) + c - cOrig;
             for (register int mc=cEdge; mc--;)
               ptr[mc] = T(int(ptr[mc]) | int(maskData[mc]));
           }
       }
    }

    return result;
	}
  
  template <class T, class U> PiiMatrix<T> hitAndMiss(const PiiMatrix<T>& image,
                                                      const PiiMatrix<U>& mask,
                                                      const PiiMatrix<U>& significance)
  {
		int maskRows = mask.rows(), maskCols = mask.columns();
		int rows = image.rows(), cols = image.columns();
		int rOrig = maskRows / 2, cOrig = maskCols / 2; //Origin
    const U* maskData;
    const U* signData;
    const T* imageData;
    
		if (maskRows > rows || maskCols > cols)
      {
        qWarning("PiiMorphology::hitAndMiss(image, structure, mask): Mask cannot be larger than image.");
        return image;
      }

		PiiMatrix<T> result(rows,cols);
    
		for (int r=0; r<=(rows-maskRows); r++)
      {
        for (int c=0; c<=(cols-maskCols); c++)
          {
            for (int mr=maskRows; mr--; )
              {
                maskData = mask.row(mr);
                signData = significance.row(mr);
                imageData = image.row(r+mr) + c;
                for (int mc=maskCols; mc--; )
                  {
                    if (signData[mc] & (maskData[mc] ^ T(imageData[mc])))
                      goto out;
                  }
              }
            result(r+rOrig, c+cOrig) = T(1);
          out:;
          }
      }

    /* PENDING
     * Image borders!
     */
    return result;
  }

  template <class T> PiiMatrix<T> thin(const PiiMatrix<T>& image, int amount)
  {
    PiiMatrix<T> result(image);
    result.detach();

    if (amount >= 0)
      {
        while (amount--)
          {
            // Loop through all border detector masks
            for (int m=8; m--;)
              // Apply the binary subtraction function (take off edges in
              // this direction)
              result.map(BottomhatFunction<T>(), hitAndMiss(result, borderMasks[m][0], borderMasks[m][1]));
          }
      }
    else
      {
        PiiMatrix<T> tmpResult(result);
        tmpResult.detach();
        // Apply the thinning operator as many times as necessary
        while (true)
          {
            // Loop through all border detector masks
            for (int m=8; m--;)
              // Apply the binary subtraction function (take off edges in
              // this direction)
              tmpResult.map(BottomhatFunction<T>(), hitAndMiss(tmpResult, borderMasks[m][0], borderMasks[m][1]));
            // compare matrices
            if (Pii::equals(tmpResult,result))
              break; // we converged
            // No luck yet. Continue
            result = tmpResult;
          }
      }
    
    return result;
  }                               

  template <class T> PiiMatrix<T> border(const PiiMatrix<T>& image)
  {
    PiiMatrix<T> result(image.rows(), image.columns());
    
    // Loop through all border detector masks
    for (int m=8; m--;)
      // Combine results into the resulting border matrix
      result |= hitAndMiss(image, borderMasks[m][0], borderMasks[m][1]);
    
    return result;
  }                               

  template <class T> PiiMatrix<T> shrink(const PiiMatrix<T>& image, int amount)
  {
    if (amount == 1)
      // Subtract border from original image
      return image.mapped(BottomhatFunction<T>(), border(image));
    else
      {
        PiiMatrix<T> result(image);
        while (amount--)
          result.map(BottomhatFunction<T>(), border(result));
        return result;
      }
  }

  template <class T> void createMask(MaskType type, PiiMatrix<T>& mask)
  {
    int rows = mask.rows();
    int cols = mask.columns();
    
    switch(type)
      {
      case EllipticalMask:
        {
          //calculate correspondence between x and y
          double a = (double)cols/2;
          double b = (double)rows/2;
          
          for ( double r=0.5; r<rows; r++ )
            {
              double x = a*std::sqrt(1-(double)(r-b)*double(r-b)/(b*b));
              double bottom = a-x;
              double top = a+x;
              
              for ( double c=0.5; c<cols; c++ )
                {
                  if ( c>bottom && c<top )
                    mask(static_cast<int>(r),static_cast<int>(c)) = T(1);
                }
            }
        }
        break;
      
      case DiamondMask:
        {
          //Calculate step
          double step = ((double)cols/2) / ((double)rows/2);
          double kc = cols/2;
          double temp = 0;
          int kp = rows / 2;
      
          if ( rows % 2 == 0) kp--;
          temp = 0;
          //Diamond upper side
          for ( int r=kp; r>=0; r-- )
            {
              for ( int c=static_cast<int>(temp-kc-0.5); c<static_cast<int>(cols-temp-kc+0.5); c++ )
                mask(r,int(c+kc)) = T(1);
              temp+=step;
            }
      
          if ( rows % 2 == 0) kp++;
          //Diamond bottom
          temp = 0;
          for ( int r=kp; r<rows; r++ )
            {
              for ( int c=static_cast<int>(temp-kc-0.5); c<static_cast<int>(cols-temp-kc+0.5); c++ )
                mask(r,int(c+kc)) = 1;
              temp+=step;
            }
        }
        break;
      default:
        mask = T(1);
      }
  }
  
  template <class T> PiiMatrix<T> createMask(MaskType type, int rows, int cols)
  {
    if (cols == 0)
      cols = rows;
    if ( rows == 0 )
      return PiiMatrix<T>(0,0);
    
    PiiMatrix<T> mask(rows, cols);
    createMask(type, mask);
    
    return mask;
  }
  
}

#endif //_PIIMORPHOLOGY_TEMPLATES_H
