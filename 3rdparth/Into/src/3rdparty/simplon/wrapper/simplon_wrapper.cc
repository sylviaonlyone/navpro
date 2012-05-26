/* This file is part of Into.
 * Copyright (C) Intopii
 * All rights reserved.
 */

#include "genicam_wrapper.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>

#include <lv.simplon.class.h>

using namespace std;

struct genicam_device
{
  genicam_device(LvInterface *i, LvDevice *d, LvStream *s) :
    iface(i), device(d), stream(s), event(0)
  {}

  LvInterface *iface;
  LvDevice *device;
  LvStream *stream;
  LvEvent *event;
  std::vector<LvBuffer*> vecHandles;
  std::vector<LvBuffer*> vecReservedHandles;
};

static int iInstanceCount = 0;
static std::vector<genicam_device*> vecDevices;
static LvSystem *pSystem = 0;
static std::string strLastError = "";

void updateLastErrorMessage(const char* prefix)
{
  strLastError = std::string(prefix) + LvLibrary::GetLastErrorMessage();
}

int setInt(genicam_device* genicamdevice, LvFeature ename, const char* name, int value)
{
  if (genicamdevice->device->SetInt32(ename, value) == LVSTATUS_OK)
    return 0;
  else
    {
      strLastError = std::string("Failed to set property(" + std::string(name) + "): " + LvLibrary::GetLastErrorMessage());
      return 1;
    }
}

int setFloat(genicam_device* genicamdevice, LvFeature ename, const char* name, double value)
{
  if (genicamdevice->device->SetFloat(ename, value) == LVSTATUS_OK)
    return 0;
  else
    {
      strLastError = std::string("Failed to set property(" + std::string(name) + "): " + LvLibrary::GetLastErrorMessage());
      return 1;
    }
}

int setEnum(genicam_device* genicamdevice, LvFeature ename, const char* name, LvEnum evalue)
{
  if (genicamdevice->device->SetEnum(ename, evalue) == LVSTATUS_OK)
    return 0;
  else
    {
      strLastError = std::string("Failed to set property(" + std::string(name) + "): " + LvLibrary::GetLastErrorMessage());
      return 1;
    }
}

int getInt(genicam_device* genicamdevice, LvFeature ename, const char* name, int *value)
{
  int32_t iValue = 0;
  if (genicamdevice->device->GetInt32(ename, &iValue) == LVSTATUS_OK)
    {
      *value = iValue;
      return 0;
    }
  else
    {
      strLastError = std::string("Failed to get property(" + std::string(name) + "): " + LvLibrary::GetLastErrorMessage());
      return 1;
    }
}

int getFloat(genicam_device* genicamdevice, LvFeature ename, const char* name, int *value)
{
  double dValue = 0;
  if (genicamdevice->device->GetFloat(ename, &dValue) == LVSTATUS_OK)
    {
      *value = (int)dValue;
      return 0;
    }
  else
    {
      strLastError = std::string("Failed to get property(" + std::string(name) + "): " + LvLibrary::GetLastErrorMessage());
      return 1;
    }
}

int getFloatMin(genicam_device* genicamdevice, LvFeature ename, const char* name, int *value)
{
  double dMin = 0;
  if (genicamdevice->device->GetFloatRange(ename, &dMin, NULL, NULL) == LVSTATUS_OK)
    {
      *value = (int)dMin;
      return 0;
    }
  else
    {
      strLastError = std::string("Failed to get property(" + std::string(name) + "): " + LvLibrary::GetLastErrorMessage());
      return 1;
    }
}

int getFloatMax(genicam_device* genicamdevice, LvFeature ename, const char* name, int *value)
{
  double dMax = 0;
  if (genicamdevice->device->GetFloatRange(ename, NULL, &dMax, NULL) == LVSTATUS_OK)
    {
      *value = (int)dMax;
      return 0;
    }
  else
    {
      strLastError = std::string("Failed to get property(" + std::string(name) + "): " + LvLibrary::GetLastErrorMessage());
      return 1;
    }
}


GENICAM_WAPI(int) genicam_initialize(void)
{
  if (++iInstanceCount == 1)
    {
      if (LvOpenLibrary() != LVSTATUS_OK)
        {
          updateLastErrorMessage("Failed to open library: ");
          return 1;
        }
      if (LvSystem::Open("", pSystem) != LVSTATUS_OK)
        {
          updateLastErrorMessage("Failed to open system: ");
          return 1;
        }
    }

  return 0;
}

