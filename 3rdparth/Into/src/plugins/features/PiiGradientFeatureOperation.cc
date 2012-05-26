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

#include "PiiGradientFeatureOperation.h"

#include <PiiYdinTypes.h>
#include <PiiMath.h>

PiiGradientFeatureOperation::PiiGradientFeatureOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("features"));
}

void PiiGradientFeatureOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_GRAY_IMAGE_CASES(calculateGradient, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiGradientFeatureOperation::calculateGradient(const PiiVariant& obj)
{
  const PiiMatrix<T> img = obj.valueAs<PiiMatrix<T> >();

  float fMaxVerticalGradient = 0;
  float fGradient;
  for (int r=img.rows()-1; r--; )
    {
      const T* row0 = img[r];
      const T* row1 = img[r+1];
      for (int c=img.columns(); c--; )
        {
          fGradient = Pii::abs(float(row1[c]) - float(row0[c]));
          if (fGradient > fMaxVerticalGradient)
            fMaxVerticalGradient = fGradient;
        }
    }

  float fMaxHorizontalGradient = 0;
  for (int r=img.rows(); r--; )
    {
      const T* row = img[r];
      for (int c=img.columns()-1; c--; )
        {
          fGradient = Pii::abs(float(row[c+1]) - float(row[c]));
          if (fGradient > fMaxHorizontalGradient)
            fMaxHorizontalGradient = fGradient;
        }
    }
  
  PiiMatrix<float> matFeatures(1, 2,
                               double(fMaxVerticalGradient),
                               double(fMaxHorizontalGradient));
}


QStringList PiiGradientFeatureOperation::features() const { return QStringList() << "VGradient" << "HGradient"; }
