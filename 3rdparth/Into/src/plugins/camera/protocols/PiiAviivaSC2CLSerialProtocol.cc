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


#include "PiiAviivaSC2CLSerialProtocol.h"
#include <cstdlib> // for free(), malloc()
#include <string.h> // for strcmp()
#include <QRegExp>
#include <QStringList>
#include <QtDebug>
#include <PiiDelay.h>


// Defines the ratio between the desibels and the value of the gain
// parameter in G-command in the serial protocol
#define AVIIVA_SC2_4010_GAIN_DB_TO_VALUE_RATIO 0.033898305

PiiAviivaSC2CLSerialProtocol::PiiAviivaSC2CLSerialProtocol()
{
  addNonEnumeratedItem("gain",
                       "Gain in decibels",
                       QVariant::Double,
                       -2.0,
                       22.0);

  addEnumeratedItem("busSpeed",
                    "Bus speed in MHz",
                    QVariant::Int,
                    QVariantList() << 0 << 1 << 2 << 20 << 30 << 40 << 60,
                    QStringList() << "2 x external clock" << "external clock" << "external clock/2" << "20" << "30" << "40" << "60");

  addNonEnumeratedItem("channelCount",
                       "Channel count",
                       QVariant::Int,
                       1,
                       3);

  addEnumeratedItem("bitsPerPixel",
                    "Bits per pixel",
                    QVariant::Int,
                    QVariantList() << 0 << 8 << 10 << 12,
                    QStringList() << "RGB parallel mode (3x8 bits)" << "RGB serial 8-bit" << "RGB serial 10-bit" << "RGB serial 12-bit");

  addNonEnumeratedItem("integrationTime",
                       "Integration time in µs",
                       QVariant::Int,
                       1,
                       32768);

}

bool PiiAviivaSC2CLSerialProtocol::setConfigurationValue(const char* name, const QVariant& value)
{
  if (device() == 0)
    return false;
  
  if (strcmp(name, "gain")==0)
    return setGain(value);
  else if (strcmp(name, "busSpeed")==0)
    return setBusSpeed(value);
  else if (strcmp(name, "bitsPerPixel")==0)
    return setBitsPerPixel(value);
  else if (strcmp(name, "channelCount")==0)
    return setChannelCount(value);
  else if (strcmp(name, "integrationTime")==0)
    return setIntegrationTime(value);
  else return PiiAviivaCLSerialProtocol::setConfigurationValue(name,value);
}

QVariant PiiAviivaSC2CLSerialProtocol::configurationValue(const char* name) const
{
  if (strcmp(name, "gain")==0)
    return gain();
  else if (strcmp(name, "busSpeed")==0)
    return busSpeed();
  else if (strcmp(name, "bitsPerPixel")==0)
    return bitsPerPixel();
  else if (strcmp(name, "channelCount")==0)
    return channelCount();
  else if (strcmp(name, "integrationTime")==0)
    return integrationTime();
  else return PiiAviivaCLSerialProtocol::configurationValue(name);
}


/* Private function, which sets the gain of the A/D convertion. The
 * gain is set as desibels, which is a double converted to QVariant.
 */
bool PiiAviivaSC2CLSerialProtocol::setGain(const QVariant& value)
{
  // The format of the command setting the gain is "G=X", where X is
  // an integer value between -65 and 643, which corresponds the
  // gain between -2 and 22 dB, with 0.0338983dB steps. 

  double dDesibels = value.toDouble();
  if (dDesibels < -2.0 || dDesibels > 22.0)
    return false;
  
  // Rounded to the nearest integer.
  int param = int(dDesibels / AVIIVA_SC2_4010_GAIN_DB_TO_VALUE_RATIO + .5);

  if ( param < -65 ) param = -65;
  else if ( param > 643 ) param = 643;
  
  return setConfigValueForCommand("G", param, true);
}

/* Private helper function for getting the gain. Gets the gain value
 * from the camera, and converts the value to decibels.
 */
double PiiAviivaSC2CLSerialProtocol::gain() const
{
  int value = configValueForCommand("G");
  double temp = double(value) * AVIIVA_SC2_4010_GAIN_DB_TO_VALUE_RATIO;

  if ( temp < -2.0 )
    return -2.0;
  else if ( temp > 22.0 )
    return 22.0;
  
  return temp;
}



/* Private helper function, which sets the bus speed. The speed is
 * assumed to be in megaherzes. Zero means that the clock is
 * 2 x external, 1 means that clock is external and 2 means that
 * clocks is external/2. The legal values for the bus speed are 0,1,2,
 * 20, 30, 40 and 60.
 */
bool PiiAviivaSC2CLSerialProtocol::setBusSpeed(const QVariant& speed)
{
  // number  internal  rate
  // 0          no      -
  // 1          no      -
  // 2          no	    -
  // 3          yes     20
  // 4          yes     30
  // 5          yes     40
  // 6          yes     60


  // Get the old Data transer state. If the old state is the same than
  // the new, the value is not set again.
  int oldState = configValueForCommand("H");
  if (oldState >=0)
    {
      int val = -1;
      switch (speed.toInt())
        {
        case 0: val = 0; break; // Set external clock (x2)
        case 1: val = 1; break; // Set external clock (x1)
        case 2: val = 2; break; // Set external clock (x0.5)
        case 20:val = 3; break; // New frequency: 20 MHz
        case 30:val = 4; break; // New frequency: 30 MHz
        case 40:val = 5; break; // New frequency: 40 MHz
        case 60:val = 6; break; // New frequency: 60 MHz
        default: //ERROR
          return false;
        }

      if ( val == oldState ) //don't set the same value again
        return true;
      else
        return setConfigValueForCommand("H",val,true);
    }

  return false;
}

