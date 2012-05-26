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


#include "PiiAviivaM2CLSerialProtocol.h"
#include <cstdlib> // for free(), malloc()
#include <string.h> // for strcmp()
#include <QRegExp>
#include <QStringList>
#include <QtDebug>
#include <PiiDelay.h>


// Defines the ratio between the desibels and the value of the gain
// parameter in G-command in the serial protocol
#define AVIIVA_4010_GAIN_DB_TO_VALUE_RATIO 21.275

PiiAviivaM2CLSerialProtocol::PiiAviivaM2CLSerialProtocol()
{
  //qDebug("PiiAviivaM2CLSerialProtocol CONSTRUCTOR BEGIN");

  addNonEnumeratedItem("gain",
                       "Gain in decibels",
                       QVariant::Double,
                       0.0,
                       40.0);

  addEnumeratedItem("busSpeed",
                    "Bus speed in MHz",
                    QVariant::Int,
                    QVariantList() << 0 << 20 << 30 << 40 << 60,
                    QStringList() << "free run" << "20" << "30" << "40" << "60");

  addNonEnumeratedItem("channelCount",
                       "Channel count",
                       QVariant::Int,
                       1,
                       2);

  addEnumeratedItem("bitsPerPixel",
                    "Bits per pixel",
                    QVariant::Int,
                    QVariantList() << 8 << 10 << 12,
                    QStringList() << "8-bit" << "10-bit" << "12-bit");

  addNonEnumeratedItem("integrationTime",
                       "Integration time in µs",
                       QVariant::Int,
                       5,
                       13000);

  //qDebug("PiiAviivaM2CLSerialProtocol CONSTRUCTOR, configurationValueCount() = %d", configurationValueCount());
  
  //qDebug("PiiAviivaM2CLSerialProtocol CONSTRUCTOR END");
  
}

bool PiiAviivaM2CLSerialProtocol::setConfigurationValue(const char* name, const QVariant& value)
{
  if (device() == 0)
    return false;
  
  if (strcmp(name, "gain")==0)
    return setGain(value);
  else if (strcmp(name, "busSpeed")==0)
    return setBusSpeed(value);
  else if (strcmp(name, "channelCount")==0)
    return setChannelCount(value);
  else if (strcmp(name, "bitsPerPixel")==0)
    return setBitsPerPixel(value);
  else if (strcmp(name, "integrationTime")==0)
    return setIntegrationTime(value);
  else return PiiAviivaCLSerialProtocol::setConfigurationValue(name,value);
}

QVariant PiiAviivaM2CLSerialProtocol::configurationValue(const char* name) const
{
  if (strcmp(name, "gain")==0)
    return gain();
  else if (strcmp(name, "busSpeed")==0)
    return busSpeed();
  else if (strcmp(name, "channelCount")==0)
    return channelCount();
  else if (strcmp(name, "bitsPerPixel")==0)
    return bitsPerPixel();
  else if (strcmp(name, "integrationTime")==0)
    return integrationTime();
  else return PiiAviivaCLSerialProtocol::configurationValue(name);
}


/* Private function, which sets the gain of the A/D convertion. The
 * gain is set as desibels, which is a double converted to QVariant.
 */
bool PiiAviivaM2CLSerialProtocol::setGain(const QVariant& value)
{
  //qDebug() << "PiiAviivaM2CLSerialProtocol::setGain(), value = " << value;
  //qDebug() << "PiiAviivaM2CLSerialProtocol::setGain(), value.toDouble() = " << value.toDouble();
  //G= 0 to 851 Gain setting from 0 to 40 dB (~0.047dB steps)
  // The format of the command setting the gain is "G=X", where X is
  // an integer value between 0 and 851, which corresponds the
  // gain between 0 and 40 dB, with 0.047dB steps. 
  double dDesibels = value.toDouble();
  if (dDesibels < 0.0 || dDesibels > 40.0)
    return false;
  // Rounded to the nearest integer.
  int param = int(dDesibels*AVIIVA_4010_GAIN_DB_TO_VALUE_RATIO+.5);

  return setConfigValueForCommand("G", param, true);
}

