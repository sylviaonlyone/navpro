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

#ifndef _PIIIMAGE_H
# error "Never use <PiiImage-templates.h> directly; include <PiiImage.h> instead."
#endif

#include <PiiGeometricObjects.h>
#include "PiiThresholding.h"
#include <fast.h>

#include <PiiMatrixUtil.h>
#include <PiiMath.h>

namespace PiiImage
{
  template <class ColorType> PiiMatrix<typename ColorType::Type> colorChannel(const PiiMatrix<ColorType>& image,
                                                                              int channel)
  {
    typedef typename ColorType::Type T;
    PiiMatrix<T> result(PiiMatrix<T>::uninitialized(image.rows(), image.columns()));

    // Reverse color indexing
    channel = (2-channel) & 3;
    const int iRows = image.rows(), iCols = image.columns();
    for (int r=0; r<iRows; ++r)
      {
        const ColorType* source = image.row(r);
        T* target = result.row(r);
        for (int c=0; c<iCols; ++c)
          target[c] = source[c].channels[channel];
      }
    return result;
  }

  template <class ColorType> void setColorChannel(PiiMatrix<ColorType>& image,
                                                  int channel,
                                                  const PiiMatrix<typename ColorType::Type>& values)
  {
    if (image.rows() != values.rows() || image.columns() != values.columns())
      return;
    
    typedef typename ColorType::Type T;

    // Reverse color indexing
    channel = (2-channel) & 3;
    const int iRows = image.rows(), iCols = image.columns();
    for (int r=0; r<iRows; ++r)
      {
        ColorType* target = image.row(r);
        const T *source = values.row(r);
      
        for (int c=0; c<iCols; ++c)
          target[c].channels[channel] = source[c];
      }
  }

  // analogous to the above
  template <class ColorType> void setColorChannel(PiiMatrix<ColorType>& image,
                                                  int channel,
                                                  typename ColorType::Type value)
  {
    typedef typename ColorType::Type T;

    // Reverse color indexing
    channel = (2-channel) & 3;
    const int iRows = image.rows(), iCols = image.columns();
    for (int r=0; r<iRows; ++r)
      {
        ColorType* target = image.row(r);
        for (int c=0; c<iCols; c++)
          target[c].channels[channel] = value;
      }
  }

  template <class T> inline T readAlphaChannel(const PiiColor<T>&) { return T(0); }
  template <class T> inline T readAlphaChannel(const PiiColor4<T>& clr) { return clr.c3; }

  template <class ColorType> void separateChannels(const PiiMatrix<ColorType>& image,
                                                   PiiMatrix<typename ColorType::Type>* channelImages,
                                                   int channels)
  {
    typedef typename ColorType::Type T;

    channels = qBound(3, channels, 4);
    
    const int iRows = image.rows(), iColumns = image.columns();
    for (int i=0; i<channels; ++i)
      channelImages[i].resize(iRows, iColumns);

    if (channels == 3)
      {
        for (int r=0; r<iRows; ++r)
          {
            const ColorType* row = image.row(r);
            T *row0 = channelImages[0].row(r),
              *row1 = channelImages[1].row(r),
              *row2 = channelImages[2].row(r);
        
            for (int c=0; c<iColumns; ++c)
              {
                row0[c] = row[c].c0;
                row1[c] = row[c].c1;
                row2[c] = row[c].c2;
              }
          }
      }
    else
      {
        for (int r=0; r<iRows; ++r)
          {
            const ColorType* row = image.row(r);
            T *row0 = channelImages[0].row(r),
              *row1 = channelImages[1].row(r),
              *row2 = channelImages[2].row(r),
              *row3 = channelImages[3].row(r);
        
            for (int c=0; c<iColumns; ++c)
              {
                row0[c] = row[c].c0;
                row1[c] = row[c].c1;
                row2[c] = row[c].c2;
                row3[c] = readAlphaChannel(row[c]);
              }
          }
      }
  }

