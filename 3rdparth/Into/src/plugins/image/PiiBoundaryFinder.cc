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

#include "PiiBoundaryFinder.h"

PiiBoundaryFinder::Data::Data(const PiiTypelessMatrix& objects,
                              PiiMatrix<unsigned char>* boundaryMask) :
  pmatBoundaryMask(boundaryMask),
  iRow(objects.rows()-1),
  iColumn(objects.columns()-1),
  iRightEdge(iColumn)
{
  if (boundaryMask != 0)
    pmatBoundaryMask->resize(objects.rows(), objects.columns());
  else
    {
      matBoundaryMask.resize(objects.rows(), objects.columns());
      pmatBoundaryMask = &matBoundaryMask;
    }
}

PiiBoundaryFinder::PiiBoundaryFinder(const PiiTypelessMatrix& objects,
                                     PiiMatrix<unsigned char>* boundaryMask) :
  d(new Data(objects, boundaryMask))
{
}

PiiBoundaryFinder::~PiiBoundaryFinder()
{
  delete d;
}

PiiMatrix<unsigned char> PiiBoundaryFinder::boundaryMask() const { return d->matBoundaryMask; }
