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

#ifndef _PIIQIMAGEUTIL_H
#define _PIIQIMAGEUTIL_H

#include <PiiColor.h>
#include <PiiQImage.h>
#include "PiiVariant.h"

/**
 * @file
 *
 * Utility functions for combining QImage with PiiVariant.
 *
 * @ingroup Ydin
 */
namespace PiiYdin
{
  template <class T> QImage* createColorQImage(const PiiVariant& obj)
  {
    return PiiColorQImage::create(obj.valueAs<PiiMatrix<T> >());
  }
  
  template <class T> QImage* createGrayQImage(const PiiVariant& obj)
  {
    return PiiGrayQImage::create(obj.valueAs<PiiMatrix<T> >());
  }

  template <class T> inline QImage* createQImage(const PiiVariant& obj)
  {
    return Pii::createQImage(obj.valueAs<PiiMatrix<T> >());
  }
}

#endif //_PIIQIMAGEUTIL_H