  // static functions to convert intermediate types back to original
  template <class T> struct Rounder { static T round(typename Pii::ToFloatingPoint<T>::Type val) { return (T)Pii::round(val); } };
  template <> struct Rounder<float> { static float round(float val) { return val; } };
  template <> struct Rounder<double> { static double round(double val) { return val; }  };
  // Round each color channel separately
  template <class T> struct Rounder<PiiColor<T> >
  {
    static PiiColor<T> round(typename Pii::ToFloatingPoint<PiiColor<T> >::Type val)
    {
      return PiiColor<T>(Rounder<T>::round(val.c0), Rounder<T>::round(val.c1), Rounder<T>::round(val.c2));
    }
  };
  template <class T> struct Rounder<PiiColor4<T> >
  {
    static PiiColor4<T> round(typename Pii::ToFloatingPoint<PiiColor4<T> >::Type val)
    {
      return PiiColor4<T>(Rounder<T>::round(val.c0), Rounder<T>::round(val.c1), Rounder<T>::round(val.c2), Rounder<T>::round(val.c3));
    }
  };

  // static functions to modify scaled rows
  template <class T> struct ScaleAdder { static void operate(T& ref, T val) { ref += val; } };
  template <class T> struct ScaleSetter { static void operate(T& ref, T val) { ref = val; } };

  // forward declaration
  template <class Func, class T> void scaleRow(const T* sourceRow, typename Pii::ToFloatingPoint<T>::Type* targetRow,
                                               int sourceColumns, int targetColumns, double step);


  template <class T> PiiMatrix<T> scale(const PiiMatrix<T>& image, int rows, int columns, Pii::Interpolation interpolation)
  {
    // Catch invalid cases
    if (rows <= 0 || columns <= 0 || image.rows() == 0 || image.columns() == 0)
      return PiiMatrix<T>();
    // No scaling needed...
    if (rows == image.rows() && columns == image.columns())
      return image;
    
    PiiMatrix<T> result(PiiMatrix<T>::uninitialized(rows, columns));
    if (interpolation == Pii::NearestNeighborInterpolation)
      {
        double stepX = (double)image.columns() / columns;
        double stepY = (double)image.rows() / rows;
        double currentRow = 0;
        for (int r=0; r<rows; r++, currentRow += stepY)
          {
            const T* sourceRow = image.row((int)currentRow);
            T* resultRow = result.row(r);
            double currentColumn = 0;
            for (int c=0; c<columns; c++, currentColumn += stepX)
              resultRow[c] = sourceRow[(int)currentColumn];
          }
      }
    else //if (interpolation == Pii::LinearInterpolation)
      {
        /* Ratio depends on whether we are scaling down or up
         *
         * Downscaling from 6 to 2 (I = input, O = output):
         *
         * IIIIII
         * \|/\|/
         *  O  O  interpolation step = 6/2 = 3
         *
         * Upscaling from 2 to 6:
         *
         * OOOOOO
         * |    |
         * I    I interpolation step = (2-1)/(6-1) = 1/5
         */
        double stepX = image.columns() >= columns ? (double)image.columns() / columns : (double)(image.columns()-1) / (columns-1);
        double stepY = image.rows() >= rows ? (double)image.rows() / rows : (double)(image.rows()-1) / (rows-1);
        
        typedef typename Pii::ToFloatingPoint<T>::Type Real;
        PiiMatrix<Real> scaledRow(1,columns);

        if (stepY == 1)
          {
            Real* scaledRowPtr = scaledRow.row(0);
            for (int r=0; r<rows; r++)
              {
                scaleRow<ScaleSetter<Real> >(image.row(r), scaledRowPtr, image.columns(), columns, stepX);
                copyCastRow(result.row(r), scaledRowPtr, columns);
              }
          }
        // If we're scaling down, calculate average over many successive rows.
        else if (stepY >= 1)
          {
            double currentRow = 0;
            Real* scaledRowPtr = scaledRow.row(0);
            int iStep = (int)std::ceil(stepY);
            for (int r=0; r<rows; r++, currentRow += stepY)
              {
                // Reset row sum
                scaledRow = Real(0);
                int iRow = (int)currentRow;
                // Each row is scaled horizontally and added to the row sum
                for (int i=0; i<iStep; i++)
                  scaleRow<ScaleAdder<Real> >(image.row(iRow+i), scaledRowPtr, image.columns(), columns, stepX);

                scaledRow /= iStep; // get average

                // Typecast and copy to the target image
                copyCastRow(result.row(r), scaledRowPtr, columns);
              }
          }
        // Scale up -> interpolate between two successive rows
        else
          {
            double currentRow = stepY;
            PiiMatrix<Real> scaledRow2(1, columns);
            Real *ptr2 = scaledRow.row(0), *ptr1 = scaledRow2.row(0);

            // Get first scaled row
            scaleRow<ScaleSetter<Real> >(image.row(0), ptr2, image.columns(), columns, stepX);
            // First row isn't interpolated vertically
            copyCastRow(result.row(0), ptr2, columns);
            int r = 1, previouslyInterpolatedRow = 0;
            for (; r<rows-1; r++, currentRow += stepY)
              {
                int iRow = (int)currentRow;
                if (iRow+1 > previouslyInterpolatedRow)
                  {
                    previouslyInterpolatedRow = iRow+1;
                    // BUG this may overflow with very large scaling ratios due to rounding errors
                    scaleRow<ScaleSetter<Real> >(image.row(iRow+1), ptr1, image.columns(), columns, stepX);
                    // Swap scaled row pointers
                    Real* tmpPtr = ptr1;
                    ptr1 = ptr2;
                    ptr2 = tmpPtr;
                  }
                double fraction = currentRow - iRow;
                T* resultRow = result.row(r);
                for (int c=columns; c--; )
                  resultRow[c] = Rounder<T>::round(ptr1[c] * Real(1.0-fraction) + ptr2[c] * Real(fraction));
              }
            // last row isn't vertically interpolated
            copyCastRow(result.row(r), ptr2, columns);
          }
      }

    return result;
  }
  /** @internal
   * make a typecasted copy of a row
   */
  template <class T, class U> inline void copyCastRow(T* target, U* source, int columns)
  {
    for (int c=columns; c--; )
      target[c] = Rounder<T>::round(source[c]);
  }

