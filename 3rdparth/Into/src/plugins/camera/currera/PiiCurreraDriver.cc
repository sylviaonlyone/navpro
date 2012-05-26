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

#include "PiiCurreraDriver.h"
#include <PiiAsyncCall.h>

#include <QMutexLocker>
#include <PiiDelay.h>
#include <QtDebug>

PiiCurreraDriver::PiiCurreraDriver() : _xiHandle(INVALID_HANDLE_VALUE),
                                       _bOpen(false),
                                       _bCapturingRunning(false),
                                       _bBufferingRunning(false),
                                       _iCameraId(-1),
                                       _iFrameBufferCount(10),
                                       _iSkippingLimit(0),
                                       _iFrameSizeInBytes(0),
                                       _pBuffer(0),
                                       _pBufferingThread(0),
                                       _pCapturingThread(0),
                                       _iFrameIndex(-1),
                                       _iLastHandledFrame(-1),
                                       _iMaxFrames(0),
                                       _iHandledFrameCount(0),
                                       _frameWaitCondition(PiiWaitCondition::Queue),
                                       _triggerMode(PiiCameraDriver::SoftwareTrigger),
                                       _bSoftwareTrigger(true),
                                       _iTriggerSource(3)
{
  _lstCriticalProperties << "frameBufferCount"
                         << "triggerSource"
                         << "imageFormat"
                         << "frameSize"
                         << "frameRect"
                         << "flipVertically"
                         << "flipHorizontally";

  _frameTimer.stop();
}

PiiCurreraDriver::~PiiCurreraDriver()
{
  close();
  delete[] _pBuffer;
}


QStringList PiiCurreraDriver::cameraList() const
{
  DWORD devices = 0;
  XI_RETURN ret = xiGetNumberDevices(&devices);
  if (ret != XI_OK)
    {
      piiWarning(tr("Couldn't find any cameras, error code:%1").arg(ret));
      return QStringList();
    }

  QStringList lstCameras;
  for (int i=0; i<(int)devices; i++)
    lstCameras << QString("%1").arg(i);
  
  return lstCameras;
}

void PiiCurreraDriver::initialize(const QString& cameraId)
{
  //qDebug("PiiCurreraDriver::initialize(%s)", qPrintable(cameraId));
  
  if (_bBufferingRunning)
    PII_THROW(PiiCameraDriverException, tr("Buffering is running. Stop the capture first."));

  if (_bCapturingRunning)
    PII_THROW(PiiCameraDriverException, tr("Capturing is running. Stop the capture first."));

  int iCameraId = cameraId.isEmpty() ? _iCameraId : cameraId.toInt();

  // Check that camera driver id has been selected
  if (iCameraId < 0)
    PII_THROW(PiiCameraDriverException, tr("Camera driver id has not been selected"));

  // Check if we must close the driver
  if (iCameraId != _iCameraId)
    close();

  XI_RETURN ret = XI_OK;
  
  // Check if we must open the device
  if (!_bOpen)
    {
      ret = XiApiExt::xiOpenDevice(iCameraId, &_xiHandle);
      if (ret != XI_OK)
        PII_THROW(PiiCameraDriverException, tr("Error: Couldn't open currera device with id %1. Error code: %2").arg(iCameraId).arg(ret));
      _xiImage.size = sizeof(XI_IMG);
    }

  // Reserve the first input for trigger
  ret = XiApiExt::xiWriteInputMode(_xiHandle, 1, XI_GPI_TRIGGER);
  if (ret != XI_OK)
    PII_THROW(PiiCameraDriverException, tr("Error: Couldn't initialize trigger input. Error code: %2").arg(ret));

  _iCameraId = iCameraId;

  // Get property map
  QVariantMap& dataMap = propertyMap();
  
  // Set image format
  if (!setImageFormat(dataMap.contains("imageFormat") ? dataMap.take("imageFormat").toInt() : 1))
    PII_THROW(PiiCameraDriverException, tr("Couldn't set image format"));

  // Set trigger source mode
  if (!setTriggerSource(dataMap.contains("triggerSource") ? dataMap.take("triggerSource").toInt() : _iTriggerSource))
    PII_THROW(PiiCameraDriverException, tr("Couldn't set trigger source"));

  // Set camera in BP_SAFE-mode
  if (!writeIntValue("buffer_policy", 1))
    PII_THROW(PiiCameraDriverException, tr("Couldn't set buffer policy"));

  
  // Update frameRect
  if (dataMap.contains("frameRect") && !setFrameRect(dataMap.take("frameRect").toRect()))
    PII_THROW(PiiCameraDriverException, tr("Couldn't set frameRect."));
  
  // Write all configuration values from the map
  for (QVariantMap::iterator i=dataMap.begin(); i != dataMap.end(); ++i)
    {
      if (!QObject::setProperty(qPrintable(i.key()), i.value()))
        PII_THROW(PiiCameraDriverException, tr("Couldn't write the configuration value '%1'").arg(i.key()));
    }
  dataMap.clear();
  
  _bSoftwareTrigger = _triggerMode == PiiCameraDriver::SoftwareTrigger;
  
  bool bWidth = true, bHeight = true;
  
  int iWidth = readIntValue("width", 0, &bWidth);
  int iHeight = readIntValue("height", 0, &bHeight);
  if (!bWidth || !bHeight)
    PII_THROW(PiiCameraDriverException, tr("Error: Couldn't initialize frameBuffer"));
  
  // check frame-size
  _iFrameSizeInBytes = iWidth * iHeight * bitsPerPixel()/8;
  unsigned char *pBuffer = new unsigned char[_iFrameBufferCount * _iFrameSizeInBytes];
  _vecBufferPointers.fill(0,_iFrameBufferCount);
  _vecFrameTimes.fill(0,_iFrameBufferCount);

  delete[] _pBuffer;
  _pBuffer = pBuffer;
  
  _iSkippingLimit = _iFrameBufferCount/2;

  _bOpen = true;
}

