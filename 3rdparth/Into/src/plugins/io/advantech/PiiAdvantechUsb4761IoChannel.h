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

#ifndef _PIIADVANTECHUSB4761IOCHANNEL_H
#define _PIIADVANTECHUSB4761IOCHANNEL_H

#include "PiiDefaultIoChannel.h"
#include "PiiAdvantechIoDriverGlobal.h"

#include <driver.h>
#undef C

class PiiAdvantechUsb4761IoDriver;

/**
 * I/O channel accessed through the AdvantechUsb4761 I/O-driver.
 *
 * @ingroup PiiIoPlugin
 */
class PII_ADVANTECHIODRIVER_EXPORT PiiAdvantechUsb4761IoChannel : public PiiDefaultIoChannel
{
  Q_OBJECT

public:
  PiiAdvantechUsb4761IoChannel(PiiAdvantechUsb4761IoDriver *parent, int channelIndex, LONG driverHandle);

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
    Data(LONG handle);
    
    LONG driverHandle;
  };
  PII_D_FUNC;
};


#endif //_PIIADVANTECHUSB4761IOCHANNEL_H
