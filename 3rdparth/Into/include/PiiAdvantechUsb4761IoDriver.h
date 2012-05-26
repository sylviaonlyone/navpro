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

#ifndef _PIIADVANTECHUSB4761IODRIVER_H
#define _PIIADVANTECHUSB4761IODRIVER_H

#include "PiiDefaultIoDriver.h"
#include "PiiAdvantechIoDriverGlobal.h"
#include <PiiAdvantechUsb4761IoChannel.h>
#include <PiiIoChannel.h>

#include <driver.h>
#undef C

/**
 * An implementation of the PiiIoChannel-interface for Advantech
 * Usb-4761 device.
 *
 * @ingroup PiiAdvantechIoDriverPlugin
 */
class PII_ADVANTECHIODRIVER_EXPORT PiiAdvantechUsb4761IoDriver : public PiiDefaultIoDriver
{
  Q_OBJECT

public:
  PiiAdvantechUsb4761IoDriver();
  ~PiiAdvantechUsb4761IoDriver();
  
  /**
   * Select the I/O unit to use. The driver may be able to handle many
   * I/O boards. The active unit is selected by a generic unit id. 
   * Possibilities include, for example, network addresses and numeric
   * board indices.
   *
   * @return @p true if the selection was successful, @p false
   * otherwise.
   */
  bool selectUnit(const QString& unit);

  /**
   * Close an initialized driver. After @p close(), #initialize() must
   * be called again before the driver is functional.
   *
   * @return @p true on success, @p false otherwise
   */
  bool close();

  /**
   * Get the total number of I/O channels.
   */
  int channelCount() const;
  
  /**
   * Initialize the driver. This function is must be called before the
   * driver can be accessed
   *
   * @return @p true if the initialization was successful, @p false
   * otherwise.
   */
  bool initialize();

protected:
  /**
   * Create a new PiiIoChannel.
   *
   * @param channel the channel number, 0 to #channelCount() - 1.
   */
  PiiIoChannel* createChannel(int channel);

private:
  /// @internal
  class Data : public PiiDefaultIoDriver::Data
  {
  public:
    Data();

    int iBoard;
    LONG driverHandle;
  };
  PII_D_FUNC;
};


#endif //_PIIADVANTECHUSB4761IODRIVER_H