  /**
   * Scale a single row of input data.
   *
   * @param sourceRow pointer to the beginning of the image row
   *
   * @param targetRow store scaled result here
   *
   * @param sourceColumns the number of columns in the source image
   *
   * @param targetColumns the number of columns in the target image
   *
   * @param step interpolation step (see above)
   *
   * Parameterize this function with either ScaleAdder or ScaleSetter
   * to either add the new values to the row or to just set them.
   */
  template <class Func, class T> void scaleRow(const T* sourceRow, typename Pii::ToFloatingPoint<T>::Type* targetRow,
                                               int sourceColumns, int targetColumns, double step)
  {
    typedef typename Pii::ToFloatingPoint<T>::Type Real;
    typedef typename Pii::ToFloatingPoint<T>::PrimitiveType RealScalar;
    
    if (step == 1)
      copyCastRow(targetRow, sourceRow, sourceColumns);
    // Scale down -> calculate average over successive pixels on this row.
    else if (step > 1)
      {
        double currentColumn = 0;
        int iStep = (int)std::ceil(step);
        for (int c=0; c<targetColumns; c++, currentColumn += step)
          {
            // Each pixel is an average over many pixels.
            // The average is calculted on full pixels only.
            int iCol = (int)currentColumn;
            Real sum(sourceRow[iCol]);
            for (int i=1; i < iStep; i++)
              sum += Real(sourceRow[iCol + i]);
            // Add or set the value
            Func::operate(targetRow[c], sum / RealScalar(iStep));
          }
      }
    // Scale up -> interpolate linearly between two neighboring pixels.
    else
      {
        double currentColumn = step;
        // Add or set the value
        Func::operate(targetRow[0], sourceRow[0]);
        int c = 1;
        for (; c<targetColumns-1; ++c, currentColumn += step)
          {
            int iCol = (int)currentColumn;
            double fraction = currentColumn - iCol;
            // NOTE this may overflow with very large scaling ratios
            Func::operate(targetRow[c], Real(sourceRow[iCol]) * RealScalar(1.0-fraction) + Real(sourceRow[iCol+1]) * RealScalar(fraction));
          }
        Func::operate(targetRow[c], sourceRow[sourceColumns-1]);
      }
  }
  