GENICAM_WAPI(int) genicam_terminate(void)
{
  int retValue = 0;
  if (--iInstanceCount == 0)
    {
      while (vecDevices.size() > 0)
        {
          genicam_close_device(vecDevices.at(0));
        }
      
      if (LvSystem::Close(pSystem) != LVSTATUS_OK)
        {
          retValue = 1;
          strLastError = std::string("Failed to close system.");
        }
      if (LvCloseLibrary() != LVSTATUS_OK)
        {
          retValue = 1;
          strLastError = std::string("Failed to close library.");
        }
    }

  return retValue;
}

GENICAM_WAPI(int) genicam_free(void* data)
{
	free(data);
  return 0;
}

GENICAM_WAPI(const char*) genicam_last_error(void)
{
  return strLastError.c_str();
}

static void append_string(char** names, const char* name, int* len, int* bufferSize)
{
  int iNameLen = strlen(name);
  if (*names == 0)
    {
      *names = (char*)malloc(iNameLen + 2);
      *len = 1;
    }
  else if (*bufferSize <= *len + iNameLen)
    {
      *bufferSize = *len + iNameLen + 1;
      *names = (char*)realloc(*names, *bufferSize);
    }
  memcpy(*names + *len - 1, name, iNameLen + 1);
  *len += iNameLen + 1;
  (*names)[*len-1] = 0;
}


GENICAM_WAPI(const char*) genicam_next_camera(const char* camera)
{
  camera += strlen(camera) + 1;
  if (*camera == 0)
    return 0;

  return camera;
}

GENICAM_WAPI(int) genicam_list_cameras(char** names, int* count)
{
  *names = 0;
  *count = 0;
  if (pSystem != 0)
    {
      /**
       * Find all devices from the all interfaces. Collect the list
       * where the the cameraId is named as INTERFACE:DEVICE.
       */
      int iLen = 0, iBufferSize = 0;
      
      uint32_t iNumberOfInterfaces;
      pSystem->GetNumberOfInterfaces(&iNumberOfInterfaces);
      for (int i=0; i<(int)iNumberOfInterfaces; i++)
        {
          std::string sInterface;
          pSystem->GetInterfaceId(i, sInterface);
          LvInterface* pInterface;
          if (pSystem->OpenInterface(sInterface.c_str(), pInterface) == LVSTATUS_OK)
            {
              uint32_t iNumberOfDevices;
              pInterface->GetNumberOfDevices(&iNumberOfDevices);
              for (int d=0; d<(int)iNumberOfDevices; d++)
                {
                  std::string sDevice;
                  pInterface->GetDeviceId(d, sDevice);
                  std::string sCameraId = sInterface + "$" + sDevice;
                  append_string(names, sCameraId.c_str(), &iLen, &iBufferSize);
                  ++*count;
                }
            }
          pSystem->CloseInterface(pInterface);
        }
    }
  else
    {
      strLastError = std::string("System is not initialized.");
      return 1;
    }
  
  return 0;
}

GENICAM_WAPI(int) genicam_open_device(const char* serial, genicam_device** device)
{
  std::string sSerial(serial);
  int delimiterPos = sSerial.find('$');

  std::string sInterface = sSerial.substr(0,delimiterPos);
  std::string sDevice = sSerial.substr(delimiterPos+1,sSerial.length()-delimiterPos-1);
  
  LvInterface* pInterface = 0;
  LvDevice* pDevice = 0;
  LvStream* pStream = 0;
  *device = 0;
  
  if (pSystem->OpenInterface(sInterface.c_str(), pInterface) != LVSTATUS_OK ||
      pInterface->OpenDevice(sDevice.c_str(), pDevice, LvDeviceAccess_Control) != LVSTATUS_OK)
    {
      updateLastErrorMessage("Failed to open device: ");
      return 1;
    }

   // Open stream
   if (pDevice->OpenStream("", pStream) != LVSTATUS_OK)
     {
       updateLastErrorMessage("Failed to open stream: ");
       return 1;
     }

   // Set the camera to continuous frame mode
   if (pDevice->SetEnum(LvDevice_AcquisitionMode, LvAcquisitionMode_Continuous) != LVSTATUS_OK ||
       pDevice->SetEnum(LvDevice_TriggerSelector, LvTriggerSelector_FrameStart) != LVSTATUS_OK ||
       pDevice->SetEnum(LvDevice_TriggerMode, LvTriggerMode_Off) != LVSTATUS_OK ||
       pDevice->SetEnum(LvDevice_ExposureMode, LvExposureMode_Timed) != LVSTATUS_OK ||
       pDevice->SetFloat(LvDevice_ExposureTime, 20000.0) != LVSTATUS_OK)
     updateLastErrorMessage("Failed to initialize default values: ");
  
   *device = new genicam_device(pInterface, pDevice, pStream);
   vecDevices.push_back(*device);
   
   return 0;
}

