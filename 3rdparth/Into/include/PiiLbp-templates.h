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

#ifndef _PIILBP_H
# error "Never use <PiiLbp-templates.h> directly; include <PiiLbp.h> instead."
#endif

#include <PiiBits.h>
#include <climits>

#define INTERPOLATE_NEIGHBOR(neighbor,i) coeffs = d->pPoints[i].coeffs; \
  neighbor = coeffs[0] * (float)*neighborPtr1[i];                       \
  if (coeffs[1]) neighbor += coeffs[1] * (float)neighborPtr1[i][1];     \
  if (coeffs[2]) neighbor += coeffs[2] * (float)*neighborPtr2[i];       \
  if (coeffs[3]) neighbor += coeffs[3] * (float)neighborPtr2[i][1];     \
  ++neighborPtr1[i];                                                    \
  ++neighborPtr2[i]


template <class MatrixClass, class T, class Roi, class UnaryFunction>
PiiMatrix<int> PiiLbp::genericLbp(const PiiMatrix<T>& image, Roi roi, UnaryFunction centerFunc)
{
  typedef typename UnaryFunction::result_type C;
  
  if (d->mode == Symmetric)
    return genericSymmetricLbp<MatrixClass>(image, roi);

  if (d->mode == Standard &&
      d->interpolation == Pii::NearestNeighborInterpolation &&
      d->iSamples == 8 &&
      d->dRadius == 1)
    return basicLbp<MatrixClass>(image, roi, centerFunc);

  // This much free space must be ensured on all sides.
  const int iMargin = (int)std::ceil(d->dRadius);
  const int iSamples = d->iSamples;
  // The LBP code is built to the MSBs of an int. It must finally be
  // shifted to LSBs.
  const int iFinalShift = sizeof(int)*8 - iSamples;
  const bool bStandardMode = d->pLookup == 0;

  if (d->interpolation == Pii::NearestNeighborInterpolation)
    {
      //Initialize the feature vector
      MatrixClass result(image.rows(), image.columns(), iMargin, featureCount(iSamples, d->mode));
      C center;
      unsigned int value;
      int bit, r, c;
      const T** neighborPtr = new const T*[iSamples];
      const T* centerPtr;
      for (r=iMargin; r<image.rows()-iMargin; ++r)
        {
          // Tell our matrix that we're about to handle a new row.
          result.changeRow(r);
          
          // Initialize pointers to center and neighbors at the start
          // of each row sweep
          for (bit=0; bit<iSamples; ++bit)
            neighborPtr[bit] = image.row(r+d->pPoints[bit].nearestY) + (d->pPoints[bit].nearestX + iMargin);
          centerPtr = image.row(r) + iMargin;

          for (c=iMargin; c<image.columns()-iMargin; ++c)
            {
              if (roi(r,c))
                {
                  center = centerFunc(*centerPtr);
                  //the first bit doesn't need to be shifted
                  value = Pii::signBit(center, C(*neighborPtr[0]));
                  ++neighborPtr[0];
                  //take the LBP bit for each sample
                  for (bit=1; bit<iSamples; ++bit)
                    {
                      value |= Pii::signBit(center, C(*neighborPtr[bit])) >> bit;
                      ++neighborPtr[bit];
                    }
              
                  // Update the result matrix.
                  if (bStandardMode)
                    result.modify(c, static_cast<unsigned int>(value >> iFinalShift));
                  else
                    result.modify(c, d->pLookup[value >> iFinalShift]);
                }
              else
                {
                  // Not in ROI -> just increment neighbor pointers
                  for (bit=0; bit<iSamples; ++bit)
                    ++neighborPtr[bit];
                }
              
              ++centerPtr;
            }
        }
      delete[] neighborPtr;
      return result;
    }
  else
    {
      MatrixClass result(image.rows(), image.columns(), iMargin, featureCount(iSamples, d->mode));
      const T *centerPtr;
      const T** neighborPtr1 = new const T*[iSamples];
      const T** neighborPtr2 = new const T*[iSamples];
      float* coeffs;
      C center;
      float neighbor;
      unsigned int value;
      int bit, r, c;
      for (r=iMargin; r<image.rows()-iMargin; ++r)
        {
          // Tell our matrix that we're about to handle a new row.
          result.changeRow(r);

          //Initialize pointers to center and neighbors at the start
          //of each row sweep. Two pointers for each neighbor are
          //needed because (in general) two rows of pixels are
          //accessed at each sample.
          for (bit=0; bit<iSamples; ++bit)
            {
              neighborPtr1[bit] = image.row(r+d->pPoints[bit].y) + (d->pPoints[bit].x + iMargin);
              //Second row is used only if it fits in the image. (It
              //won't if ceil(radius) = radius).
              if (r+d->pPoints[bit].y+1 < image.rows())
                neighborPtr2[bit] = image.row(r+d->pPoints[bit].y+1) + (d->pPoints[bit].x + iMargin);
            }
          centerPtr = image.row(r) + iMargin;

          for (c=iMargin; c<image.columns()-iMargin; ++c)
            {
              if (roi(r,c))
                {
                  center = centerFunc(*centerPtr);
                  //the first bit doesn't need to be shifted
                  INTERPOLATE_NEIGHBOR(neighbor, 0);
                  value = Pii::floatSignBit(center, neighbor);
                  //take the LBP bit for each sample
                  for (bit=1; bit<iSamples; ++bit)
                    {
                      INTERPOLATE_NEIGHBOR(neighbor, bit);
                      value |= Pii::floatSignBit(center, neighbor) >> bit;
                    }
              
                  // Update the result matrix.
                  if (bStandardMode)
                    result.modify(c, static_cast<unsigned int>(value >> iFinalShift));
                  else
                    result.modify(c, d->pLookup[value >> iFinalShift]);
                }
              else
                {
                  // Not in ROI -> just increment neighbor pointers
                  for (bit=0; bit<iSamples; ++bit)
                    {
                      ++neighborPtr1[bit];
                      ++neighborPtr2[bit];
                    }
                }
              
              ++centerPtr;
            }
        }
      delete[] neighborPtr1;
      delete[] neighborPtr2;
      return result;
    }
}

