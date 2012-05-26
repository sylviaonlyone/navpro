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

#include <PiiFireWireInterface.h>
#include <QString>
#include <QList>
#include <windows.h>
#include <1394camera.h>
#include <stdio.h>

#include <iostream>

struct PiiFireWireInterface::Impl
{
  C1394Camera camera;    
};

PiiFireWireInterface::PiiFireWireInterface()
{
  _pImpl = new Impl;
}

PiiFireWireInterface::~PiiFireWireInterface()
{
  delete _pImpl;
}

/* CAMERA INITIALIZATION FUNCTIONS. */
bool PiiFireWireInterface::refreshCameraList()
{
  if ( _pImpl->camera.CheckLink() != CAM_SUCCESS )
    return false;
  else
    return true;
}

bool PiiFireWireInterface::selectCamera( int node )
{
  if ( _pImpl->camera.SelectCamera(node) != CAM_SUCCESS )
    return false;
  else
    return true;
}
bool PiiFireWireInterface::initializeCamera()
{
  if ( _pImpl->camera.InitCamera() != CAM_SUCCESS )
    return false;
  else
    return true;
}

/* CAPTURE FUNCTIONS */
bool PiiFireWireInterface::startCapture()
{
  if ( _pImpl->camera.StartImageCapture() != CAM_SUCCESS )
    return false;
  else
    return true;
}
bool PiiFireWireInterface::stopCapture()
{
  if ( _pImpl->camera.StopImageCapture() != CAM_SUCCESS )
    return false;
  else
    return true;
}
bool PiiFireWireInterface::capture()
{
  if ( _pImpl->camera.CaptureImage() != CAM_SUCCESS )
    return false;
  else
    return true;
}

void PiiFireWireInterface::getImage( unsigned char *data )
{
  _pImpl->camera.getRGB(data);
}

unsigned char* PiiFireWireInterface::getBuffer()
{
  return _pImpl->camera.m_pData;
}
  
/* GET FUNCTIONS. */
bool PiiFireWireInterface::isInitialized()
{
  return _pImpl->camera.m_cameraInitialized;
}

QString PiiFireWireInterface::getVendor()
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return QString("Not initialized");
  
  return QString(_pImpl->camera.m_nameVendor);
}
QString PiiFireWireInterface::getModel()
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return QString("Not initialized");

  return QString(_pImpl->camera.m_nameModel);
}
int PiiFireWireInterface::getNode()
{
  int res = _pImpl->camera.GetNode();
  if ( res == CAM_ERROR )
    return -1;
  else
    return res;
}
int PiiFireWireInterface::getCameraCount()
{
  return _pImpl->camera.GetNumberCameras();
}
int PiiFireWireInterface::getVideoFormat()
{
  return _pImpl->camera.GetVideoFormat();
}
int PiiFireWireInterface::getVideoMode()
{
  return _pImpl->camera.GetVideoMode();
}
int PiiFireWireInterface::getFrameRate()
{
  return _pImpl->camera.GetVideoFrameRate();
}
int PiiFireWireInterface::getWidth()
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return -1;
  
  if ( getVideoFormat() == 7 )
    {
      _pImpl->camera.m_controlSize.Status();
      return _pImpl->camera.m_controlSize.m_width;
    }
  else
    return _pImpl->camera.m_width;
}
int PiiFireWireInterface::getHeight()
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return -1;
  
  if ( getVideoFormat() == 7 )
    {
      _pImpl->camera.m_controlSize.Status();
      return _pImpl->camera.m_controlSize.m_height;
      }
  else
    return _pImpl->camera.m_height;
}
int PiiFireWireInterface::getLeft()
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return -1;
  
  if ( getVideoFormat() == 7 )
    {
      _pImpl->camera.m_controlSize.Status();
      return _pImpl->camera.m_controlSize.m_left;
    }
  else
    return 0;
}
int PiiFireWireInterface::getTop()
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return -1;
  
  if ( getVideoFormat() == 7 )
    {
      _pImpl->camera.m_controlSize.Status();
      return _pImpl->camera.m_controlSize.m_top;
    }
  else
    return 0;
}

QList<int> PiiFireWireInterface::getSupportedFormats()
{
  QList<int> res;
  for ( int i=0; i<8; i++)
    if ( _pImpl->camera.m_bxAvailableFormats[i] )
      res.append(i);
  return res;
}
QList<int> PiiFireWireInterface::getSupportedModes( int format )
{
  QList<int> res;
  if ( getVideoFormat() == 7 )
    {
    	for (int i=0; i<8; i++)
        if (_pImpl->camera.m_controlSize.ModeSupported(i))
          res.append(i);
    }
  else
    {
      for ( int i=0; i<8; i++)
        if ( _pImpl->camera.m_bxAvailableModes[format][i] )
          res.append(i);
    }
  
  return res;
}
QList<int> PiiFireWireInterface::getSupportedFrameRates( int format, int mode )
{
  QList<int> res;
  for ( int i=0; i<8; i++)
    if ( _pImpl->camera.m_videoFlags[format][mode][i] )
      res.append(i);
  return res;
}

/* SET FUNCTIONS */
bool PiiFireWireInterface::setVideoFormat(int format)
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return false;
  
  if ( _pImpl->camera.SetVideoFormat(format) != CAM_SUCCESS )
    return false;
  else
    {
      if ( format == 7 )
        _pImpl->camera.m_controlSize.Inquire();
      return true;
    }
}
bool PiiFireWireInterface::setVideoMode(int mode)
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return false;
  
  if ( _pImpl->camera.SetVideoMode(mode) != CAM_SUCCESS )
    return false;
  else
    {
      if ( getVideoFormat() == 7 )
        _pImpl->camera.m_controlSize.Inquire();
      return true;
    }
}
bool PiiFireWireInterface::setFrameRate(int framerate)
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return false;
  
  if ( _pImpl->camera.SetVideoFrameRate(framerate) != CAM_SUCCESS )
    return false;
  else
    return true;
}
bool PiiFireWireInterface::setSize(int width, int height)
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return false;
  
  if ( width > (int)_pImpl->camera.m_controlSize.m_maxH || width == 0)
    width = (int)_pImpl->camera.m_controlSize.m_maxH;
  if ( height > (int)_pImpl->camera.m_controlSize.m_maxV || height == 0)
    height = (int)_pImpl->camera.m_controlSize.m_maxV;

  if ( width < 2)
    width = 2;
  if ( height < 2 )
    height = 2;
  if ( _pImpl->camera.m_controlSize.SetSize(width, height) != CAM_SUCCESS )
    return false;
  else
    {
      _pImpl->camera.m_controlSize.Inquire();
      return true;
    }
}
bool PiiFireWireInterface::setPosition(int left, int top)
{
  if ( !_pImpl->camera.m_cameraInitialized )
    return false;
  
  if ( _pImpl->camera.m_controlSize.SetPosition(left,top) != CAM_SUCCESS )
    return false;
  else
    return true;
}

  
