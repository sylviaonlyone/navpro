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

#include "PiiUndistortOperation.h"

#include <PiiYdinTypes.h>

PiiUndistortOperation::Data::Data() :
  interpolation(Pii::LinearInterpolation)
{
  intrinsic.focalLength.x = 1000;
  intrinsic.focalLength.y = 1000;
  intrinsic.center.x = NAN;
  intrinsic.center.y = NAN;
  intrinsic.k1 = 0;
  intrinsic.k2 = 0;
  intrinsic.p1 = 0;
  intrinsic.p2 = 0;
}

PiiUndistortOperation::PiiUndistortOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiOutputSocket("image"));
}

void PiiUndistortOperation::check(bool reset)
{
  PiiDefaultOperation::check(reset);

  PII_D;
  if (Pii::isNan(d->intrinsic.focalLength.x) ||
      Pii::isNan(d->intrinsic.focalLength.y) ||
      Pii::isNan(d->intrinsic.k1) ||
      Pii::isNan(d->intrinsic.k2) ||
      Pii::isNan(d->intrinsic.p1) ||
      Pii::isNan(d->intrinsic.p2))
    PII_THROW(PiiExecutionException, tr("Camera parameters cannot be NaNs."));

  if (reset)
    invalidate();
}

void PiiUndistortOperation::invalidate()
{
  PII_D;
  d->dmatMap.resize(0,0);
  d->imatMap.resize(0,0);
}

void PiiUndistortOperation::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_ALL_IMAGE_CASES(undistort, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiUndistortOperation::undistort(const PiiVariant& obj)
{
  PII_D;
  PiiMatrix<T> matImage(obj.valueAs<PiiMatrix<T> >());

  PiiTypelessMatrix* pMat = d->interpolation == Pii::LinearInterpolation ?
    static_cast<PiiTypelessMatrix*>(&d->dmatMap) :
    static_cast<PiiTypelessMatrix*>(&d->imatMap);

  if (pMat->rows() != matImage.rows() ||
      pMat->columns() != matImage.columns())
    {
      PiiCalibration::CameraParameters intrinsic(_d()->intrinsic);
      if (Pii::isNan(intrinsic.center.x))
        intrinsic.center.x = double(matImage.columns()/2 - 0.5);
      if (Pii::isNan(intrinsic.center.y))
        intrinsic.center.y = double(matImage.rows()/2 - 0.5);
      if (d->interpolation == Pii::LinearInterpolation)
        d->dmatMap = PiiCalibration::undistortMap(matImage.rows(), matImage.columns(), intrinsic);
      else
        d->imatMap = PiiCalibration::undistortMapInt(matImage.rows(), matImage.columns(), intrinsic);
    }
  if (d->interpolation == Pii::LinearInterpolation)
    emitObject(PiiImage::remap(matImage, d->dmatMap));
  else
    emitObject(PiiImage::remap(matImage, d->imatMap));
}


void PiiUndistortOperation::setFocalX(double focalX) { _d()->intrinsic.focalLength.x = focalX; invalidate(); }
double PiiUndistortOperation::focalX() const { return _d()->intrinsic.focalLength.x; }
void PiiUndistortOperation::setFocalY(double focalY) { _d()->intrinsic.focalLength.y = focalY; invalidate(); }
double PiiUndistortOperation::focalY() const { return _d()->intrinsic.focalLength.y; }
void PiiUndistortOperation::setCenterX(double centerX) { _d()->intrinsic.center.x = centerX; invalidate(); }
double PiiUndistortOperation::centerX() const { return _d()->intrinsic.center.x; }
void PiiUndistortOperation::setCenterY(double centerY) { _d()->intrinsic.center.y = centerY; invalidate(); }
double PiiUndistortOperation::centerY() const { return _d()->intrinsic.center.y; }
void PiiUndistortOperation::setRadial2nd(double radial2nd) { _d()->intrinsic.k1 = radial2nd; invalidate(); }
double PiiUndistortOperation::radial2nd() const { return _d()->intrinsic.k1; }
void PiiUndistortOperation::setRadial4th(double radial4th) { _d()->intrinsic.k2 = radial4th; invalidate(); }
double PiiUndistortOperation::radial4th() const { return _d()->intrinsic.k2; }
void PiiUndistortOperation::setTangential1st(double tangential1st) { _d()->intrinsic.p1 = tangential1st; invalidate(); }
double PiiUndistortOperation::tangential1st() const { return _d()->intrinsic.p1; }
void PiiUndistortOperation::setTangential2nd(double tangential2nd) { _d()->intrinsic.p2 = tangential2nd; invalidate(); }
double PiiUndistortOperation::tangential2nd() const { return _d()->intrinsic.p2; }

void PiiUndistortOperation::setCameraParameters(const PiiVariant& cameraParameters)
{
  if (cameraParameters.type() != PiiYdin::DoubleMatrixType)
    return;
  PiiMatrix<double> matParams = cameraParameters.valueAs<PiiMatrix<double> >();
  if (matParams.rows() != 1 || matParams.columns() != 8)
    return;
  PII_D;
  d->intrinsic.focalLength.x = matParams(0);
  d->intrinsic.focalLength.y = matParams(1);
  d->intrinsic.center.x = matParams(2);
  d->intrinsic.center.y = matParams(3);
  d->intrinsic.k1 = matParams(4);
  d->intrinsic.k2 = matParams(5);
  d->intrinsic.p1 = matParams(6);
  d->intrinsic.p2 = matParams(7);
  invalidate();
}

PiiVariant PiiUndistortOperation::cameraParameters() const
{
  const PII_D;
  return PiiVariant(PiiMatrix<double>(1,6,
                                      d->intrinsic.focalLength.x,
                                      d->intrinsic.focalLength.y,
                                      d->intrinsic.center.x,
                                      d->intrinsic.center.y,
                                      d->intrinsic.k1,
                                      d->intrinsic.k2,
                                      d->intrinsic.p1,
                                      d->intrinsic.p2));
}

void PiiUndistortOperation::setInterpolation(Pii::Interpolation interpolation) { _d()->interpolation = interpolation; }
Pii::Interpolation PiiUndistortOperation::interpolation() const { return _d()->interpolation; }
