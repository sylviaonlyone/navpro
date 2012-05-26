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

#include "PiiFeaturePointMatcher.h"

namespace PiiMatching
{
  Match::Data::Data() :
    iRefCount(1),
    iModelIndex(-1)
  {}

  Match::Data::Data(int modelIndex,
                    const PiiMatrix<double> transformParams,
                    const QList<QPair<int,int> >& matchedPoints) :
    iRefCount(1),
    iModelIndex(modelIndex),
    matTransformParams(transformParams),
    lstMatchedPoints(matchedPoints)
  {}
  
  Match::Match() :
    d(new Data)
  {}
  
  Match::Match(int modelIndex,
               const PiiMatrix<double> transformParams,
               const QList<QPair<int,int> >& matchedPoints) :
    d(new Data(modelIndex, transformParams, matchedPoints))
  {}
      
  Match::Match(const Match& other) :
    d(other.d)
  {
    d->iRefCount.ref();
  }

  Match::~Match()
  {
    if (!d->iRefCount.deref())
      delete d;
  }

  Match& Match::operator= (const Match& other)
  {
    other.d->iRefCount.ref();
    if (!d->iRefCount.deref())
      delete d;
    d = other.d;
    return *this;
  }

  int Match::modelIndex() const { return d->iModelIndex; }
  PiiMatrix<double> Match::transformParams() const { return d->matTransformParams; }
  QList<QPair<int,int> > Match::matchedPoints() const { return d->lstMatchedPoints; }
  int Match::matchedPointCount() const { return d->lstMatchedPoints.size(); }
}
