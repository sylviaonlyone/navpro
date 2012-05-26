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

#ifndef _PIIGENICAMDRIVER_H
#define _PIIGENICAMDRIVER_H

//#define BUILD_GENICAM_WAPI
#include "genicam_wrapper.h"

extern "C"
{
  GENICAM_TYPEDEF(int, GenicamIntVoidFunc)();
  GENICAM_TYPEDEF(int, GenicamIntDevicepFunc)(genicam_device*);
  GENICAM_TYPEDEF(int, GenicamIntCharppIntpFunc)(char**,int*);
  GENICAM_TYPEDEF(const char*, GenicamCCharpCCharpFunc)(const char*);
  GENICAM_TYPEDEF(const char*, GenicamCCharpVoidFunc)();
  GENICAM_TYPEDEF(int, GenicamIntCCharpDeviceppFunc)(const char*,genicam_device**);
  GENICAM_TYPEDEF(int, GenicamIntVoidpFunc)(void*);
  GENICAM_TYPEDEF(int, GenicamIntDevicepCCharpIntFunc)(genicam_device*,const char*,int);
  GENICAM_TYPEDEF(int, GenicamIntDevicepCCharpIntpFunc)(genicam_device*,const char*,int*);
  GENICAM_TYPEDEF(int, GenicamIntDevicepUCharppIntFunc)(genicam_device*,unsigned char**,int);
  GENICAM_TYPEDEF(int, GenicamIntDevicepUCharpIntFunc)(genicam_device*,unsigned char*,int);
}

#include <PiiWaitCondition.h>
#include <QThread>
#include <QMutex>
#include <PiiCameraDriver.h>

/// @internal
class PiiGenicamDriver : public PiiCameraDriver
{
  Q_OBJECT

  Q_PROPERTY(int frameBufferCount READ frameBufferCount WRITE setFrameBufferCount);
  Q_PROPERTY(double frameRate READ frameRate WRITE setFrameRate);
  Q_PROPERTY(int exposureTime READ exposureTime WRITE setExposureTime);
  Q_PROPERTY(double gain READ gain WRITE setGain);
  Q_PROPERTY(QRect frameRect READ frameRect WRITE setFrameRect);
  Q_PROPERTY(int packetSize READ packetSize WRITE setPacketSize);
  Q_PROPERTY(QRect autoExposureArea READ autoExposureArea WRITE setAutoExposureArea);
  Q_PROPERTY(int autoExposureTarget READ autoExposureTarget WRITE setAutoExposureTarget);
  Q_PROPERTY(bool flipHorizontally READ flipHorizontally WRITE setFlipHorizontally);
  Q_PROPERTY(int maxHeight READ maxHeight);
  Q_PROPERTY(QSize sensorSize READ sensorSize);
  
  
protected:
  PiiGenicamDriver(const QString& wrapperLibrary);
  ~PiiGenicamDriver();
  
public:
  QStringList cameraList() const;
  void initialize(const QString& cameraId);
  bool close();
  bool startCapture(int frames);
  bool stopCapture();
  void* frameBuffer(int frameIndex) const;
  bool isOpen() const;
  bool isCapturing() const;
  bool triggerImage();
  bool requiresInitialization(const char* name) const;
  bool setTriggerMode(PiiCameraDriver::TriggerMode mode);
  PiiCameraDriver::TriggerMode triggerMode() const;
  int bitsPerPixel() const;
  int cameraType() const;
  QSize frameSize() const;
  bool setFrameSize(const QSize& frameSize);
  int imageFormat() const;
  bool setImageFormat(int format);
  QSize resolution() const;
  
  int frameBufferCount() const;
  double frameRate() const;
  int exposureTime() const;
  double gain() const;
  QRect frameRect() const;
  int packetSize() const;
  QRect autoExposureArea() const;
  int autoExposureTarget() const;
  bool flipHorizontally() const;
  int maxHeight() const;
  QSize sensorSize() const;

  bool setFrameBufferCount(int frameBufferCount);
  bool setFrameRate(double frameRate);
  bool setExposureTime(int exposureTime);
  bool setGain(double gain);
  bool setFrameRect(const QRect& frameRect);
  bool setPacketSize(int packetSize);
  bool setAutoExposureArea(const QRect& autoExposureArea);
  bool setAutoExposureTarget(int autoExposureTarget);
  bool setFlipHorizontally(bool flipHorizontally);
  

protected:
  template <class T> T resolveLib(QLibrary& lib, const QString& name);
  void initialize();

  QString _strWrapperLibrary;
  bool _bInitialized;
  
  GenicamIntVoidFunc genicamInitialize;
  GenicamIntVoidFunc genicamTerminate;
  GenicamIntCharppIntpFunc genicamListCameras;
  GenicamCCharpCCharpFunc genicamNextCamera;
  GenicamCCharpVoidFunc genicamLastError;
  GenicamIntCCharpDeviceppFunc genicamOpenDevice;
  GenicamIntDevicepFunc genicamCloseDevice;
  GenicamIntVoidpFunc genicamFree;
  GenicamIntDevicepCCharpIntFunc genicamSetProperty;
  GenicamIntDevicepCCharpIntpFunc genicamGetProperty;
  GenicamIntDevicepUCharpIntFunc genicamRegisterFramebuffers;
  GenicamIntDevicepFunc genicamDeregisterFramebuffers;
  GenicamIntDevicepUCharppIntFunc genicamGrabFrame;
  GenicamIntDevicepFunc genicamRequeueBuffers;
  GenicamIntDevicepFunc genicamStartCapture;
  GenicamIntDevicepFunc genicamStopCapture;

  void capture();
  void stopCapturing();

  genicam_device* _pDevice;

  int _iImageSize;
  unsigned char* _pBuffer;
  
  QStringList _lstCriticalProperties;
  bool _bOpen, _bCapturingRunning;
  QString _strCameraId;
  
  QVector<unsigned char*> _vecBufferPointers;
  
  QThread *_pCapturingThread;
  unsigned int _iFrameIndex;
  int _iMaxFrames, _iHandledFrameCount;
  PiiWaitCondition _triggerWaitCondition;
  PiiCameraDriver::TriggerMode _triggerMode;
  bool _bSoftwareTrigger;
  int _iFrameBufferCount;

private:
  int readIntValue(const char* name, int defaultValue = 0, bool *ok = 0) const;
  bool writeIntValue(const char* name, int value);
  int _iMaxHeight;
  QSize _sensorSize;

};

#endif //_PIIGENICAMDRIVER_H
