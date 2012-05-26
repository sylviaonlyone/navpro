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


#include "PiiAviivaCLSerialProtocol.h"
#include <cstdlib> // for free(), malloc()
#include <string.h> // for strcmp()
#include <QRegExp>
#include <QStringList>
#include <QtDebug>
#include <PiiDelay.h>


PiiAviivaCLSerialProtocol::PiiAviivaCLSerialProtocol()
{
  qDebug("PiiAviivaCLSerialProtocol CONSTRUCTOR BEGIN");

  addEnumeratedItem("triggerMode",
                    "Trigger mode",
                    QVariant::Int,
                    QVariantList() << 1 << 2 << 3 << 4,
                    QStringList() << "free run" << "external trigger" <<
                    "external trigger and integration" <<
                    "external trigger and integration, two inputs");
    
  addNonEnumeratedItem("cameraId",
                       "Camera id",
                       QVariant::String,
                       QVariant(),
                       QVariant(),
                       PiiMetaConfigurationValue::Readable);
}

bool PiiAviivaCLSerialProtocol::setConfigurationValue(const char* name, const QVariant& value)
{
  if (device() == 0)
    return false;
  
  if (strcmp(name, "triggerMode")==0)
    return setTriggerMode(value);
  else if (strcmp(name, "userCameraId")==0) // NOT YET IMPLEMENTED
    return setUserCameraId(value);
  else return false;
}

QVariant PiiAviivaCLSerialProtocol::configurationValue(const char* name) const
{
  if (strcmp(name, "triggerMode")==0)
    return int(triggerMode());
  else if (strcmp(name, "userCameraId")==0) //NOT YET IMPLEMENTED
    return userCameraId();  
  else if (strcmp(name, "cameraId")==0)
    return cameraId();
  else return QVariant();
}

bool PiiAviivaCLSerialProtocol::setConfigValueForCommand(const QString& command, int value, bool checkOK)
{

  //qDebug() << "PiiAviivaCLSerialProtocol::setConfigValueForCommand() BEGIN, command = " << command << " value = " << value ;

  if (device()->write(QString("%1=%2\r").arg(command).arg(value).toLocal8Bit()) < 0)
    return false;

  if (false == checkOK)
    return true;
  
  int maxSize = 5;
  char* buffer = (char*)malloc(sizeof(char)*maxSize);
  qint64 bytesRead = device()->readLine(buffer, maxSize);
  QString qBuffer = QString(buffer);
  free(buffer);
  //qDebug() << "PiiAviivaCLSerialProtocol::setConfigValueForCommand() qBuffer = " << qBuffer;
  if (bytesRead > 0)
    {
      if (qBuffer.contains("OK"))
        {
          //qDebug("PiiAviivaCLSerialProtocol::setConfigValueForCommand() RETURN true");
          return true;
        }
    }

  // If the acknowledgement message doesn't arrive, the value
  // is read, and checked, if it is the same than the value
  // set. One millisecond delay is necessary, because
  // sometimes the value is not ready to be read right after it has
  // been set. The commands, where this happens are especially 'H' and
  // 'M'
  PiiDelay::msleep(1);
  int readValue = configValueForCommand(command);
  //qDebug("PiiAviivaCLSerialProtocol::readValue = %d", readValue);
  if (value == readValue)
    return true;
  
  //qDebug("PiiAviivaCLSerialProtocol::setConfigValueForCommand() RETURN false");
  
  return false;
}

int PiiAviivaCLSerialProtocol::configValueForCommand(const QString& command) const