bool PiiCurreraDriver::close()
{
  if (!_bOpen)
    return false;

  stopCapture();

  delete _pCapturingThread;
  delete _pBufferingThread;
  _pCapturingThread = 0;
  _pBufferingThread = 0;
  
  bool result = true;
  XI_RETURN ret = XiApiExt::xiCloseDevice(_xiHandle);
  if (ret != XI_OK)
    {
      piiWarning(tr("Couldn't close the device. Error code:%1").arg(ret));
      result = false;
    }
  _bOpen = false;

  return result;
}

bool PiiCurreraDriver::setTriggerMode(PiiCameraDriver::TriggerMode mode)
{
  _triggerMode = mode;
  return true;
}

PiiCameraDriver::TriggerMode PiiCurreraDriver::triggerMode() const
{
  return _triggerMode;
}

bool PiiCurreraDriver::triggerImage()
{
  _triggerWaitCondition.wakeOne();
  return true;
}

bool PiiCurreraDriver::startCapture(int frames)
{
  if (!_bOpen || listener() == 0 || _bCapturingRunning || _bBufferingRunning)
    return false;
  
  // Create and start the buffering and capturing threads
  if (_pBufferingThread == 0)
    _pBufferingThread = Pii::createAsyncCall(this, &PiiCurreraDriver::buffer);

  if (_pCapturingThread == 0)
    _pCapturingThread = Pii::createAsyncCall(this, &PiiCurreraDriver::capture);
  
  _bCapturingRunning = true;
  _bBufferingRunning = true;
  _iFrameIndex = -1;
  _iLastHandledFrame = -1;
  _iHandledFrameCount = 0;
  _iMaxFrames = _bSoftwareTrigger ? 0 : frames;
  
  // Start acquisition
  if (xiStartAcquisition(_xiHandle) != XI_OK)
    {
      piiWarning(tr("Couldn't start acquisition with camera id (%1)").arg(_iCameraId));
      _bCapturingRunning = false;
      _bBufferingRunning = false;
    }
  else
    {
      _pBufferingThread->start();
      _pCapturingThread->start();
    }
  
  return true;
}

