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

#ifndef _PIIGRADIENTFEATUREOPERATION_H
#define _PIIGRADIENTFEATUREOPERATION_H

#include <PiiDefaultOperation.h>

/**
 * An operation that calculates the maximum vertical and horizontal
 * gradient in a gray-level image.
 *
 * @inputs
 *
 * @in image - input image. Any gray-level image.
 * 
 * @outputs
 *
 * @out features - a 1-by-2 PiiMatrix<float> that contains the maximum
 * absolute value of vertical and horizontal gradient, in this order.
 *
 * @ingroup PiiFeaturesPlugin
 */
class PiiGradientFeatureOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Feature names (read only).
   */
  Q_PROPERTY(QStringList features READ features);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiGradientFeatureOperation();

  QStringList features() const;

protected:
  void process();

private:
  template <class T> void calculateGradient(const PiiVariant& obj);
};


#endif //_PIIGRADIENTFEATUREOPERATION_H
