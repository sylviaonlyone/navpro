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

#ifndef _PIIDEFAULTIOCHANNEL_H
#define _PIIDEFAULTIOCHANNEL_H

#include <PiiIoChannel.h>

class PiiDefaultIoDriver;

/**
 * The default implementation of the PiiIoChannel-interface for input/output channels.
 *
 * @ingroup PiiIoPlugin
 */
class PII_IO_EXPORT PiiDefaultIoChannel : public PiiIoChannel
{
  Q_OBJECT
  
  /**
   * Enable input signal. Do nothing if type of this channel is Output.
   */
  Q_PROPERTY(bool signalEnabled READ signalEnabled WRITE setSignalEnabled);

  /**
   * Get index of the channel.
   */
  Q_PROPERTY(int channel READ channelIndex);

  /**
   * The name of the channel.
   */
  Q_PROPERTY(QString channelName READ channelName WRITE setChannelName);

  /**
   * The mode of an channel can assume five differenct values:
   *
   * @lip NoMode - the default state.
   *
   * @lip TransitionInput - notice all transitions.
   *
   * @lip PulseInput - notice only pulses.
   *
   * @lip TransitionOutput - change the output state.
   *
   * @lip PulseOutput - emit the pulse depends on pulseWidth and pulseDelay.
   *
   */
  Q_PROPERTY(ChannelMode channelMode READ channelMode WRITE setChannelMode);
  Q_ENUMS(ChannelMode);
  
  /**
   * The width of an output pulse in milliseconds. This property is
   * effective only if the channel is in @p PulseOutput mode.
   */
  Q_PROPERTY(int pulseWidth READ pulseWidth WRITE setPulseWidth);
  /**
   * Initial delay of an output pulse, in milliseconds. This property
   * is effective in  @p TransitionOutput and @p PulseOutput modes.
   */
  Q_PROPERTY(int pulseDelay READ pulseDelay WRITE setPulseDelay);

  /**
   * Maybe we wan't to decide what is an active state of the pulse.
   * This is the property where you can change the polarity of the
   * channel state. True means, that channel is 1-active and false means
   * that channel is 0-active.
   */
  Q_PROPERTY(bool activeState READ activeState WRITE setActiveState);

  /**
   * Returns the current state of the channel
   */
  Q_PROPERTY(int currentState READ currentState);
    

public:
  ~PiiDefaultIoChannel();
  
  /**
   * Channel mode.
   */
  enum ChannelMode
    {
      NoMode,
      TransitionInput,
      PulseInput,
      TransitionOutput,
      PulseOutput
    };


  /**
   * Check input change. If the current state is changed, we will emit
   * signalReceived() -signal.
   */
  void checkInputChange();

  /**
   * Change output state.
   */
  void changeOutputState(bool /*active*/) {};

  /**
   * Send output signal.
   */
  void sendSignal(double value = 0);

  /**
   * Get a parent driver.
   */
  PiiDefaultIoDriver* driver();

  /**
   * Get and set signal enabled flag.
   */
  void setSignalEnabled(bool signalEnabled);
  bool signalEnabled() const;

  /**
   * Get and set channel name.
   */
  void setChannelName(const QString& channelName);
  QString channelName() const;

  /**
   * Get and set channel mode.
   */
  void setChannelMode(const ChannelMode& channelMode);
  ChannelMode channelMode() const;

  /**
   * Get and set pulse width.
   */
  void setPulseWidth(int pulseWidth);
  int pulseWidth() const;

  /**
   * Get and set pulse delay.
   */
  void setPulseDelay(int pulseDelay);
  int pulseDelay() const;

  /**
   * Get and set active state.
   */
  void setActiveState(bool activeState);
  bool activeState() const;

  /**
   * Get a channel index.
   */
  int channelIndex() const;

protected:
  /**
   * Returns the current state of the channel
   */
  virtual int currentState() const = 0;

  /**
   * Check that channel mode is okay.
   */
  virtual void checkMode() = 0;

  class Data
  {
  public:
    Data();
    virtual ~Data();
    
    PiiDefaultIoDriver *pDriver;
    int iChannelIndex;
    bool bSignalEnabled;
    QString strChannelName;
    ChannelMode channelMode;
    int iPulseWidth;
    int iPulseDelay;
    int iPreviousInputState;
    int iInputStateCounter;
    bool bActiveState;
    bool bCurrentState;
  } *d;

  PiiDefaultIoChannel(Data* data, PiiDefaultIoDriver *driver, int channelIndex);
};

#endif //_PIIDEFAULTIOCHANNEL_H
