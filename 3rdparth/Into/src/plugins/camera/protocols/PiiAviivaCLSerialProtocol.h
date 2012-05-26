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

#ifndef _PIIAVIIVACLSERIALPROTOCOL_H
#define _PIIAVIIVACLSERIALPROTOCOL_H

#include "PiiCameraConfigurationProtocol.h"
#include <QList>
#include <QString>

/**
 * This class is the serial protocol base class for Aviiva CL cameras.
 */
class PiiAviivaCLSerialProtocol : public PiiCameraConfigurationProtocol
{
  Q_OBJECT

public:

  PiiAviivaCLSerialProtocol();
  virtual ~PiiAviivaCLSerialProtocol() {}
  
  /**
   * Implements the corresponding function in the base class. This
   * class knows the following configuration value items:
   *
   * @lip triggerMode - the trigger mode. The possible values are the
   * values defined by the enum @p TriggerMode.
   *
   * @lip cameraId - the camera id in as a string.
   *
   */
  virtual bool setConfigurationValue(const char* name, const QVariant& value);

  /**
   * Implements the corresponding function in the base class.
   */
  virtual QVariant configurationValue(const char* name) const;

protected:
  /**
   * The trigger mode of the camera. The mode of camera triggering and
   * integration time can be set.
   *
   * @lip FreeRun - Free run with integration time setting.
   *
   * @lip ExternalTrigger - External trigger with integration time setting.
   *
   * @lip ExternalTriggerAndIntegration - Trigger and Integration time controlled.
   *
   * @lip ExternalTriggerAndIntegration2 - Trigger and integration
   * time controlled by two inputs.
   */
  enum TriggerMode
  {
    UndefinedTriggerMode = 0,
    FreeRun = 1,
    ExternalTrigger,
    ExternalTriggerAndIntegration,
    ExternalTriggerAndIntegration2
  };

  /* This is a helper function for writing the commands via serial
   * device to the camera. The serial protocol command is given as the
   * first parameter. The second parameter contains the value as an
   * integer. The parameter checkOK determines, whether the
   * acknowledgement message ("OK") is checked after sending the
   * command. If checkOK == true, "OK" is checked, else it is not
   * checked. If setting the configuration value succeeds, true is
   * returned, else false is returned. With the true value of checkOK,
   * the existence of ack message "OK" affects to the return value.
   */
  bool setConfigValueForCommand(const QString& command, int value, bool checkOK = true);

  /* This is helper function for camera configuration readout.
   * This function is for reading the configuration values, which are
   * integers.
   * The function writes the command "!=3" to the serial port, reads the
   * response, parses the value corresponding the command given as a
   * paremeter from the response, and converts the value to integer. 
   *
   * -1 is returned on error.
   */
  int configValueForCommand(const QString& command) const;

private:
  bool setTriggerMode(const QVariant& mode);
  PiiAviivaCLSerialProtocol::TriggerMode triggerMode() const;
  bool setUserCameraId(const QVariant& id);
  QString userCameraId() const;
  QString cameraId() const;
};


#endif //_PIIAVIIVACLSERIALPROTOCOL_H