/* Private helper function for getting the gain. Gets the gain value
 * from the camera, and converts the value to decibels.
 */
double PiiAviivaM2CLSerialProtocol::gain() const
{
  int value = configValueForCommand("G");
  //qDebug("PiiAviivaM2CLSerialProtocol::gain() value = %d", value);
 
  return double(value)/AVIIVA_4010_GAIN_DB_TO_VALUE_RATIO; 
}

/* Private helper function, which sets the buss speed. The speed is
 * assumed to be in megaherzes. Zero means that the clock is
 * external. The legal values for the bus speed are 0, 20, 30, 40 and 60.
 *
 * Note, that internally in  Aviiva M2, the settings "Channel count" and
 * "Bus speed" are combined to one setting "Data transfer". Thats why
 * in some cases, when changing the bus speed, the channel count will
 * also be changed. This happens for example, when setting the bus
 * speed to 60, and the channel count was 2. After setting the speed
 * the channel count will be 1, because the bus speed 60 is possible only
 * with channel count 1.
 */
bool PiiAviivaM2CLSerialProtocol::setBusSpeed(const QVariant& speed)
{
  //qDebug("PiiAviivaM2CLSerialProtocol::setBusSpeed()");  
// number  internal outputs  rate
// 0          no      2        -
// 1          no      1        -
// 3          yes	    1        20
// 5          yes     1        30
// 6          yes     2        20
// 7          yes     1        40
// 8          yes     2        30
// 9          yes     1        60
// 10         yes     1        -


  // Get the old Data transer state. If the old state is the same than
  // the new, the value is not set again.
  int oldState = configValueForCommand("H");
  if (oldState >=0)
    {
      //Note,that the acknowledgement ("OK"-message) is not waited,
      //because it never arrives, when using "H="-command. That's why
      //the third parameter of the function setConfigValueForCommand()
      //is allways false in the following switch-case.
      int iSpeed = speed.toInt();
      switch (iSpeed)
        {
        case 0: // Set external clock
          switch (oldState)
            {
            case 0: // two outputs, external clock
            case 1: // one output, external clock
            case 10: // one output, external clock (divided by 2)
              return true; // No change
            case 3: // one output, 20 MHz
            case 5: // one output, 30 MHz
            case 9: // one output, 60 MHz
            case 7: // one output, 40 MHz
              // Set one output, external clock
              return setConfigValueForCommand("H", 1, true);
            case 6: // two outputs, 20 MHz
            case 8: // two outputs, 30 MHz
              // Set two outputs, external clock
              return setConfigValueForCommand("H", 0, true);
            default: //ERROR
              return false;
            }
        case 20: // New frequency: 20 MHz
          switch (oldState)
            {
            case 3: // one output, 20 MHz
            case 6: // two outputs, 20 MHz
              return true; // No changes
            case 1: // one output, external clock
            case 5: // one output, 30 MHz
            case 9: // one output, 60 MHz
            case 7: // one output, 40 MHz
            case 10: // one output, external clock (divided by 2)
              // Set one output, 20 MHz
              return setConfigValueForCommand("H", 3, true);
            case 0: // two outputs, external clock
            case 8: // two outputs, 30 MHz
              // Set two outputs, 20 MHz
              return setConfigValueForCommand("H", 6, true);          
            default: //ERROR
              return false;
            }
        case 30: // New frequency: 30 MHz
          switch (oldState)
            {
            case 5: // one output, 30 MHz
            case 8: // two outputs, 30 MHz
              return true; // No changes          
            case 1: // one output, external clock
            case 3: // one output, 20 MHz
            case 9: // one output, 60 MHz
            case 7: // one output, 40 MHz
            case 10: // one output, external clock (divided by 2)
              // Set one output, 30 MHz
              return setConfigValueForCommand("H", 5, true);
            case 0: // two outputs, external clock
            case 6: // two outputs, 20 MHz
              // Set two outputs, 30 MHz
              return setConfigValueForCommand("H", 8, true);
            default: //ERROR
              return false;
            }
        case 40: // New frequency: 40 MHz
          switch (oldState)
            {
            case 7: // one output, 40 MHz
              return true; // No changes 
            case 0: // two outputs, external clock
            case 1: // one output, external clock
            case 3: // one output, 20 MHz
            case 5: // one output, 30 MHz
            case 6: // two outputs, 20 MHz
            case 8: // two outputs, 30 MHz
            case 9: // one output, 60 MHz
            case 10: // one output, external clock (divided by 2)
              // Set one output, 40 MHz. There are no option for two outputs
              // with 40 MHz. That's why two outputs will be changed to one output.
              return setConfigValueForCommand("H", 7, true);
            default: //ERROR
              return false;
            }
        case 60: // New frequency: 60 MHz
          switch (oldState)
            {
            case 9: // one output, 60 MHz
              return true; // No changes 
            case 0: // two outputs, external clock
            case 1: // one output, external clock
            case 3: // one output, 20 MHz
            case 5: // one output, 30 MHz
            case 6: // two outputs, 20 MHz
            case 7: // one output, 40 MHz
            case 8: // two outputs, 30 MHz
            case 10: // one output, external clock (divided by 2)
              // Set one output, 60 MHz. There are no option for two outputs
              // with 60 MHz. That's why two outputs will be changed to one output.
              return setConfigValueForCommand("H", 9, true);
            default: //ERROR
              return false;
            }
        default: //ERROR
          return false;
        }
    }

  return false;
}