GENICAM_WAPI(int) genicam_close_device(genicam_device* genicamdevice)
{
  int retValue = 0;
  retValue = genicam_deregister_framebuffers(genicamdevice);

  // First close stream
  if (genicamdevice->device->CloseStream(genicamdevice->stream) != LVSTATUS_OK ||
      genicamdevice->iface->CloseDevice(genicamdevice->device) != LVSTATUS_OK ||
      pSystem->CloseInterface(genicamdevice->iface) != LVSTATUS_OK)
    {
      retValue = 1;
      updateLastErrorMessage("Failed to close device: ");
    }
  
  vecDevices.erase(std::find(vecDevices.begin(), vecDevices.end(), genicamdevice));

  return retValue;
}

GENICAM_WAPI(int) genicam_register_framebuffers(genicam_device* genicamdevice, unsigned char* buffer, int count)
{
  // Resize vecHandles
  genicamdevice->vecHandles.resize(count,0);
  
  // Open event
  int iImageSize;
  if (genicamdevice->stream->OpenEvent(LvEventType_NewBuffer, genicamdevice->event) != LVSTATUS_OK ||
      genicamdevice->stream->GetInt32(LvStream_LvCalcPayloadSize, &iImageSize) != LVSTATUS_OK)
    {
      updateLastErrorMessage("Failed to open event: ");
      return 1;
    }
  
  // Open and queue buffers
  for (int i=0; i<count; ++i)
    {
      if (genicamdevice->stream->OpenBuffer((void*)(buffer + i*iImageSize),
                                            iImageSize, NULL,
                                            0, genicamdevice->vecHandles[i]) != LVSTATUS_OK ||
          genicamdevice->vecHandles[i]->Queue() != LVSTATUS_OK)
        {
          updateLastErrorMessage("Failed to open/queue buffer: ");
          return 1;
        }
    }
  
  return 0;
}

GENICAM_WAPI(int) genicam_deregister_framebuffers(genicam_device* genicamdevice)
{
  int retValue = 0;
  if (genicamdevice->stream->CloseEvent(genicamdevice->event) != LVSTATUS_OK ||
      genicamdevice->stream->FlushQueue(LvQueueOperation_AllDiscard) != LVSTATUS_OK)
    {
      retValue = 1;
      updateLastErrorMessage("Failed to close event: ");
    }

  // Close buffers
  for (unsigned int i=genicamdevice->vecHandles.size(); i--;)
    {
      if (genicamdevice->vecHandles[i] != 0)
        {
          if (genicamdevice->stream->CloseBuffer(genicamdevice->vecHandles[i]) != LVSTATUS_OK)
            {
              retValue = 1;
              updateLastErrorMessage("Failed to close buffer: ");
            }
        }
    }
  
  genicamdevice->vecHandles.clear();
  genicamdevice->vecReservedHandles.clear();

  return retValue;
}


