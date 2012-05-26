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

#ifndef _PIICAMERALINKSERIALDEVICE_H
#define _PIICAMERALINKSERIALDEVICE_H

#include <QIODevice>
#include <QFlags>

/**
 * This class provides a standard interface for setting the
 * configuration values for the camers through MultiCam driver
 * interface. Serial protocol classes inherited from @p
 * PiiCameraConfigurationProtocol can use this class for sending the
 * configuratoin values to the camera and reading the configuration values
 * from the camera.
 */
class PiiCameraLinkSerialDevice : public QIODevice
{
  Q_OBJECT

  friend class PiiCameraLinkSerialReadThread;
  
public:
  /**
   * Constructor.
   */
  PiiCameraLinkSerialDevice();
  ~PiiCameraLinkSerialDevice();
  
  /**
   * Overrides the corresponding virtual function of the base class.
   * Closes the serial connection.
   */
  void close();  

  /**
   * Implements the corresponding function of the base class. The
   * function outputs the number of bytes that are received from the
   * part, but that are not yet read out.
   */
  qint64 bytesAvailable() const;
  
  /**
   * Returns the number of Camera Link serial ports available. -1 is
   * returned on error.
   */
  int portCount() const;

  /**
   * Overrides the corresponding function of the base class.
   */
  bool isSequential() const;
  
  /**
   * Implements the correseponding function in the base class.
   * Initializes the serial connection using the port number _iPortIndex. If the
   * device is allready open, the old connection is closed before 
   */
  bool open(OpenMode mode);

  /**
   * Re-implements the corresponding function of the base class. Returns
   * @p true when data is available in the port, or @p false, if
   * timeout @p msecs, or error occurs. Unlike described in
   * the base class documentation, this function doesn't wait for
   * readyRead() signal.
   */
  bool waitForReadyRead(int msecs);
  
  /**
   * Return the baud rates that are available for the serial
   * communication for the port being used. The device should be in
   * open state, before this is used. If the device is not open, an
   * empty list is allways returned.
   */
  QList<unsigned long> supportedBaudRates() const;
  
  /**
   * Set a port index. Default is 0.
   */
  void setPortIndex(int index);

  /**
   * Set a baud rate. Default is 9600.
   */
  void setBaudRate(int baudRate);

  /**
   * Function for debuggin purposes.
   */
  void commandLineTest();
  
protected:
  /**
   * Implements the pure virtual function of the base class.
   */
  qint64 readData(char* data, qint64 maxSize);

  /**
   * Implements the pure virtual function of the base class.
   */  
  qint64 writeData(const char* data, qint64 maxSize);

private:
  /**
   * Initializes the serial connection for the port given as a
   * parameter. The index of the serial port is given as a parameter
   * to the function. The portIndex should be between 0 and the return
   * value of the function #portCount(). This function doesn't
   * derive from the base class, but works similarly than the
   * corresponding version of the same function with the one parameter.
   */
  bool connectToPort(OpenMode mode, int portIndex, int baudRate);

  /**
   * Initialize the baud rate for the serial communication. The baud rate
   * should be one of the baud rates returned by the function
   * #baudRates(). The device must be in the open state, before
   * this function has an effect. True is returned upon successfull
   * baud rate setting.
   */
  bool initBaudRate(unsigned int rate);


  /// @internal
  class Data
  {
  public:
    Data();
    ~Data();
    
    // Pointer to the SerialRef structure, which is used in the
    // functions of camera link serial api.
    void* pSerialRef;
    
    int iPortIndex, iBaudRate;
  } *d;
};

#endif //_PIICAMERALINKSERIALDEVICE_H
