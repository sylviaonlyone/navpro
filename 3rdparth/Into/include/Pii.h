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

#ifndef _PII_H
#define _PII_H

/** @file */

#include <QObject>
#include "PiiGlobal.h"

/**
 * This namespace contains general-purpose functions, type
 * definitions, and data structures that are independent of other
 * modules and plug-ins.
 *
 * @ingroup Core
 */
#ifdef Q_MOC_RUN
class Pii
#else
namespace Pii
#endif
{
#ifdef Q_MOC_RUN
  Q_GADGET
    
  Q_ENUMS(Interpolation);
public:
#endif
  extern PII_CORE_EXPORT const QMetaObject staticMetaObject;
  
  /**
   * Pointer ownership modes. If the ownership of a pointer is
   * released to the receiver in a function or constructor call, @p
   * ReleaseOwnership is used. If the ownership is retained by the
   * caller, this is indicated by @p RetainOwnership.
   */
  enum PtrOwnership { ReleaseOwnership, RetainOwnership };

  /**
   * Interpolation is needed in many occasions where discrete signals
   * are sampled at arbitrary positions. Many operations work with
   * multiple interpolation modes.
   *
   * @li @p NearestNeighborInterpolation means that the interpolated
   * value is that of the nearest discretized neighbor.
   *
   * @li @p LinearInterpolation means that the interpolated value is
   * built by a weighted sum of the nearest neighbors. With
   * two-dimensional signals, linear interpolation is in fact
   * bi-linear.
   */
  enum Interpolation { NearestNeighborInterpolation, LinearInterpolation };

  /**
   * An enumeration that specifies the direction of operation for
   * certain matrix operations.
   */
  enum MatrixDirection { Horizontally = 1, Vertically = 2 };
  Q_DECLARE_FLAGS(MatrixDirections, MatrixDirection);
  Q_FLAGS(MatrixDirection MatrixDirections);
  
}; // namespace Pii

/// @cond null
Q_DECLARE_OPERATORS_FOR_FLAGS(Pii::MatrixDirections);

#endif //_PII_H
