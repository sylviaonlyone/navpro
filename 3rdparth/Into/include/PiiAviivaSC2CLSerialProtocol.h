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

#ifndef _PIIAVIIVASC2CLSERIALPROTOCOL_H
#define _PIIAVIIVASC2CLSERIALPROTOCOL_H

#include <QList>
#include <QString>
#include "PiiAviivaCLSerialProtocol.h"

/**
 * This class implements the serial protocol for the Aviiva SC2 CL camera.
 */
class PiiAviivaSC2CLSerialProtocol : public PiiAviivaCLSerialProtocol
{
  Q_OBJECT

public:

  PiiAviivaSC2CLSerialProtocol();
  
  /**
   * Implements the corresponding function in the base class. This
   * class knows the following configuration value items:
   *
   * @lip gain - gain of the A/D converter in decibels (double). Valid
   * range of values is between -2.0 and 22.0. Note that when reading
   * the prevously set gain, the amount might not be exaclty the same
   * compared to the value set. This is because the gain is stored in
   * the camera with the precision of 0.033898dB (the exact value is
   * 24/708 decibels).
   *
   * @lip busSpeed - the data transfer speed (int). The speed is set
   * as megaherzes. For one channel the following speeds are
   * possible: 20, 30, 40 and 60. Also external clock
   * is possible (values 0,1 and 2). O = external clock x 2, 1 =
   * external clock and 2 = external clock/2.
   *
   * @lip channelCount - the amount of outputs to transfer the data.
   * The possible values are 1 and 3. Note, that the channel count
   * affects to the possible values of the @p bitsPerPixel
   * configuration values.
   *
   * @lip bitsPerPixel - determines the bits per pixel (the output
   * format). The possible values are 8, 10 and 12. Note, that the
   * bits per pixel affects to the possible values of the @p
   * channelCount configuration values.
   *
   * @lip integrationTime - determines the integration time (µs) in
   * @p freeRun or @p externalTrigger modes (the @p triggerMode
   * configuration value). Valid value range is from 1 to 32768.
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


#endif //_PIIAVIIVASC2CLSERIALPROTOCOL_H
