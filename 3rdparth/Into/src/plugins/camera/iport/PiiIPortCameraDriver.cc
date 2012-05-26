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

#include "PiiIPortCameraDriver.h"

#include <QFile>
#include <QString>

#include <CyCameraRegistry.h>
#include <CyConfig.h>
#include <CyXMLDocument.h>
#include <CyTypes.h>

#include <cstdio>
#include <iostream>
#include <string>

#include "PiiIPortCameraDriver.h"


PiiIPortCameraDriver::PiiIPortCameraDriver() :
  _pCamera(0), _pGrabber(0),
  _bCameraConnected(false), _bCapturingRunning(false),
  _iFrameIndex(0)
{}

PiiIPortCameraDriver::~PiiIPortCameraDriver()
{
  releaseFrameBuffers();
  close();
}

/*
 * In Pleora camera driver unit name should be filename to XML configuration
 * file for selected camera unit.
 */
bool PiiIPortCameraDriver::selectUnit(const QString& unit)
{
  QFile file(unit);

  // Test for configuration file existence.
  if(file.exists())
    {
      _strUnit = unit;
      return true;
    }

  _strUnit = "";
  return false;
}


void PiiIPortCameraDriver::initialize()
{
  // Check that camera unit has been selected.
  if(_strUnit.length() < 1)
    throw PiiCameraDriverException("Camera unit hasn't been selected.");

  char cameraType[128];

  CyString strXmlFileName(_strUnit.toStdString());
  CyXMLDocument cCfgXmlDoc(strXmlFileName);
  CyConfig cCamCfg;
  CyCameraRegistry cReg;
  CyResult res;

  // Read XML file
  cCfgXmlDoc.LoadDocument();

  // Load configuration
  if ( (res=cCamCfg.LoadFromXML(cCfgXmlDoc))!= CY_RESULT_OK)
    throw PiiCameraDriverException(tr("Invalid configuration file: %1. Error code: %d.").arg(_strUnit).arg(res));

  // Seek to first configuration index - this must be done even if
  // there is only one camera configuration.
  cCamCfg.GoFirst();

  _pGrabber = new CyGrabber();
  
  // Connect to grabber
  if (_pGrabber->Connect(cCamCfg) != CY_RESULT_OK)
    {
      delete _pGrabber; _pGrabber = 0;
      throw PiiException(tr("Cannot connect to grabber. (%1)").arg(_strUnit));
    }

  // Read camera type from configuration
  cCamCfg.GetDeviceType(cameraType, sizeof(cameraType));

  // And find the right camera from registry
  if (cReg.FindCamera(cameraType) != CY_RESULT_OK)
    {
      _pGrabber->Disconnect();
      delete _pGrabber;
      _pGrabber = 0;

      throw PiiCameraDriverException(tr("Cannot find camera type from camera registry. (%1)").arg(_strUnit));
    }

  // Create the camera interface instance for camera type found from registry.
  if (cReg.CreateCamera(&_pCamera, _pGrabber) != CY_RESULT_OK)
    {
      _pGrabber->Disconnect();
      delete _pGrabber;
      _pGrabber = 0;

      throw PiiCameraDriverException(tr("Cannot create camera interface instance. (%1)").arg(_strUnit));
    }

  // Load camera settings from configuration.
  if(_pCamera->LoadFromXML(cCfgXmlDoc) != CY_RESULT_OK)
    {
      _pGrabber->Disconnect();
      delete _pCamera;
      delete _pGrabber;
    
      _pCamera = 0;
      _pGrabber = 0;
    
      throw PiiCameraDriverException(tr("Cannot load camera configuration from XML. (%1)").arg(_strUnit));
    }

  // Send loaded settings to camera
  if(_pCamera->UpdateToCamera() != CY_RESULT_OK)
    {
      _pGrabber->Disconnect();
      delete _pCamera;
      delete _pGrabber;

      _pCamera = 0;
      _pGrabber = 0;

      throw PiiCameraDriverException(tr("Cannot send configuration to camera.").arg(_strUnit));
    }

  _lstFrameBuffers.resize(_iFrameBufferCount);
  _bCameraConnected = true;
}

/*
 * Close open camera device.
 */
bool PiiIPortCameraDriver::close()
{
  if (!_bCameraConnected)
    return false;

  stopCapture();

  bool result = true;
  if (_pGrabber->Disconnect() != CY_RESULT_OK)
    result = false;
  
  delete _pCamera;
  delete _pGrabber;
  _pCamera = 0;
  _pGrabber = 0;
  _bCameraConnected = false;
  
  return result;
}


bool PiiIPortCameraDriver::startCapture(int /*frames*/)
{
  if (!_bCameraConnected || listener() == 0 || _bCapturingRunning)
    return false;

  // Start the feeding thread
  _pThread = new PiiIPortCameraDriverThread(this);
  
  // Start capturing thread.
  start();

  return true;
}

bool PiiIPortCameraDriver::stopCapture()
{
  if (!_bCapturingRunning)
    return false;

  _bCapturingRunning = false;

  wait();
  _pThread->stop();
  delete _pThread;
  _pThread = 0;

  releaseFrameBuffers();

  return true;
}

QSize PiiIPortCameraDriver::bufferSize() const
{
  QSize frameSize = frameSize();
  return QSize(frameSize.width(), frameSize.height() * _iFrameBufferCount);
}

QSize PiiIPortCameraDriver::frameSize() const
{
  if (!_pCamera)
    return QSize();
  
  __int64 x, y;
  _pCamera->GetParameter(CY_CAMERA_PARAM_SIZE_X, x);
  _pCamera->GetParameter(CY_CAMERA_PARAM_SIZE_Y, y);
  return QSize(x,y);
}                      