  template <class T> PiiMatrix<T> rotate(const PiiMatrix<T>& image, double theta,
                                         PiiImage::TransformedSize handling,
                                         T backgroundColor)
  {
    if (theta == 0)
      return image;
    else if (theta >= M_PI*2)
      theta = Pii::mod(theta, M_PI*2);
    else if (theta < 0)
      theta = Pii::mod(theta, M_PI*2) + M_PI*2;

    const int iRows = image.rows(), iCols = image.columns();

    if (handling == ExpandAsNecessary)
      {
        if (Pii::almostEqualRel(theta, 3 * M_PI_2))
          {
            int iLastRow = iCols-1;
            PiiMatrix<T> result(PiiMatrix<T>::uninitialized(iCols, iRows));
            for (int r=0; r<iRows; ++r)
              {
                const T* row = image.row(r);
                for (int c=0; c<iCols; ++c)
                  result(iLastRow-c, r) = row[c];
              }
            return result;
          }
        else if (Pii::almostEqualRel(theta, M_PI_2))
          {
            int iLastCol = iRows-1;
            PiiMatrix<T> result(PiiMatrix<T>::uninitialized(iCols, iRows));
            for (int r=0; r<iRows; ++r)
              {
                const T* row = image.row(r);
                for (int c=0; c<iCols; ++c)
                  result(c, iLastCol-r) = row[c];
              }
            return result;
          }
      }
    if (Pii::almostEqualRel(theta, M_PI))
      {
        PiiMatrix<T> result(PiiMatrix<T>::uninitialized(iRows, iCols));
        int iLastRow = iRows-1, iLastCol = iCols-1;
        for (int r=0; r<iRows; ++r)
          {
            const T* row = image.row(r);
            T* targetRow = result.row(iLastRow-r);
            for (int c=0; c<iCols; ++c)
              targetRow[iLastCol-c] = row[c];
          }
        return result;
      }

    return transform(image,
                     createRotationTransform(float(theta),
                                             image.columns()/2.0,
                                             image.rows()/2.0),
                     handling,
                     backgroundColor);
  }

  template <class T> PiiMatrix<T> crop(const PiiMatrix<T>& image,
                                       int x, int y,
                                       int width, int height,
                                       const PiiMatrix<float>& transform)
  {
    PiiMatrix<T> matResult(PiiMatrix<T>::uninitialized(height, width));
    const int iRows = image.rows(), iCols = image.columns();
    float fX, fY;
    
    for (int iY=0; iY<height; ++iY)
      {
        T* pResultRow = matResult[iY];
        for (int iX=0; iX<width; ++iX)
          {
            transformHomogeneousPoint(transform, float(iX + x), float(iY + y), &fX, &fY);
            if (fX >= 0 && fX <= iCols-1 &&
                fY >= 0 && fY <= iRows-1)
              pResultRow[iX] = T(Pii::valueAt(image, fY, fX));
            else
              pResultRow[iX] = T(0);
          }
      }
    return matResult;
  }

  // PENDING use histogram-based filtering for large neighborhoods
  template <class T> PiiMatrix<T> medianFilter(const PiiMatrix<T>& image,
                                               int filterRows, int filterColumns,
                                               Pii::ExtendMode mode)
  {
    if (filterColumns <= 0) filterColumns = filterRows;
    int rows = filterRows >> 1, cols = filterColumns >> 1;
    PiiMatrix<T> result(Pii::extend(image, rows, rows, cols, cols, mode));
    // Make sure the image won't be shared (it would if mode was ExtendNot)
    result.detach();

    // Allocate an array to which the entire neigbhborhood will be
    // stored.
    int neighborhoodSize = filterRows * filterColumns;
    T* neighborhood = new T[neighborhoodSize];
    int filterRowBytes = filterColumns * sizeof(T);

    T** rowPtrs = new T*[filterRows];
    // Modify the result matrix in place
    for (int r=0; r<=result.rows()-filterRows; ++r)
      {
        // Initialize the rows that will be affected when filtering a
        // single row of image.
        for (int fr=filterRows; fr--; )
          rowPtrs[fr] = result.row(r+fr);

        for (int c=0; c<=result.columns()-filterColumns; ++c)
          {
            // fill in the neighborhood array
            T* ptr = neighborhood;
            for (int fr=filterRows; fr--; ptr+=filterColumns)
              memcpy(ptr, rowPtrs[fr]+c, filterRowBytes);
            // It is safe to store the result here because this pixel
            // won't be used again.
            rowPtrs[0][c] = Pii::median(neighborhood, neighborhoodSize);
          }
      }
    delete[] neighborhood;
    delete[] rowPtrs;
    if (mode != Pii::ExtendNot)
      return result(0, 0, image.rows(), image.columns());
    else
      return result(0, 0, -filterRows, -filterColumns);
  }

