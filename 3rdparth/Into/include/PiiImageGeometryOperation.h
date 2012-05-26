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

#ifndef _PIIIMAGEGEOMETRYOPERATION_H
#define _PIIIMAGEGEOMETRYOPERATION_H

#include <PiiDefaultOperation.h>

#define PII_IMAGE_GEOMETRY_OPERATION_FEATURECNT 4

/**
 * Calculates simple features based on image geometry.
 *
 * @inputs
 *
 * @in image - input image. Any matrix.
 * 
 * @outputs
 *
 * @out features - a feature vector (1x4 PiiMatrix<double>) containing
 * the width, height, area (width*height) and aspect ratio
 * (width/height) of the image.
 *
 * @ingroup PiiFeaturesPlugin
 */
class PiiImageGeometryOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Return all the features that are supported by this operation. In
   * the current version of the operation the supported features can only be
   * read, not set.
   */
  Q_PROPERTY(QStringList features READ features);  

  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiImageGeometryOperation();
  
  QStringList features() const;
 
protected:
  void process();

private:
  template <class T> void calculateFeatures(const PiiVariant& obj);

  static const char* _pFeatureNames[PII_IMAGE_GEOMETRY_OPERATION_FEATURECNT];
};

#endif //_PIIIMAGEGEOMETRYOPERATION_H
