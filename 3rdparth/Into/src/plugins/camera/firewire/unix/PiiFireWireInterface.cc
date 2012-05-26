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
#include <iostream>

struct PiiFireWireInterface::Impl
{
  const char *name;
};

PiiFireWireInterface::PiiFireWireInterface()
{
  _pImpl = new Impl;
  _pImpl->name = "unix";
  //std::cout << "PiiFireWireInterface() " << _pImpl->name << std::endl;
}

PiiFireWireInterface::~PiiFireWireInterface()
{
  //std::cout << "~PiiFireWireInterface() " << _pImpl->name << std::endl;
}


/* CAMERA INITIALIZATION FUNCTIONS. */
bool PiiFireWireInterface::refreshCameraList() { return true; }
bool PiiFireWireInterface::selectCamera( int /*node*/ ) { return true; }
bool PiiFireWireInterface::initializeCamera(){ return true; }

/* CAPTURE FUNCTIONS */
bool PiiFireWireInterface::startCapture(){ return true; }
bool PiiFireWireInterface::stopCapture(){ return true; }
bool PiiFireWireInterface::capture(){ return true; }
void PiiFireWireInterface::image( unsigned char* /*data*/ ){ }
unsigned char* PiiFireWireInterface::buffer() { return 0; }
  
  
/* GET FUNCTIONS. */
bool PiiFireWireInterface::isInitialized() { return true; }
QString PiiFireWireInterface::vendor(){ return QString("UnixVendor"); }
QString PiiFireWireInterface::model(){ return QString("UnixModel"); }
int PiiFireWireInterface::node(){ return 0; }
int PiiFireWireInterface::cameraCount(){ return 0; }
int PiiFireWireInterface::videoFormat(){ return 0; }
int PiiFireWireInterface::videoMode(){ return 0; }
int PiiFireWireInterface::frameRate(){ return 0; }
QList<int> PiiFireWireInterface::supportedFormats()
{
  QList<int> res;
  res.append(1);
  return res;
}
QList<int> PiiFireWireInterface::supportedModes( int /*format*/ )
{
  QList<int> res;
  res.append(1);
  return res;
}
QList<int> PiiFireWireInterface::supportedFrameRates( int /*format*/, int /*mode*/ )
{
  QList<int> res;
  res.append(1);
  return res;
}
int PiiFireWireInterface::width() { return 0; }
int PiiFireWireInterface::height() { return 0; }
int PiiFireWireInterface::left() { return 0; }
int PiiFireWireInterface::top() { return 0; }

/* SET FUNCTIONS */
bool PiiFireWireInterface::setVideoFormat(int /*format*/){ return true; }
bool PiiFireWireInterface::setVideoMode(int /*mode*/){ return true; }
bool PiiFireWireInterface::setFrameRate(int /*framerate*/){ return true; }
bool PiiFireWireInterface::setSize(int /*width*/, int /*height*/) { return true; }
bool PiiFireWireInterface::setPosition(int /*left*/, int /*top*/) { return true; }


