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

#include "PiiLineScanEmulator.h"
#include <PiiAsyncCall.h>

#include <QMutexLocker>
#include <PiiDelay.h>
#include <QtDebug>

PiiLineScanEmulator::PiiLineScanEmulator() : _bOpen(false),
                                             _bCapturingRunning(false),
                                             _bBufferingRunning(false),
                                             _iSkippingLimit(0),
                                             _iFrameSizeInBytes(0),
                                             _pBufferingThread(0),
                                             _pCapturingThread(0),
                                             _iFrameIndex(-1),
                                             _iLastHandledFrame(-1),
                                             _iMaxFrames(0),
                                             _iHandledFrameCount(0),
                                             _frameWaitCondition(PiiWaitCondition::Queue),
                                             _triggerMode(PiiCameraDriver::SoftwareTrigger),
                                             _bSoftwareTrigger(true),
                                             _resolution(4096,1024),
                                             _iWidth(-1),
                                             _iHeight(-1),
                                             _iOffsetX(0),
                                             _iOffsetY(0),
                                             _iFrameBufferCount(5),
                                             _iFrameRate(10),
                                             _strDefectImagePattern(""),
                                             _dDefectProbability(0.0),
                                             _backgroundColor(Qt::gray),
                                             _dFieldOfView(0),
                                             _borderColor(Qt::black),
                                             _dEdgeElasticity(0.1),
                                             _iLeftEdgeLimit(0),
                                             _iRightEdgeLimit(0),
                                             _iTextureBlockSize(128),
                                             _bFirstScanLine(true),
                                             _pTextureGenerator(0),
                                             _iLineCounter(0),
                                             _iCurrentLineIndex(0),
                                             _iCurrLineInImage(0),
                                             _dTempProbability(0),
                                             _dTotalDefectRate(0),
                                             _dpMultipliers(0),
                                             _dLeftEdgePos(0),
                                             _dRightEdgePos(0)
{
  _lstCriticalProperties = QStringList() << "maxResolution"
                                         << "frameSize"
                                         << "frameRect"
                                         << "frameBufferCount";
    
}

PiiLineScanEmulator::~PiiLineScanEmulator()
{
  close();
}

QVariant PiiLineScanEmulator::property(const char* name)
{
  QVariant value;
  if (strncmp(name, "generator.", 10) == 0)
    {
      if (_pTextureGenerator != 0)
        value = _pTextureGenerator->property(name+10);
      else if (_mapGeneratorProperties.contains(name))
        value = _mapGeneratorProperties.value(name);
    }
  else
    value = PiiCameraDriver::property(name);

  return value;
}

bool PiiLineScanEmulator::setProperty(const char* name, const QVariant& value)
{
  if (strncmp(name, "generator.", 10)==0)
    {
      if (_pTextureGenerator != 0)
        return _pTextureGenerator->setProperty(name+10, value);
      else
        _mapGeneratorProperties[name+10] = value;
    }
  else
    return PiiCameraDriver::setProperty(name,value);

  return false;
}


QStringList PiiLineScanEmulator::cameraList() const
{
  return QStringList();
}

