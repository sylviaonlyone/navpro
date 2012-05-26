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

#include "PiiImageUnwarpOperation.h"
#include "PiiImageDistortions.h"

#include <PiiYdinTypes.h>

PiiImageUnwarpOperation::Data::Data() :
  dFocalLength(1e100),
  dCameraDistance(1),
  dRadius(0),
  dMaxSectorAngle(0),
  dCenter(NAN),
  bRadiusConnected(false),
  bDistanceConnected(false),
  bCenterConnected(false)
{
}

PiiImageUnwarpOperation::PiiImageUnwarpOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  addSocket(d->pImageInput = new PiiInputSocket("image"));
  addSocket(d->pRadiusInput = new PiiInputSocket("radius"));
  addSocket(d->pDistanceInput = new PiiInputSocket("distance"));
  addSocket(d->pCenterInput = new PiiInputSocket("center"));
  d->pRadiusInput->setOptional(true);
  d->pDistanceInput->setOptional(true);
  d->pCenterInput->setOptional(true);
  
  addSocket(d->pImageOutput = new PiiOutputSocket("image"));
  addSocket(d->pRadiusOutput = new PiiOutputSocket("radius"));
  addSocket(d->pDistanceOutput = new PiiOutputSocket("distance"));
  addSocket(d->pAngleOutput = new PiiOutputSocket("start angle"));
  addSocket(d->pSectorOutput = new PiiOutputSocket("sector"));
  addSocket(d->pScaleOutput = new PiiOutputSocket("scale"));
}

void PiiImageUnwarpOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  d->bRadiusConnected = d->pRadiusInput->isConnected();
  d->bDistanceConnected = d->pDistanceInput->isConnected();
  d->bCenterConnected = d->pCenterInput->isConnected();
}

void PiiImageUnwarpOperation::process()
{
  PII_D;
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_ALL_IMAGE_CASES(unwarp, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(d->lstInputs[0]);
    }
}

template <class T> void PiiImageUnwarpOperation::unwarp(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> image = obj.valueAs<PiiMatrix<T> >();
  double
    dRadius = d->dRadius,
    dDistance = d->dCameraDistance,
    dSector = d->dMaxSectorAngle / 180 * M_PI,
    dStartAngle = 0,
    dCenter = d->dCenter;

  if (d->bRadiusConnected)
    dRadius = PiiYdin::primitiveAs<double>(d->pRadiusInput);
  if (d->bCenterConnected)
    dCenter = PiiYdin::primitiveAs<double>(d->pCenterInput);
  if (d->bDistanceConnected)
    dDistance = PiiYdin::primitiveAs<double>(d->pDistanceInput);

  PiiMatrix<T> matResult(PiiImage::unwarpCylinder(image,
                                                  d->dFocalLength,
                                                  dCenter,
                                                  &dDistance,
                                                  &dRadius,
                                                  &dSector,
                                                  &dStartAngle));
  d->pImageOutput->emitObject(matResult);

  d->pScaleOutput->emitObject(PiiMatrix<double>(1,2,
                                                dSector / matResult.columns(),
                                                (dDistance - dRadius) / d->dFocalLength));
  d->pDistanceOutput->emitObject(dDistance);
  d->pRadiusOutput->emitObject(dRadius);
  d->pAngleOutput->emitObject(dStartAngle);
  d->pSectorOutput->emitObject(dSector);
}

void PiiImageUnwarpOperation::setFocalLength(double focalLength) { _d()->dFocalLength = focalLength; }
double PiiImageUnwarpOperation::focalLength() const { return _d()->dFocalLength; }
void PiiImageUnwarpOperation::setCameraDistance(double cameraDistance) { _d()->dCameraDistance = cameraDistance; }
double PiiImageUnwarpOperation::cameraDistance() const { return _d()->dCameraDistance; }
void PiiImageUnwarpOperation::setRadius(double radius) { _d()->dRadius = radius; }
double PiiImageUnwarpOperation::radius() const { return _d()->dRadius; }
void PiiImageUnwarpOperation::setMaxSectorAngle(double maxSectorAngle) { _d()->dMaxSectorAngle = maxSectorAngle; }
double PiiImageUnwarpOperation::maxSectorAngle() const { return _d()->dMaxSectorAngle; }
void PiiImageUnwarpOperation::setCenter(double center) { _d()->dCenter = center; }
double PiiImageUnwarpOperation::center() const { return _d()->dCenter; }
