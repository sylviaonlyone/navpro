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

#ifndef _PIIAVIIVAM2CLSERIALPROTOCOL_H
#define _PIIAVIIVAM2CLSERIALPROTOCOL_H

#include <QList>
#include <QString>
#include "PiiAviivaCLSerialProtocol.h"

/**
 * This class implements the serial protocol for the Aviiva M2 CL camera.
 */
class PiiAviivaM2CLSerialProtocol : public PiiAviivaCLSerialProtocol
{
  Q_OBJECT

public:

  PiiAviivaM2CLSerialProtocol();
  
  /**
   * Implements the corresponding function in the base class. This
   * class knows the following configuration value items:
   *
   * @lip gain - gain of the A/D converter in decibels (double). Valid
   * range of values is between 0.0 and 40.0. Note that when reading
   * the prevously set gain, the amount might not be exaclty the same
   * compared to the value set. This is because the gain is stored in
   * the camer with the precision of 0.047dB (the exact value is
   * 40/851 decibels).
   *
   * @lip busSpeed - the data transfer speed (int). The speed is set
   * as megaherzes. The possible speeds depend of the @p channelCount
   * configuration value. For one channel the following speeds are
   * possible: 20, 30, 40 and 60. For two channels the possible speeds
   * are 20 and 30. For both one and two channels also external clock
   * is possible. When @p busSpeed is set to 0, it means that external
   * clock is used.
   *
   * @lip channelCount - the amount of outputs to transfer the data.
   * The possible values are 1 (multiplexed) and 2. Note, that the
   * channel count affects to the possible values of the @p busSpeed
   * configuration values. For example when the channel count is
   * changed from 1 to 2, and the value of bus speed is 60 MHz, the bus
   * speed is lowered automatically to 30 MHz. Also if the bus speed
   * were 40 MHz, in the same situation the bus speed is lowered to 20
   * MHz. This is done because there is no option for 40 and 60 MHz
   * data rates for two channel counts in Aviiva M2 CL camera.
   *
   * @lip bitsPerPixel - determines the bits per pixel (the output
   * format). The possible values are 8, 10 and 12.
   *
   * @lip integrationTime - determines the integration time (µs) in
   * @p freeRun or @p externalTrigger modes (the @p triggerMode
   * configuration value). Valid value range is from 5 to 13000.
   *
   */
  bool setConfigurationValue(const char* name, const QVariant& value);

  /**
   * Implements the corresponding function in the base class.
   */
  QVariant configurationValue(const char* name) const;

private:

  bool setGain(const QVariant& value);
  double gain() const;
  bool setBusSpeed(const QVariant& speed);
  int busSpeed() const;
  bool setChannelCount(const QVariant& count);
  int channelCount() const;
  bool setBitsPerPixel(const QVariant& bits);
  int bitsPerPixel() const;
  bool setIntegrationTime(const QVariant& time);
  int integrationTime() const;
};


#endif //_PIIAVIIVAM2CLSERIALPROTOCOL_H