void PiiLineScanEmulator::initialize(const QString& cameraId)
{
  qDebug("PiiLineScanEmulator::initialize(%s)", qPrintable(cameraId));
  
  if (_bBufferingRunning)
    PII_THROW(PiiCameraDriverException, tr("Buffering is running. Stop the capture first."));

  if (_bCapturingRunning)
    PII_THROW(PiiCameraDriverException, tr("Capturing is running. Stop the capture first."));

  close();

  QVariantMap& dataMap = propertyMap();

  // Check if we must initialize textureGenerator
  if (!_bOpen)
    {
      if (dataMap.contains("textureGenerator"))
        setTextureGenerator(dataMap.take("textureGenerator").toString());

      // Write all generator values from the map
      if (_pTextureGenerator != 0)
        {
          for (QVariantMap::iterator i=_mapGeneratorProperties.begin(); i != _mapGeneratorProperties.end(); ++i)
            _pTextureGenerator->setProperty(qPrintable(i.key()), i.value());
        }
      _mapGeneratorProperties.clear();
    }

  if (dataMap.contains("maxResolution"))
    setMaxResolution(dataMap.take("maxResolution").toSize());

  // Write all configuration values from the map
  for (QVariantMap::iterator i=dataMap.begin(); i != dataMap.end(); ++i)
    {
      if (!QObject::setProperty(qPrintable(i.key()), i.value()))
        PII_THROW(PiiCameraDriverException, tr("Couldn't write the configuration value '%1'").arg(i.key()));
    }
  dataMap.clear();

  _bSoftwareTrigger = _triggerMode == PiiCameraDriver::SoftwareTrigger;

  // Initialize frameBuffer
  releaseFrames();
  _iFrameBufferHeight = _iFrameBufferCount * _iHeight;
  _frameBuffer = PiiMatrix<unsigned char>(_iFrameBufferHeight, _iWidth);
  _frameBuffer = 0;
  _vecBufferPointers.fill(0,_iFrameBufferCount);
  _iSkippingLimit = _vecBufferPointers.size() / 2;

  loadImages();

  _bOpen = true;
}

bool PiiLineScanEmulator::close()
{
  if (!_bOpen)
    return false;

  stopCapture();

  delete _pCapturingThread;
  delete _pBufferingThread;
  delete _pTextureGenerator;
  _pCapturingThread = 0;
  _pBufferingThread = 0;
  _pTextureGenerator = 0;
  
  _bOpen = false;

  return true;
}

bool PiiLineScanEmulator::setTriggerMode(PiiCameraDriver::TriggerMode mode)
{
  _triggerMode = mode;
  return true;
}

PiiCameraDriver::TriggerMode PiiLineScanEmulator::triggerMode() const
{
  return _triggerMode;
}

bool PiiLineScanEmulator::triggerImage()
{
  _triggerWaitCondition.wakeOne();
  return true;
}

