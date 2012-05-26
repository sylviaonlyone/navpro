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

#ifndef _PIITIMER_H
#define _PIITIMER_H

#include "PiiGlobal.h"

/**
 * PiiTimer uses QTime to achieve microsecond-resolution timing of
 * intervals. The functionality is almost equal to QTime, but this
 * class provides microsecond-resolution timing (on systems that
 * support such resolution) and adds a stop() function that can be
 * used to stop counting.
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiTimer
{
public:
  /**
   * Constructs a new timer that is initialized to the system's
   * current time. The timer starts counting automatically.
   */
  PiiTimer();

  PiiTimer(const PiiTimer& other);

  PiiTimer& operator= (const PiiTimer& other);

  ~PiiTimer();
  
  /**
   * Restarts counting. Returns the number of microseconds elapsed
   * since the last restart or construction of the object, whichever
   * is later.
   */
  qint64 restart();

  /**
   * Stops counting. When the timer is stopped, elapsed time is always
   * 0. One needs to call #restart() to restart counting.
   */
  void stop();

  /**
   * Returns the time elapsed from the construction of this object or
   * the previous restart() call, whichever is later. Time is measured
   * in microseconds. If the timer is stopped, returns 0.
   */
  qint64 microseconds() const;
  
  /**
   * Returns the elapsed time in milliseconds.
   */
  qint64 milliseconds() const;

  /**
   * Returns the elapsed time in seconds.
   */
  double seconds() const;

private:
  class Data;
  Data* d;
};

#endif //_PIITIMER_H
