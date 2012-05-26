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

#ifndef _PIIFFT_TEMPLATES_H
#define _PIIFFT_TEMPLATES_H

#ifndef _PIIFFT_H
# error "Never use <PiiFft-templates.h> directly; include <PiiFft.h> instead."
#endif

#include <cmath>

template <class T> PiiFft<T>::PiiFft() : _trig(new std::complex<T>[37]), _twiddle(new std::complex<T>[37]), _z(new std::complex<T>[37]),
                                         _count(0), _factorCount(0), _prevPrimeRadix(37)
{
  _pi  = T(4*std::atan(1.0));
  c3_1 = T(std::cos(2*_pi/3)-1);
  c3_2 = T(std::sin(2*_pi/3));
  u5   = T(2*_pi/5);
  c5_1 = T((std::cos(u5)+std::cos(2*u5))/2-1);
  c5_2 = T((std::cos(u5)-std::cos(2*u5))/2);
  c5_3 = T(-std::sin(u5));
  c5_4 = T(-(std::sin(u5)+std::sin(2*u5)));
  c5_5 = T((std::sin(u5)-std::sin(2*u5)));
  c8   = T(1/std::sqrt(2.0));

  for ( int i=0; i<20; i++ )
    {
      _actualRadix[i] = 0;
      _sofarRadix[i] = 0;
      _remainRadix[i] = 0;
    }
}

template <class T> PiiFft<T>::~PiiFft()
{
  delete[] _trig;
  delete[] _twiddle;
  delete[] _z;
}

template <class T>
template <class S> PiiMatrix<std::complex<T> > PiiFft<T>::forwardFft(const PiiMatrix<S>& source)
{
  int cols = source.columns();
  int rows = source.rows();

  PiiMatrix<std::complex<T> > result(rows, cols);
  
  if ( cols > 1 )
    {
      for ( int r=0; r<rows; r++ )
        {
          if (!forward1d<S>(source.row(r), result.row(r), cols) )
            return PiiMatrix<std::complex<T> >(1,1);
        }
    }
  else
    result = source;

  if ( rows > 1 )
    {
      if (_tempSource.columns() != rows)
        reallocateBuffers(rows);
      std::complex<T> *src = _tempSource.row(0), *res = _tempResult.row(0);

      for ( int c=0; c<cols; c++ )
        {
          for ( int r=0; r<rows; r++ )
            src[r] = result(r,c);
          
          if ( !forward1d<std::complex<T> >(src, res, rows) )
            return PiiMatrix<std::complex<T> >(1,1);
          for ( int r=0; r<rows; r++ )
            result(r,c) = res[r];
        }
    }
  return result;
}

template <class T> void PiiFft<T>::reallocateBuffers(int len)
{
  _tempSource.resize(1,len);
  _tempResult.resize(1,len);
}

template <class T>
template <class S> PiiMatrix<std::complex<T> > PiiFft<T>::inverseFft(const PiiMatrix<std::complex<S> >& source)
{
  int cols = source.columns();
  int rows = source.rows();
  
  PiiMatrix<std::complex<T> > result(rows, cols);

  if ( cols > 1 )
    {
      for ( int r=0; r<rows; r++ )
        {
          if (!inverse1d<S>(source.row(r), result.row(r), cols))
            return PiiMatrix<std::complex<T> >(1,1);
        }
    }
  else
    result = source;

  if ( rows > 1 )
    {
      if (_tempSource.columns() != rows)
        reallocateBuffers(rows);
      std::complex<T> *src = _tempSource.row(0), *res = _tempResult.row(0);

      for ( int c=0; c<cols; c++ )
        {
          for ( int r=0; r<rows; r++ )
            src[r] = result(r,c);
          
          if ( !inverse1d<T>(src, res, rows) )
            return PiiMatrix<std::complex<T> >(1,1);
          for ( int r=0; r<rows; r++ )
            result(r,c) = res[r];
        }
    }

  return result;
}


