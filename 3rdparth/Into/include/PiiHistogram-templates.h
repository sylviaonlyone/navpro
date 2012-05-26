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

#ifndef _PIIHISTOGRAM_H
# error "Never use <PiiHistogram-templates.h> directly; include <PiiHistogram.h> instead."
#endif


namespace PiiImage
{
  template <class T, class U, class Roi> PiiMatrix<T> histogram(const PiiMatrix<U>& image, const Roi& roi, unsigned int levels)
  {
    if (levels < 1)
      levels = unsigned(Pii::maxAll(image)) + 1;
    PiiMatrix<T> result(1, int(levels));
    T* vector = result.row(0);

    const int iRows = image.rows(), iCols = image.columns();
    for (int r=0; r<iRows; ++r)
      {
        const U* row = image.row(r);
        for (int c=0; c<iCols; ++c)
          if (unsigned(row[c]) < levels && roi(r,c)) ++vector[unsigned(row[c])];
      }
    return result;
  }

  template <class T, class U, class Roi> PiiMatrix<T> histogram(const PiiMatrix<U>& image, const Roi& roi, const PiiQuantizer<U>& quantizer)
  {
    PiiMatrix<T> result(1, quantizer.levels());
    T* vector = result.row(0);

    const int iRows = image.rows(), iCols = image.columns();
    for (int r=0; r<iRows; ++r)
      {
        const U* row = image.row(r);
        for (int c=0; c<iCols; ++c)
          if (roi(r,c)) ++vector[quantizer.quantize(row[c])];
      }
    return result;
  }

  template <class T, class U> PiiMatrix<T> normalize(const PiiMatrix<U>& histogram)
  {
    PiiMatrix<T> result(PiiMatrix<T>::uninitialized(histogram.rows(), histogram.columns()));
    const int iRows = histogram.rows(), iColumns = histogram.columns();
    for (int r=0; r<iRows; ++r)
      {
        const U* pRow = histogram.row(r);
        T sum = Pii::accumulateN(pRow, iColumns, std::plus<T>(), T(0));
        if (sum != 0)
          Pii::transformN(pRow, iColumns, result.row(r),
                          std::bind2nd(std::multiplies<T>(), 1.0/sum));
        else
          memset(result.row(r), 0, sizeof(T)*iColumns);
      }
    return result;
  }

  template <class T> int percentile(const PiiMatrix<T>& cumulative, T value)
  {
		int start = 0, end = cumulative.columns();
    const T* values = cumulative.row(0);
    // Values in a cumulative distribution are monotonically
    // increasing -> use binary search
		while (start < end)
			{
				int half = (start + end) >> 1;
				if (value <= values[half])
					end = half;
				else
					start = half+1;
			}
		return end < cumulative.columns() ? end : -1;
  }

  template <class T, class U> PiiMatrix<U> backProject(const PiiMatrix<T>& img, const PiiMatrix<U>& histogram)
  {
    PiiMatrix<U> result(PiiMatrix<U>::uninitialized(img.rows(), img.columns()));
    const U* pHistogram = histogram.row(0);
    for (int r=img.rows(); r--; )
      {
        const T* sourceRow = img.row(r);
        U* targetRow = result.row(r);
        for (int c=img.columns(); c--; )
          targetRow[c] = pHistogram[(int)sourceRow[c]];
      }
    return result;
  }

  template <class T, class U> PiiMatrix<U> backProject(const PiiMatrix<T>& ch1, const PiiMatrix<T>& ch2,
                                                       const PiiMatrix<U>& histogram)
  {
    PiiMatrix<U> result(PiiMatrix<U>::uninitialized(ch1.rows(), ch1.columns()));
    for (int r=ch1.rows(); r--; )
      {
        const T* ch1Row = ch1.row(r);
        const T* ch2Row = ch2.row(r);
        U* targetRow = result.row(r);
        for (int c=ch1.columns(); c--; )
          targetRow[c] = histogram((int)ch1Row[c], (int)ch2Row[c]);
      }
    return result;
  }
  
	template <class T> PiiMatrix<T> equalize(const PiiMatrix<T>& img, unsigned int levels)
	{
		PiiMatrix<T> result(img.rows(), img.columns());

    unsigned int maxValue = (unsigned int)Pii::maxAll(img);
    if (levels < maxValue)
      levels = maxValue + 1;
    if (levels == 0)
      return img;
    
		PiiMatrix<int> dist = cumulative(histogram(img, levels));
    int* pDist = dist.row(0);
    
		PiiMatrix<T> newDist(1, levels);
    T* pNewDist = newDist.row(0);
		int sum = pDist[levels-1]; // total number of pixels
    // If the distribution was really equalized, this would be the
    // ideal frequency for each gray level.
    double idealFreq = double(sum)/levels;

    int j = 0;
		// Find out the new value for each pixel.
		for(unsigned int i=0; i<levels; i++)
			{
        int expectedShare = int(idealFreq * (i+1) + 0.5);
        while (pDist[j] < expectedShare)
          pNewDist[j++] = i;
        pNewDist[j] = i;
			}
		return backProject(img, newDist);
	}
	
	/*Matrix<int> HistogramOperation::ContrastStretching::transformedImage(const Matrix<int>& mat)
	{ 
		Matrix<int> result(mat.rows(), mat.columns());
		// if user does not give new values let's find them
		if( 0 == _iOldMin && 0 == _iOldMax)
			{
				_iOldMin = mat.min();
				_iOldMax = mat.max();
			}
		
		List<int> newValues(_iMax+1);
		newValues.setLength(_iMax+1);
		double conversion = (double)(_iMax - _iMin)/(double)(_iOldMax-_iOldMin);

		for(int i=0; i<newValues.length() ; i++)
			{ // checking if the value of i is in the right range 
				if( i >= _iOldMin && i < _iMax )
					newValues[i] = (int)(conversion*(i-_iOldMin)+_iMin);
				else newValues[i] = i;
			}

		// changing the values of pixels
		const int* matData = mat._d();
		int* resultData = result._d();
		
		for(int i=0; i< mat.rows()*mat.columns();i++,matData++,resultData++)
			*resultData = newValues[*matData];// place the new values
			
		return result;
	}

	Matrix<int> HistogramOperation::ZNormalization::transformedImage(const Matrix<int>& mat)
	{
		Matrix<int> result(mat.rows(), mat.columns());

		double oldMean = 0;
		double oldDev = 0;
		// then calculate the old mean value and old Variance
		Math::meanAndVariance(mat, oldMean, oldDev);
    // calculate the old standard deviation
		oldDev = sqrt(oldDev);
   
		int* resultData = result._d();
		const int* sourceData = mat._d();

		for(int i=0;i<result.rows()*result.columns();i++,sourceData++,resultData++)
			{
				int newValue = (int)(_dNewMean+_dNewDev*((double)(*sourceData) - oldMean)/oldDev+0.5);
				// check the limit of newValue
				if(newValue <0) newValue = 0;
				else if(newValue > _iLevels) newValue = _iLevels;

				*resultData = newValue;
			}
		
		return result;
    }*/
}
