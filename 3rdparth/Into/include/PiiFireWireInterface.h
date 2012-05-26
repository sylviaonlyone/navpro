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

#ifndef _PIIFIREWIREINTERFACE_H
#define _PIIFIREWIREINTERFACE_H

#include <QString>
#include <QList>
#include <PiiColor.h>
#include <PiiMatrix.h>

/**
 * An interface to IEEE-1394 cameras.
 *
 * @ingroup PiiCameraPlugin
 */
class PiiFireWireInterface
{

public:

  PiiFireWireInterface();
  ~PiiFireWireInterface();
  
  /* CAMERA INITIALIZATION FUNCTIONS. */
  
  //find all cameras and select first
  bool refreshCameraList();

  //select i:s camera from the list
  bool selectCamera( int node );

  //initialize current camera
  bool initializeCamera();


  /* CAPTURE FUNCTIONS */
  bool startCapture();
  bool stopCapture();
  bool capture();
  void image(unsigned char *data);
  unsigned char* buffer();
  
  /* GET FUNCTIONS. */
  bool isInitialized();
  QString vendor();
  QString model();
  int node();
  int cameraCount();
  int videoFormat();
  int videoMode();
  int frameRate();
  int width();
  int height();
  int left();
  int top();
  QList<int> supportedFormats();
  QList<int> supportedModes( int format );
  QList<int> supportedFrameRates( int format, int mode );
  

  /* SET FUNCTIONS */
  bool setVideoFormat(int format);
  bool setVideoMode(int mode);
  bool setFrameRate(int frameRate);
  bool setSize(int width, int height);
  bool setPosition(int left, int top);
  
private:
  struct Impl;
  Impl *_pImpl;
};


#endif //_PIIFIREWIREINTERFACE_H
