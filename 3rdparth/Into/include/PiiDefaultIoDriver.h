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

#ifndef _PIIDEFAULTIODRIVER_H
#define _PIIDEFAULTIODRIVER_H

#include <PiiIoDriver.h>
#include <PiiIoThread.h>
#include <PiiIoDriverException.h>
#include <QVector>

class PiiDefaultIoChannel;

/**
 * The default implementation of the PiiIoDriver-interface for input/output drivers.
 *
 * @ingroup PiiIoPlugin
 */
class PII_IO_EXPORT PiiDefaultIoDriver : public PiiIoDriver
{
public:
  ~PiiDefaultIoDriver();

  /**
   * Initialize the driver. This function must be called before the
   * driver can be accessed
   *
   * @return @p true if the initialization was successful, @p false
   * otherwise.
   */
  bool initialize();

  /**
   * Get a pointer to a PiiIoChannel object that stores the
   * configuration of the given channel.
   *
   * @param channel the channel number, 0 to #getChannelCount() - 1.
   */
  PiiIoChannel* channel(int channel);
  
protected:
  /**
   * Create a PiiIoChannel depends on given channel-index.
   */
  virtual PiiIoChannel* createChannel(int channel) = 0;
  
  class Data
  {
  public:
    Data();
    virtual ~Data();

    QVector<PiiIoChannel*> lstChannels;
  } *d;

  PiiDefaultIoDriver();
  PiiDefaultIoDriver(Data* data);

private:
  friend class PiiDefaultIoChannel;

  /**
   * Send a signal to an output channel.
   *
   * @param channel - the pointer to the output-channel
   * @param value - true = on, false = off
   * @param day - a day number from the unix-timestamp
   * @param msecs - msecs from 00:00:00 of the given day
   * @param pulseWidth - pulse width in msecs.
   */
  void sendSignal(PiiIoChannel *channel, bool value, int day, int msecs, int pulseWidth);

  /**
   * Add the input channel in the polling input list.
   */
  void addPollingInput(PiiIoChannel *input) { _pSendingThread->addPollingInput(input); }

  /**
   * Remove the input channel from the polling input list.
   */
  void removePollingInput(PiiIoChannel *input) { _pSendingThread->removePollingInput(input); }
  
  static int _iInstanceCounter;
  static PiiIoThread *_pSendingThread;

};

#endif //_PIIDEFAULTIODRIVER_H