GENICAM_WAPI(int) genicam_set_property(genicam_device* genicamdevice, const char* name, int value)
{
  if (!strcmp(name, "width"))
    {
      if (value < 0 && getInt(genicamdevice,LvDevice_WidthMax, name, &value) != 0)
        return 1;
      return setInt(genicamdevice, LvDevice_Width, name, value);
    }
  else if (!strcmp(name, "height"))
    {
      if (value < 0 && getInt(genicamdevice,LvDevice_HeightMax, name, &value) != 0)
        return 1;
      return setInt(genicamdevice, LvDevice_Height, name, value);
    }
  else if (!strcmp(name, "frameRate"))
    {
      if (value <= 0)
        return setEnum(genicamdevice, LvDevice_LvAcquisitionFrameRateControlMode, name, LvAcquisitionFrameRateControlMode_Off);
      else
        {
          if (setEnum(genicamdevice, LvDevice_LvAcquisitionFrameRateControlMode, name, LvAcquisitionFrameRateControlMode_On) == 0 &&
              setFloat(genicamdevice, LvDevice_AcquisitionFrameRate, name, (double(value) / 1000.0)) == 0)
            return 0;
          else
            return 1;
        }
    }
  else if (!strcmp(name, "imageFormat"))
    {
      int format;
      switch(value)
        {
        case 2: format = LvPixelFormat_BayerRG8; break;
        case 3: format = LvPixelFormat_BayerBG8; break;
        case 4: format = LvPixelFormat_BayerGB8; break;
        case 5: format = LvPixelFormat_BayerGR8; break;
        default: format = LvPixelFormat_Mono8; break;
        }
      return setEnum(genicamdevice, LvDevice_PixelFormat, name, format);
    }
  else if (!strcmp(name, "exposure"))
    return setFloat(genicamdevice, LvDevice_ExposureTime, name, (double)value);
  else if (!strcmp(name, "gain"))
    return setFloat(genicamdevice, LvDevice_Gain, name, (double)value);
  else if (!strcmp(name, "offsetX"))
    return setInt(genicamdevice, LvDevice_OffsetX, name, value);
  else if (!strcmp(name, "offsetY"))
    return setInt(genicamdevice, LvDevice_OffsetY, name, value);
  else if (!strcmp(name, "packetSize"))
    return setInt(genicamdevice, LvDevice_GevSCPSPacketSize, name, value);
  /*else if (!strcmp(name, "autoExposureTarget"))
    {
    if (value > 0)
    {
    genicamdevice->device->ExposureAuto.SetValue(ExposureAuto_Continuous);
    genicamdevice->device->AutoTargetValue.SetValue(value);
    }
    else
    genicamdevice->device->ExposureAuto.SetValue(ExposureAuto_Off);
    }
    else if (!strcmp(name, "autoExposureAreaOffsetX"))
    genicamdevice->device->AutoFunctionAOIOffsetX.SetValue(value);
    else if (!strcmp(name, "autoExposureAreaOffsetY"))
    genicamdevice->device->AutoFunctionAOIOffsetY.SetValue(value);
    else if (!strcmp(name, "autoExposureAreaWidth"))
    genicamdevice->device->AutoFunctionAOIWidth.SetValue(value);
    else if (!strcmp(name, "autoExposureAreaHeight"))
    genicamdevice->device->AutoFunctionAOIHeight.SetValue(value);
  */

  strLastError = std::string("simplon-driver doesn't support property '" + std::string(name) + "'");
  return 1;
}