bool PiiCurreraDriver::stopCapture()
{
  if (!_bBufferingRunning && !_bCapturingRunning)
    return false;

  // Stop the buffering and capturing threads
  stopBuffering();
  stopCapturing();
  
  return true;
}

void PiiCurreraDriver::stopBuffering()
{
  _bBufferingRunning = false;
  _triggerWaitCondition.wakeAll();
  _pBufferingThread->wait();
}

void PiiCurreraDriver::stopCapturing()
{
  _bCapturingRunning = false;
  _frameWaitCondition.wakeAll();
  _pCapturingThread->wait();
}

void PiiCurreraDriver::buffer()
{
  XI_RETURN ret = XI_OK;
  
  while(_bBufferingRunning)
    {
      if (_bSoftwareTrigger)
        _triggerWaitCondition.wait();
      else
        PiiDelay::msleep(1);
      
      if (!_bBufferingRunning)
        break;
      
      _frameBufMutex.lock();

      // Increase the frame index
      _iFrameIndex++;

      int iFrameIndex = frameIndex(_iFrameIndex);

      bool bFakeFrame = false;
      unsigned char *buffer;
      
      // If our buffer is full, we must allocate fake buffer and still
      // increase frame index. Free this buffer at the end.
      if (_iFrameIndex - _iLastHandledFrame > (unsigned int)_iFrameBufferCount)
        {
          bFakeFrame = true;
          buffer = (unsigned char*)malloc(_iFrameSizeInBytes);
        }
      else
        buffer = _pBuffer + iFrameIndex*_iFrameSizeInBytes;
      
      _xiImage.bp = buffer;
      _xiImage.bp_size = _iFrameSizeInBytes;

      // Generate trigger if necessary
      if (_iTriggerSource == 3)
        {
          ret = xiSetParamInt(_xiHandle, "trigger_software", 0);
          if (ret != XI_OK && ret != 49)
            piiWarning(tr("Error: Couldn't write 'trigger_software'. Error code: %").arg(ret));
        }
      
      // Get image
      XI_RETURN ret = XiApiExt::xiGetImage(_xiHandle, 1000, &_xiImage);
      if (ret != XI_OK)
        {
          piiWarning(tr("Couldn't get image, Error code: %1").arg(ret));
          if (bFakeFrame)
            free(buffer);
          _iFrameIndex--;
          
          _frameBufMutex.unlock();
          continue;
        }

      // Check if we must free fake buffer
      if (bFakeFrame)
        free(buffer);
      else
        {
          _vecBufferPointers[iFrameIndex] = buffer;
          _vecFrameTimes[iFrameIndex] = _frameTimer.restart();
        }

      _frameBufMutex.unlock();
      _frameWaitCondition.wakeOne();
    }
}

void PiiCurreraDriver::capture()
{
  _pCapturingThread->setPriority(QThread::HighestPriority);

  while (_bCapturingRunning)
    {
      _frameWaitCondition.wait();
      if (!_bCapturingRunning)
        break;
      
      // Check if we already handled the last captured frame
      if (_iFrameIndex == _iLastHandledFrame)
        continue;

      _frameBufMutex.lock();

      // Increase handled frame counter
      _iLastHandledFrame++;
      
      // Check if we must skip frames...
      if (_iFrameIndex - _iLastHandledFrame > (unsigned int)_iSkippingLimit)
        {
          // Inform listener
          listener()->framesMissed(_iLastHandledFrame, _iFrameIndex-1);
          _iLastHandledFrame = _iFrameIndex;

          // Clear the wait condition queue
          _frameWaitCondition.wakeAll();
        }

      // Inform listener that a frame has been captured
      listener()->frameCaptured(_iLastHandledFrame, 0, _vecFrameTimes[frameIndex(_iLastHandledFrame)]);

      _frameBufMutex.unlock();
      
      // Check if we must stop capturing
      if (_iMaxFrames > 0 && _iHandledFrameCount++ >= _iMaxFrames)
        {
          stopBuffering();
          _bCapturingRunning = false;
        }
    }

  // Stop acquisition.
  xiStopAcquisition(_xiHandle);

  // Inform listener
  listener()->captureFinished();

}

