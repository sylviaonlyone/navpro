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

#ifndef _PIIIPORTCAMERADRIVER_H
#define _PIIIPORTCAMERADRIVER_H

#include <QMutex>
#include <QThread>
#include <QString>

#include <CyMemoryManager.h>
#include <CyCameraInterface.h>
#include <CyGrabber.h>
#include <CyTypes.h>

#include <PiiCameraDriver.h>

#include <QVector>

class PiiIPortCameraDriverThread;

/**
 * A camera driver that uses the Pleora IPort API for image capture.
 *
 * @ingroup PiiCameraPlugin
 */
class PiiIPortCameraDriver : public QThread, public PiiCameraDriver
{
  Q_OBJECT

  /**
   * The number of frame buffers to reserve for storage.
   */
  Q_PROPERTY(int frameBufferCount READ frameBufferCount WRITE setFrameBufferCount);
  
public:
  PiiIPortCameraDriver();

  ~PiiIPortCameraDriver();

  /**
   * Select the camera unit to use. The active unit is selected by a
   * generic unit id. With the IPort driver case, the unit name should
   * be a name of an XML configuration file for the selected camera
   * unit.
   *
   * @return @p true if the selection was successful, @p false
   * otherwise.
   */
  bool selectUnit(const QString& unit);

  void initialize() = 0;

  bool close();

  bool startCapture(int frames = 0);

  bool stopCapture();

  void* frameBuffer(int frameIndex = 0);

  QSize bufferSize() const;

  QSize frameSize() const;

  bool isOpen() const;

  bool isCapturing() const;

  void setFrameBufferCount(int frameBufferCount);
  int frameBufferCount() const;

protected:
  void run();

private:
  void releaseFrameBuffer(int frameIndex);
  void releaseFrameBuffers();

  // Structure to keep multiple buffers in memory.
  struct FrameBufItem 
  {
    FrameBufItem() : pFrameBuf(0), length(0), lockId(0) {}
    void* pFrameBuf;
    unsigned long length;
    CyBuffer::ReadLockID lockId;
  };

  // Camera unit identification string
  QString _strUnit;
  
  bool _bCameraConnected;
  bool _bCapturingRunning;
  
  QSize _frameSize;
  QMutex _frameBufMutex;
  unsigned int _iFrameIndex;

  CyGrabber* _pGrabber;
  CyCameraInterface* _pCamera;
  CyImageBuffer* _pCBuffer;

  int _iFrameBufferCount;
  QVector<FrameBufItem> _lstFrameBuffers;

  friend class PiiIPortCameraDriverThread;
  PiiIPortCameraDriverThread* _pThread;
};

/**
 * An extra thread that feeds the listener with data.
 */
class PiiIPortCameraDriverThread : public QThread
{
  Q_OBJECT

public:
  PiiIPortCameraDriverThread(PiiIPortCameraDriver* parent);

  void stop();
  void frameCaptured(int frameIndex);

protected:
  void run();

private:
  PiiIPortCameraDriver* _pParent;
  bool _bRunning;
  volatile int _iLastCapturedFrame;
  int _iLastHandledFrame;
};

#endif //_PIIIPORTCAMERADRIVER_H