bool PiiLineScanEmulator::startCapture(int frames)
{
  if (!_bOpen || listener() == 0 || _bCapturingRunning || _bBufferingRunning)
    return false;
  
  // Create and start the buffering and capturing threads
  if (_pBufferingThread == 0)
    _pBufferingThread = Pii::createAsyncCall(this, &PiiLineScanEmulator::buffer);

  if (_pCapturingThread == 0)
    _pCapturingThread = Pii::createAsyncCall(this, &PiiLineScanEmulator::capture);
  
  _bCapturingRunning = true;
  _bBufferingRunning = true;
  _iFrameIndex = -1;
  _iLastHandledFrame = -1;
  _iCurrentLineIndex = 0;
  _iHandledFrameCount = 0;
  _iMaxFrames = _bSoftwareTrigger ? 0 : frames;
  
  // Start acquisition
  if (false) //!_pEmulator->startAcquisition())
    {
      piiWarning(tr("Couldn't start acquisition with emulator"));
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

bool PiiLineScanEmulator::stopCapture()
{
  if (!_bBufferingRunning && !_bCapturingRunning)
    return false;

  // Stop the buffering and capturing threads
  stopBuffering();
  stopCapturing();
  
  return true;
}

void PiiLineScanEmulator::stopBuffering()
{
  _bBufferingRunning = false;
  _triggerWaitCondition.wakeAll();
  _pBufferingThread->wait();
}

void PiiLineScanEmulator::stopCapturing()
{
  _bCapturingRunning = false;
  _frameWaitCondition.wakeAll();
  _pCapturingThread->wait();
}

void PiiLineScanEmulator::buffer()
{
  generateMultipliers();
  _leftTargetPoint = QPoint(0,0);
  _rightTargetPoint = QPoint(0,0);
  _dLeftEdgePos = 0;
  _dRightEdgePos = 0;
  
  int msecs = 1000/_iFrameRate;

  while(_bBufferingRunning)
    {
      if (_bSoftwareTrigger)
        _triggerWaitCondition.wait();
      else
        PiiDelay::msleep(msecs);
      
      if (!_bBufferingRunning)
        break;
      
      _frameBufMutex.lock();

      // Generate lines (takes one frame)
      int iStartLineIndex = _iCurrentLineIndex;
      generateLine();
      while (_iCurrentLineIndex % _iHeight != 0)
        {
          if (true == newDefect() && _lstImages.size() > 0)
            {
              // Add a new image to the list of current defects
              //QImage tempImage = getRandomImage();
              int index = getRandomImage();
              _lstCurrDefImages << index;
              _lstDefectImageCoords << getRandomCoord(_lstImages[index]);
            }
          generateLine();
        }

      // Fake gain
      if (_dGain > 0)
        {
          double dFactor = _dGain + 1.0;
          for (int r=iStartLineIndex; r<(iStartLineIndex + _iHeight); r++)
            for (int c=0; c<_frameBuffer.columns(); c++)
              _frameBuffer(r,c) = (unsigned char)qBound(0.0,dFactor * _frameBuffer(r,c), 255.0);
        }
      
      // Increase frame index
      _iFrameIndex++;
      int iFrameIndex = _iFrameIndex % _vecBufferPointers.size();
      _vecBufferPointers[iFrameIndex] = _frameBuffer.row(iStartLineIndex);
      
      _frameBufMutex.unlock();
      _frameWaitCondition.wakeOne();
    }
}

void PiiLineScanEmulator::capture()
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
          releaseFrames(_iLastHandledFrame, _iFrameIndex-1);
          _iLastHandledFrame = _iFrameIndex;

          // Clear the wait condition queue
          _frameWaitCondition.wakeAll();
        }

      // Inform listener that a frame has been captured
      listener()->frameCaptured(_iLastHandledFrame, 0);

      _frameBufMutex.unlock();
      
      // Check if we must stop capturing
      if (_iMaxFrames > 0 && _iHandledFrameCount++ >= _iMaxFrames)
        {
          stopBuffering();
          _bCapturingRunning = false;
        }
    }

  // Inform listener
  listener()->captureFinished();

}

void PiiLineScanEmulator::releaseFrames(int start, int end)
{
  if (end == -1)
    end = _vecBufferPointers.size()-1;
  
  for (int i=start; i<=end; i++)
    {
      int index = i%_vecBufferPointers.size();
      if (_vecBufferPointers[index] != 0)
        _vecBufferPointers[index] = 0;
    }
}

void* PiiLineScanEmulator::frameBuffer(int frameIndex) const
{
  frameIndex %= _vecBufferPointers.size();
  if (frameIndex < 0)
    frameIndex += _vecBufferPointers.size();

  return _vecBufferPointers[frameIndex];
}

bool PiiLineScanEmulator::isOpen() const
{
  return _bOpen;
}

bool PiiLineScanEmulator::isCapturing() const
{
  return _bCapturingRunning;
}

bool PiiLineScanEmulator::requiresInitialization(const char* name) const
{
  return _lstCriticalProperties.contains(QString(name));
}

void PiiLineScanEmulator::setMaxResolution(const QSize& resolution)
{
  _resolution = resolution;

  if (_iWidth < 0)
    _iWidth = _resolution.width();
  else
    _iWidth = qMin(_iWidth, _resolution.width()-_iOffsetX);
  
  if (_iHeight < 0)
    _iHeight = _resolution.height();
  else
    _iHeight = qMin(_iHeight, _resolution.height()-_iOffsetY);
}

bool PiiLineScanEmulator::setFrameSize(const QSize& frameSize)
{
  _iWidth = qMin(frameSize.width(), _resolution.width() - _iOffsetX);
  _iHeight = qMin(frameSize.height(), _resolution.height() - _iOffsetY);
  
  return true;
}

