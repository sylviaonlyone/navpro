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

#ifndef _PIICOLORS_H
# error "Never use <PiiColors-templates.h> directly; include <PiiColors.h> instead."
#endif

#include <PiiMath.h>
#include <PiiPrincipalComponents.h>
#include <PiiMetaTemplate.h>
#include <PiiTypeTraits.h>

namespace PiiColors
{
  template <class ColorType> PiiMatrix<int> toIndexed(const PiiMatrix<ColorType>& clrImage,
                                                      int redLevels, int greenLevels, int blueLevels)
  {
    typedef PiiImage::Traits<ColorType> Traits;
    
    PiiMatrix<int> result(PiiMatrix<int>::uninitialized(clrImage.rows(), clrImage.columns()));
    if (greenLevels == 0)
      greenLevels = redLevels;
    if (blueLevels == 0)
      blueLevels = redLevels;
    int iRedStep = blueLevels * greenLevels;
    float fRedScale = float(redLevels) / Traits::max();
    float fGreenScale = float(greenLevels) / Traits::max();
    float fBlueScale = float(blueLevels) / Traits::max();
    for (int r=0; r<clrImage.rows(); ++r)
      {
        const ColorType* pSourceRow = clrImage[r];
        int* pTargetRow = result[r];
        for (int c=0; c<clrImage.columns(); ++c)
          pTargetRow[c] =
            iRedStep * int(fRedScale * pSourceRow[c].rgbR) +
            blueLevels * int(fGreenScale * pSourceRow[c].rgbG) +
            int(fBlueScale * pSourceRow[c].rgbB);
      }
    return result;
  }
  
  template <class T> void normalizedRgb(const PiiMatrix<T>& image,
                                        PiiMatrix<typename T::Type>& ch1,
                                        PiiMatrix<typename T::Type>& ch2,
                                        float multiplier,
                                        int ch1Index, int ch2Index)
  {
    ch1Index = (2-ch1Index) & 3;
    ch2Index = (2-ch2Index) & 3;
    // Reserve space for color channels
    ch1.resize(image.rows(), image.columns());
    ch2.resize(image.rows(), image.columns());

    for (int r=image.rows(); r--; )
      {
        const T* row = image.row(r);
        typename T::Type* ch1Row = ch1.row(r), *ch2Row = ch2.row(r);
        for (int c=image.columns(); c--; )
          {
            T pixel = row[c];
            float normalizer = pixel.rgbR + pixel.rgbG + pixel.rgbB;
            if (normalizer == 0)
              {
                ch1Row[c] = 0;
                ch2Row[c] = 0;
                continue;
              }
            normalizer = multiplier / normalizer;
            ch1Row[c] = typename T::Type(normalizer * pixel.channels[ch1Index]);
            ch2Row[c] = typename T::Type(normalizer * pixel.channels[ch2Index]);
          }
      }
  }

  template <class ColorType> void measureColorDistribution(const PiiMatrix<ColorType>& clrImage,
                                                           PiiMatrix<double>& baseVectors,
                                                           PiiMatrix<double>& center,
                                                           int maxPixels)
  {
    int iImagePixels = clrImage.rows() * clrImage.columns();
    if (maxPixels <= 0)
      maxPixels = iImagePixels;

    PiiMatrix<double> matData(maxPixels, 3);

    const int iCols = clrImage.columns(), iRows = clrImage.rows();
    // Collect colors as row vectors into matData
    if (maxPixels >= iImagePixels)
      {
        // Take all pixels
        int iPixelCount = 0;
        for (int r=0; r<iRows; ++r)
          {
            const ColorType* pImageRow = clrImage[r];
            for (int c=0; c<iCols; ++c, ++iPixelCount)
              matData.rowAs<PiiColor<double> >(iPixelCount) = pImageRow[c];
          }
      }
    else
      {
        // Downsample image
        double dSampleRatio = double(iImagePixels) / maxPixels;
        for (int i=0; i<maxPixels; ++i)
          {
            int iLocation = int(dSampleRatio*i);
            matData.rowAs<PiiColor<double> >(i) = clrImage((iLocation / iCols) % iRows,
                                                           iLocation % iCols);
          }
      }

    // Move to zero mean
    center = Pii::subtractMean(matData, Pii::Vertically);
    // Find the new base vectors
    PiiMatrix<double> matSingularValues;
    baseVectors = Pii::principalComponents(matData, &matSingularValues);
    matSingularValues.map(Pii::Square<double>());
    // Scale base by the corresponding singular values
    Pii::transformRows(baseVectors, matSingularValues.rowBegin(0), std::divides<double>());
  }