/* Returns the bus speed as megahertzes (integer). Zero means that the
 * external clock is used. Negative value means that an error has been
 * occured.
 */
int PiiAviivaM2CLSerialProtocol::busSpeed() const
{
  int value = configValueForCommand("H");
  if (value<0)
    return false;
  switch(value)
    {
    case 0:
    case 1:
    case 10:
      return 0;
    case 3:
    case 6:
      return 20;
    case 5:
    case 8:
      return 30;
    case 7:
      return 40;
    case 9:
      return 60;
    default:
      return -1;
    }
  return -1;
}

/* Private function, which sets the channel count. Legal values are 1
 * and 2.
 *
 * Note, that internally in  Aviiva M2, the settings "Channel count" and
 * "Bus speed" are combined to one setting "Data transfer". Thats why
 * in some cases, when changing the channel count from 1 to 2, the Bus
 * speed will also be changed. This is the case for example, when the
 * bus speed is 60, and the channel count will be set to one. Then the
 * bus speed will be chaned to 30, because the combination bus speed
 * 60 with the channel count 2 is not possible. Also when the bus
 * speed is 40, and the channel count will be set to 2, the bus
 * speed will be automatically changed to 20.
 */
bool PiiAviivaM2CLSerialProtocol::setChannelCount(const QVariant& count)
{
  //qDebug() << "PiiAviivaM2CLSerialProtocol::setChannelCount(), value = " << count;
  int iCount = count.toInt();
  if (iCount > 0 && iCount < 3)
    {
      int value = configValueForCommand("H");
      if (value<0)
        return false;
      
      //Note,that the acknowledgement ("OK"-message) is not waited,
      //because it never arrives, when using "H="-command. That's why
      //the third parameter of the function setConfigValueForCommand
      //is allways false in the following switch-case.
      if (iCount == 1)
        {
          int val = 0;
          switch(value)
            {
            case 0: val = 1; break; // two outputs, external clock
            case 6: val = 3; break; // two outputs, 20 MHz or should it be 7? Should we double the frequency?
            case 8: val = 5; break; // two outputs, 30 MHz

            case 1: // one output, external clock
            case 3: // one output, 20 MHz
            case 5: // one output, 30 MHz
            case 7: // one output, 40 MHz
            case 9: // one output, 60 MHz
            case 10: // one output, external clock (divided by 2)
              return true; // No change
              
            default: // ERROR
              return false;
            }
          return setConfigValueForCommand("H", val, true);
        }
      else // count == 2
        {
          int val = 0;
          switch(value)
            {
            case 0: // two outputs, external clock
            case 6: // two outputs, 30 MHz
            case 8: // two outputs, 20 MHz
              return true; //No change

            case 1: val = 0; break; // one output, external clock
            case 3: val = 6; break; // one output, 20 MHz
            case 5: val = 8; break; // one output, 30 MHz
            case 7: val = 6; break; // one output, 40 MHz. The
                                    // frequency is changed to 20 MHz,
                                    // because 40 MHz for two outputs doesn't exist.
            case 9: val = 8; break; // one output, 60 MHz The
                                    // frequency is changed to 30 MHz, because 60 MHz for
                                    // two outputs doesn't exist.
            case 10:val = 0; break; // one output, external clock (divided by 2)
                                    // Is changed to two outputs with external clock, without
                                    // frequency division by two.

            default: // ERROR
              return false;
            }
          return setConfigValueForCommand("H", val, true);
        }
    }

  // ERROR, incorrect amount of channels
  return false;
}

