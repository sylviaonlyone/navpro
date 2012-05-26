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

#ifndef _PIIIODRIVER_H
#define _PIIIODRIVER_H

#include "PiiIoGlobal.h"
#include <QObject>
class PiiIoChannel;

/**
 * An interface for input/output drivers.
 *
 * @ingroup PiiIoPlugin
 */
class PII_IO_EXPORT PiiIoDriver : public QObject
{
  Q_OBJECT

public:
  /**
   * Select the I/O unit to use. The driver may be able to handle many
   * I/O boards. The active unit is selected by a generic unit id. 
   * Possibilities include, for example, network addresses and numeric
   * board indices.
   *
   * @return @p true if the selection was successful, @p false
   * otherwise.
   */
  virtual bool selectUnit(const QString& unit) = 0;

  /**
   * Initialize the driver. This function is must be called before the
   * driver can be accessed
   *
   * @return @p true if the initialization was successful, @p false
   * otherwise.
   */
  virtual bool initialize() = 0;

  /**
   * Close an initialized driver. After @p close(), #initialize() must
   * be called again before the driver is functional. The default
   * implementation returns @p true.
   *
   * @return @p true on success, @p false otherwise
   */
  virtual bool close() { return true; }

  /**
   * Get the total number of I/O channels.
   */
  virtual int channelCount() const = 0;

  /**
   * Get a pointer to a PiiIoChannel object that stores the
   * configuration of the given channel.
   *
   * @param channel the channel number, 0 to #getChannelCount() - 1.
   */
  virtual PiiIoChannel* channel(int channel) = 0;

};

PII_SERIALIZATION_NAME(PiiIoDriver);

#endif //_PIIIODRIVER_H