  template <class ColorType> PiiMatrix<PiiColor<float> > normalizeColorDistribution(const PiiMatrix<ColorType>& clrImage,
                                                                                    const PiiMatrix<double>& baseVectors,
                                                                                    const PiiMatrix<double>& center)
  {
    PiiMatrix<PiiColor<float> > resultImg(clrImage.rows(), clrImage.columns());
    // Convert input arguments to floats.
    PiiMatrix<float> matBaseVectors(baseVectors);
    PiiColor<float> clrTranslation(center(0), center(1), center(2));

    const int iCols = clrImage.columns(), iRows = clrImage.rows();
    // Convert each pixel
    for (int r=0; r<iRows; ++r)
      {
        const ColorType* pSource = clrImage[r];
        PiiColor<float>* pTarget = resultImg[r];
        for (int c=0; c<iCols; ++c)
          {
            Pii::multiply((PiiColor<float>(pSource[c]) - clrTranslation).constBegin(), // Translate to zero
                          baseVectors, // Multiply by the translation matrix
                          pTarget[c].begin()); // Store result to target image
          }
      }
    return resultImg;
  }

  template <class ColorType, class UnaryFunction>
  PiiMatrix<typename UnaryFunction::result_type> matchColors(const PiiMatrix<ColorType>& clrImage,
                                                             const PiiMatrix<double>& baseVectors,
                                                             const PiiMatrix<double>& center,
                                                             UnaryFunction func)
  {
    typedef typename UnaryFunction::result_type ResultType;
    
    PiiMatrix<ResultType> resultImg(PiiMatrix<int>::uninitialized(clrImage.rows(), clrImage.columns()));
    // Convert input arguments to floats.
    PiiMatrix<float> matBaseVectors(baseVectors);
    PiiColor<float> clrTranslation(center(2), center(1), center(0));

    const int iRows = clrImage.rows(), iCols = clrImage.columns();
    // Convert each pixel
    for (int r=0; r<iRows; ++r)
      {
        const ColorType* pSource = clrImage[r];
        ResultType* pTarget = resultImg[r];
        for (int c=0; c<iCols; ++c)
          {
            PiiColor<float> normalizedColor;
            Pii::multiply((PiiColor<float>(pSource[c]) - clrTranslation).constBegin(), // Translate to zero
                              baseVectors, // Multiply by the translation matrix
                              normalizedColor.begin()); // Store result
            // Apply a user-specified function to the color's distance
            // from origin.
            pTarget[c] = func(Pii::square(normalizedColor.c0) +
                              Pii::square(normalizedColor.c1) +
                              Pii::square(normalizedColor.c2));
          }
      }
    return resultImg;
  }


  /// @cond null
  template <class T> struct UnsignedHueLimit
  {
    static T limitHue(T value) { return value < 360 ? value : value + 360; }
  };

  template <class T> struct SignedHueLimit
  {
    static T limitHue(T value) { return value >= 0 ? value : value + 360; }
  };

  // Default behavior: 360 hue values, 255 saturation values
  template <class T> struct HsvScalingTraits
  {
    typedef float CalculationType;
    // Used to calculate endpoints and scaling for piecewise linear
    // hue approximation.
    static CalculationType hue360() { return 360; }
    static CalculationType maxSaturation() { return 255; }
    static T limitHue(T value)
    {
      // Unsigned and signed hue limits are different.
      typedef typename Pii::IfClass<Pii::IsUnsigned<T>,
        UnsignedHueLimit<T>, SignedHueLimit<T> >::Type LimitType;
      return LimitType::limitHue(value);
    }
    static T round(CalculationType value) { return Pii::round<T>(value); }
  };

  // Floating-point behavior: hue circle is 2pi, saturation in [0,1]
  template <class T> struct HsvScalingTraitsFloat
  {
    typedef T CalculationType;
    static CalculationType hue360() { return 2*M_PI; }
    static CalculationType maxSaturation() { return 1.0; }
    static T limitHue(T value) { return value >= 0 ? value : value + 2*M_PI; }
    static T round(CalculationType value) { return value; }
  };

  template <> struct HsvScalingTraits<float> : HsvScalingTraitsFloat<float> {};
  template <> struct HsvScalingTraits<double> : HsvScalingTraitsFloat<double> {};

  // Unsigned char behavior: hue and saturation are in [0,255]
  template <> struct HsvScalingTraits<unsigned char>
  {
    typedef float CalculationType;
    static CalculationType hue360() { return 256; }
    static CalculationType maxSaturation() { return 255; }
    static unsigned char limitHue(unsigned char value) { return value; }
    static unsigned char round(CalculationType value) { return Pii::round<unsigned char>(value); }
  };
  /// @endcond