template <class T>
template <class S> bool PiiFft<T>::forward1d(const S* source, std::complex<T>* destination, int count)
{
  if (count == 0)
    return false;
  
  if (_count != count)
    {
      factorize(count);
      _count = count;
    }

  _remainRadix[0] = _count;
  _sofarRadix[1] = 1;
  _remainRadix[1] = _count / _actualRadix[1];
  for ( int i = 2; i<=_factorCount; i++ )
    {
      _sofarRadix[i]  = _sofarRadix[i-1] * _actualRadix[i-1]; 
      _remainRadix[i] = _remainRadix[i-1] / _actualRadix[i]; 
    }
  
  reorderSeries(source, destination);

  for ( int i=1; i<=_factorCount; i++)
    synthesizeFft(_sofarRadix[i], _actualRadix[i], _remainRadix[i], destination);

  return true;
  
}

template <class T>
template <class S> bool PiiFft<T>::inverse1d(const std::complex<S>* source, std::complex<T>* destination, int count)
{
  S s;
  
  if ( count == 0 )
    return false;

  std::complex<S> *tmpPtr = new std::complex<S>[count];
  
  for ( int i=count; i--; )
    tmpPtr[i] = std::complex<S>(source[i].real(), -source[i].imag());

  forward1d(tmpPtr, destination, count);
  
  s = 1.0 / count;
  for ( int i=count; i--; )
    {
      destination[i] = (T)s * std::conj(destination[i]);
      //destination[i].real() =  s * destination[i].real();
      //destination[i].imag() = -s * destination[i].imag();
    }

  delete[] tmpPtr;
  
  return true;
}


/********** PRIVATE FUNCTIONS **********/

/**
 * Reorder the series in X to a permuted sequence in Y so that the later step can
 * be done in place, and the final Fft result is in correct order.
 * The series X and Y must be different series!
 */
template <class T>
template <class S> void PiiFft<T>::reorderSeries(const S* source, std::complex<T>* dest)
{
  int i,j,k;
  int *counts = new int[_factorCount+1];
  
  for (i=1; i<=_factorCount; i++)
    counts[i]=0;

  k=0;

  for (i=0; i<=_count-2; i++)
    {
      dest[i] = source[k];
      j = 1;
      k = k + _remainRadix[j];
      counts[1]++;
      while (counts[j] >= _actualRadix[j])
        {
          counts[j] = 0;
          k = k - _remainRadix[j-1] + _remainRadix[j+1];
          j = j + 1;
          counts[j] = counts[j] + 1;
        }
    }
  
  dest[_count-1] = source[_count-1];
  delete[] counts;
}


template <class T> void PiiFft<T>::factorize(int count)
{
  int i = 0, k;
  int factors[20];
  const int iRadixCount = 6;
  const int iRadices[7] = {0,2,3,4,5,8,10};

  if ( count == 1 )
    {
      _factorCount = 1;
      factors[1] = 1;
    }
  else
    _factorCount = 0;


  // Factorise the original series length Count into known factors and rest value
  i = iRadixCount;
  while(count > 1 && i > 0)
    {
      if ( count % iRadices[i] == 0 )
        {
          count = count / iRadices[i];
          _factorCount++;
          factors[_factorCount] = iRadices[i];
        }
      else
        i--;
    }

  // substitute factors 2*8 with more optimal 4*4
  if ( factors[_factorCount] == 2 )
    {
      i = _factorCount - 1;
      while( i > 0 && factors[i] != 8 )
        i--;

      if ( i > 0 )
        {
          factors[_factorCount] = 4;
          factors[i] = 4;
        }
    }

  // Analyse the rest value and see if it can be factored in primes
  if ( count > 1 )
    {
      for ( k = 2; k<std::sqrt((double)count)+1; k++ )
        {
          while (count % k == 0)
            {
              count = count / k;
              _factorCount++;
              factors[_factorCount] = k;
            }
        }

      if ( count > 1)
        {
          _factorCount++;
          factors[_factorCount] = count;
        }
    }

  _actualRadix[0] = 0;
  for ( i=1; i<=_factorCount; i++ )
    _actualRadix[i] = factors[_factorCount - i + 1];
  
}