template <class MatrixClass, class T, class Roi> PiiMatrix<int> PiiLbp::genericSymmetricLbp(const PiiMatrix<T>& image, Roi roi)
{
  if (d->interpolation == Pii::NearestNeighborInterpolation &&
      d->iSamples == 8 &&
      d->dRadius == 1)
    return basicSymmetricLbp<MatrixClass>(image, roi);

  // This much free space must be ensured on all sides.
  const int iMargin = (int)std::ceil(d->dRadius);
  const int iSamples = d->iSamples;
  // The LBP code is built to the MSBs of an int. It must finally be
  // shifted to LSBs.
  const int iHalfSamples = iSamples >> 1;
  const int iFinalShift = sizeof(int)*8 - iHalfSamples;

 if (d->interpolation == Pii::NearestNeighborInterpolation)
    {
      //Initialize the feature vector
      MatrixClass result(image.rows(), image.columns(),
                         iMargin, 1 << iHalfSamples);
      unsigned int value;
      int bit, r, c, secondBit;
      const T** neighborPtr = new const T*[iSamples];
      for (r=iMargin; r<image.rows()-iMargin; ++r)
        {
          // Tell our matrix that we're about to handle a new row.
          result.changeRow(r);
          
          // Initialize pointers to neighbors at the start of each row
          for (bit=0; bit<iSamples; ++bit)
            neighborPtr[bit] = image.row(r+d->pPoints[bit].nearestY) + (d->pPoints[bit].nearestX + iMargin);

          for (c=iMargin; c<image.columns()-iMargin; ++c)
            {
              if (roi(r,c))
                {
                  //the first bit doesn't need to be shifted
                  value = Pii::signBit(*neighborPtr[0], *neighborPtr[iHalfSamples]);
                  ++neighborPtr[0];
                  ++neighborPtr[iHalfSamples];
                  //take the LBP bit for each sample
                  for (bit=1, secondBit=iHalfSamples+1; bit<iHalfSamples; ++bit, ++secondBit)
                    {
                      value |= Pii::signBit(*neighborPtr[bit], *neighborPtr[secondBit]) >> bit;
                      ++neighborPtr[bit];
                      ++neighborPtr[secondBit];
                    }
              
                  // Update the result matrix.
                  result.modify(c, static_cast<unsigned int>(value >> iFinalShift));
                }
              else
                {
                  // Not in ROI -> just increment neighbor pointers
                  for (bit=0; bit<iSamples; ++bit)
                    ++neighborPtr[bit];
                }
            }
        }
      delete[] neighborPtr;
      return result;
    }
  else
    {
      MatrixClass result(image.rows(), image.columns(),
                         iMargin, 1 << iHalfSamples);
      const T** neighborPtr1 = new const T*[iSamples];
      const T** neighborPtr2 = new const T*[iSamples];
      float* coeffs;
      float neighbor1, neighbor2;
      unsigned int value;
      int bit, r, c, secondBit;
      for (r=iMargin; r<image.rows()-iMargin; ++r)
        {
          // Tell our matrix that we're about to handle a new row.
          result.changeRow(r);

          //Initialize pointers to neighbors at the start
          //of each row sweep. Two pointers for each neighbor are
          //needed because (in general) two rows of pixels are
          //accessed at each sample.
          for (bit=0; bit<iSamples; ++bit)
            {
              neighborPtr1[bit] = image.row(r+d->pPoints[bit].y) + (d->pPoints[bit].x + iMargin);
              //Second row is used only if it fits in the image. (It
              //won't if ceil(radius) = radius).
              if (r+d->pPoints[bit].y+1 < image.rows())
                neighborPtr2[bit] = image.row(r+d->pPoints[bit].y+1) + (d->pPoints[bit].x + iMargin);
            }

          for (c=iMargin; c<image.columns()-iMargin; ++c)
            {
              if (roi(r,c))
                {
                  //the first bit doesn't need to be shifted
                  INTERPOLATE_NEIGHBOR(neighbor1, 0);
                  INTERPOLATE_NEIGHBOR(neighbor2, iHalfSamples);
                  value = Pii::floatSignBit(neighbor1, neighbor2);
                  //take the LBP bit for each sample
                  for (bit=1,secondBit=iHalfSamples+1; bit<iHalfSamples; ++bit, ++secondBit)
                    {
                      INTERPOLATE_NEIGHBOR(neighbor1, bit);
                      INTERPOLATE_NEIGHBOR(neighbor2, secondBit);
                      value |= Pii::floatSignBit(neighbor1, neighbor2) >> bit;
                    }

                  // Update the result matrix.
                  result.modify(c, static_cast<unsigned int>(value >> iFinalShift));
                }
              else
                {
                  // Not in ROI -> just increment neighbor pointers
                  for (bit=0; bit<iSamples; ++bit)
                    {
                      ++neighborPtr1[bit];
                      ++neighborPtr2[bit];
                    }
                }
            }
        }
      delete[] neighborPtr1;
      delete[] neighborPtr2;
      return result;
    }
}

