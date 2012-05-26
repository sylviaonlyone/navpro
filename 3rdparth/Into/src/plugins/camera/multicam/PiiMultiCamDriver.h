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

#ifndef _PIIMULTICAMDRIVER_H
#define _PIIMULTICAMDRIVER_H

#include "PiiCameraDriver.h"
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include "multicam.h"

extern "C" void multiCamCaptureFunction(PMCCALLBACKINFO callBackInfo);

class PiiMultiCamDriver;

/**
 * @internal
 *
 * A thread that feeds data to a listener from the MultiCam driver. 
 * This should be a private inner class in PiiMultiCamDriver, but @p
 * moc doesn't allow it.
 */
class PiiMultiCamCaptureThread : public QThread
{
  Q_OBJECT
  
public:
  PiiMultiCamCaptureThread(PiiMultiCamDriver* parent);
  
  void stop();
  void startThread(Priority priority = InheritPriority) { _bRunning = true; start(priority); }

protected:
  void run();

private:
  PiiMultiCamDriver* _pParent;
  volatile bool _bRunning;
  int _iSignalCounter;
};

class PiiCameraLinkSerialDevice;
class PiiCameraConfigurationProtocol;

//TODO: (in this class)
//     -metaconfiguration values
//          -also the camera meta configuration values should be
//           available through PiiMultiCamDriver meta configuration
//           values ??

/**
 * An implementation of the PiiCameraDriver interface for EureSys
 * grabber boards. Uses the MultiCam camera driver.
 *
 * @note Changing the properties of the camera driver does @b not
 * affect the camera itself. The configuration to the camera must be
 * set with camera-specific tools such as a CameraLink terminal
 * program. The configuration of the camera must match the values set
 * to the driver.
 *
 * @ingroup PiiCameraPlugin
 */
class PiiMultiCamDriver : public PiiCameraDriver
{
  Q_OBJECT

  /**
   * The camera link mode. Used to select the way the camera link bus
   * operates. The mode must be selected before driver is initialized.
   */
  Q_PROPERTY(CameraLinkMode cameraLinkMode READ cameraLinkMode WRITE setCameraLinkMode);
  Q_ENUMS(CameraLinkMode);

  /**
   * The index of the selected board. The board index varies from 0 to
   * N-1, where N is the number of frame grabber boards in the system.
   */
  Q_PROPERTY(int board READ board WRITE setBoard);

  /**
   * The name of the acquisition channel. Valid names depend on the
   * type of the selected board. For example, use A, B, or M (medium
   * mode, uses both A and B) for the Expert 2 board.
   */
  Q_PROPERTY(QString channel READ channel WRITE setChannel);
  
  /**
   * Camera model name. The name must be supported by MultiCam (refer
   * to MultiCam documentation). Use "AVIIVA4010" for Atmel Aviiva
   * 4010.
   */
  Q_PROPERTY(QString cameraModel READ cameraModel WRITE setCameraModel);

  /**
   * Defines the serial protocol. The implementation for the serial
   * protocol must exist in camera plugin-in. For Aviiva cameras the
   * protocol name is the complete camera model name without the last
   * for digits. For example for the cameras AVIIVA M2 CL 4010 and
   * AVIIVA M2 CL 2014 the protocol name is "AviivaM2CL". 
   */
  Q_PROPERTY(QString serialProtocol READ serialProtocol WRITE setSerialProtocol);
  
  /**
   * MultiCam configuration name for the selected camera. The
   * configuration name is composed of the following parts:
   *
   * @li [Scan][Rate][CamMode][SyncMode]{_[Suffix]} for analog area-scan cameras
   * @li [Scan][Rate][CamMode][ExpMode] for digital area-scan cameras
   * @li [Scan][Size][CamMode][ExpMode] for line-scan cameras
   *
   * Scan is either L for line scan, P for proggressive scan, or I for
   * interlaced scan. Size is the line length in pixels, Rate is the
   * scanning rate (fps). CamMode is of the following:
   * 
   * @li S - The line rate is synchronously generated inside the
   * camera (free run)
   * @li R - The line rate is externally controlled through a
   * line-reset pulse (triggered)
   *
   * ExpMode is one of the following:
   *
   * @li C - The line exposure duration is controlled by the camera
   * @li G - The line exposure duration is controlled by the frame grabber
   * @li P - Permanent exposure, i.e. no exposure control
   */
  Q_PROPERTY(QString cameraConfig READ cameraConfig WRITE setCameraConfig);

