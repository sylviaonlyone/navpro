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

#include "PiiHistogramBackProjector.h"

#include <PiiYdinTypes.h>
#include <PiiColor.h>
#include <PiiMath.h>
#include "PiiHistogram.h"

PiiHistogramBackProjector::Data::Data() :
  bChannel2Connected(false),
  bModelConnected(false)
{
}

PiiHistogramBackProjector::PiiHistogramBackProjector() :
  PiiDefaultOperation(new Data, Threaded)
{
  addSocket(new PiiInputSocket("image"));
  addSocket(new PiiInputSocket("channel1"));
  addSocket(new PiiInputSocket("model"));
  addSocket(new PiiOutputSocket("image"));
  inputAt(1)->setOptional(true);
  inputAt(2)->setOptional(true);
}

PiiInputSocket* PiiHistogramBackProjector::input(const QString& name) const
{
  // channel0 is an alias for the image input
  if (name == "channel0")
    return inputAt(0);
  return PiiDefaultOperation::input(name);
}

void PiiHistogramBackProjector::check(bool reset)
{
  PII_D;
  d->bChannel2Connected = inputAt(1)->isConnected();
  d->bModelConnected = inputAt(2)->isConnected();
  if (!d->bModelConnected && !d->varModel.isValid())
    PII_THROW(PiiExecutionException, tr("Model input is not connected and model has not been set."));

  d->varTmpModel = d->varModel;
  
  PiiDefaultOperation::check(reset);
}

void PiiHistogramBackProjector::process()
{
  PiiVariant obj = readInput();
  PiiVariant obj2;
  if (_d()->bChannel2Connected)
    {
      obj2 = inputAt(1)->firstObject();
      if (obj2.type() != obj.type())
        PII_THROW(PiiExecutionException, tr("The types of channel images must match in two-dimensional histogram back-projection."));
    }

  // First resolve the type of input
  switch (obj.type())
    {
      // NOTE: if this is ever changed, change the limit check in
      // backProject. It now assumes unsigned types.
      PII_INT_GRAY_IMAGE_CASES_M(backProject, (obj, obj2));
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

// Convert input objects into matrices and resolve the model type
template <class T> void PiiHistogramBackProjector::backProject(const PiiVariant& obj1, const PiiVariant& obj2)
{
  PII_D;
  static const char* errorMsg = QT_TR_NOOP("Model histogram is of an unsupported type.");

  // Read model histogram from input if it is connected
  if (d->bModelConnected)
    d->varTmpModel = inputAt(2)->firstObject();
  
  // Two-dimensional back-projection
  if (obj2.isValid())
    {
      const PiiMatrix<T>& ch1 = obj1.valueAs<PiiMatrix<T> >();
      const PiiMatrix<T>& ch2 = obj2.valueAs<PiiMatrix<T> >();
      if (ch1.columns() != ch2.columns() ||
          ch1.rows() != ch2.rows())
        PII_THROW(PiiExecutionException, tr("The sizes of channel images must match in two-dimensional histogram back-projection."));

      switch (d->varTmpModel.type())
        {
          PII_PRIMITIVE_MATRIX_CASES_M(backProject, (ch1, ch2));
          PII_COLOR_IMAGE_CASES_M(backProject, (ch1, ch2));
        default:
          PII_THROW(PiiExecutionException, tr(errorMsg));
        }
    }
  // One-dimensional back-projection
  else
    {
      const PiiMatrix<T>& image = obj1.valueAs<PiiMatrix<T> >();
      switch (d->varTmpModel.type())
        {
          PII_PRIMITIVE_MATRIX_CASES(backProject, image);
          PII_COLOR_IMAGE_CASES(backProject, image);
        default:
          PII_THROW(PiiExecutionException, tr(errorMsg));
        }
    }
}

// Convert model to matrix and use already resolved input channel matrices
template <class T, class U> void PiiHistogramBackProjector::backProject(const PiiMatrix<U>& ch1, const PiiMatrix<U>& ch2)
{
  static const char* errorMsg = QT_TR_NOOP("The values in channel %1 (%2-%3) exceed model dimensions (0-%4).");
  const PiiMatrix<T> model = _d()->varTmpModel.valueAs<PiiMatrix<T> >();

  // Check that the input matrices index a valid range of rows/columns
  U maxVal =  Pii::maxAll(ch1);
  if (int(maxVal) >= model.rows()) // maxVal >= 0, because U is unsigned
    PII_THROW(PiiExecutionException, tr(errorMsg).arg(0).arg(0).arg(maxVal).arg(model.rows()-1));
  maxVal = Pii::maxAll(ch2);
  if (int(maxVal) >= model.columns())
    PII_THROW(PiiExecutionException, tr(errorMsg).arg(1).arg(0).arg(maxVal).arg(model.columns()-1));
  
  emitObject(PiiImage::backProject(ch1, ch2, model));
}

// Convert model to matrix and use already resolved input matrix
template <class T, class U> void PiiHistogramBackProjector::backProject(const PiiMatrix<U>& image)
{
  const PiiMatrix<T>& model = _d()->varTmpModel.valueAs<PiiMatrix<T> >();
  U maxVal = Pii::maxAll(image);
  if (int(maxVal) >= model.columns())
    PII_THROW(PiiExecutionException, tr("Values in input image (%1-%2) exceed model size (0-%3).").arg(0).arg(maxVal).arg(model.columns()-1));

  emitObject(PiiImage::backProject(image, model));
}

void PiiHistogramBackProjector::setModel(const PiiVariant& model) { _d()->varModel = model; }
PiiVariant PiiHistogramBackProjector::model() const { return _d()->varModel; }
