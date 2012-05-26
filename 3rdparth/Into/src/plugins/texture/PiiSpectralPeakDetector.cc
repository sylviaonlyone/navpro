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

#include "PiiSpectralPeakDetector.h"

#include <PiiYdinTypes.h>
#include <PiiMatrixUtil.h>
#include <PiiMathDefs.h>
//#include <iostream>

PiiSpectralPeakDetector::Data::Data() :
  dPeakThreshold(0.4),
  bCompositionConnected(false),
  dMinWaveLength(0),
  dMaxWaveLength(INFINITY)
{
}

PiiSpectralPeakDetector::PiiSpectralPeakDetector() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("peaks"));
  addSocket(new PiiOutputSocket("composition"));
}

void PiiSpectralPeakDetector::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if (d->dPeakThreshold <= 0)
    PII_THROW(PiiExecutionException, tr("Peak threshold must be greater than zero."));
  
  d->bCompositionConnected = outputAt(1)->isConnected();
}

void PiiSpectralPeakDetector::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_NUMERIC_MATRIX_CASES(findPeaks, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

void PiiSpectralPeakDetector::markPeak(PiiMatrix<float>& powerSpectrum,
                                       int row, int column, double aspectRatio,
                                       PiiMatrix<double>& peaks)
{
  PII_D;
  int iCols = powerSpectrum.columns(), iRows = powerSpectrum.rows();

  // Calculate position accurately by a weighted average.
  double dLocationX = 0, dLocationY = 0, dSum = 0, dPeakValue = 0;
  for (int r=-1; r<=1; ++r)
    for (int c=-1; c<=1; ++c)
      {
        int iRow = row + r, iCol = column + c;
        if (iRow >= 0 && iRow < iRows && iCol >= 0 && iCol < iCols)
          {
            double dValue = powerSpectrum(iRow, iCol);
            if (dValue > dPeakValue)
              dPeakValue = dValue;
            dLocationY += dValue * r;
            dLocationX += dValue * c;
            dSum += dValue;
            // Black out this peak
            powerSpectrum(iRow, iCol) = 0;
          }
      }

  // dSum must be non-zero because d->dPeakThreshold is always positive
  dLocationX = column + dLocationX/dSum;
  dLocationY = row + dLocationY/dSum;

  if (dLocationX >= iCols/2)
    dLocationX -= iCols;

  // Horizontal frequency in pixels
  double dFreqX = (Pii::abs(dLocationX)+1) / iCols;
  // Vertical frequency
  double dFreqY = double(dLocationY + 1) / iRows;

  // Resultant
  double dWaveLength = 1 / Pii::hypotenuse(dFreqY, dFreqX);

  if (dWaveLength >= d->dMinWaveLength && dWaveLength <= d->dMaxWaveLength)
    {
      double dAngle = dLocationX != 0 ? atan(aspectRatio*dLocationY/dLocationX) : M_PI_2;
      //qDebug("row = %d, col = %d, dSum = %lf, dLocationX = %lf, dLocationY = %lf, dAngle = %lf", row, column, dSum, dLocationX, dLocationY, dAngle/M_PI*180);
      
      peaks.insertRow(-1, dLocationX, dLocationY, dPeakValue/(iCols*iRows), dWaveLength, dAngle);
    }
}

template <class T> void PiiSpectralPeakDetector::findPeaks(const PiiVariant& obj)
{
  PII_D;
  PiiMatrix<float> img(obj.valueAs<PiiMatrix<T> >());

  float fMean = Pii::meanAll<float>(img);
  // Reduce aperture effect
  img -= fMean;
  PiiMatrix<std::complex<float> > matTransformed(d->fft.forwardFft(img));
  // Power spectrum is symmetric for real signals. We thus take only
  // the upper half. In fact, this is the square root of the real
  // power spectrum, but it contains essentially the same information.
  PiiMatrix<float> matPowerSpectrum(Pii::abs(matTransformed(0,0,matTransformed.rows()/2,-1)));

  int iRows = matPowerSpectrum.rows(), iCols = matPowerSpectrum.columns(), iHalfCols = iCols / 2;
  double dAspectRatio = double(img.columns()) / img.rows();
  double dThreshold = d->dPeakThreshold * img.columns() * img.rows();
  const float* pSpectrumRow = 0;

  PiiMatrix<double> matPeaks(0,5);
  matPeaks.reserve(32);
  
  for (int r = 1; r < matPowerSpectrum.rows(); ++r)
    {
      pSpectrumRow = matPowerSpectrum[r];
      for (int c = 1; c < iCols; ++c)
        if (pSpectrumRow[c] >= dThreshold)
          markPeak(matPowerSpectrum, r, c, dAspectRatio, matPeaks);
    }

  // Handle horizontal frequencies separately
  pSpectrumRow = matPowerSpectrum[0];
  for (int c = 1; c < iHalfCols; ++c)
    if (pSpectrumRow[c] >= dThreshold)
      markPeak(matPowerSpectrum, 0, c, dAspectRatio, matPeaks);

  // Vertical frequencies...
  for (int r = 1; r < iRows; ++r)
    if (matPowerSpectrum(r,0) >= dThreshold)
      markPeak(matPowerSpectrum, r, 0, dAspectRatio, matPeaks);

  // Sort to descending peak magnitude order
  Pii::sortRows(matPeaks, std::greater<double>(), 2);
  emitObject(matPeaks);

  //Pii::matlabPrint(std::cout, matPeaks);

  if (d->bCompositionConnected)
    {
      PiiMatrix<std::complex<float> > matPeakSpectrum(matTransformed.rows(), matTransformed.columns());
      for (int r=0; r<matPeaks.rows(); ++r)
        {
          int iRow = Pii::round<int>(matPeaks(r,1));
          int iColumn = Pii::round<int>(matPeaks(r,0));
          if (iColumn < 0) iColumn += iCols;
          matPeakSpectrum(iRow, iColumn) = matTransformed(iRow, iColumn);
          // The spectrum is symmetric. Replicate the peaks unless the
          // frequency is zero.
          if (iColumn != 0 && iRow != 0)
            {
              iRow = matPeakSpectrum.rows()-iRow;
              iColumn = iCols-iColumn;
              matPeakSpectrum(iRow, iColumn) = matTransformed(iRow, iColumn);
            }
        }
      outputAt(1)->emitObject(Pii::real(d->fft.inverseFft(matPeakSpectrum)) + fMean);
    }
}

void PiiSpectralPeakDetector::setPeakThreshold(double peakThreshold) { _d()->dPeakThreshold = peakThreshold; }
double PiiSpectralPeakDetector::peakThreshold() const { return _d()->dPeakThreshold; }
void PiiSpectralPeakDetector::setMinWaveLength(double minWaveLength) { _d()->dMinWaveLength = minWaveLength; }
double PiiSpectralPeakDetector::minWaveLength() const { return _d()->dMinWaveLength; }
void PiiSpectralPeakDetector::setMaxWaveLength(double maxWaveLength) { _d()->dMaxWaveLength = maxWaveLength; }
double PiiSpectralPeakDetector::maxWaveLength() const { return _d()->dMaxWaveLength; }
