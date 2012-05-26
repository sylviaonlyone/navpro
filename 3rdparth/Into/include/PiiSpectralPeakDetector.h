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

#ifndef _PIISPECTRALPEAKDETECTOR_H
#define _PIISPECTRALPEAKDETECTOR_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>
#include <PiiFft.h>

/**
 * Find repeating components in textures. This operation uses the FFT
 * to find repeating components in a surface texture. It computes the
 * locations of the peaks and the wavelengths and orientations of the
 * corresponding waves in the image.
 *
 * @inputs
 *
 * @in image - Any gray-level image.
 * 
 * @outputs
 *
 * @out peaks - detected peaks. A N-by-5 (possibly empty) matrix in
 * which each row represents a detected peak. The stored values are
 * (in this order): horizontal location of the peak in the spectrum in
 * pixels, vertical location, relative strength of the peak, wave
 * length in pixels, and angle with respect to the y axis in radians. 
 * The angle is measured clockwise. (PiiMatrix<double>)
 *
 * @out composition - an image composed of the detected peaks. This
 * image illustrates the main frequency components present in the
 * image. (PiiMatrix<float>)
 *
 * @ingroup PiiTexturePlugin
 */
class PiiSpectralPeakDetector : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Detection threshold. Assume @f$P(\omega)@f$ is the power spectrum
   * of the input image whose size is @f$w \times h@f$. The threshold
   * @f$t@f$ is compared to the normalized square root of the power
   * spectrum, and a peaks must match the following rule:
   *
   * @f[
   * \frac{\sqrt{P(\omega)}}{w h} \ge t
   * @f]
   *
   * The default value is 0.4.
   */
  Q_PROPERTY(double peakThreshold READ peakThreshold WRITE setPeakThreshold);
  /**
   * The minimum wavelength accepted (in pixels). Use this and the
   * #maxWaveLength property to control the range of accepted
   * wavelengths. Frequency components outside of the accepted range
   * will be ignored. The default value is 0.
   */
  Q_PROPERTY(double minWaveLength READ minWaveLength WRITE setMinWaveLength);
  /**
   * The maximum wavelength accepted (in pixels). Use this and the
   * #minWaveLength property to control the range of accepted
   * wavelengths. Frequency components outside of the accepted range
   * will be ignored. The default value is infinity.
   */
  Q_PROPERTY(double maxWaveLength READ maxWaveLength WRITE setMaxWaveLength);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiSpectralPeakDetector();

  void check(bool reset);

  void setPeakThreshold(double peakThreshold);
  double peakThreshold() const;
  void setMinWaveLength(double minWaveLength);
  double minWaveLength() const;
  void setMaxWaveLength(double maxWaveLength);
  double maxWaveLength() const;

protected:
  void process();

private:
  template <class T> void findPeaks(const PiiVariant& obj);
  void markPeak(PiiMatrix<float>& powerSpectrum,
                int row, int column, double aspectRatio,
                PiiMatrix<double>& peaks);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiFft<float> fft;
    double dPeakThreshold;
    bool bCompositionConnected;
    double dMinWaveLength;
    double dMaxWaveLength;
  };
  PII_D_FUNC;
};


#endif //_PIISPECTRALPEAKDETECTOR_H
