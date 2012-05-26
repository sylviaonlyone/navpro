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

#ifndef _PIICURRERAIOCHANNEL_H
#define _PIICURRERAIOCHANNEL_H

#include "PiiDefaultIoChannel.h"
#include "PiiCurreraIoDriverGlobal.h"

#include <xiApiExt.h>

class PiiCurreraIoDriver;

/**
 * I/O channel accessed through the Currera I/O-driver.
 *
 * @ingroup PiiIoPlugin
 */
class PII_CURRERAIODRIVER_EXPORT PiiCurreraIoChannel : public PiiDefaultIoChannel
{
  Q_OBJECT

public:
  PiiCurreraIoChannel(PiiCurreraIoDriver *parent, int channelIndex, HANDLE driverHandle);

  /**
   * Change output state.
   */
  void changeOutputState(bool active);
  
protected:
  /**
   * Returns the current state of the channel. 1 = High, 0 = Low. -1
   * means that we couldn't get the real state.
   */
  int currentState() const;

  /**
   * Check that channel mode is okay.
   */
  void checkMode();

private:
  /// @internal
  class Data : public PiiDefaultIoChannel::Data
  {
  public:
    Data(HANDLE handle);
    
    HANDLE driverHandle;
  };
  PII_D_FUNC;
};


#endif //_PIICURRERAIOCHANNEL_H
