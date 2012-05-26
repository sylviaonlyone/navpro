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

#ifndef _PIICURRERADRIVER_H
#define _PIICURRERADRIVER_H

#include <PiiCameraDriver.h>
#include <PiiMetaConfigurationValue.h>
#include <PiiWaitCondition.h>
#include <PiiCurreraDriverGlobal.h>
#include <PiiTimer.h>

#include <QThread>
#include <QMutex>

#include <xiApiExt.h>

class PII_CURRERADRIVER_EXPORT PiiCurreraDriver : public PiiCameraDriver
{
  Q_OBJECT

  /**
   * The size of the frame buffer.
   */
  Q_PROPERTY(int frameBufferCount READ frameBufferCount WRITE setFrameBufferCount);

  /**
   * TRG_OFF = 0,								//!< Camera works in free run mode.
   * TRG_EDGE_RISING = 1,						//!< External trigger (rising edge).
   * TRG_EDGE_FALLING = 2,						//!< External trigger (falling edge).	
   * TRG_SOFTWARE = 3							//!< Software(manual) trigger.
   */
  Q_PROPERTY(int triggerSource READ triggerSource WRITE setTriggerSource);

  /**
   * Exposure time in microSeconds.
   */
  Q_PROPERTY(int exposureTime READ exposureTime WRITE setExposureTime);

  /**
   * Gain value must be between 0-1.
   */
  Q_PROPERTY(double gain READ gain WRITE setGain);

  /**
   * Set frameRect. QRect(x y width height).
   */
  Q_PROPERTY(QRect frameRect READ frameRect WRITE setFrameRect);

  /**
   * If true the image will flipped horizontally.
   */
  Q_PROPERTY(bool flipHorizontally READ flipHorizontally WRITE setFlipHorizontally);

  /**
   * If true the image will flipped vertically.
   */
  Q_PROPERTY(bool flipVertically READ flipVertically WRITE setFlipVertically);
  
public:
  /**
   * Construct a new PiiCurreraDriver.
   */
  PiiCurreraDriver();

  /**
   * Destroy the PiiCurreraDriver.
   */
  ~PiiCurreraDriver();

  QStringList cameraList() const;
  void initialize(const QString& cameraId);
  bool close();
  bool startCapture(int frames);
  bool stopCapture();
  void* frameBuffer(int frameIndex) const;
  bool isOpen() const;
  bool isCapturing() const;
  bool triggerImage();
  bool setTriggerMode(PiiCameraDriver::TriggerMode mode);
  PiiCameraDriver::TriggerMode triggerMode() const;
  int bitsPerPixel() const;
  QSize frameSize() const;
  bool setFrameSize(const QSize& frameSize);
  int imageFormat() const;
  bool setImageFormat(int format);
  QSize resolution() const;

  
  int frameBufferCount() const;
  int triggerSource() const;
  int exposureTime() const;
  double gain() const;
  QRect frameRect() const;
  bool flipHorizontally() const;
  bool flipVertically() const;

  bool setFrameBufferCount(int frameBufferCount);
  bool setTriggerSource(int triggerSource);
  bool setExposureTime(int exposureTime);
  bool setGain(double gain);
  bool setFrameRect(const QRect& frameRect);
  bool setFlipHorizontally(bool flipHorizontally);
  bool setFlipVertically(bool flipVertically);

private:
  int frameIndex(int frameIndex) const;
  void buffer();
  void capture();
  void stopBuffering();
  void stopCapturing();

  bool requiresInitialization(const char* name) const;

  bool writeExtensionValue(MM_EXTENSION extension, int value);
  bool writeIntValue(const char* name, int value);

  int readExtensionValue(MM_EXTENSION extension, int defaultValue = 0, bool *ok = 0) const;
  int readConfigValue(const char* name, int defaultValue = 0, bool *ok = 0);
  int readIntValue(const char* name, int defaultValue = 0, bool *ok = 0) const;
  
  HANDLE _xiHandle;
  XI_IMG _xiImage;
  QStringList _lstCriticalProperties;
  bool _bOpen, _bCapturingRunning, _bBufferingRunning;
  int _iCameraId;

  int _iFrameBufferCount, _iSkippingLimit, _iFrameSizeInBytes;
  unsigned char* _pBuffer;
  QVector<unsigned char*> _vecBufferPointers;
  QVector<int> _vecFrameTimes;
  PiiTimer _frameTimer;
  
  QThread *_pBufferingThread, *_pCapturingThread;
  QMutex _frameBufMutex;
  unsigned int _iFrameIndex, _iLastHandledFrame;
  int _iMaxFrames, _iHandledFrameCount;
  PiiWaitCondition _frameWaitCondition, _triggerWaitCondition;
  PiiCameraDriver::TriggerMode _triggerMode;
  bool _bSoftwareTrigger;
  int _iTriggerSource;
};

#endif //_PIICURRERADRIVER_H