GENICAM_WAPI(int) genicam_get_property(genicam_device* genicamdevice, const char* name, int *value)
{
  *value = 0;
  if (!strcmp(name, "exposure"))
    return getFloat(genicamdevice,LvDevice_ExposureTime, name, value);
  else if (!strcmp(name, "frameRate"))
    {
      LvEnum mode;
      genicamdevice->device->GetEnum(LvDevice_LvAcquisitionFrameRateControlMode, &mode);
      if (mode == LvAcquisitionFrameRateControlMode_On)
        {
            double dValue = 0;
            if (genicamdevice->device->GetFloat(LvDevice_AcquisitionFrameRate, &dValue) == LVSTATUS_OK)
              {
                *value = (int)(dValue*1000);
                return 0;
              }
            else
              {
                strLastError = std::string("Failed to get property(" + std::string(name) + "): " + LvLibrary::GetLastErrorMessage());
                return 1;
              }
        }
      else
        {
          *value = 0;
          return 0;
        }
    }
  else if (!strcmp(name, "imageFormat"))
    {
      LvEnum format;
      if (genicamdevice->device->GetEnum(LvDevice_PixelFormat, &format) != LVSTATUS_OK)
        {
          updateLastErrorMessage("Failed to get imageFormat: ");
          return 1;
        }
      /**
         InvalidFormat = 0,
         MonoFormat = 1,
         BayerRGGBFormat,
         BayerBGGRFormat,
         BayerGBRGFormat,
         BayerGRBGFormat,
         RgbFormat = 16,
         BgrFormat
      */
      switch (format)
        {
        case LvPixelFormat_Mono8:
        case LvPixelFormat_Mono10:
        case LvPixelFormat_Mono12:
        case LvPixelFormat_Mono16:
          *value = 1; break;
        case LvPixelFormat_BayerRG8:
        case LvPixelFormat_BayerRG10:
        case LvPixelFormat_BayerRG12:
        case LvPixelFormat_BayerRG16:
          *value = 2; break;
        case LvPixelFormat_BayerBG8:
        case LvPixelFormat_BayerBG10:
        case LvPixelFormat_BayerBG12:
        case LvPixelFormat_BayerBG16:
          *value = 3; break;
        case LvPixelFormat_BayerGB8:
        case LvPixelFormat_BayerGB10:
        case LvPixelFormat_BayerGB12:
        case LvPixelFormat_BayerGB16:
          *value = 4; break;
        case LvPixelFormat_BayerGR8:
        case LvPixelFormat_BayerGR10:
        case LvPixelFormat_BayerGR12:
        case LvPixelFormat_BayerGR16:
          *value = 5; break;
        case LvPixelFormat_RGB8Planar:
        case LvPixelFormat_RGB10Planar:
        case LvPixelFormat_RGB12Planar:
        case LvPixelFormat_RGB16Planar:
          *value = 16; break;
        case LvPixelFormat_BGR8Packed:
        case LvPixelFormat_BGR10Packed:
        case LvPixelFormat_BGR12Packed:
          *value = 17; break;
        default: *value = 0; break;
        }
      return 0;
    }
  else if (!strcmp(name, "bitsPerPixel"))
    {
      LvEnum format;
      if (genicamdevice->device->GetEnum(LvDevice_PixelFormat, &format) != LVSTATUS_OK)
        {
          updateLastErrorMessage("Failed to get bitsPerPixel: ");
          return 1;
        }
      switch (format)
        {
        case LvPixelFormat_Mono8:
        case LvPixelFormat_BayerRG8:
        case LvPixelFormat_BayerBG8:
        case LvPixelFormat_BayerGB8:
        case LvPixelFormat_BayerGR8:
        case LvPixelFormat_BGR8Packed:
        case LvPixelFormat_RGB8Planar:
          *value = 8; break;
        case LvPixelFormat_Mono10:
        case LvPixelFormat_BayerRG10:
        case LvPixelFormat_BayerBG10:
        case LvPixelFormat_BayerGB10:
        case LvPixelFormat_BayerGR10:
        case LvPixelFormat_RGB10Planar:
        case LvPixelFormat_BGR10Packed:
          *value = 10; break;
        case LvPixelFormat_Mono12:
        case LvPixelFormat_BayerRG12:
        case LvPixelFormat_BayerBG12:
        case LvPixelFormat_BayerGB12:
        case LvPixelFormat_BayerGR12:
        case LvPixelFormat_RGB12Planar:
        case LvPixelFormat_BGR12Packed:
          *value = 12; break;
        case LvPixelFormat_Mono16:
        case LvPixelFormat_BayerRG16:
        case LvPixelFormat_BayerBG16:
        case LvPixelFormat_BayerGB16:
        case LvPixelFormat_BayerGR16:
        case LvPixelFormat_RGB16Planar:
          *value = 16; break;
        default : *value = 8; break;
        }
      return 0;
    }
  else if (!strcmp(name, "gain"))
    return getFloat(genicamdevice,LvDevice_Gain, name, value);
  else if (!strcmp(name, "gain$min"))
    return getFloatMin(genicamdevice, LvDevice_Gain, name, value);
  else if (!strcmp(name, "gain$max"))
    return getFloatMax(genicamdevice, LvDevice_Gain, name, value);
  else if (!strcmp(name, "offsetX"))
    return getInt(genicamdevice, LvDevice_OffsetX, name, value);
  else if (!strcmp(name, "offsetY"))
    return getInt(genicamdevice, LvDevice_OffsetY, name, value);
  else if (!strcmp(name, "width"))
    return getInt(genicamdevice, LvDevice_Width, name, value);
  else if (!strcmp(name, "width$min"))
    return getIntMin(genicamdevice, LvDevice_Width, name, value);
  else if (!strcmp(name, "width$max"))
    return getIntMax(genicamdevice, LvDevice_Width, name, value);
  else if (!strcmp(name, "sensorWidth"))
    return getInt(genicamdevice, LvDevice_SensorWidth, name, value);
  else if (!strcmp(name, "height"))
    return getInt(genicamdevice, LvDevice_Height, name, value);
  else if (!strcmp(name, "height$min"))
    return getIntMin(genicamdevice, LvDevice_Height, name, value);
  else if (!strcmp(name, "height$max"))
    return getIntMax(genicamdevice, LvDevice_Height, name, value);
  else if (!strcmp(name, "sensorHeight"))
    return getInt(genicamdevice, LvDevice_SensorHeight, name, value);
  else if (!strcmp(name, "payloadSize"))
    {
      int iImageSize = 0;
      if (genicamdevice->stream->GetInt32(LvStream_LvCalcPayloadSize, &iImageSize) != LVSTATUS_OK)
        {
          updateLastErrorMessage("Failed to get payloadSize: ");
          return 1;
        }
      else
        {
          *value = iImageSize;
          return 0;
        }
    }
  else if (!strcmp(name, "packetSize"))
    return getInt(genicamdevice, LvDevice_GevSCPSPacketSize, name, value);
  
  /*else if (!strcmp(name, "autoExposureTarget"))
    {
    if (genicamdevice->device->ExposureAuto.GetValue() == ExposureAuto_Continuous)
    return genicamdevice->device->AutoTargetValue.GetValue();
    else
    return 0;
    }
    else if (!strcmp(name, "autoExposureAreaOffsetX"))
    return genicamdevice->device->AutoFunctionAOIOffsetX.GetValue();
    else if (!strcmp(name, "autoExposureAreaOffsetY"))
    return genicamdevice->device->AutoFunctionAOIOffsetY.GetValue();
    else if (!strcmp(name, "autoExposureAreaWidth"))
    return genicamdevice->device->AutoFunctionAOIWidth.GetValue();
    else if (!strcmp(name, "autoExposureAreaHeight"))
    return genicamdevice->device->AutoFunctionAOIHeight.GetValue();
  */
  strLastError = std::string("Does not support the property (" + std::string(name) + ")");
  return 1;
}