/*
 * This thread only stores captured buffers to the frame buffer list.
 * Another thread is created for feeding the data to the listener.
 */
void PiiIPortCameraDriver::run()
{
  _bCapturingRunning = true;
  _iFrameIndex = 0;
  CyResult res;

  _pCBuffer = new CyImageBuffer(frameSize().width(), frameSize().height(),
                                CyGrayscale8::ID);

  // Set queue size
  res = _pCBuffer->SetQueueSize(_iFrameBufferCount);

  //std::cout << "Result code for cBuffer.SetQueueSize(9216000): " << res << std::endl;
  qDebug("Buffer capacity: %li Queue size: %li", _pCBuffer->GetCapacity(), _pCBuffer->GetQueueSize());

  if (_pGrabber->StartGrabbing(CyChannel(0), *_pCBuffer, 0) != CY_RESULT_OK)
    {
      qWarning("Can't start grabbing. (%s)", qPrintable(_strUnit));
      _bCapturingRunning = false;
      return;
    }

  while(_bCapturingRunning)
    {
      //qDebug("Queue items before: %i", _pCBuffer->GetQueueItemCount());
      
      _frameBufMutex.lock();

      // If the next frame is already reserved, fail...
      if (_lstFrameBuffers[_iFrameIndex].pFrameBuf != 0)
        {
          listener()->framesMissed(_iFrameIndex, _iFrameIndex);
          _frameBufMutex.unlock();
          continue;
        }

      res = _pCBuffer->LockForRead((void**)&_lstFrameBuffers[_iFrameIndex].pFrameBuf,
                                   &_lstFrameBuffers[_iFrameIndex].length,
                                   &_lstFrameBuffers[_iFrameIndex].lockId, 
                                   CY_BUFFER_FLAG_ERROR_IF_EMPTY);      

      if (res != CY_RESULT_OK)
        {
          // This shouldn't occur but let's still test.
          qWarning("Could not acquire frame buffer lock. Error code: %d", res);
          
          // Make sure the pointer is zero.
          _lstFrameBuffers[_iFrameIndex].pFrameBuf = 0;
          _frameBufMutex.unlock();
          continue;
        }

      _frameBufMutex.unlock();
      
      _pThread->frameCaptured(_iFrameIndex);
      
      // Increase frame counter (circular indexing).
      _iFrameIndex = (_iFrameIndex + 1) % _iFrameBufferCount;
    }
  
  // Stop grabber.
  _pGrabber->StopGrabbing(CyChannel(0));
  
  // Inform listener
  listener()->captureFinished();
  
  _iFrameIndex = 0;
  
  delete _pCBuffer;
  _pCBuffer = 0;
}

void* PiiIPortCameraDriver::frameBuffer(int frameIndex)
{
  if (frameIndex >= _iFrameBufferCount)
    frameIndex %= _iFrameBufferCount;
  else
    while (frameIndex < 0) frameIndex += _iFrameBufferCount;

  QMutexLocker locker(&_frameBufMutex);

  return _lstFrameBuffers[frameIndex].pFrameBuf;
}

bool PiiIPortCameraDriver::isOpen() const
{
  return _bCameraConnected;
}

bool PiiIPortCameraDriver::isCapturing() const
{
  return isRunning();
}

/*
 * Release Pleora frame buffer locking for this index.
 */
void PiiIPortCameraDriver::releaseFrameBuffer(int frameIndex)
{
  QMutexLocker locker(&_frameBufMutex);

  if (_lstFrameBuffers[frameIndex].pFrameBuf != 0)
    {    
      _pCBuffer->SignalReadEnd(_lstFrameBuffers[frameIndex].lockId);
      _lstFrameBuffers[frameIndex].pFrameBuf = 0;
    }
}

void PiiIPortCameraDriver::releaseFrameBuffers()
{
  QMutexLocker locker(&_frameBufMutex);

  for (int i=_lstFrameBuffers.size(); i--; )
    if (_lstFrameBuffers[i].pFrameBuf != 0)
      {    
        _pCBuffer->SignalReadEnd(_lstFrameBuffers[i].lockId);
        _lstFrameBuffers[i].pFrameBuf = 0;
      }
}


PiiIPortCameraDriverThread::PiiIPortCameraDriverThread(PiiIPortCameraDriver* parent) :
  _pParent(parent), _bRunning(false), _iLastCapturedFrame(-1), _iLastHandledFrame(-1)
{
  // Start automatically
  start();
}

void PiiIPortCameraDriverThread::stop()
{
  _bRunning = false;
  wait();
}

void PiiIPortCameraDriverThread::frameCaptured(int frameIndex)
{
  _iLastCapturedFrame = frameIndex;
}


void PiiIPortCameraDriverThread::run()
{
  setPriority(QThread::HighestPriority);

  _bRunning = true;
  _iLastHandledFrame = -1;
  _iLastCapturedFrame = -1;

  while (_bRunning)
    {
      // If frame queue is empty, sleep a little while so that this
      // loop doesn't affect system performance.
      if (_iLastCapturedFrame == _iLastHandledFrame)
        {
          usleep(75);
          continue;
        }

      _iLastHandledFrame = (_iLastHandledFrame+1) % _pParent->_iFrameBufferCount;
      
      // Inform listener that a frame has been captured.
      _pParent->listener()->frameCaptured(_iLastHandledFrame);
      // Release read lock
      _pParent->releaseFrameBuffer(_iLastHandledFrame);
    }
}

void PiiIPortCameraDriver::setFrameBufferCount(int frameBufferCount) { _iFrameBufferCount = frameBufferCount; }
int PiiIPortCameraDriver::frameBufferCount() const { return _iFrameBufferCount; }
