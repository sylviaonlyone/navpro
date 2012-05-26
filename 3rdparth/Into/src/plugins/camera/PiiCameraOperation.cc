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

#include "PiiCameraOperation.h"
#include <PiiMatrix.h>
#include <PiiBayerConverter.h>
#include <PiiUtil.h>

#include <QSettings>

#include <PiiLog.h>
#include <QtDebug>

PiiCameraOperation::Data::Data() : pCameraDriver(0),
                                   strCameraId(""),
                                   bTriggered(false),
                                   iImageWidth(0),
                                   iImageHeight(0),
                                   iBitsPerPixel(8),
                                   bCopyImage(false),
                                   bUseRawImage(false)
{
}

PiiCameraOperation::PiiCameraOperation(Data* dat) : PiiImageReaderOperation(dat)
{
}

PiiCameraOperation::PiiCameraOperation() : PiiImageReaderOperation(new Data)
{
}

PiiCameraOperation::~PiiCameraOperation()
{
  PII_D;

  if (d->pCameraDriver != 0)
    {
      d->pCameraDriver->close();
      delete d->pCameraDriver;
    }
}

void PiiCameraOperation::check(bool reset)
{
  PII_D;

  if (d->pCameraDriver == 0)
    PII_THROW(PiiExecutionException, tr("Camera driver has not been set."));

  // If the trigger input is connected, we must change the trigger
  // mode to the SoftwareTrigger
  d->bTriggered = d->pTriggerInput->isConnected();
  d->pCameraDriver->setTriggerMode(d->bTriggered ? PiiCameraDriver::SoftwareTrigger : PiiCameraDriver::HardwareTrigger);
  setProcessingMode(d->bTriggered ? Threaded : NonThreaded);

  try
    {
      d->pCameraDriver->initialize(d->strCameraId);
    }
  catch (PiiException& ex)
    {
      PII_THROW(PiiExecutionException, tr("Couldn't initialize driver: %1").arg(ex.message()));
    }

  QSize frameSize = d->pCameraDriver->frameSize();
  d->iImageWidth = frameSize.width();
  d->iImageHeight = frameSize.height();
  d->imageFormat = (PiiCamera::ImageFormat)d->pCameraDriver->imageFormat();
  d->iBitsPerPixel = d->pCameraDriver->bitsPerPixel();
  
  PiiImageReaderOperation::check(reset);
}

void PiiCameraOperation::process()
{
  PII_D;
  d->pCameraDriver->triggerImage();
  d->waitCondition.wait();
}

void PiiCameraOperation::start()
{
  PII_D;

  if (d->pCameraDriver == 0)
    PII_THROW(PiiExecutionException, tr("Camera driver has not been set."));

  if (!d->pCameraDriver->isCapturing() && !d->pCameraDriver->startCapture(d->iMaxImages))
    PII_THROW(PiiExecutionException, tr("Couldn't start capture"));

  PiiImageReaderOperation::start();
}

void PiiCameraOperation::interrupt()
{
  PII_D;
  
  d->waitCondition.wakeAll();
  d->pCameraDriver->stopCapture();
  
  PiiImageReaderOperation::interrupt();
}

void PiiCameraOperation::pause()
{
  PII_D;

  d->waitCondition.wakeAll();
  /**
   * PENDING
   *
   * Maybe we can't stop capture here?
   */
  d->pCameraDriver->stopCapture();
  PiiImageReaderOperation::pause();
}

void PiiCameraOperation::stop()
{
  PII_D;
  
  d->waitCondition.wakeAll();
  d->pCameraDriver->stopCapture();

  PiiImageReaderOperation::stop();
}

void PiiCameraOperation::frameCaptured(int frameIndex, void *frameBuffer, qint64 elapsedTime)
{
  PII_D;
  
  if (frameIndex >= 0)
    {
      Pii::PtrOwnership ownership = frameBuffer != 0 ? Pii::ReleaseOwnership : Pii::RetainOwnership;
      void *pFrameBuffer = ownership == Pii::ReleaseOwnership ? frameBuffer : d->pCameraDriver->frameBuffer(frameIndex);
      
      if (pFrameBuffer == 0)
        qWarning("PiiCameraOperation::frameCapture(), pFrameBuffer == 0");
      else
        {
          switch (d->iBitsPerPixel)
            {
            case 8:
              convert<unsigned char>(pFrameBuffer, ownership, frameIndex, elapsedTime);
              break;
            case 16:
              convert<unsigned short>(pFrameBuffer, ownership, frameIndex, elapsedTime);
              break;
              /*
                case 24:
                convertColor<PiiColor<unsigned char> >(pFrameBuffer, ownership, frameIndex);
                break;
                case 32:
                convertColor<PiiColor4<unsigned char> >(pFrameBuffer,
                ownership, frameIndex);
                break;
              */
            }

          // If we are triggered-mode we must wake one now
          if (d->bTriggered)
            d->waitCondition.wakeOne();
        }
    }
  else if (d->bTriggered)
    d->waitCondition.wakeOne();
}