template <class T> void PiiFft<T>::synthesizeFft(int sofarRadix, int radix, int remainRadix, std::complex<T>* dest)
{
  T omega = 2 * _pi/(T)(sofarRadix*radix);
  std::complex<T> cosSin(std::cos(omega), -std::sin(omega));
  std::complex<T> S(1,0);

  int groupOffset, dataOffset, adr;
  int groupNo, dataNo, blockNo, twNo;

  if ( isPrimeFactor(radix) )
    initializeTrigonomials(radix);
  
  dataOffset = 0;
  groupOffset = dataOffset;
  adr = groupOffset;
  
  for (dataNo=0; dataNo<sofarRadix; dataNo++)
    {
      if (sofarRadix>1)
        {
          _twiddle[0] = std::complex<T>(1.0,0.0);
          _twiddle[1] = S; 
          
          for (twNo=2; twNo<radix; twNo++)
            _twiddle[twNo] = S * _twiddle[twNo-1];
          
          S = cosSin * S;
        }

        for (groupNo=0; groupNo<remainRadix; groupNo++)
          {
            if ((sofarRadix>1) && (dataNo > 0))
              {
                _z[0] = dest[adr];
                blockNo=1;

                do {
                  adr = adr + sofarRadix;
                  _z[blockNo] = _twiddle[blockNo] * dest[adr];
                  blockNo++;
                } while (blockNo < radix);
              }
            else
              {
                for (blockNo=0; blockNo<radix; blockNo++)
                  {
                    _z[blockNo] = dest[adr];
                    adr = adr + sofarRadix;
                  }
              }

            switch(radix)
              {
              case  2  : fft2(_z); break;
              case  3  : fft3(_z); break;
              case  4  : fft4(_z); break;
              case  5  : fft5(_z); break;
              case  8  : fft8(_z); break;
              case 10  : fft10(_z); break;
              default  : fftPrime(radix); break;
              }
            
            adr = groupOffset;
            for (blockNo=0; blockNo<radix; blockNo++)
              {
                dest[adr] = _z[blockNo];
                adr = adr + sofarRadix;
              }
            groupOffset = groupOffset + sofarRadix * radix;
            adr = groupOffset;
          }
        dataOffset++;
        groupOffset = dataOffset;
        adr = dataOffset;
    }
  
}

template <class T> void PiiFft<T>::initializeTrigonomials( int radix )
{
  if ( radix > _prevPrimeRadix )
    {
      delete[] _trig;
      delete[] _twiddle;
      delete[] _z;
      _trig = new std::complex<T>[radix];
      _twiddle = new std::complex<T>[radix];
      _z = new std::complex<T>[radix];
      _prevPrimeRadix = radix;
    }
  
  int i;
  T w;
  std::complex<T> x;

  
  w = 2 * _pi / radix;
  _trig[0] = std::complex<T>(1,0);
  x = std::complex<T>(std::cos(w), -std::sin(w)); 
  _trig[1] = x;
  
  for (i=2; i<radix; i++)
    _trig[i] = x * _trig[i-1];


}

