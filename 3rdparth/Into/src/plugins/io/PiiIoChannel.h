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

#ifndef _PIIIOCHANNEL_H
#define _PIIIOCHANNEL_H
#include <QObject>
#include <PiiIoDriver.h>

/**
 * An interface for input/output channels.
 *
 * @ingroup PiiIoPlugin
 */
class PII_IO_EXPORT PiiIoChannel : public QObject
{
  Q_OBJECT

public:
  /**
   * Check input change.
   */
  virtual void checkInputChange() = 0;

  /**
   * Change output state.
   */
  virtual void changeOutputState(bool active) = 0;
  
public slots:
  /**
   * Send a signal to an output channel.
   */
  virtual void sendSignal(double value = 0) = 0;
  
signals:
  /**
   * This signal is emitted just after when the channel has received a
   * phycisal signal (transition/pulse) from the device.
   */
  void signalReceived(int channel, double value);

};

#endif //_PIIIOCHANNEL_H
