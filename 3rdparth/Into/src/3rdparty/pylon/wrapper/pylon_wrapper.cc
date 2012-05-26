/* This file is part of Into.
 * Copyright (C) Intopii
 * All rights reserved.
 */

#include "genicam_wrapper.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>

#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigECamera.h>

typedef Pylon::CBaslerGigECamera PylonCameraType;
typedef PylonCameraType::StreamGrabber_t PylonGrabberType;

using namespace Pylon;
using namespace Basler_GigECameraParams;
using namespace Basler_GigEStreamParams;
using namespace std;

struct genicam_device
{
  genicam_device(PylonCameraType* c,
               PylonGrabberType* g) :
    camera(c), grabber(g)
  {}
               
  PylonCameraType *camera;
  PylonGrabberType *grabber;
  std::vector<void*> vecHandles;
  std::vector<void*> vecReservedHandles;
};

static int iInstanceCount = 0;
static std::vector<genicam_device*> vecDevices;
static std::string strLastError = "";

GENICAM_WAPI(int) genicam_initialize(void)
{
  if (++iInstanceCount == 1)
    {
      try
        {
          Pylon::PylonInitialize();
        }
      catch (GenICam::GenericException& e)
        {
          strLastError = std::string("Failed to initialize Pylon-driver. Reason: " + std::string(e.GetDescription()));
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
          try
            {
              genicam_close_device(vecDevices.at(0));
            }
          catch (GenICam::GenericException& e)
            {
              strLastError = std::string("Failed to close pylon-device. Reason: " + std::string(e.GetDescription()));
              retValue = 1;
            }
        }
      
      try
        {
          Pylon::PylonTerminate();
        }
      catch (GenICam::GenericException& e)
        {
          strLastError = std::string("Failed to terminate pylon-driver. Reason: " + std::string(e.GetDescription()));
          retValue = 1;
        }
    }

  return retValue;
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


GENICAM_WAPI(int) genicam_list_cameras(char **names, int* count)
{
  ITransportLayer *pTl = 0;
  *names = 0;
  *count = 0;

  try
    {
      pTl = CTlFactory::GetInstance().CreateTl(PylonCameraType::DeviceClass());
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to create CTlFactory-instance. Reason: " + std::string(e.GetDescription()));
      return 0;
    }

  int iLen = 0, iBufferSize = 0;
  if (pTl != 0)
    {
      try
        {
          // Get all attached cameras
          DeviceInfoList_t lstDevices;
          if (pTl->EnumerateDevices(lstDevices) != 0)
            {
              DeviceInfoList_t::iterator it;
              for (it=lstDevices.begin(); it!=lstDevices.end(); ++it )
                {
                  append_string(names, it->GetSerialNumber().c_str(), &iLen, &iBufferSize);
                  ++*count;
                }
            }
        }
      catch (GenICam::GenericException& e)
        {
          strLastError = std::string("Failed to list cameras. Reason: " + std::string(e.GetDescription()));
          *count = 0;
          *names = 0;
          return 1;
        }
    }
  return 0;
}

GENICAM_WAPI(const char*) genicam_next_camera(const char* camera)
{
  camera += strlen(camera) + 1;
  if (*camera == 0)
    return 0;
  return camera;
}

GENICAM_WAPI(int) genicam_open_device(const char* serial, genicam_device** device)
{
  // Create the transport layer object needed to enumerate or
  // create a camera object of type CameraType::DeviceClass()
  // Throw exception if the specific transport layer is not available
  ITransportLayer *pTl = 0;
  *device = 0;
  try
    {
      pTl = CTlFactory::GetInstance().CreateTl(PylonCameraType::DeviceClass());
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to create transport layer object. Reason: " + std::string(e.GetDescription()));
      return 1;
    }
  if (pTl == 0)
    {
      strLastError = std::string("Failed to create transport layer object");;
      return 1;
    }
  
  // Get all attached cameras and throw exception if no camera is found
  DeviceInfoList_t devices;
  try
    {
      if (pTl->EnumerateDevices(devices) == 0)
        {
          strLastError = std::string("No cameras were found.");
          return 1;
        }
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to enumerate devices. Reason: " + std::string(e.GetDescription()));
      return 1;
    }
  

  PylonCameraType* pCamera = 0;
  // Create the camera object of the selected camera
  // The camera object is used to set and get all available
  // camera features.
  try
    {
      for (DeviceInfoList_t::iterator i = devices.begin(); i != devices.end(); ++i)
        {
          if (!strcmp(i->GetSerialNumber().c_str(), serial))
            {
              //_pCamera->Attach(pTl->CreateDevice(di));
              IPylonDevice *pDevice = pTl->CreateDevice(*i);
              CDeviceInfo info = pDevice->GetDeviceInfo();
              
              // Create camera object
              pCamera = new PylonCameraType(pDevice);
              break;
            }
        }
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to create camera object. Reason: " + std::string(e.GetDescription()));
      return 1;
    }

  if (pCamera == 0)
    {
      strLastError = std::string("No camera matching the given camera id was found.");
      return 1;
    }

  unsigned int uiGrabbers = 0;
  try
    {
      uiGrabbers = pCamera->GetNumStreamGrabberChannels();
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to get stream grabbers count. Reason: " + std::string(e.GetDescription()));
      delete pCamera;
      return 1;
    }
  
  if (uiGrabbers <= 0)
    {
      delete pCamera;
      strLastError = std::string("Couldn't find any stream grabbers.");
      return 1;
    }
      
  // Get the first stream grabber object of the selected camera
  PylonGrabberType* pGrabber = new PylonGrabberType(pCamera->GetStreamGrabber(0));

  try
    {
      // Open the camera
      pCamera->Open();
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to open camera. Reason: " + std::string(e.GetDescription()));
      delete pCamera;
      return 1;
    }
      
  try
    {
      // Open the stream grabber
      pGrabber->Open();
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to open grabber. Reason: " + std::string(e.GetDescription()));
      delete pCamera;
      delete pGrabber;
      return 1;
    }


  try
    {
      pCamera->PixelFormat.SetValue(PixelFormat_Mono8);
      
      // Set the camera to continuous frame mode
      pCamera->TriggerSelector.SetValue(TriggerSelector_AcquisitionStart);
      pCamera->TriggerMode.SetValue(TriggerMode_Off);
      pCamera->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
      pCamera->ExposureMode.SetValue(ExposureMode_Timed);
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to configure camera. Reason: " + std::string(e.GetDescription()));
      delete pCamera;
      delete pGrabber;
      return 1;
    }
  
  *device = new genicam_device(pCamera, pGrabber);
  vecDevices.push_back(*device);
  return 0;
}

GENICAM_WAPI(int) genicam_free(void* data)
{
	free(data);
  return 0;
}

GENICAM_WAPI(int) genicam_close_device(genicam_device* device)
{
  int retValue = 0;
  try
    {
      genicam_deregister_framebuffers(device);
      
      // Free all resources used for grabbing
      device->grabber->Close();
      device->camera->Close();
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to close device. Reason: " + std::string(e.GetDescription()));
      retValue = 1;
    }
  
  delete device->grabber;
  delete device->camera;
  delete device;
  
  vecDevices.erase(std::find(vecDevices.begin(), vecDevices.end(), device));

  return retValue;
}

GENICAM_WAPI(int) genicam_set_property(genicam_device* device, const char* name, int value)
{
  try
    {
      if (!strcmp(name, "width"))
        device->camera->Width.SetValue(value);
      else if (!strcmp(name, "height"))
        device->camera->Height.SetValue(value);
      else if (!strcmp(name, "frameRate"))
        {
          if (device->camera->DeviceScanType.GetValue() == DeviceScanType_Areascan)
            {
              device->camera->AcquisitionFrameRateEnable.SetValue(value <= 0 ? false : true);
              if (value > 0)
                device->camera->AcquisitionFrameRateAbs.SetValue(double(value) / 1000.0);
            }
        }
      else if (!strcmp(name, "imageFormat"))
        {
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
          int format;
          switch(value)
            {
            case 2: format = PixelFormat_BayerRG8; break;
            case 3: format = PixelFormat_BayerBG8; break;
            case 4: format = PixelFormat_BayerGB8; break;
            case 5: format = PixelFormat_BayerGR8; break;
            default: format = PixelFormat_Mono8; break;
            }
          device->camera->PixelFormat.SetValue((PixelFormatEnums)format);
        }
      else if (!strcmp(name, "exposure") || !strcmp(name, "exposureTime"))
        device->camera->ExposureTimeRaw.SetValue(value);
      else if (!strcmp(name, "gain"))
        device->camera->GainRaw.SetValue(value);
      else if (!strcmp(name, "offsetX"))
        device->camera->OffsetX.SetValue(value);
      else if (!strcmp(name, "offsetY"))
        device->camera->OffsetY.SetValue(value);
      else if (!strcmp(name, "packetSize"))
        device->camera->GevSCPSPacketSize.SetValue(value);
      else if (!strcmp(name, "flipHorizontally"))
        device->camera->ReverseX.SetValue(value == 0 ? false : true);
      else if (!strcmp(name, "autoExposureTarget"))
        {
          if (value > 0)
            {
              device->camera->ExposureAuto.SetValue(ExposureAuto_Continuous);
              device->camera->AutoTargetValue.SetValue(value);
            }
          else
            device->camera->ExposureAuto.SetValue(ExposureAuto_Off);
        }
      else if (!strcmp(name, "autoExposureAreaOffsetX"))
        device->camera->AutoFunctionAOIOffsetX.SetValue(value);
      else if (!strcmp(name, "autoExposureAreaOffsetY"))
        device->camera->AutoFunctionAOIOffsetY.SetValue(value);
      else if (!strcmp(name, "autoExposureAreaWidth"))
        device->camera->AutoFunctionAOIWidth.SetValue(value);
      else if (!strcmp(name, "autoExposureAreaHeight"))
        device->camera->AutoFunctionAOIHeight.SetValue(value);
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to set camera property (" + std::string(name) + "). Reason: " + std::string(e.GetDescription()));
      return 1;
    }

  return 0;
}

GENICAM_WAPI(int) genicam_get_property(genicam_device* device, const char* name, int *value)
{
  try
    {
      if (!strcmp(name, "exposure") || !strcmp(name, "exposureTime"))
        *value = (int)device->camera->ExposureTimeRaw.GetValue();
      else if (!strcmp(name, "frameRate"))
        {
          if (device->camera->DeviceScanType.GetValue() == DeviceScanType_Areascan &&
              device->camera->AcquisitionFrameRateEnable.GetValue() == true)
            *value = (int)(device->camera->AcquisitionFrameRateAbs.GetValue() * 1000);
          else
            *value = 0;
        }
      else if (!strcmp(name, "imageFormat"))
        {
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
          
          int format = device->camera->PixelFormat.GetValue();
          switch (format)
            {
            case PixelFormat_BayerRG8: *value = 2; break;
            case PixelFormat_BayerBG8: *value = 3; break;
            case PixelFormat_BayerGB8: *value = 4; break;
            case PixelFormat_BayerGR8: *value = 5; break;
            case PixelFormat_Mono8: *value = 1; break;
            default :*value = 0; break;
            }
        }
      else if (!strcmp(name, "bitsPerPixel"))
        {
          int pixelSize = device->camera->PixelSize.GetValue();
          switch(pixelSize)
            {
            case PixelSize_Bpp8: *value = 8; break;
            case PixelSize_Bpp12: *value = 12; break;
            case PixelSize_Bpp14: *value = 14; break;
            case PixelSize_Bpp16: *value = 16; break;
            case PixelSize_Bpp24: *value = 24; break;
            case PixelSize_Bpp32: *value = 32; break;
            case PixelSize_Bpp36: *value = 36; break;
            case PixelSize_Bpp48: *value = 48; break;
            case PixelSize_Bpp64: *value = 64; break;
            default: *value = 8; break;
            }
        }
      else if (!strcmp(name, "scanType"))
        *value = device->camera->DeviceScanType.GetValue() == DeviceScanType_Areascan ? 0 : 1;
      else if (!strcmp(name, "gain"))
        *value = (int)device->camera->GainRaw.GetValue();
      else if (!strcmp(name, "gain$min"))
        *value = (int)device->camera->GainRaw.GetMin();
      else if (!strcmp(name, "gain$max"))
        *value = (int)device->camera->GainRaw.GetMax();
      else if (!strcmp(name, "offsetX"))
        *value = (int)device->camera->OffsetX.GetValue();
      else if (!strcmp(name, "offsetY"))
        *value = (int)device->camera->OffsetY.GetValue();
      else if (!strcmp(name, "width"))
        *value = (int)device->camera->Width.GetValue();
      else if (!strcmp(name, "width$min"))
        *value = (int)device->camera->Width.GetMin();
      else if (!strcmp(name, "width$max"))
        *value = (int)device->camera->WidthMax.GetValue();
      else if (!strcmp(name, "sensorWidth"))
        *value = (int)device->camera->SensorWidth.GetValue();
      else if (!strcmp(name, "height"))
        *value = (int)device->camera->Height.GetValue();
      else if (!strcmp(name, "height$min"))
        *value = (int)device->camera->Height.GetMin();
      else if (!strcmp(name, "height$max"))
        *value = (int)device->camera->HeightMax.GetValue();
      else if (!strcmp(name, "sensorHeight"))
        *value = (int)device->camera->SensorHeight.GetValue();
      else if (!strcmp(name, "payloadSize"))
        *value = (int)device->camera->PayloadSize();
      else if (!strcmp(name, "packetSize"))
        *value = (int)device->camera->GevSCPSPacketSize.GetValue();
      else if (!strcmp(name, "flipHorizontally"))
        *value = (int)device->camera->ReverseX.GetValue();
      else if (!strcmp(name, "autoExposureTarget"))
        {
          if (device->camera->ExposureAuto.GetValue() == ExposureAuto_Continuous)
            *value = (int)device->camera->AutoTargetValue.GetValue();
          else
            *value = 0;
        }
      else if (!strcmp(name, "autoExposureAreaOffsetX"))
        *value = (int)device->camera->AutoFunctionAOIOffsetX.GetValue();
      else if (!strcmp(name, "autoExposureAreaOffsetY"))
        *value = (int)device->camera->AutoFunctionAOIOffsetY.GetValue();
      else if (!strcmp(name, "autoExposureAreaWidth"))
        *value = (int)device->camera->AutoFunctionAOIWidth.GetValue();
      else if (!strcmp(name, "autoExposureAreaHeight"))
        *value = (int)device->camera->AutoFunctionAOIHeight.GetValue();
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to get camera property. Reason: " +  std::string(e.GetDescription()));
      return 1;
    }
  return 0;
}

GENICAM_WAPI(int) genicam_register_framebuffers(genicam_device* device, unsigned char* buffer, int count)
{
  try
    {
      device->vecHandles.resize(count,0);
      
      // Create an image buffer
      unsigned uiImageSize = (unsigned)device->camera->PayloadSize();
      
      // Init grabber parameters
      device->grabber->MaxBufferSize.SetValue(uiImageSize);
      device->grabber->MaxNumBuffer.SetValue(count);
      
      // Allocate all resources for grabbing. Critical parameters like
      // image size must not be changed until FinishGrab() is called.
      device->grabber->PrepareGrab();
      
      // Buffers used for grabbing must be registered at the stream
      // grabber. The registration returns a handle to be used for
      // queuing the buffer
      for (int i=0; i<count; ++i)
        {
          device->vecHandles[i] = device->grabber->RegisterBuffer(buffer + i*uiImageSize, uiImageSize);
          device->grabber->QueueBuffer(device->vecHandles[i], 0);
        }
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to register frame buffers. Reason: " + std::string(e.GetDescription()));
      return 1;
    }

  return 0;
}

GENICAM_WAPI(int) genicam_deregister_framebuffers(genicam_device* device)
{
  try
    {
      device->grabber->CancelGrab();
      
      // Get all buffers back
      for (GrabResult r; device->grabber->RetrieveResult(r););
      
      // Deregister handles
      for (unsigned int i=0; i<device->vecHandles.size(); ++i)
        {
          if (device->vecHandles[i] != 0)
            device->grabber->DeregisterBuffer(device->vecHandles[i]);
        }
      
      device->vecHandles.clear();
      device->vecReservedHandles.clear();

      device->grabber->FinishGrab();
    }
    catch (GenICam::GenericException& e)
      {
        strLastError = std::string("Failed to deregister frame buffers. Reason: " + std::string(e.GetDescription()));
        return 1;
      }

  return 0;
}


GENICAM_WAPI(int) genicam_grab_frame(genicam_device* device, unsigned char** buffer, int timeout)
{
  GrabResult grabResult;
  *buffer = 0;
  try
    {
    if (device->grabber->GetWaitObject().Wait(timeout))
        {
          //Get an item from the grabber's output queue
          if (device->grabber->RetrieveResult(grabResult))
            {
              device->vecReservedHandles.push_back(grabResult.Handle());
              if (!grabResult.Succeeded())
                {
                  strLastError = std::string("Failed to grab frame: " + std::string(grabResult.GetErrorDescription()));
                  return 1;
                }
            }
          else
            {
              strLastError = std::string("Failed to retrieve an item from the output queue.");
              return 1;
            }
        }
      else
        {
          strLastError = std::string("Timeout occurred when waiting for a grabbed image.");
          return 1;
        }
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to grab frame. Reason: " + std::string(e.GetDescription()));
      return 1;
    }

  *buffer = (unsigned char*)grabResult.Buffer();
  return 0; //(unsigned char*)grabResult.Buffer();
}

GENICAM_WAPI(int) genicam_requeue_buffers(genicam_device* device)
{
  try
    {
      for (unsigned int i=0; i<device->vecReservedHandles.size(); ++i)
        device->grabber->QueueBuffer(device->vecReservedHandles[i], 0);
      device->vecReservedHandles.clear();
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to requeue buffers. Reason: " + std::string(e.GetDescription()));
      return 1;
    }

  return 0;
}

GENICAM_WAPI(int) genicam_start_capture(genicam_device* device)
{
  try
    {
      device->camera->AcquisitionStart.Execute();
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to start capture. Reason: " + std::string(e.GetDescription()));
      return 1;
    }

  return 0;
}

GENICAM_WAPI(int) genicam_stop_capture(genicam_device* device)
{
  try
    {
      device->camera->AcquisitionStop.Execute();
    }
  catch (GenICam::GenericException& e)
    {
      strLastError = std::string("Failed to stop capture. Reason: " + std::string(e.GetDescription()));
      return 1;
    }

  return 0;
}