  template <class T> PiiMatrix<T> makeFilter(PrebuiltFilterType type, unsigned int size)
  {
    switch (type)
      {
      case SobelXFilter:
        return PiiMatrix<T>(sobelX);
      case SobelYFilter:
        return PiiMatrix<T>(sobelY);
      case RobertsXFilter:
        return PiiMatrix<T>(robertsX);
      case RobertsYFilter:
        return PiiMatrix<T>(robertsY);
      case PrewittXFilter:
        return PiiMatrix<T>(prewittX);
      case PrewittYFilter:
        return PiiMatrix<T>(prewittY);
      case UniformFilter:
        {
          PiiMatrix<T> result(PiiMatrix<T>::uninitialized(size, size));
          result = static_cast<T>(1.0 / (size*size));
          return result;
        }
      case GaussianFilter:
        return PiiMatrix<T>(makeGaussian(size));
      case LoGFilter:
        return PiiMatrix<T>(makeLoGaussian(size));
      default:
        return PiiMatrix<T>();
      }        
  }

  template <class T> bool separateFilter(const PiiMatrix<T>& filter,
                                         PiiMatrix<T>& horizontalFilter,
                                         PiiMatrix<T>& verticalFilter)
  {
    // First check that filter is a rank 1 matrix.
    if (Pii::rank(PiiMatrix<double>(filter)) != 1)
      return false;
    
    const int iRows = filter.rows(), iCols = filter.columns();
    horizontalFilter.resize(1, iCols);
    verticalFilter.resize(iRows, 1);

    T minNorm = Pii::Numeric<T>::maxValue();
    int iMinRow = 0;
    
    // Initialize vertical filter by the sum of absolute values on
    // each filter row. This determines the scaling factors (but not
    // the sign) of the row vectors (remember that the vectors are all
    // linearly dependent).
    for (int r=0; r<iRows; ++r)
      {
        T norm = T(Pii::norm1(filter(r,0,1,-1)));
        verticalFilter(r,0) = norm;
        // We use the row with the smallest non-zero norm as the
        // horizontal filter. There must be at least one such value
        // because rank is one.
        if (norm > 0 && norm < minNorm)
          {
            minNorm = norm;
            iMinRow = r;
          }
      }

    // Store horizontal filter and find its first non-zero entry.
    horizontalFilter = filter(iMinRow,0,1,-1);
    bool iFirstNonZero = 0;
    for (int c=0; c<iCols; ++c)
      if (horizontalFilter(0,c) != 0)
        {
          iFirstNonZero = c;
          break;
        }

    // Scale the vertical filter so that the smallest non-zero
    // multiplier is one. Check the signs at the same time.
    for (int r=0; r<iRows; ++r)
      {
        T& scale = verticalFilter(r,0);
        scale /= minNorm;

        // If signs are different, negate the scaling factor.
        if (horizontalFilter(0,iFirstNonZero) * filter(r,iFirstNonZero) < 0)
          scale = -scale;
      }

    return true;
  }