template <class T> void PiiCameraOperation::convert(void *frameBuffer, Pii::PtrOwnership ownership, int frameIndex, qint64 elapsedTime)
{
  PII_D;
  
  PiiMatrix<T> image(d->iImageHeight, d->iImageWidth, frameBuffer, ownership);

  if (!d->bUseRawImage)
    {
      switch(d->imageFormat)
        {
        case PiiCamera::BayerBGGRFormat:
          emitImage(bayerToRgb(image, PiiCamera::BggrDecoder<T>(), PiiCamera::Rgb4Pixel<>()), ownership, frameIndex, elapsedTime);
          break;
        default: emitImage(image, ownership, frameIndex, elapsedTime);
        }
    }
  else
    emitImage(image, ownership, frameIndex, elapsedTime);
}

template <class T> void PiiCameraOperation::emitImage(const PiiMatrix<T>& image, Pii::PtrOwnership ownership, int frameIndex, qint64 elapsedTime)
{
  PII_D;

  if (d->bCopyImage && ownership == Pii::RetainOwnership)
    {
      PiiMatrix<T> img(image);
      img.detach();
      d->pImageOutput->emitObject(processImage(PiiVariant(img), frameIndex, elapsedTime));
    }
  else
    d->pImageOutput->emitObject(processImage(PiiVariant(image), frameIndex, elapsedTime));
}

PiiVariant PiiCameraOperation::processImage(const PiiVariant& image, int /*frameIndex*/, qint64 /*elapsedTime*/)
{
  return image;
}

QVariant PiiCameraOperation::property(const char* name)
{
  PII_D;
  
  if (strncmp(name, "driver.", 7) == 0)
    {
      if (d->pCameraDriver != 0)
        return d->pCameraDriver->property(name+7);
    }
  else
    return PiiImageReaderOperation::property(name);

  return QVariant();
}


bool PiiCameraOperation::setProperty(const char* name, const QVariant& value)
{
  PII_D;
  
  if (strncmp(name, "driver.", 7)==0)
    {
      if (d->pCameraDriver != 0)
        return d->pCameraDriver->setProperty(name+7, value);
    }
  else
    return PiiImageReaderOperation::setProperty(name,value);

  return false;
}

bool PiiCameraOperation::loadCameraConfig(const QString& fileName)
{
  PII_D;

  if (d->pCameraDriver == 0)
    {
      piiWarning(tr("Camera driver has not been set."));
      return false;
    }

  if (!QFile::exists(fileName))
    {
      piiWarning(tr("Configuration file %1 doesn't exists.").arg(fileName));
      return false;
    }
  
  QSettings configSettings(fileName, QSettings::IniFormat);
  QStringList allKeys = configSettings.allKeys();
  for (int i=0; i<allKeys.size(); i++)
    d->pCameraDriver->setProperty(qPrintable(allKeys[i]), configSettings.value(allKeys[i]));
  
  return true;
}

bool PiiCameraOperation::saveCameraConfig(const QString& fileName)
{
  PII_D;

  if (d->pCameraDriver == 0)
    {
      piiWarning(tr("Camera driver has not been set."));
      return false;
    }

  QSettings configSettings(fileName, QSettings::IniFormat);
  configSettings.clear();

  QList<QPair<QString, QVariant> > lstProperties = Pii::propertyList(*d->pCameraDriver, 1, Pii::WritableProperties);
  for (int i=0; i<lstProperties.size(); i++)
    configSettings.setValue(lstProperties[i].first, lstProperties[i].second);
  
  return true;
}

void PiiCameraOperation::setDriverName(const QString& driverName)
{
  PII_D;
  PiiCameraDriver *pCameraDriver = PiiYdin::createResource<PiiCameraDriver>(driverName);
  if (pCameraDriver != 0)
    {
      if (d->pCameraDriver != 0)
        {
          d->pCameraDriver->close();
          delete d->pCameraDriver;
        }
      d->pCameraDriver = pCameraDriver;
      d->pCameraDriver->setObjectName("driver");
      d->pCameraDriver->setParent(this);
      d->pCameraDriver->setListener(this);
    }
  else
    piiWarning(tr("Camera driver %1 is not available.").arg(driverName));
}

QString PiiCameraOperation::driverName() const
{
  if (_d()->pCameraDriver != 0)
    return _d()->pCameraDriver->metaObject()->className();

  return QString();
}

QObject* PiiCameraOperation::driver() const
{
  return _d()->pCameraDriver;
}

QStringList PiiCameraOperation::cameraList() const
{
  try
    {
      if (_d()->pCameraDriver != 0)
        return _d()->pCameraDriver->cameraList();
    }
  catch (PiiCameraDriverException& ex)
    {
      piiWarning(ex.message());
    }

  return QStringList();
}

void PiiCameraOperation::setCameraId(const QString& cameraId)
{
  _d()->strCameraId = cameraId;
}

QString PiiCameraOperation::cameraId() const
{
  return _d()->strCameraId;
}

void PiiCameraOperation::framesMissed(int startIndex, int endIndex)
{
  piiWarning(tr("Missing frames %1 - %2").arg(startIndex).arg(endIndex));
}

void PiiCameraOperation::captureFinished()
{
  //operationStopped();
}

void PiiCameraOperation::captureError(const QString& message)
{
  piiWarning(tr("Error in capturing image: %1").arg(message));
}


void PiiCameraOperation::setCopyImage(bool copy)
{
  _d()->bCopyImage = copy;
}

bool PiiCameraOperation::copyImage() const
{
  return _d()->bCopyImage;
}
  
void PiiCameraOperation::setUseRawImage(bool useRawImage) { _d()->bUseRawImage = useRawImage; }
bool PiiCameraOperation::useRawImage() const { return _d()->bUseRawImage; }
