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

#include "PiiFireWireOperation.h"
#include <PiiYdinTypes.h>
#include <QDir>
#include <QFileInfo>
#include <QList>

using namespace PiiYdin;
using namespace Pii;

PiiFireWireOperation::Data::Data() :
  iMaxFrames(-1), iCurrentIndex(0), iWidth(0), iHeight(0), iLeft(0), iTop(0)
{
}

PiiFireWireOperation::PiiFireWireOperation() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  //Initialize camera.
  d->pFwi = new PiiFireWireInterface();
  d->pFwi->refreshCameraList();

  //setCamera(0);
  //setFrameType(GrayScale);

  d->pTrigger = new PiiInputSocket("trigger");
  d->pTrigger->setOptional(true);
  addSocket(d->pTrigger);
  
  d->pOutputSocket = new PiiOutputSocket("image");
  addSocket(d->pOutputSocket);

  connect(this, SIGNAL(stateChanged(int)), SLOT(stopCapture(int)), Qt::DirectConnection);
}

PiiFireWireOperation::~PiiFireWireOperation()
{
  PII_D;
  d->pFwi->stopCapture();
  delete d->pFwi;
}

void PiiFireWireOperation::stopCapture(int state)
{
  PII_D;
  if ( state == PiiOperation::Stopped )
    d->pFwi->stopCapture();
}

void PiiFireWireOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if ( d->pFwi->cameraCount() == 0 )
    PII_THROW(PiiExecutionException,tr("IEEE1394 source cannot start because no cameras were found."));

  if ( !d->pFwi->isInitialized() )
    PII_THROW(PiiExecutionException,tr("Camera has not been initialized"));
  
  if ( !d->pFwi->startCapture() )
    PII_THROW(PiiExecutionException,tr("No camera has been selected and/or camera initialization has not been successfully completed."));
  
}

void PiiFireWireOperation::process()
{
  PII_D;
  if ( !d->pFwi->capture() )
    PII_THROW(PiiExecutionException,tr("Couldn't capture image. Camera initialization may not have been successfully completed."));
  
  if ( d->iMaxFrames == -1 || d->iCurrentIndex < d->iMaxFrames  )
    {
      if ( d->frameType == GrayScale )
        {
          if ( d->videoFormatType == Gray8 )
            {
              PiiMatrix<unsigned char> img(PiiMatrix<unsigned char>::uninitialized(d->pFwi->height(),
                                                                                   d->pFwi->width()));
              memcpy(img[0], d->pFwi->buffer(), d->pFwi->height()*d->pFwi->width());
              d->pOutputSocket->emitObject(img);
            }
          else
            {
              PiiMatrix<PiiColor<unsigned char> > img(d->pFwi->height(), d->pFwi->width());
              d->pFwi->image((unsigned char*)img[0]);
              d->pOutputSocket->emitObject(img);
            }
        }
      else
        {
          if ( d->videoFormatType == RGB )
            {
              PiiMatrix<PiiColor<unsigned char> > img(PiiMatrix<PiiColor<unsigned char> >::uninitialized(d->pFwi->height(), d->pFwi->width()));
              memcpy(img[0], d->pFwi->buffer(), d->pFwi->height()*d->pFwi->width()*3);
              d->pOutputSocket->emitObject(img);
            }
          else
            {
              PiiMatrix<PiiColor<unsigned char> > img(d->pFwi->height(), d->pFwi->width());
              d->pFwi->image((unsigned char*)img[0]);
              d->pOutputSocket->emitObject(img);
            }
        }
    }
  else
    operationStopped();
   
  d->iCurrentIndex++;
  
}

void PiiFireWireOperation::setCamera(int index)
{
  PII_D;
  d->pFwi->selectCamera( index );
  d->pFwi->initializeCamera();

  //select first supported format, mode and fastest frame rate
  if ( d->pFwi->isInitialized() )
    {
      int format = supportedFormats().value(0).toInt();
      setFormat(format);
      
      int mode = supportedModes().value(0).toInt();
      setMode(mode);
      
      //setFrameRate(-1);
    }
  
}

void PiiFireWireOperation::setFrameType(FrameType type)
{
  PII_D;
  //the frame type affects the type of the output socket
  d->frameType = type;
}