  template <class Clr> Clr rgbToHsv(const Clr& rgbColor)
  {
    typedef typename Clr::Type T;
    typedef HsvScalingTraits<T> Traits;
    typedef typename Traits::CalculationType Real;
    
    T r = rgbColor.rgbR, g = rgbColor.rgbG, b = rgbColor.rgbB,
      min = qMin(qMin(r,g),b), max = qMax(qMax(r,g),b), delta = max - min,
      h = 0;

    if (max == 0) // pitch black
      return Clr();

    if (delta == 0) // gray, no hue, say 0
      h = 0;
    // Piecewise linear approximation
    else if (r == max) // between yellow & magenta
      h = Traits::round((Traits::hue360()/6) * (g - b) / delta);
    else if (g == max) // between cyan & yellow
      h = Traits::round((Traits::hue360()/3) + (Traits::hue360()/6) * (b - r) / delta);
    else // between magenta & cyan
      h = Traits::round((2*Traits::hue360()/3) + (Traits::hue360()/6) * (r - g) / delta);

    return Clr(Traits::limitHue(h), Traits::round(Traits::maxSaturation()*delta/max), max);
  }

  template <class Clr> Clr hsvToRgb(const Clr& hsvColor)
  {
    typedef typename Clr::Type T;
    typedef HsvScalingTraits<T> Traits;
    typedef typename Traits::CalculationType Real;

    if (hsvColor.hsvS == 0) // Gray
      return Clr(hsvColor.hsvV);

    // Scale hue to [0,6)
    Real scaledH = 6.0 * hsvColor.hsvH / Traits::hue360();
    // Sector is in [0,...,5]
    int sector = int(scaledH);
    // Factorial is the position within the sector, in [0,1]
    Real factorial = scaledH - sector;
    Real scaledS = Real(hsvColor.hsvS) / Traits::maxSaturation();
    // Conventional component naming is (p,q,t).
    // We don't know the RGB correspondents yet.
    T v = hsvColor.hsvV;
    T p = Traits::round(Real(v) * (1.0 - scaledS));

    // Sector determines the correspondents.
    if (sector & 1)
      {
        T q = Traits::round(Real(v) * (1.0 - scaledS * factorial));
        switch (sector)
          {
          case 1: return Clr(q,v,p);
          case 3: return Clr(p,q,v);
          case 5: return Clr(v,p,q);
          }
      }
    else
      {
        T t = Traits::round(Real(v) * (1.0 - scaledS * (1.0 - factorial)));
        switch (sector)
          {
          case 0: return Clr(v,t,p);
          case 2: return Clr(p,v,t);
          case 4: return Clr(t,p,v);
          }
      }
    return Clr(); // never encountered, suppresses a compiler warning
  }

  template <class Clr> Clr xyzToLab(const Clr& xyzColor,
                                    const Clr& whitePoint)
  {
    typedef typename Clr::Type T;
    T xPerXn = xyzColor.xyzX/whitePoint.xyzX;
    T yPerYn = xyzColor.xyzY/whitePoint.xyzY;
    T zPerZn = xyzColor.xyzZ/whitePoint.xyzZ;

#define PII_LAB_F(x) x = T(x > 0.008856451679035631 ? Pii::pow(x,T(1.0/3.0)) : 7.787037037037036*x + 16.0/116.0)
    PII_LAB_F(xPerXn);
    PII_LAB_F(yPerYn);
    PII_LAB_F(zPerZn);
#undef PII_LAB_F
    
    return Clr(116*yPerYn - 16,
               500*xPerXn - yPerYn,
               200*yPerYn - zPerZn);
  }

  template <class Clr> Clr labToXyz(const Clr& labColor,
                                    const Clr& whitePoint)
  {
    typedef typename Clr::Type T;
    T fy = (labColor.labL + 16) / 116;
    T fx = fy + labColor.labA/500;
    T fz = fy - labColor.labB/200;

#define PII_LAB_F(f,c) T(f > 0.2068965517241379 ? c * Pii::pow(f,T(3.0)) : (f-16.0/116.0) * 0.1284185493460167 * c)
    return Clr(PII_LAB_F(fx, whitePoint.xyzX),
               PII_LAB_F(fy, whitePoint.xyzY),
               PII_LAB_F(fz, whitePoint.xyzZ));
#undef PII_LAB_F
  }
}