int PiiCurreraDriver::frameIndex(int frameIndex) const
{
  int iFrameIndex = frameIndex % _iFrameBufferCount;
  if(iFrameIndex < 0)
    iFrameIndex += _iFrameBufferCount;

  return iFrameIndex;
}

void* PiiCurreraDriver::frameBuffer(int index) const
{
  return _vecBufferPointers[frameIndex(index)];
}

bool PiiCurreraDriver::isOpen() const
{
  return _bOpen;
}

bool PiiCurreraDriver::isCapturing() const
{
  return _bCapturingRunning;
}

bool PiiCurreraDriver::requiresInitialization(const char* name) const
{
  if (_lstCriticalProperties.contains(QString(name)))
    return true;

  return PiiCameraDriver::requiresInitialization(name);
}


int PiiCurreraDriver::frameBufferCount() const
{
  return _iFrameBufferCount;
}
int PiiCurreraDriver::triggerSource() const
{
  return readIntValue("trigger_source", 3);
}

int PiiCurreraDriver::exposureTime() const
{
  return readIntValue("exposure", 1);
}
bool PiiCurreraDriver::flipHorizontally() const
{
  return readExtensionValue(EXT_HORIZONTAL_FLIP, 0) != 0;
}
bool PiiCurreraDriver::flipVertically() const
{
  return readExtensionValue(EXT_VERTICAL_FLIP, 0) != 0;
}
double PiiCurreraDriver::gain() const
{
  int iMin = readIntValue("gain:min", 0);
  int iMax = readIntValue("gain:max", 2);
  int iGain = readIntValue("gain", 1);

  return qBound(0.0, (double)(iGain-iMin) / (double)(iMax-iMin), 1.0);
}
QRect PiiCurreraDriver::frameRect() const
{
  return QRect(readIntValue("offsetX", 0),
               readIntValue("offsetY", 0),
               readIntValue("width", 0),
               readIntValue("height", 0));
}
QSize PiiCurreraDriver::frameSize() const
{
  return QSize(readIntValue("width", 0),
               readIntValue("height", 0));
}
int PiiCurreraDriver::imageFormat() const
{
  int iFormat = readIntValue("imgdataformat", 0);
  
  switch(iFormat)
    {
    case 0:
    case 1: return (int)PiiCamera::MonoFormat;
    case 3: return (int)PiiCamera::RgbFormat;
    default: return (int)PiiCamera::InvalidFormat;
    }
  
  return (int)PiiCamera::InvalidFormat;
}

QSize PiiCurreraDriver::resolution() const
{
  return QSize(readIntValue("width:max", 0) + readIntValue("offsetX",0),
               readIntValue("height:max", 0) + readIntValue("offsetY",0));
}
int PiiCurreraDriver::bitsPerPixel() const
{
  int iBitsPerPixel = readIntValue("imgdataformat", 0);
  switch (iBitsPerPixel)
    {
    case 1: return 16;
    case 3: return 32;
    default: return 8;
    }

  return 8;
}


bool PiiCurreraDriver::setFrameBufferCount(int frameBufferCount)
{
  _iFrameBufferCount = frameBufferCount;
  return true;
}

bool PiiCurreraDriver::setTriggerSource(int mode)
{
  if (writeIntValue("trigger_source", mode))
    {
      _iTriggerSource = mode;
      return true;
    }
  return false;
}

bool PiiCurreraDriver::setExposureTime(int exposureTime)
{
  return writeIntValue("exposure", exposureTime);
}
bool PiiCurreraDriver::setGain(double gain)
{
  int iMin = readIntValue("gain:min", 0);
  int iMax = readIntValue("gain:max", 2);
  int iGain = iMin + (double)(iMax-iMin) * qBound(0.0,gain,1.0);
  return writeIntValue("gain", iGain);
}