  template <class ResultType, class T, class U>
  PiiMatrix<ResultType> filter(const PiiMatrix<T>& image,
                               const PiiMatrix<U>& horizontalFilter,
                               const PiiMatrix<U>& verticalFilter,
                               Pii::ExtendMode mode)
  {
    if (horizontalFilter.rows() != 1 || verticalFilter.columns() != 1)
      return PiiMatrix<ResultType>(image);
    
    if (mode == Pii::ExtendZeros)
      return PiiDsp::filter<ResultType>(PiiDsp::filter<ResultType>(image, horizontalFilter, PiiDsp::FilterOriginalSize),
                                        verticalFilter, PiiDsp::FilterOriginalSize);

    const int rows = verticalFilter.rows() >> 1, cols = horizontalFilter.columns() >> 1;
    return PiiDsp::filter<ResultType>(PiiDsp::filter<ResultType>(Pii::extend(image, rows, rows, cols, cols, mode),
                                                                 horizontalFilter, PiiDsp::FilterValidPart),
                                      verticalFilter, PiiDsp::FilterValidPart);
  }

  template <class ResultType, class ImageType>
  PiiMatrix<ResultType> filter(const PiiMatrix<ImageType>& image,
                               PrebuiltFilterType type,
                               Pii::ExtendMode mode,
                               int filterSize)
  {
    switch (type)
      {
      case SobelXFilter:
      case SobelYFilter:
      case PrewittXFilter:
      case PrewittYFilter:
        {
          // Separable
          typedef typename Pii::Combine<ImageType,int>::Type FilterType;
          PiiMatrix<int> filter2D = makeFilter<int>(type, filterSize), hFilter, vFilter;
          separateFilter(filter2D, hFilter, vFilter);
          return PiiMatrix<ResultType>(filter<FilterType>(image, hFilter, vFilter, mode));
        }
      case RobertsXFilter:
      case RobertsYFilter:
        {
          // Not separable
          typedef typename Pii::Combine<ImageType,int>::Type FilterType;
          return PiiMatrix<ResultType>(filter<FilterType>(image, makeFilter<int>(type, filterSize), mode));
        }
      case GaussianFilter:
        {
          // Separable, but must use doubles
          typedef typename Pii::Combine<ImageType,double>::Type FilterType;
          PiiMatrix<double> filter2D = makeFilter<double>(type, filterSize), hFilter, vFilter;
          separateFilter(filter2D, hFilter, vFilter);
          return PiiMatrix<ResultType>(filter<FilterType>(image, hFilter, vFilter, mode));
        }
      case UniformFilter:
      case LoGFilter:
      default:
        {
          // Not separable, and must use doubles
          typedef typename Pii::Combine<ImageType,double>::Type FilterType;
          return PiiMatrix<ResultType>(filter<FilterType>(image, makeFilter<double>(type, filterSize), mode));
        }
      }
  }

  template <class T, class U, class Quantizer>
  PiiMatrix<T> suppressNonMaxima(const PiiMatrix<T>& magnitude,
                                 const PiiMatrix<U>& direction,
                                 Quantizer quantizer)
  {
    int rows = magnitude.rows(), cols = magnitude.columns();
    PiiMatrix<T> result(rows, cols);
    // Direction vectors for eight gradient angles
    int directions[8][2] = { {1,0}, {1,1}, {0,1}, {-1,1},
                             {-1,0}, {-1,-1}, {0,-1}, {1,-1} };
    
    // Leave borders unhandled. Unless the gradient is exactly
    // vertical or horizontal, it is impossible to find the ridge.
    for (int r=1; r < rows-1; ++r)
      {
        const U* dirRow = direction.row(r);
        const T* magRow = magnitude.row(r);
        T* resultRow = result.row(r);
        for (int c=1; c < cols-1; ++c)
          {
            int angle = quantizer(dirRow[c]);
            T currentMag = magRow[c];
            // Look for the steepest change in the gradient direction.
            // If the gradient magnitude on both sides is smaller,
            // this is the top. If the largest gradient area is wider
            // than one pixel, we take the edge at the positive
            // gradient direction.
            if (magnitude(r + directions[angle][1],
                          c + directions[angle][0]) < currentMag &&
                magnitude(r - directions[angle][1],
                          c - directions[angle][0]) <= currentMag)
              resultRow[c] = currentMag;
          }
      }
    // Handle top and bottom row specially
    for (int r=0; r < rows; r += rows-1)
      {
        const U* dirRow = direction.row(r);
        const T* magRow = magnitude.row(r);
        T* resultRow = result.row(r);
        for (int c=1; c < cols-1; ++c)
          {
            int angle = quantizer(dirRow[c]);
            // Only accept horizontal gradients
            if ((angle & 3) != 0) // faster than (angle != 0 || angle != 4)
              continue;
            
            T currentMag = magRow[c];
            // Same stuff as above, but only applies to horizontal gradient
            if (magnitude(r, c + directions[angle][0]) < currentMag &&
                magnitude(r, c - directions[angle][0]) <= currentMag)
              resultRow[c] = currentMag;
          }
      }
    // Handle left and right column specially
    for (int r=1; r < rows-1; ++r)
      {
        const U* dirRow = direction.row(r);
        const T* magRow = magnitude.row(r);
        T* resultRow = result.row(r);
        for (int c=0; c < cols; c += cols-1)
          {
            int angle = quantizer(dirRow[c]);
            // Only accept vertical gradients
            if ((angle & 3) != 2) // faster than (angle != 2 || angle != 6)
              continue;
            
            T currentMag = magRow[c];
            // Same stuff as above, but only applies to vertical gradient
            if (magnitude(r + directions[angle][1], c) < currentMag &&
                magnitude(r - directions[angle][1], c) <= currentMag)
              resultRow[c] = currentMag;
          }
      }
    // Corners are still unhandled, but we wouldn't be able to tell
    // if they are at a local gradient maximum anyway. Independent of
    // the gradient direction there is no way to inspect both sides of
    // the pixel.
    return result;
  }

