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

#include "PiiMatrixFlipper.h"

#include <PiiYdinTypes.h>
#include <PiiMatrixUtil.h>

PiiMatrixFlipper::Data::Data() :
  flipMode(FlipHorizontally)
{
}

PiiMatrixFlipper::PiiMatrixFlipper() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

void PiiMatrixFlipper::process()
{
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_ALL_MATRIX_CASES(flip, obj);
      PII_COLOR_IMAGE_CASES(flip, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
}

template <class T> void PiiMatrixFlipper::flip(const PiiVariant& obj)
{
  PII_D;
  int direction;
  switch (d->flipMode)
    {
    case FlipBoth:
      direction = Pii::Horizontally | Pii::Vertically;
      break;
    case FlipHorizontally:
      direction = Pii::Horizontally;
      break;
    default:
      direction = Pii::Vertically;
      break;
    }
  emitObject(Pii::flipped(obj.valueAs<PiiMatrix<T> >(), (Pii::MatrixDirections)direction));
}

void PiiMatrixFlipper::setFlipMode(const FlipMode& flipMode) { _d()->flipMode = flipMode; }
PiiMatrixFlipper::FlipMode PiiMatrixFlipper::flipMode() const { return _d()->flipMode; }