  /**
   * The number of bytes to reserve for a frame buffer. Independent of
   * this setting the driver will reserve space for at least @ref
   * setSignalInterval() "signalInterval" frames. The default size
   * is 1MB.
   */
  Q_PROPERTY(int bufferMemory READ bufferMemory WRITE setBufferMemory);

  /**
   * The source of trigger signals. The default value is @p NoTrigger.
   */
  Q_PROPERTY(TriggerSource triggerSource READ triggerSource WRITE setTriggerSource);
  Q_ENUMS(TriggerSource);

  /**
   * Triggering rate. The interpretation of this value depends on
   * #triggerSource. It has no effect in @p NoTrigger and @p
   * ExternalTrigger modes. In the @p InternalClockTrigger mode it
   * determines the clock frequency, in Hz. In @p EncoderTrigger mode
   * it is used as a conversion ratio from encoder pulses to trigger
   * pulses. For example, setting @p triggerRate to 0.5 sends every
   * second encoder pulse to the camera.
   */
  Q_PROPERTY(double triggerRate READ triggerRate WRITE setTriggerRate);
  
public:
  /**
   * Valid Camera Link operation modes. The mode type starts with the
   * standard configuration type (Base, Medium or Full) followed by
   * the number of taps (channels), a "T", and finally the number of
   * bits per tap. For example, a base mode camera with 1 acquisition
   * channel (tap) and 8 bits per tap is denoted by @p Base1T8.
   */
  enum CameraLinkMode
    {
      Base1T8 = 1,
      Base1T10,
      Base1T12,
      Base1T14,
      Base1T16,
      Base1T24,
      Base2T8,
      Base2T10,
      Base2T12,
      Base3T8,
      Base1T30B2,
      Base1T36B2,
      Base1T42B2,
      Base1T48B2,
      Base2T14B2,
      Base2T16B2,
      Base2T24B2,
      Base3T10B2,
      Base3T12B2,
      Base3T14B2,
      Base3T16B2,
      Base4T8B2,
      Base4T10B2,
      Base4T12B2,
      Base1T24B3,
      Base1T30B3,
      Base1T36B3,
      Base1T42B3,
      Base1T48B3,
      Medium1T30,
      Medium1T36,
      Medium1T42,
      Medium1T48,
      Medium2T14,
      Medium2T16,
      Medium2T24,
      Medium3T10,
      Medium3T12,
      Medium3T14,
      Medium3T16,
      Medium4T8,
      Medium4T10,
      Medium4T12,
      Medium8T8B2,
      Full8T8
    };

  /**
   * The source of trigger signals.
   *
   * @lip NoTrigger - the camera needs no external trigger. Sometimes
   * referred to as a free-running mode. Frame duration may be
   * controlled for example by exposure time.
   *
   * @lip InternalClockTrigger - the frame grabber card generates a
   * regular pulse train using its internal clock circuit. The
   * frequency of the trigger signal is controlled by the #triggerRate
   * property.
   *
   * @lip ExternalTrigger - trigger signals received from an external
   * source are directly sent to the camera through the camera bus.
   *
   * @lip EncoderTrigger - the trigger signals are generated by a
   * pulse encoder. The conversion ratio from encoder pulses to
   * trigger pulses is controlled by the #triggerRate property.
   */
  enum TriggerSource
    {
      NoTrigger,
      InternalClockTrigger,
      ExternalTrigger,
      EncoderTrigger,
    };

  /**
   * Construct a new %PiiMultiCamDriver.
   */
  PiiMultiCamDriver();
  /**
   * Destroy the %PiiMultiCamDriver.
   */
  ~PiiMultiCamDriver();