GENICAM_WAPI(int) genicam_grab_frame(genicam_device* genicamdevice, unsigned char** buffer, int timeout)
{
  LvBuffer *pBuffer;
  *buffer = 0;
  if (genicamdevice->event->WaitAndGetNewBuffer(pBuffer, timeout) == LVSTATUS_OK)
    {
      void* pData = 0;
      pBuffer->GetPtr(LvBuffer_Base, &pData);
      if (pData != 0)
        {
          genicamdevice->vecReservedHandles.push_back(pBuffer);
          *buffer = (unsigned char*)pData;
          return 0; //(unsigned char*)pData;
        }
      else
        {
          updateLastErrorMessage("Failed to get data: ");
          return 1;
        }
    }

  updateLastErrorMessage("Failed to get data: ");
  return 1;
}

GENICAM_WAPI(int) genicam_requeue_buffers(genicam_device* genicamdevice)
{
  for (unsigned int i=0; i<genicamdevice->vecReservedHandles.size(); ++i)
    {
      if (genicamdevice->vecReservedHandles[i]->Queue() != LVSTATUS_OK)
        {
          updateLastErrorMessage("Failed to queue: ");
          return 1;
        }
    }
  genicamdevice->vecReservedHandles.clear();

  return 0;
}

GENICAM_WAPI(int) genicam_start_capture(genicam_device* genicamdevice)
{
  if (genicamdevice->device->AcquisitionStart() != LVSTATUS_OK)
    {
      updateLastErrorMessage("Failed to start acquisition: ");
      return 1;
    }

  return 0;
}

GENICAM_WAPI(int) genicam_stop_capture(genicam_device* genicamdevice)
{
  if (genicamdevice->device->AcquisitionStop() != LVSTATUS_OK)
    {
      updateLastErrorMessage("Failed to stop acquisition: ");
      return 1;
    }
  
  return 0;
}