void PiiFireWireOperation::setFrameRate( int frameRate )
{
  PII_D;
  if ( frameRate == -1 )
    d->pFwi->setFrameRate( supportedFrameRates().last().toInt() );
  else
    d->pFwi->setFrameRate(frameRate);
}
 
void PiiFireWireOperation::initVideoFormatType()
{
  PII_D;
  int iFormat = format();
  int iMode = mode();

  if (iFormat == 0)
    {
      switch(iMode)
        {
        case 0:
        case 1:
        case 2:
        case 3:
          d->videoFormatType = YUV;
          break;
        case 4:
          d->videoFormatType = RGB;
          break;
        case 5:
          d->videoFormatType = Gray8;
          break;
        case 6:
          d->videoFormatType = Gray16;
          break;
        default:
          d->videoFormatType = Other;
          break;
        }
    }
  else if (iFormat == 1 || iFormat == 2 )
    {
      switch(iMode)
        {
        case 0:
        case 3:
          d->videoFormatType = YUV;
          break;
        case 1:
        case 4:
          d->videoFormatType = RGB;
          break;
        case 2:
        case 5:
          d->videoFormatType = Gray8;
          break;
        case 6:
        case 7:
          d->videoFormatType = Gray16;
          break;
        default:
          d->videoFormatType = Other;
          break;
        }
    }
  else
    d->videoFormatType = Format7;

}

void PiiFireWireOperation::setSize()
{
  PII_D;
  d->pFwi->setSize(d->iWidth, d->iHeight);
}

void PiiFireWireOperation::setPosition()
{
  PII_D;
  d->pFwi->setPosition(d->iLeft, d->iTop);
}

int PiiFireWireOperation::camera() const { return _d()->pFwi->node(); }
int PiiFireWireOperation::format() const { return _d()->pFwi->videoFormat(); }
void PiiFireWireOperation::setFormat( int format ) { _d()->pFwi->setVideoFormat(format); initVideoFormatType(); }
int PiiFireWireOperation::mode() const { return _d()->pFwi->videoMode(); }
void PiiFireWireOperation::setMode( int mode ) { _d()->pFwi->setVideoMode(mode); initVideoFormatType(); }
int PiiFireWireOperation::frameRate() const { return _d()->pFwi->frameRate(); }
int PiiFireWireOperation::width() const { return _d()->pFwi->width(); }
void PiiFireWireOperation::setWidth( int width ) { _d()->iWidth = width; setSize(); }
int PiiFireWireOperation::height() const { return _d()->pFwi->height(); }
void PiiFireWireOperation::setHeight( int height ) { _d()->iHeight = height; setSize(); }
int PiiFireWireOperation::left() const { return _d()->pFwi->left(); }
void PiiFireWireOperation::setLeft( int left ) { _d()->iLeft = left; setPosition(); }
int PiiFireWireOperation::top() const { return _d()->pFwi->top(); }
void PiiFireWireOperation::setTop( int top ) { _d()->iTop = top; setPosition(); }
QString PiiFireWireOperation::vendor() const { return _d()->pFwi->vendor(); }
QString PiiFireWireOperation::model() const { return _d()->pFwi->model(); }
int PiiFireWireOperation::cameraCount() const { return _d()->pFwi->cameraCount(); }
QVariantList PiiFireWireOperation::supportedFormats() const { return Pii::listToVariants(_d()->pFwi->supportedFormats()); }
QVariantList PiiFireWireOperation::supportedModes() const { return Pii::listToVariants(_d()->pFwi->supportedModes(format())); }
QVariantList PiiFireWireOperation::supportedFrameRates() const { return Pii::listToVariants(_d()->pFwi->supportedFrameRates(format(), mode())); }
int PiiFireWireOperation::maxFrames() const { return _d()->iMaxFrames; }
void PiiFireWireOperation::setMaxFrames(int cnt) { _d()->iMaxFrames = cnt; }
PiiFireWireOperation::FrameType PiiFireWireOperation::frameType() const { return _d()->frameType; }
int PiiFireWireOperation::currentFrameIndex() const { return _d()->iCurrentIndex; }