bool PiiLineScanEmulator::setFrameRect(const QRect& frameRect)
{
  int iWidth = frameRect.width() & ~3;
  int iHeight = frameRect.height() & ~3;
  int iOffsetX = frameRect.x() & ~3;
  int iOffsetY = frameRect.y() & ~3;
  int iMaxWidth = _resolution.width();
  int iMaxHeight = _resolution.height();
  int iMinWidth = 1;
  int iMinHeight = 1;

  if (iWidth < 0) iWidth = iMaxWidth;
  if (iHeight < 0) iHeight = iMaxHeight;

  iOffsetX = qBound(0, iOffsetX, iMaxWidth - iMinWidth);
  iOffsetY = qBound(0, iOffsetY, iMaxHeight - iMinHeight);
  iWidth = qBound(iMinWidth, iWidth, iMaxWidth - iOffsetX);
  iHeight = qBound(iMinHeight, iHeight, iMaxHeight - iOffsetY);

  _iWidth = iWidth;
  _iHeight = iHeight;
  _iOffsetX = iOffsetX;
  _iOffsetY = iOffsetY;

  return true;
}


/* This is a private function, which load the defect images into the
   internal image buffer. */
bool PiiLineScanEmulator::loadImages()
{
  // 1) Make the file list
  
  QDir directory = QFileInfo(_strDefectImagePattern).dir();
  QString glob = QFileInfo(_strDefectImagePattern).fileName();

  // Take the names of matching file names relative to "directory"
  QStringList files = directory.entryList(QStringList() << glob, //take all matching entries
					  QDir::Files | QDir::Readable | QDir::CaseSensitive, //only readable files
					  QDir::Name); //sort by name

  // 2) Add the full path in the beginning of each file.
  files.replaceInStrings ( QRegExp("^"), directory.path() + "/" );

  QList<QImage> images, alphaMasks;
  foreach( QString fileName, files)
    {
      QImage img;
      if (!img.load(fileName))
        {
          qWarning("PiiLineScanEmulator cannot load %s.", qPrintable(fileName));
          continue;
        }
      // Currently, only 8 bit grey-scale images are supported. Check
      // if the image meet the requirements.
      switch (img.format())
        {
        case QImage::Format_Indexed8:
          images << img;
          alphaMasks << QImage();
          break;
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
          {
            if (img.hasAlphaChannel())
              alphaMasks << img.alphaChannel();
            else
              alphaMasks << QImage();
            
            QImage gsImage(img.width(), img.height(), QImage::Format_Indexed8);
            // Convert to gray scale
            for (int r=img.height(); r--; )
              {
                QRgb* colorLine = reinterpret_cast<QRgb*>(img.scanLine(r));
                uchar* gsLine = gsImage.scanLine(r);
                for (int c=img.width(); c--; )
                  gsLine[c] = (qRed(colorLine[c]) + qGreen(colorLine[c]) + qBlue(colorLine[c])) / 3;
              }
            images << gsImage;
          }
          break;
        default:
          // do not store the image
          break;
        }
    }

  // 3) Set images to the generator
  setImages(images, alphaMasks);

  return true;
}

void PiiLineScanEmulator::generateTexture()
{
  // If texture generator is not set, use a constant color for the background
  if (_pTextureGenerator == 0)
    memset(_frameBuffer.row(_iCurrentLineIndex), _backgroundColor.red(), _iWidth);
  // Else use the generator to produce background texture (in blocks)
  else if (_iCurrentLineIndex % _iTextureBlockSize == 0)
    {
      int iRows = qMin(_iTextureBlockSize, _iFrameBufferHeight - _iCurrentLineIndex);
      _pTextureGenerator->generateTexture(_frameBuffer,
                                          _iCurrentLineIndex, 0,
                                          iRows,
                                          _frameBuffer.columns(),
                                          _bFirstScanLine);

      _bFirstScanLine = false;
    }
}