{
  qint64 written =device()->write(QString("!=3\r").toLocal8Bit());
  if (written < 4)
    return -1; // write error
  //qDebug() << "PiiAviivaCLSerialProtocol::configValueForCommand(), written = " << written;
  
  int maxSize = 1028;
  char* buffer = (char*)malloc(sizeof(char)*maxSize);
  memset (buffer, 0, maxSize*sizeof(char));
  //qint64 bytesRead = device()->readLine(buffer, maxSize);
  qint64 bytesRead = device()->read(buffer, maxSize);
  if (bytesRead < 0)
    return -1; // read error.
  //qDebug() << "PiiAviivaCLSerialProtocol::configValueForCommand, bytesRead = " << bytesRead;
  //qDebug() << "PiiAviivaCLSerialProtocol::configValueForCommand, buffer = " << QString(buffer).replace('\r','\n');

  // The responce is expected have the following format. In reality,
  // there are no line breaks ('\n'-character) in the end of each
  // line, but a carriage return character ('\r'). That has to be
  // taken into account in parsing the string.
//  E=1
//  G=100
//  I=160
//  M=2
//  O=0
//  T=0
//  H=9
//  P=0
//  Y=0
//  S=2
//  A=1
//  B=0
//  >OK

  
  QString qBuffer = QString(buffer);
  free(buffer);

  //There is a special marks in the Atmel SC2 protocols, so we must
  //use escape-function for converting special marks. Such as + and -.
  QString value;
  QRegExp rxgain(QString("%1=(\\d+)\\D").arg(QRegExp::escape(command)));
  int pos = rxgain.indexIn(qBuffer);
  if (pos > -1)
   {
     value = rxgain.cap(1);
   }

  //qDebug() << "PiiAviivaCLSerialProtocol::PiiAviivaCLSerialProtocol() value = " << value;
 if (value.isEmpty())
   return -1;
 else
   return value.toInt();
}


/* Private function for setting the trigger mode. The parameter
 * is assumed to contain the enum type TriggerMode as an integer. True
 * is returned on successfull trigger mode setting. Else false is returned.
 */
bool PiiAviivaCLSerialProtocol::setTriggerMode(const QVariant& mode)
{
  //qDebug() << "PiiAviivaCLSerialProtocol::setTriggerMode(), value = " << mode;
  // The command "M=" is used.
  int iTime = mode.toInt();
  if (iTime >=1 && iTime <=4)
    {
      return setConfigValueForCommand("M", iTime, false);
    }
  return false; // ERROR, invalid value    
}

/* Private helper function for getting the trigger mode.
 * UndefinedTriggerMode is returned on error.
 */
PiiAviivaCLSerialProtocol::TriggerMode PiiAviivaCLSerialProtocol::triggerMode() const
{
  int value = configValueForCommand("M");
  if (value < 0)
    return UndefinedTriggerMode; // ERROR
  else
    return PiiAviivaCLSerialProtocol::TriggerMode(value);
      

}


/* This is a private helper function for camera configuration readout.
 * This function is for reading the configuration values, which are
 * integers.
 * The function writes the command "!=3" to the serial port, reads the
 * response, parses the value corresponding the command given as a
 * paremeter from the response, and converts the value to integer. 
 *
 * -1 is returned on error.
 */
QString PiiAviivaCLSerialProtocol::cameraId() const

{
  qint64 written = device()->write(QString("!=0\r").toLocal8Bit());
  if (written < 4)
    return QString(); // write error
  //qDebug() << "PiiAviivaCLSerialProtocol::cameraId(), written = " << written;
  
  int maxSize = 1028;
  char* buffer = (char*)malloc(sizeof(char)*maxSize);
  memset (buffer, 0, maxSize*sizeof(char));
  qint64 bytesRead = device()->read(buffer, maxSize);
  if (bytesRead < 0)
    return QString(); // read error.
  //qDebug() << "PiiAviivaCLSerialProtocol::cameraId, bytesRead = " << bytesRead;
  //qDebug() << "PiiAviivaCLSerialProtocol::cameraId, buffer = " << QString(buffer).replace('\r','\n');

  // The responce is expected have the following format. In reality,
  // there are no line breaks ('\n'-character) in the end of each
  // line, but a carriage return character ('\r'). That has to be
  // taken into account in parsing the string.

  //AT71-M2CL4010-BB1-A-0603A0473-V100
  //>OK

  QString qBuffer = QString(buffer);
  free(buffer);

  // Remove the possible second line (">OK") from the string
  qBuffer = qBuffer.section('\r',0,0);
  
  return qBuffer;
}


//TODO: IMPLEMENT THIS FUNCTION
bool PiiAviivaCLSerialProtocol::setUserCameraId(const QVariant& /*id*/)
{
  return false;
}

//TODO: IMPLEMENT THIS FUNCTION
QString PiiAviivaCLSerialProtocol::userCameraId() const
{
  return "";
}


  