  /**
   * Implements the corresponding function of the base class.
   */
  bool setConfigurationValue(const char* name, const QVariant& value);

  /**
   * Implements the corresponding function of the base class.
   */  
  QVariant configurationValue(const char* /*name*/) const;
  
  /**
   * Select the camera unit.
   *
   * @param unit file name (absolute or relative path) containing
   * camera configuration. The camera configuration file must contain
   * a QSettings-compatible file of property name-value pairs.
   *
   * An example of a camera configuration file:
   *
   * @code
   * # Refer to MultiCam documentation for valid camera names.
   * cameraModel = AVIIVA4010
   * # Refer to MultiCam documentation for valid configuration names.
   * cameraConfig = L4096SC
   * # Base mode camera, 1 channel (tap), and 8 bits per pixel
   * cameraLinkMode = Base1T8
   * # Physical connection is through the first board's B channel.
   * board = 0
   * channel = B
   * # Use a pulse encoder for triggering.
   * triggerSource = EncoderTrigger
   * # Trigger on every fourth encoder pulse.
   * triggerRate = 0.25
   * @endcode
   */
  bool selectUnit(const QString& unit);

  void initialize();

  void setSignalInterval(int signalInterval);

  int signalInterval() const;
    
  bool startCapture(int frames);

  bool stopCapture();

  void* frameBuffer(int frameIndex = 0) const;

  void frameBuffers(int firstFrameIndex, int cnt, void** scanLines) const;
  
  QSize bufferSize() const;

  QSize frameSize() const;

  bool close();

  bool isOpen() const;

  bool isCapturing() const;
  
  void setBoard(int board);
  int board() const;

  void setCameraLinkMode(CameraLinkMode cameraLinkMode);
  CameraLinkMode cameraLinkMode() const;

  void setChannel(const QString& channel);
  QString channel() const;

  void setCameraModel(const QString& cameraModel);
  QString cameraModel() const;

  void setSerialProtocol(const QString& serialProtocol);
  QString serialProtocol() const;
  
  void setCameraConfig(const QString& cameraConfig);
  QString cameraConfig() const;

  void setBufferMemory(int bufferMemory);
  int bufferMemory() const;

  void setTriggerSource(TriggerSource triggerSource);
  TriggerSource triggerSource() const;

  void setTriggerRate(double triggerRate);
  double triggerRate() const;

private:
  friend void multiCamCaptureFunction(PMCCALLBACKINFO callBackInfo);
  friend class PiiMultiCamCaptureThread;
  
  void surfaceFilled();
  void setTopology();
  void createChannel();
  void setTapConfiguration();
  void setCameraConfig();
  void setTriggerMode();
  void initTriggerRate();
  void createSurface(void* address, int size, int pitch);
  void createSerialDevice();

  void storeChannelCountForCamera();
  void storeBitsPerPixelForCamera();
  void storeTriggerModeForCamera();
  void storeCameraSettingsInMap();
  
  void initSerialProtocol();
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    void *pAllocatedBuffer;
    unsigned char *pFrameBuffer;
    int iBufferMemory;
    int iBufferWidth, iBufferHeight, iFrameHeight, iSignalInterval;
    int iLastCapturedBuffer;
    bool bOpen;
    
    int iBoard, iBoardType;
    QString strChannel, strCameraModel, strSerialProtocol, strCameraConfig;
    CameraLinkMode cameraLinkMode;
    TriggerSource triggerSource;
    double dTriggerRate;
    
    MCHANDLE hChannel;
    QList<MCHANDLE> lstSurfaces;
    
    QMutex surfaceMutex;
    QWaitCondition captureCondition;
    PiiMultiCamCaptureThread captureThread;
    
    // The following member variable stores temporary the camera
    // settings, before the intialize() function is called.
    QVariantMap mapTempCameraSettings;  
    PiiCameraLinkSerialDevice* pSerialDevice;
    PiiCameraConfigurationProtocol* pSerialProtocol;
  };
  PII_D_FUNC;
};


#endif //_PIIMULTICAMDRIVER_H