  template <class T> PiiMatrix<T> transform(const PiiMatrix<T>& image,
                                            const PiiMatrix<float>& transform,
                                            TransformedSize handling,
                                            T backgroundColor)
  {
    int x = 0, y = 0;
    float fX, fY;

    int iMinX = Pii::Numeric<int>::maxValue(), iMinY = Pii::Numeric<int>::maxValue(),
      iMaxX = Pii::Numeric<int>::minValue(), iMaxY = Pii::Numeric<int>::minValue();
    
    if (handling == ExpandAsNecessary)
      {
#define PII_CHECK_EXTREMA_WITH_POINT \
        transformHomogeneousPoint(transform, float(x), float(y), &fX, &fY); \
        if (fX < iMinX) iMinX = int(floor(fX));  \
        if (fX > iMaxX) iMaxX = int(ceil(fX));   \
        if (fY < iMinY) iMinY = int(floor(fY));    \
        if (fY > iMaxY) iMaxY = int(ceil(fY))
        // Find extrema by transforming old corner coordinates.
        // Origin first (initially, x and y are zeros)
        PII_CHECK_EXTREMA_WITH_POINT;
        // Top right
        x = image.columns();
        PII_CHECK_EXTREMA_WITH_POINT;
        // Bottom right
        y = image.rows();
        PII_CHECK_EXTREMA_WITH_POINT;
        // Bottom left
        x = 0;
        PII_CHECK_EXTREMA_WITH_POINT;
#undef PII_CHECK_EXTREMA_WITH_POINT
      }
    else
      {
        iMinX = iMinY = 0;
        iMaxX = image.columns()-1;
        iMaxY = image.rows()-1;
      }

    //qDebug("x: %d-%d, y: %d-%d", iMinX, iMaxX, iMinY, iMaxY);
    // Create the result matrix
    PiiMatrix<T> result(PiiMatrix<T>::uninitialized(iMaxY-iMinY+1, iMaxX-iMinX+1));
    result = backgroundColor;

    // This matrix transforms coordinates from the new domain to the
    // old one.
    PiiMatrix<float> matInverseTransform = Pii::inverse(transform);

    int lastX = image.columns()-1, lastY = image.rows()-1;

    // Loop through all pixels in the transformed domain    
    for (y=iMinY; y<=iMaxY; ++y)
      {
        // Row pointer is out of bound on purpose.
        T* pResultRow = result[y-iMinY] - iMinX;
        for (x=iMinX; x<=iMaxX; ++x)
          {
            transformHomogeneousPoint(matInverseTransform, float(x), float(y), &fX, &fY);
            if (fX >= 0 && fX <= lastX &&
                fY >= 0 && fY <= lastY)
              pResultRow[x] = T(Pii::valueAt(image, fY, fX));
          }
      }
    return result;
  }
  