bool PiiLineScanEmulator::setTextureGenerator(const QString& textureGenerator)
{
  PiiTextureGenerator *pGenerator = PiiYdin::createResource<PiiTextureGenerator>(textureGenerator);
  if (pGenerator != 0)
    {
      delete _pTextureGenerator;
      _pTextureGenerator = pGenerator;
      _pTextureGenerator->setObjectName("generator");
      _pTextureGenerator->setParent(this);
    }
  else
    piiWarning(tr("TextureGenerator %1 is not available.").arg(textureGenerator));

  // Ensure that the generator will not try to continue from what
  // the previous one left behind.
  _bFirstScanLine = true;
  
  return true;
}

QString PiiLineScanEmulator::textureGenerator() const
{
  if (_pTextureGenerator == 0)
    return "";
  return _pTextureGenerator->metaObject()->className();
}


/*******************************
 * ScanLineGenerator -functions.
 *******************************/
void PiiLineScanEmulator::setImages(const QList<QImage>& images, const QList<QImage>& alphaMasks)
{
  _lstImages = images;
  _lstAlphaMasks = alphaMasks;
}

void PiiLineScanEmulator::generateMultipliers()
{
  delete[] _dpMultipliers;

  int width = _iWidth;
  if (_dFieldOfView != 0 && width > 1)
    {
      _dpMultipliers = new double[width];
      // Field of view ranges from -fov/2 -> fov/2
      // Convert to radians at the same time
      double theta = -_dFieldOfView / 360 * M_PI;
      // Angle step
      double step = _dFieldOfView / 180 * M_PI / (width-1);
      // This generates 1.0 at the center. Multipliers decrease
      // towards the edges of the scan line according to the cos^4
      // law.
      for (int i=0; i<width; i++, theta += step)
        _dpMultipliers[i] = ::pow(::cos(theta),4);
    }
}

/* Returns true, if a new defect image should be generated. Else
   returns false. */
bool PiiLineScanEmulator::newDefect()
{
  if (((double)rand() / RAND_MAX) < _dTempProbability)
    return true;
  else
    return false;
}

/* This is a private function, which returns a random image from
   _lstImages. The function assumes that the _lstImages contain some
   images. Else the program will crash. */
int PiiLineScanEmulator::getRandomImage()
{
  return rand() % _lstImages.size();
}

  /* This is a private function, which returns a random coordinate for
   the image given as a parameter. The x-coordinate is generated
   randomly, but the y-coordinate is always 0. The reason for that is
   explained in the documentation of the _lstDefectImageCoords in
   h-file. The image given as a parameter is needed for ensuring, that
   the image won't appear outside of the buffer in the x-direction. If
   the width of the image is greater than the buffer width, then 0 is
   allways generated for the x. In that case the image won't fit in
   the buffer.  */
QPoint PiiLineScanEmulator::getRandomCoord(const QImage& image)
{
  int maxX = _iWidth - image.width();
  if (maxX < 0)
    return QPoint(0,0); // The image doesn't fit in the web.
  return QPoint(rand() % maxX,0);
}

/* Genarates a new line in the buffer. If there are images in _lstCurrDefImages
   single lines of them are added in the line. If the last line
   of an image is read, the image is removed from _lstCurrDefImages.*/
