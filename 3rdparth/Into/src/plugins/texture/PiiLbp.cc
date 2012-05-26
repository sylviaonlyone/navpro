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

#include <PiiMath.h>
#include "PiiLbp.h"

#include <iostream>
using namespace Pii;

PiiLbp::Data::Data(int samples, double radius,
                   PiiLbp::Mode mode, Pii::Interpolation interpolation) :
  iSamples(samples), dRadius(radius), mode(mode),
  interpolation(interpolation),
  pLookup(0), pPoints(0)
{
}

PiiLbp::Data::~Data()
{
  delete[] pPoints;
  delete[] pLookup;
}

PiiLbp::PiiLbp(int samples, double radius,
               Mode mode, Pii::Interpolation interpolation) :
  d(new Data(samples, radius, mode, interpolation))
{
  d->update();
}

PiiLbp::~PiiLbp()
{
  delete d;
}

void PiiLbp::setParameters(int samples, double radius, Mode mode, Pii::Interpolation interpolation)
{
  d->iSamples = samples;
  d->dRadius = radius;
  d->mode = mode;
  d->interpolation = interpolation;
  d->update();
}

int PiiLbp::featureCount(int samples, Mode mode)
{
  switch (mode)
    {
    case Standard:
      return 1 << samples;
    case Uniform:
      return samples*(samples-1)+3;
    case RotationInvariant:
      {
        /* PENDING
         * Find out what this means and comment.
         */
        int *patterns = new int[samples];
        memset(patterns, 0, samples*sizeof(int));
        patterns[0] = 2;
        int result = 2;
        for (int i=2; i<=samples; i++)
          {
            if (samples % i) continue;
            int sum = 2;
            for (int j=2; j<i; j++)
              if (!(i % j) || i == samples)
                sum += patterns[j-1];
            patterns[i-1] = (1 << i) - sum;
            result += patterns[i-1] / i;
          }
        delete[] patterns;
        return result;
      }
    case UniformRotationInvariant:
      return samples+2;
    case Symmetric:
      return 1 << (samples >> 1);
    }
  return 0;
}

unsigned short* PiiLbp::createLookupTable(int samples, Mode mode)
{
  int iLast = featureCount(samples, mode)-1;
  //standard mode needs no look-up table, and ushort limits the possible values
  if (mode == Standard || mode == Symmetric || iLast > USHRT_MAX)
    return 0;

  unsigned short *newMap = new unsigned short[1<<samples];
  unsigned short index = 0, last = (unsigned short)iLast;
  switch (mode)
    {
    case Standard:
    case Symmetric:
      break;
    case Uniform:
      for (unsigned int i=0; i< 1u<<samples; i++)
        {
          if (countTransitions(i,samples) <= 2) //uniform
            newMap[i] = index++;
          else
            newMap[i] = last;
        }
      break;
      
    case RotationInvariant:
      {
        unsigned short *tmpMap = new unsigned short[1<<samples];
        int newMax = 0;
        for (int i=0;i< 1<<samples; i++)
          tmpMap[i] = USHRT_MAX;
        for (unsigned int i=0; i< 1u<<samples; i++)
          {
            int rotMin = rotateToMinimum(i,samples);
            if (tmpMap[rotMin] == USHRT_MAX)
              tmpMap[rotMin] = newMax++;
            
            newMap[i] = tmpMap[rotMin];
          }
        delete[] tmpMap;
      }
      break;

    case UniformRotationInvariant:
      for (unsigned int i=0; i< 1u<<samples; i++)
        {
          if (countTransitions(i,samples) <= 2) //uniform
            newMap[i] = countOnes(i,samples);
          else //non-uniform -> put into miscellaneous
            newMap[i] = last;
        }
      break;
    }
  return newMap;
}

void PiiLbp::Data::update()
{
  delete[] pPoints;
  pPoints = new InterpolationPoint[iSamples];
  
  //initialize _iSamples samples evenly spaced on a circle
  double step = 2 * M_PI / iSamples;
  double angle = 0;

  //The samples are initialized so that _pPoints[0] refers to the last
  //sample and _pPoints[_iSamples-1] to the first one. Y coordinates
  //are negated because matrices are addressed top to bottom.
  for (int i=iSamples; i--; angle += step)
    {
      double tmpX = dRadius * cos(angle);
      double tmpY = -dRadius * sin(angle);
      //The (integer) coordinates of the pixel the upper left corner
      //of this sample hits.
      pPoints[i].x = (int)floor(tmpX);
      pPoints[i].y = (int)floor(tmpY);
      //Points for nearest neighbor "interpolation"
      pPoints[i].nearestX = Pii::round<int>(tmpX);
      pPoints[i].nearestY = Pii::round<int>(tmpY);
      //The location of the upper left corner within this pixel
      double offsetX = tmpX - pPoints[i].x;
      double offsetY = tmpY - pPoints[i].y;

      //Get rid of errors caused by finite-precision arithmetic
      if (Pii::abs(offsetX) < 1.0e-10)
        offsetX = 0;
      else if (Pii::abs(offsetX-1) < 1.0e-10)
        {
          offsetX = 0;
          ++pPoints[i].x;
        }
      if (Pii::abs(offsetY) < 1.0e-10)
        offsetY = 0;
      else if (Pii::abs(offsetY-1) < 1.0e-10)
        {
          offsetY = 0;
          ++pPoints[i].y;
        }

      //qDebug("pPoints[%i] = (%d + %lf, %d + %lf)", i, pPoints[i].x, offsetX, pPoints[i].y, offsetY);
                        
      double dx = 1-offsetX;
      double dy = 1-offsetY;

      //interpolation coefficients for bilinear interpolation
      pPoints[i].coeffs[0] = (float)(dx*dy);
      pPoints[i].coeffs[1] = (float)(offsetX*dy);
      pPoints[i].coeffs[2] = (float)(dx*offsetY);
      pPoints[i].coeffs[3] = (float)(offsetX*offsetY);
    }

  delete[] pLookup;
  pLookup = PiiLbp::createLookupTable(iSamples, mode);
}

int PiiLbp::sampleCount() const
{
  return d->iSamples;
}

double PiiLbp::neighborhoodRadius() const
{
  return d->dRadius;
}

PiiLbp::Mode PiiLbp::mode() const
{
  return d->mode;
}

Pii::Interpolation PiiLbp::interpolation() const
{
  return d->interpolation;
}