bool PiiCurreraDriver::setFrameRect(const QRect& frameRect)
{
  int iWidth = frameRect.width() & ~3;
  int iHeight = frameRect.height() & ~3;
  int iOffsetX = frameRect.x() & ~3;
  int iOffsetY = frameRect.y() & ~3;
  int iMaxWidth = readIntValue("width:max") + readIntValue("offsetX",0);
  int iMaxHeight = readIntValue("height:max") + readIntValue("offsetY",0);
  int iMinWidth = 32;// readIntValue("width:min", 32);
  int iMinHeight = 32; //readIntValue("height:min", 32);

  if (iWidth < 0) iWidth = iMaxWidth;
  if (iHeight < 0) iHeight = iMaxHeight;
  
  iOffsetX = qBound(0, iOffsetX, iMaxWidth - iMinWidth);
  iOffsetY = qBound(0, iOffsetY, iMaxHeight - iMinHeight);
  iWidth = qBound(iMinWidth, iWidth, iMaxWidth - iOffsetX);
  iHeight = qBound(iMinHeight, iHeight, iMaxHeight - iOffsetY);

  bool bRet =  writeIntValue("offsetX", 0);
  bRet = writeIntValue("offsetY", 0) && bRet;
  bRet = writeIntValue("width", iWidth) && bRet;
  bRet = writeIntValue("height", iHeight) && bRet;
  bRet = writeIntValue("offsetX", iOffsetX) && bRet;
  bRet = writeIntValue("offsetY", iOffsetY) && bRet;
  
  return bRet;
}

bool PiiCurreraDriver::setFrameSize(const QSize& frameSize)
{
  return writeIntValue("width", frameSize.width()) &&
    writeIntValue("height", frameSize.height());
}
bool PiiCurreraDriver::setImageFormat(int value)
{
  int format = 0;
  if (value == (int)PiiCamera::MonoFormat)
    format = bitsPerPixel() == 8 ? 0 : 1;
  else if (value == (int)PiiCamera::RgbFormat)
    format = 3;
  else
    format = 0;
  
  return writeIntValue("imgdataformat", format);
}

bool PiiCurreraDriver::setFlipHorizontally(bool flipHorizontally)
{
  return writeExtensionValue(EXT_HORIZONTAL_FLIP, flipHorizontally ? 1 : 0);
}

bool PiiCurreraDriver::setFlipVertically(bool flipVertically)
{
  return writeExtensionValue(EXT_VERTICAL_FLIP, flipVertically ? 1 : 0);
}


bool PiiCurreraDriver::writeExtensionValue(MM_EXTENSION extension, int value)
{
	MM40_RETURN ret = mmSetExtention(_xiHandle, extension, value);
	if (ret != MM40_OK)
	{
		piiWarning(tr("Error: Couldn't write the extension value '%1' to the camera. Error code: %2").arg(extension).arg(ret));
		return false;
	}
	return true;
}

int PiiCurreraDriver::readExtensionValue(MM_EXTENSION extension, int defaultValue, bool *ok) const
{
  int value = 0;
  MM40_RETURN ret = mmGetExtention(_xiHandle, extension, LPDWORD(&value));
  if (ret != MM40_OK)
    {
      piiWarning(tr("Error: Couldn't read the extension value '%1' from the camera. Error code: %2").arg(extension).arg(ret));
      if (ok)
        *ok = false;

      return defaultValue;
    }

  if (ok)
    *ok = true;

  return value;
}

bool PiiCurreraDriver::writeIntValue(const char* name, int value)
{
  XI_RETURN ret = xiSetParamInt(_xiHandle, name, value);
  if (ret != XI_OK)
    {
      piiWarning(tr("Error: Couldn't write the configuration value '%1' to the camera. Error code: %2").arg(name).arg(ret));
      return false;
    }

  return true;
}

int PiiCurreraDriver::readIntValue(const char* name, int defaultValue, bool *ok) const
{
  int value = 0;
  XI_RETURN ret = xiGetParamInt(_xiHandle, name, &value);
  if (ret != XI_OK)
    {
      piiWarning(tr("Error: Couldn't read the configuration value '%1' from the camera. Error code: %2").arg(name).arg(ret));
      if (ok)
        *ok = false;
      return defaultValue;
    }

  if (ok)
    *ok = true;

  return value;
}