template <class T> inline void PiiFft<T>::fftPrime(int radix)
{
  int i,j,k,n,max;
  std::complex<T> re, im;
  int cols = ((radix+1) / 2);
  std::complex<T> *v = new std::complex<T>[cols];
  std::complex<T> *w = new std::complex<T>[cols];

  n = radix;
  max = (n + 1)/2;
  for (j = 1; j < max; j++)
    {
      v[j] = std::complex<T>(_z[j].real() + _z[n-j].real(), _z[j].imag() - _z[n-j].imag());
      w[j] = std::complex<T>(_z[j].real() - _z[n-j].real(), _z[j].imag() + _z[n-j].imag());
      //v[j].real() = _z[j].real() + _z[n-j].real();
      //v[j].imag() = _z[j].imag() - _z[n-j].imag();
      //w[j].real() = _z[j].real() - _z[n-j].real();
      //w[j].imag() = _z[j].imag() + _z[n-j].imag();
    }

  for (j = 1; j < max; j++)
    {
      _z[j] = _z[0];
      _z[n-j] = _z[0]; 
      k = j;
      for (i = 1; i < max; i++)
        {
          re = std::complex<T>(_trig[k].real() * v[i].real(), _trig[k].real() * w[i].imag());
          im = std::complex<T>(_trig[k].imag() * w[i].real(), _trig[k].imag() * v[i].imag());
          //re.real() = _trig[k].real() * v[i].real();
          //im.imag() = _trig[k].imag() * v[i].imag();
          //re.imag() = _trig[k].real() * w[i].imag();
          //im.real() = _trig[k].imag() * w[i].real();

          _z[n-j] = std::complex<T>(_z[n-j].real() + re.real() + im.imag(), _z[n-j].imag() + re.imag() - im.real());
          _z[j] = std::complex<T>(_z[j].real() + re.real() - im.imag(), _z[j].imag() + re.imag() + im.real());
          
          //_z[n-j].real() += (re.real() + im.imag());
          //_z[n-j].imag() += (re.imag() - im.real());
          //_z[j].real()   += (re.real() - im.imag());
          //_z[j].imag()   += (re.imag() + im.real());
          
          k = k + j;
          if (k >= n)
            k = k - n;
        }
    }

  for ( j = 1; j < max; j++)
    {
      _z[0] = std::complex<T>(_z[0].real() + v[j].real(), _z[0].imag() + w[j].imag() );
      //_z[0].real() += v[j].real(); 
      //_z[0].imag() += w[j].imag();
    }
  delete[] w;
  delete[] v;

}

template <class T> inline void PiiFft<T>::fft2(std::complex<T>* z)
{
  std::complex<T> t1;

  t1 = z[0] + z[1];
  z[1] = z[0] - z[1];
  z[0] = t1;
}

template <class T> inline void PiiFft<T>::fft3(std::complex<T>* z)
{
  std::complex<T> t1, m1, m2, s1;

  t1 = z[1] + z[2];
  z[0] += t1;
  m1 = c3_1 * t1;
  m2 = std::complex<T>(c3_2 * (z[1].imag() - z[2].imag()),c3_2 * (z[2].real() - z[1].real()));
  //m2.real() = c3_2 * (z[1].imag() - z[2].imag());
  //m2.imag() = c3_2 * (z[2].real() - z[1].real());
  s1 = z[0] + m1;
  z[1] = s1 + m2;
  z[2] = s1 - m2;
  
}

template <class T> inline void PiiFft<T>::fft4(std::complex<T>* z)
{
  std::complex<T> t1, t2, m2, m3;
  
  t1 = z[0] + z[2];
  t2 = z[1] + z[3];

  m2 = z[0] - z[2];
  m3 = std::complex<T>(z[1].imag() - z[3].imag(), z[3].real() - z[1].real());
  //m3.real() = z[1].imag() - z[3].imag();
  //m3.imag() = z[3].real() - z[1].real();
  
  z[0] = t1 + t2;
  z[2] = t1 - t2;
  z[1] = m2 + m3;
  z[3] = m2 - m3;
}