void PiiLineScanEmulator::generateLine()
{
  // Generate background
  generateTexture();

  unsigned char* line = _frameBuffer.row(_iCurrentLineIndex);
  
  // New defect pixel in this line.
  int newDefPixels = 0;

  // Copy the single lines from the the defect images to the buffer.
  for (int i = 0; i<_lstCurrDefImages.size(); i++)
    {
      QImage& tempImage = _lstImages[_lstCurrDefImages[i]];
      QImage& alphaMask = _lstAlphaMasks[_lstCurrDefImages[i]];
      QPoint& tempCoord = _lstDefectImageCoords[i];
      // If there is no alpha mask, just copy image data to the frame
      // buffer.
      if (alphaMask.isNull())
        memcpy(line+tempCoord.x(), tempImage.scanLine(tempCoord.ry()++), tempImage.width());
      else // If there is an alpha channel, blend
        {
          uchar* pTarget = line+tempCoord.x();
          uchar* pSource = tempImage.scanLine(tempCoord.y());
          uchar* pAlpha = alphaMask.scanLine(tempCoord.y());
          for (int c=tempImage.width(); c--; )
            pTarget[c] = ((255 - pAlpha[c]) * pTarget[c] + pAlpha[c] * pSource[c]) / 255;;
          tempCoord.ry()++;
        }      

      newDefPixels += tempImage.width();

      // Check if the whole defect image has been drawn. If yes,
      // remove it from the list of current defect images and its
      // coordinates from the list of current defect coordinates.
      if ( tempCoord.y() >= tempImage.height() )
        {
          _lstCurrDefImages.removeAt(i);
          _lstDefectImageCoords.removeAt(i);
          i--; //Important!!
        }
    }

  // Generate material edges
  int iLeftEdgeLimit = qMax(0,_iLeftEdgeLimit-_iOffsetX);
  if (iLeftEdgeLimit > 0)
    {
      // Update edge position
      _dLeftEdgePos = updateEdgePos(_dLeftEdgePos, _leftTargetPoint, iLeftEdgeLimit);
      // Copy border color
      memset(line, _borderColor.red(), int(_dLeftEdgePos));
    }
  int iRightEdgeLimit = qMax(0,_iRightEdgeLimit - (_resolution.width() - (_iOffsetX + _iWidth)));
  if (iRightEdgeLimit > 0)
    {
      _dRightEdgePos = updateEdgePos(_dRightEdgePos, _rightTargetPoint, iRightEdgeLimit);
      memset(line + _iWidth - int(_dRightEdgePos), _borderColor.red(), int(_dRightEdgePos));
    }
  
  // Simulate vignetting
  if (_dpMultipliers)
    for (int i=_iWidth; i--; )
      line[i] = static_cast<unsigned char>(_dpMultipliers[i] * line[i]);
  
  updateTotalDefRate(double(newDefPixels)/_iWidth);

  _iCurrentLineIndex++;
  if (_iCurrentLineIndex >= _iFrameBufferHeight)
    _iCurrentLineIndex = 0;

} 

double PiiLineScanEmulator::updateEdgePos(double pos, QPoint& targetPoint, int limit)
{
  // If we passed the target point, generate new coordinates
  if (_iLineCounter >= targetPoint.y())
    {
      targetPoint.rx() = rand() % limit;
      targetPoint.ry() += 100;
    }
  // Update edge position
  if (pos < targetPoint.x())
    pos += _dEdgeElasticity;
  else
    pos -= _dEdgeElasticity;
  // Ensure valid range
  if (pos > limit)
    pos = limit;
  else if (pos < 0)
    pos = 0;
  return pos;
}


/* This private function updates the total defect rate. The defect
   rate of the current row is given as a paramater. The member
   variable _dTempProbability, which is used in deciding if the new
   defects will be generated, is also adjusted in this function.  */
void PiiLineScanEmulator::updateTotalDefRate(double currRowDefRate)
{
  // Update the total defect rate by using the equtation:
  // newRate = oldRate - (oldRate - defectRateInLine)/totalLines
  // e.g rate = 1/5103*0.25 + 5102/5103*0.5392431
  _dTotalDefectRate = _dTotalDefectRate - ( _dTotalDefectRate - currRowDefRate )/++(_iLineCounter);

  if (_dTotalDefectRate > _dDefectProbability && _dTempProbability > 0 )
    _dTempProbability -= .02;
  else if (_dTempProbability < 1)
    _dTempProbability += .001;

}