template <class MatrixClass, class T, class Roi, class UnaryFunction>
PiiMatrix<int> PiiLbp::basicLbp(const PiiMatrix<T>& image, Roi roi, UnaryFunction centerFunc)
{
  typedef typename UnaryFunction::result_type C;
  
  const T *r0, *r1, *r2;
  register unsigned int value;
  int r, c;
  C center;
  MatrixClass result(image.rows(), image.columns(), 1, 256);
  
  for (r=1; r<image.rows()-1; ++r)
    {
      result.changeRow(r);
      
      //Initialize row pointers to the beginning of three successive
      //rows.
      r0 = image.row(r-1);
      r1 = image.row(r);
      r2 = image.row(r+1);
      
      for (c=1; c<image.columns()-1; ++c)
        {
          if (roi(r,c))
            {
              //initialize center value
              center = centerFunc(r1[1]);

              //set LBP bits by addressing the neighbors counter-clockwise
              value = Pii::signBit(center, C(r1[2])) >> 31;
              value |= Pii::signBit(center, C(r0[2])) >> 30;
              value |= Pii::signBit(center, C(r0[1])) >> 29;
              value |= Pii::signBit(center, C(*r0)) >> 28;
              value |= Pii::signBit(center, C(*r1)) >> 27;
              value |= Pii::signBit(center, C(*r2)) >> 26;
              value |= Pii::signBit(center, C(r2[1])) >> 25;
              value |= Pii::signBit(center, C(r2[2])) >> 24;

              result.modify(c, value);
            }

          ++r0; ++r1; ++r2;
        }
    }
  return result;
}

template <class MatrixClass, class T, class Roi> PiiMatrix<int> PiiLbp::basicSymmetricLbp(const PiiMatrix<T>& image, Roi roi)
{
  const T *r0, *r1, *r2;
  register unsigned int value;
  int r, c;
  MatrixClass result(image.rows(), image.columns(), 1, 16);
  
  for (r=1; r<image.rows()-1; ++r)
    {
      result.changeRow(r);
      
      // Initialize row pointers to the beginning of three successive
      // rows.
      r0 = image.row(r-1);
      r1 = image.row(r);
      r2 = image.row(r+1);
      
      for (c=1; c<image.columns()-1; ++c)
        {
          if (roi(r,c))
            {
              // Set LBP bits by addressing the neighbors counter-clockwise
              value = Pii::signBit(*r1, r1[2]) >> 31;
              value |= Pii::signBit(*r2, r0[2]) >> 30;
              value |= Pii::signBit(r2[1], r0[1]) >> 29;
              value |= Pii::signBit(r2[2], *r0) >> 28;

              result.modify(c, value);
            }
          ++r0; ++r1; ++r2;
        }
    }
  return result;
}
