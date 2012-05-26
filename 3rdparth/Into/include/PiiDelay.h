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

#ifndef _PIIDELAY_H
#define _PIIDELAY_H

#include <QThread>
#include "PiiGlobal.h"

/**
 * A utility class that reveals the protected msleep() member of
 * QThread so that program flow can be delayed anywhere.
 *
 * @code
 * PiiDelay::msleep(1000); // stops for one second
 * @endcode
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiDelay : public QThread
{
public:
  /**
   * Suspend the current thread for @a msec milliseconds.
   */
  static void msleep(int msec);
  /**
   * Suspend the current thread for @a usec microseconds.
   */
  static void usleep(int usec);
};

#endif //_PIIDELAY_H