/* Private helper function, which returns the channel count. Returns -1
 * on error.
 */
int PiiAviivaM2CLSerialProtocol::channelCount() const
{
  int value = configValueForCommand("H");
  if (value<0)
    return -1;
  switch(value)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 9:
    case 10:
      return 1;
    case 0:
    case 6:
    case 8:
      return 2;
    default:
      return -1;
    }
  return -1;
}

/* This is private helper function for setting the bits per pixel. The
 * bits given as a parameter is assumed to be an integer. The legal
 * values are 8, 10 and 12. Success of setting the value is returned.
 *
 * The function sets the "Output format" setting in Aviiva 4010 M2 CL
 * camera. It uses the command "S=".
 */
bool PiiAviivaM2CLSerialProtocol::setBitsPerPixel(const QVariant& bits)
{
  //qDebug() << "PiiAviivaM2CLSerialProtocol::setBitsPerPixel(), value = " << bits;
  int iBits = bits.toInt();
  switch (iBits)
    {
    case 8:
      return setConfigValueForCommand("S", 2, true);
    case 10:
      return setConfigValueForCommand("S", 1, true);
    case 12:
      return setConfigValueForCommand("S", 0, true);
    }
  return false; // ERROR, invalid value
}

/* This is a private helper function for getting the bits per pixel
 * value from the camera. -1 is returned on error.
 */
int PiiAviivaM2CLSerialProtocol::bitsPerPixel() const
{
  int value = configValueForCommand("S");
  switch (value)
    {
    case 0:
      return 12;
    case 1:
      return 10;
    case 2:
      return 8;
    }
  return -1; //ERROR
}

/* Private helper function for setting the integration time. The time
 * given as a paremter is assumed to be in microseconds as an integer.
 * The valid range of values is from 5 to 13000. Success in setting
 * the new integration time is returned from the function.
 */
bool PiiAviivaM2CLSerialProtocol::setIntegrationTime(const QVariant& time)
{
  //qDebug("PiiAviivaM2CLSerialProtocol::setIntegrationTime()");
  // The command "I=" is used.
  int iTime = time.toInt();
  if (iTime >=5 && iTime <=13000)
    {
      return setConfigValueForCommand("I", iTime, true);
    }
  return false; // ERROR, invalid value  
}

/* This is a private helper function, which gets the integeration time
 * from the camera. The time returned is in microseconds. -1 is
 * returned on error.
 */
int PiiAviivaM2CLSerialProtocol::integrationTime() const
{
  return configValueForCommand("I");
}


