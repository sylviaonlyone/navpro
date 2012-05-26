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

#ifndef _PIISYNCHRONIZED_H
#define _PIISYNCHRONIZED_H

#include "PiiGlobal.h"
#include "PiiPreprocessor.h"
#include <QMutex>

/// @file

/// @internal
struct PII_CORE_EXPORT PiiMutexLocker
{
  explicit inline PiiMutexLocker(const QMutex& mutex) : pMutex(const_cast<QMutex*>(&mutex)), bLocked(true)
  {
    pMutex->lock();
  }
    
  explicit inline PiiMutexLocker(const QMutex* mutex) : pMutex(const_cast<QMutex*>(mutex)), bLocked(true)
  {
    pMutex->lock();
  }

  ~PiiMutexLocker() { pMutex->unlock(); }

  operator bool () const { return bLocked; }

  void setUnlocked() { bLocked = false; }

  QMutex* pMutex;
  bool bLocked;
};

/// @internal
#define PII_SYNCHRONIZED(LOCK,MUTEX) for (PiiMutexLocker LOCK(MUTEX); LOCK; LOCK.setUnlocked())

/**
 * Declares a critical section in which mutual exclusion is handled by
 * the given @a MUTEX. Critical sections protected by the same mutex
 * won't be executed simultaneously.
 *
 * @code
 * void MyOperation::pause()
 * {
 *   synchronized (stateLock())
 *     {
 *       if (state() == Running)
 *         {
 *           piiDebug("I'm pausing!");
 *           setState(Pausing);
 *         }
 *     }
 * }
 * @endcode
 */
#define synchronized(MUTEX) PII_SYNCHRONIZED(PII_JOIN(piiSyncronizedLock,__LINE__), MUTEX)

#endif //_PIISYNCHRONIZED_H
