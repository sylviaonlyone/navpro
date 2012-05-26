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

#ifndef _PIIPROGRESSCONTROLLER_H
#define _PIIPROGRESSCONTROLLER_H

#include "PiiGlobal.h"
#include "PiiMathDefs.h"

/**
 * An interface for objects that control the functioning of operations
 * that take a long time.
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiProgressController
{
public:
  virtual ~PiiProgressController();

  /**
   * Check if the slow operation is still allowed to continue. The
   * implementation should return @p true if the operation is still
   * allowed to run and @p false otherwise.
   *
   * @param progressPercentage an estimate of the current state of
   * progress as a percentage (0.0 - 1.0). If the value is @p NaN, the
   * current state of progress is not known to the caller.
   */
  virtual bool canContinue(double progressPercentage = NAN) = 0;
};


#endif //_PIIPROGRESSCONTROLLER_H