template <class T> inline void PiiFft<T>::fft5(std::complex<T>* z)
{
  std::complex<T> t1, t2, t3, t4, t5;
  std::complex<T> m1, m2, m3, m4, m5;
  std::complex<T> s1, s2, s3, s4, s5;

  t1 = z[1] + z[4];
  t2 = z[2] + z[3];
  t3 = z[1] - z[4];
  t4 = z[3] - z[2];

  t5 = t1 + t2;
  z[0] = z[0] + t5;
  m1 = c5_1 * t5;
  m2 = c5_2 * (t1 - t2);

  m3 = std::complex<T>(-c5_3 * (t3.imag() + t4.imag()), c5_3 * (t3.real() + t4.real()));
  m4 = std::complex<T>(-c5_4 * t4.imag(), c5_4 * t4.real() );
  m5 = std::complex<T>(-c5_5 * t3.imag(), c5_5 * t3.real());
  //m3.real() = -c5_3 * (t3.imag() + t4.imag()); 
  //m3.imag() =  c5_3 * (t3.real() + t4.real());
  //m4.real() = -c5_4 * t4.imag();
  //m4.imag() =  c5_4 * t4.real();
  //m5.real() = -c5_5 * t3.imag();
  //m5.imag() =  c5_5 * t3.real();
  
  s3 = m3 - m4;
  s5 = m3 + m5;
  s1 = z[0] + m1;
  s2 = s1 + m2;
  s4 = s1 - m2;
  
  z[1] = s2 + s3;
  z[2] = s4 + s5;
  z[3] = s4 - s5;
  z[4] = s2 - s3;  
}

template <class T> inline void PiiFft<T>::fft8(std::complex<T>* z)
{
  T gem;

  _a[0] = z[0];
  _a[1] = z[2];
  _a[2] = z[4];
  _a[3] = z[6];

  _b[0] = z[1];
  _b[1] = z[3];
  _b[2] = z[5];
  _b[3] = z[7];  
    
  fft4(_a);
  fft4(_b);
   
  gem = c8 * (_b[1].real() + _b[1].imag());
  _b[1] = std::complex<T>(gem, c8 * (_b[1].imag() - _b[1].real()));
  //_b[1].imag() = c8 * (_b[1].imag() - _b[1].real());
  //_b[1].real() = gem;
  
  gem = _b[2].imag();
  _b[2] = std::complex<T>(gem, -_b[2].real());
  //_b[2].imag() = -_b[2].real();
  //_b[2].real() = gem;

  gem = c8 * (_b[3].imag() - _b[3].real());
  _b[3] = std::complex<T>(gem, -c8 * (_b[3].real() + _b[3].imag()));
  //_b[3].imag() = -c8 * (_b[3].real() + _b[3].imag());
  //_b[3].real() = gem;

  z[0] = _a[0] + _b[0];
  z[1] = _a[1] + _b[1];
  z[2] = _a[2] + _b[2];
  z[3] = _a[3] + _b[3];

  z[4] = _a[0] - _b[0];
  z[5] = _a[1] - _b[1];
  z[6] = _a[2] - _b[2];
  z[7] = _a[3] - _b[3];
  
}

template <class T> inline void PiiFft<T>::fft10(std::complex<T>* z)
{
  
  _a[0] = z[0];
  _a[1] = z[2];
  _a[2] = z[4];
  _a[3] = z[6];
  _a[4] = z[8];

  _b[0] = z[5];
  _b[1] = z[7];
  _b[2] = z[9];
  _b[3] = z[1];
  _b[4] = z[3];
  
  fft5(_a);
  fft5(_b);

  z[0] = _a[0] + _b[0];
  z[6] = _a[1] + _b[1];
  z[2] = _a[2] + _b[2];
  z[8] = _a[3] + _b[3];
  z[4] = _a[4] + _b[4];
  z[5] = _a[0] - _b[0];
  z[1] = _a[1] - _b[1];
  z[7] = _a[2] - _b[2];
  z[3] = _a[3] - _b[3];
  z[9] = _a[4] - _b[4];

}

template <class T> bool PiiFft<T>::isPrimeFactor( int radix )
{
  switch(radix)
    {
    case 2:
    case 3:
    case 4:
    case 5:
    case 8:
    case 10:
      return false;
      break;
    default:
      return true;
    }
}


#endif //_PIIFFT_TEMPLATES_H