/* Returns the bus speed as megahertzes (integer). 0 = external clock
 * x 2, 1 = external clock x 1 and 2 = external clock / 2. Negative
 * value means that an error has been occured.
 */
int PiiAviivaSC2CLSerialProtocol::busSpeed() const
{
  int value = configValueForCommand("H");
  if (value < 0)
    return -1;
  
  switch(value)
    {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    case 3: return 20;
    case 4: return 30;
    case 5: return 40;
    case 6: return 60;
    default:
      return -1;
    }
}



/* Private function, which sets the channel count. Legal values are 1
 * and 3.
 *
 * Note, that internally in  Aviiva SC2, the settings "Channel count" and
 * "bits per pixel" are combined to one setting "output format".
 * That's why we will set the Serial RGB 8 bits -mode to the default in
 * setChannelCount()-function and Parallel RGB (3x8 bits) in
 * setBitsPrePixel-function when we want to use 8 bits.
 */
bool PiiAviivaSC2CLSerialProtocol::setChannelCount(const QVariant& count)
{
  int channelCount = count.toInt();
  if (channelCount == 1 || channelCount == 3)
    {
      int oldValue = configValueForCommand("S");
      if (oldValue<0)
        return false;
      
      if (channelCount == 1)
        {
          if ( oldValue == 0 ) //set to default (Serial RGB 8 bits)
            return setConfigValueForCommand("S", 1, true);
          else
            return true; //no change
        }
      else // channel count == 3
        {
          if ( oldValue == 0 ) //no change
            return true; 
          else //must change to the 0
            return setConfigValueForCommand("S", 0, true);
        }
    }

  // ERROR, incorrect amount of channels
  return false;
}

/* Private helper function, which returns the channel count. Returns -1
 * on error.
 */
int PiiAviivaSC2CLSerialProtocol::channelCount() const
{
  int value = configValueForCommand("S");
  if (value<0)
    return -1;
  switch(value)
    {
    case 0:
      return 3;
    case 1:
    case 2:
    case 3:
      return 1;
      
    default:
      return -1;
    }
  return -1;
}


/* This is private helper function for setting the bits per pixel. The
 * bits given as a parameter is assumed to be an integer. The legal
 * values are 8, 10 and 12. Success of setting the value is returned.
 *
 * Note, that internally in  Aviiva SC2, the settings "Channel count" and
 * "bits per pixel" are combined to one setting "output format".
 * That's why we will set the Serial RGB 8 bits -mode to the default in
 * setChannelCount()-function and Parallel RGB (3x8 bits) in
 * setBitsPrePixel-function when we want to use 8 bits.
 *
 * The function sets the "Output format" setting in Aviiva 4010 SC2 CL
 * camera. It uses the command "S=".
 */
bool PiiAviivaSC2CLSerialProtocol::setBitsPerPixel(const QVariant& bits)
{
  int iBits = bits.toInt();

  int oldValue = configValueForCommand("S");
  if (oldValue<0)
    return false;
  else
    {
      
      switch (iBits)
        {
        case 8:
          {
            if ( oldValue == 0 || oldValue == 1 )
              return true; //no change
            else  //Set default to Parallel RGB (3 x 8)
              return setConfigValueForCommand("S", 0, true);
          }

        case 10:
          {
            if ( oldValue == 2 )
              return true; //no change
            else
              return setConfigValueForCommand("S", 2, true);
          }
        case 12:
          {
            if ( oldValue == 3 )
              return true; //no change
            else
              return setConfigValueForCommand("S", 3, true);
          }

      
        default: return false; //ERROR
        }
    }
  
  return false; // ERROR, invalid value
}

/* This is a private helper function for getting the bits per pixel
 * value from the camera. -1 is returned on error.
 */
int PiiAviivaSC2CLSerialProtocol::bitsPerPixel() const
{
  int value = configValueForCommand("S");
  switch (value)
    {
    case 0:
    case 1: return 8;
    case 2: return 10;
    case 3: return 12;
    default: return -1; //ERROR
    }
}

/* Private helper function for setting the integration time. The time
 * given as a paremter is assumed to be in microseconds as an integer.
 * The valid range of values is from 1 to 32768. Success in setting
 * the new integration time is returned from the function.
 */
bool PiiAviivaSC2CLSerialProtocol::setIntegrationTime(const QVariant& time)
{
  // The command "I=" is used.
  int iTime = time.toInt();
  if (iTime >=1 && iTime <=32768)
    return setConfigValueForCommand("I", iTime, true);

  return false; // ERROR, invalid value  
}

/* This is a private helper function, which gets the integeration time
 * from the camera. The time returned is in microseconds. -1 is
 * returned on error.
 */
int PiiAviivaSC2CLSerialProtocol::integrationTime() const
{
  return configValueForCommand("I");
}