  template <class T> PiiMatrix<int> detectEdges(const PiiMatrix<T>& image,
                                                int smoothWidth,
                                                T lowThreshold, T highThreshold)
  {
    // Filter the source image if necessary
    // PENDING filtterin ja tuloksen tyyppi?
    PiiMatrix<T> matSource(smoothWidth != 0 ?
                           filter<T>(image, GaussianFilter, Pii::ExtendReplicate, smoothWidth) :
                           image);

    PiiMatrix<T> matGradientX = filter<T>(matSource, SobelXFilter, Pii::ExtendZeros);
    PiiMatrix<T> matGradientY = filter<T>(matSource, SobelYFilter, Pii::ExtendZeros);
    PiiMatrix<T> matMagnitude = gradientMagnitude(matGradientX, matGradientY);                                 

    // Automatic threshold if not explicitly given
    if (highThreshold == 0)
      {
        // Use the famous two-sigma rule (TM) as a threshold.
        float fMean = 0;
        float fStd = Pii::stdAll<float>(matMagnitude, &fMean);
        highThreshold = T(fMean + fStd * 2);
      }
    if (lowThreshold == 0)
      lowThreshold = T(0.4 * highThreshold);

    return hysteresisThreshold(suppressNonMaxima(matMagnitude,
                                                 gradientDirection(matGradientX, matGradientY),
                                                 RadiansToPoints<float>()),
                               lowThreshold, highThreshold);
  }

  template <class T> PiiMatrix<int> detectFastCorners(const PiiMatrix<T>& image, T threshold)
  {
    int pixel[16];
    fast9_make_offsets(pixel, image.stride());

    PiiMatrix<int> matCorners = fast9_detect(image, pixel, threshold);
    QVector<int> vecScores = fast9_score(image, matCorners, pixel, threshold);
    return fast_suppress_nonmax(matCorners, vecScores);
  }

  template <class T, class U> PiiMatrix<T> remap(const PiiMatrix<T>& image,
                                                 const PiiMatrix<PiiPoint<U> >& map)
  {
    const int iRows = map.rows(), iCols = map.columns();
    PiiMatrix<T> matResult(iRows, iCols);
    for (int r=0; r<iRows; ++r)
      {
        const PiiPoint<U>* pMapRow = map[r];
        T* pResultRow = matResult[r];
        for (int c=0; c<iCols; ++c)
          {
            PiiPoint<U> pt = pMapRow[c];
            if (pt.x >= 0 && pt.x < iCols &&
                pt.y >= 0 && pt.y < iRows)
              pResultRow[c] = T(Pii::valueAt(image, pt.y, pt.x));
          }
      }
    return matResult;
  }

  /// @internal
  template <class T> inline T transformHomogeneousPoint(const T* transform, T x, T y)
  {
    return transform[0] * x + transform[1] * y + transform[2];
  }

  template <class T> void transformHomogeneousPoint(const PiiMatrix<T>& transform,
                                                    T sourceX, T sourceY,
                                                    T* transformedX, T* transformedY)
  {
    *transformedX = transformHomogeneousPoint(transform[0], sourceX, sourceY);
    *transformedY = transformHomogeneousPoint(transform[1], sourceX, sourceY);
  }
  
  template <class T, class U> PiiMatrix<U> transformHomogeneousPoints(const PiiMatrix<T>& transform,
                                                                      const PiiMatrix<U>& points)
  {
    const int iRows = points.rows();
    PiiMatrix<U> matResult(PiiMatrix<U>::uninitialized(iRows, 2));
    const T* pTr0 = transform[0], *pTr1 = transform[1];
    for (int r=0; r<iRows; ++r)
      {
        const U* pSource = points[r];
        U* pTarget = matResult[r];
        pTarget[0] = U(transformHomogeneousPoint(pTr0, T(pSource[0]), T(pSource[1])));
        pTarget[1] = U(transformHomogeneousPoint(pTr1, T(pSource[0]), T(pSource[1])));
      }
    return matResult;
  }
}
