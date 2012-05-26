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

#ifndef _PIIMATRIXFLIPPER_H
#define _PIIMATRIXFLIPPER_H

#include <PiiDefaultOperation.h>

/**
 * Flip a matrix vertically or horizontally.
 *
 * @inputs
 *
 * @in input - any matrix
 * 
 * @outputs
 *
 * @out output - the input matrix flipped either vertically or
 * horizontally.
 *
 * @ingroup PiiBasePlugin
 */
class PiiMatrixFlipper : public PiiDefaultOperation
{
  Q_OBJECT
  /**
   * Flip mode. Default is @p FlipHorizontally.
   */
  Q_PROPERTY(FlipMode flipMode READ flipMode WRITE setFlipMode);
  Q_ENUMS(FlipMode);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Flip modes.
   *
   * @lip FlipHorizontally - retain row order but turn data on each
   * row around.
   *
   * @lip FlipVertically - retain column order but turn rows around.
   *
   * @lip FlipBoth - flip horizontally and then vertically (same as
   * 180 degrees rotation)
   */
  enum FlipMode { FlipHorizontally, FlipVertically, FlipBoth };
  
  PiiMatrixFlipper();

protected:
  void process();

  void setFlipMode(const FlipMode& flipMode);
  FlipMode flipMode() const;

private:
  template <class T> void flip(const PiiVariant& obj);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    FlipMode flipMode;
  };
  PII_D_FUNC;
};


#endif //_PIIMATRIXFLIPPER_H
