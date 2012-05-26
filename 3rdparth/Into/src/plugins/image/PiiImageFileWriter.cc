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

#include "PiiImageFileWriter.h"
#include <PiiYdinTypes.h>
#include <PiiColor.h>
#include "PiiImage.h"
#include <QFileInfo>
#include <QDir>

PiiImageFileWriter::Data::Data() :
  strNamePrefix("img"),
  strExtension("bmp"),
  iNextIndex(0),
  bStripPath(false),
  bLockFiles(true),
  bWriteEnabled(true),
  bAutoCreateDirectory(false),
  pixelSize(1.0,1.0),
  iCompression(-1),
  bKeyValuesConnected(false),
  bNameInputConnected(false),
  imageObject(0),
  nameObject(0),
  bStoreAlpha(false)
{
}

PiiImageFileWriter::PiiImageFileWriter() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  d->pImageInput = new PiiInputSocket("image");
  addSocket(d->pImageInput);
  
  d->pNameInput = new PiiInputSocket("filename");
  d->pNameInput->setOptional(true);
  addSocket(d->pNameInput);
  
  d->pKeyInput = new PiiInputSocket("key");
  d->pKeyInput->setOptional(true);
  d->pKeyInput->setGroupId(1);
  addSocket(d->pKeyInput);
  
  d->pValueInput = new PiiInputSocket("value");
  d->pValueInput->setOptional(true);
  d->pValueInput->setGroupId(1);
  addSocket(d->pValueInput);
  
  d->pNameOutput = new PiiOutputSocket("filename");
  addSocket(d->pNameOutput);

  d->iStaticInputCount = inputCount();
  setProtectionLevel("metaFields", WriteWhenStoppedOrPaused);
}

void PiiImageFileWriter::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);
  if (reset)
    {
      d->iNextIndex = 0;
      clearKeyValues();
    }
  
  if (d->pKeyInput->isConnected() != d->pValueInput->isConnected())
    PII_THROW(PiiExecutionException, tr("Key and value inputs must be connected or disconnected together."));

  d->bKeyValuesConnected = d->pKeyInput->isConnected();
  d->bNameInputConnected = d->pNameInput->isConnected();
}

void PiiImageFileWriter::clearKeyValues()
{
  PII_D;
  d->lstKeys.clear();
  d->lstValues.clear();
}

void PiiImageFileWriter::syncEvent(SyncEvent* event)
{
  if (event->type() == SyncEvent::StartInput)
    clearKeyValues();
  else
    processImage();
}

void PiiImageFileWriter::process()
{
  PII_D;
  if (activeInputGroup() == 0)
    {
      d->imageObject = d->pImageInput->firstObject();
      if (d->bNameInputConnected)
        d->nameObject = d->pNameInput->firstObject();

      d->lstStaticMeta.clear();
      for (int i=0; i<d->lstMetaFields.size(); ++i)
        d->lstStaticMeta << inputAt(i + d->iStaticInputCount)->firstObject();
      
      if (!d->bKeyValuesConnected)
        processImage();
    }      
  else if (activeInputGroup() == 1)
    {
      PiiVariant keyObj = d->pKeyInput->firstObject();
      if (keyObj.type() != PiiYdin::QStringType)
        PII_THROW_UNKNOWN_TYPE(d->pKeyInput);

      PiiVariant valueObj = d->pValueInput->firstObject();
      if (valueObj.type() != PiiYdin::QStringType)
        PII_THROW_UNKNOWN_TYPE(d->pValueInput);
      
      d->lstKeys << keyObj.valueAs<QString>();
      d->lstValues << valueObj.valueAs<QString>();
    }
}

void PiiImageFileWriter::processImage()
{
  PII_D;
  using namespace PiiYdin;

  if (!d->bWriteEnabled )
    {
      d->imageObject = PiiVariant();
      d->nameObject = PiiVariant();
      return;
    }

  QString strFileName;
      
  QString dir(d->strOutputDirectory.isEmpty() ? "." : d->strOutputDirectory);
  if (d->bNameInputConnected)
    {
      if (d->nameObject.type() != PiiYdin::QStringType)
        PII_THROW_UNKNOWN_TYPE(d->pNameInput);

      QFileInfo info(d->nameObject.valueAs<QString>());
      if (d->bStripPath)
        strFileName = QString("%1/%2%3").arg(dir).arg(d->strNamePrefix).arg(info.fileName());
      else
        {
          QString path = info.path();
          if (!info.isAbsolute() && !path.isEmpty())
            strFileName = QString("%1/%2/%3%4").arg(dir).arg(path).arg(d->strNamePrefix).arg(info.fileName());
          else
            strFileName = QString("%1/%2%3").arg(dir).arg(d->strNamePrefix).arg(info.fileName());
        }
    }
  else
    strFileName = QString("%1/%2%3.%4").arg(dir).arg(d->strNamePrefix).arg(d->iNextIndex, 6, 10, QChar('0')).arg(d->strExtension);
      
  //trying to make directory if it is not available
  if (d->bAutoCreateDirectory)
    {
      QFileInfo fileNameInfo(strFileName);
          
      QDir directory;
      if (!directory.exists(fileNameInfo.path()))
        {
          if (!directory.mkpath(fileNameInfo.path()))
            {
              qWarning("PiiImageFileWriter : Couldn't create a new image directory");
              return;
            }
        }
    }
      
  switch (d->imageObject.type())
    {
      PII_GRAY_IMAGE_CASES_M(writeGrayImage, (d->imageObject, strFileName));
    case UnsignedCharColorMatrixType:
      writeColorImage<PiiColor<unsigned char> >(d->imageObject, strFileName);
      break;
    case UnsignedCharColor4MatrixType:
      writeColorImage<PiiColor4<unsigned char> >(d->imageObject, strFileName);
      break;
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }
      
  d->iNextIndex++;
  d->imageObject = PiiVariant();
  d->nameObject = PiiVariant();
}

template <class T> void PiiImageFileWriter::writeGrayImage(const PiiVariant& obj, const QString& fileName)
{
  writeImage(Pii::createQImage(PiiImage::to8Bit(obj.valueAs<PiiMatrix<T> >())), fileName, _d()->bLockFiles);
}

template <class T> void PiiImageFileWriter::writeColorImage(const PiiVariant& obj, const QString& fileName)
{
  PII_D;
  QImage* pImage = Pii::createQImage(obj.valueAs<PiiMatrix<T> >());
  // If the input image has four channels and storing alpha channel is
  // enabled, change image format.
  if (sizeof(T) == 4 && d->bStoreAlpha)
    Pii::setQImageFormat(pImage, QImage::Format_ARGB32);
  writeImage(pImage, fileName, d->bLockFiles);
}

void PiiImageFileWriter::writeKeyValues(QImage* image)
{
  PII_D;
  image->setDotsPerMeterX(static_cast<int>(1000.0 / d->pixelSize.width()));
  image->setDotsPerMeterY(static_cast<int>(1000.0 / d->pixelSize.height()));
  for (int i=0; i<d->lstKeys.size(); i++)
    image->setText(d->lstKeys[i], d->lstValues[i]);
  // If the operation was paused while processing many key/value pairs
  // and the number of meta fields was changed, lstStaticMeta may be
  // empty.
  const int iMetaCnt = qMin(d->lstStaticMeta.size(), d->lstMetaFields.size());
  for (int i=0; i<iMetaCnt; ++i)
    {
      QString strValue = PiiYdin::convertToQString(d->lstStaticMeta[i]);
      if (strValue.isNull())
        PII_THROW_UNKNOWN_TYPE(inputAt(d->iStaticInputCount + i));
      image->setText(d->lstMetaFields[i], strValue);
    }
}

// There is no advisory file locking on Windows
#ifdef Q_OS_WIN
bool PiiImageFileWriter::writeImage(QImage* image, const QString& fileName, bool /*lock*/)
{
  // Delete image on return
  PiiSmartPtr<QImage> pImage(image);

  writeKeyValues(image);
  QString format = QFileInfo(fileName).suffix();
  if (format.isEmpty()) format = _d()->strExtension;
  return image->save(fileName, qPrintable(format), _d()->iCompression);
}
// On Unix, we can selectively protect against concurrent usage
#else
#include <sys/file.h>
#include <QFile>

bool PiiImageFileWriter::writeImage(QImage* image, const QString& fileName, bool lock)
{
  PII_D;
  // Delete image on return
  PiiSmartPtr<QImage> pImage(image);
  writeKeyValues(image);

  QString format = QFileInfo(fileName).suffix();
  if (format.isEmpty()) format = d->strExtension;

  // Must manually open the file to obtain its handle
  QFile f(fileName);
  // Append here ensures we don't truncate the file until we get the
  // lock.
  if (!f.open(QIODevice::WriteOnly | QIODevice::Append))
    return false;
  // If locking is requested and we can't do it, fail. This probably
  // happens only with network file systems such as Samba and NFS.
  if (lock && flock(f.handle(), LOCK_EX) == -1)
    {
      f.close();
      return false;
    }
  // Now we have the lock -> truncate the file
  if (!f.resize(0))
    {
      // Should not happen, but we never know...
      f.close();
      return false;
    }
  // Save to the locked file
  bool result = image->save(&f, format.toAscii().constData(), d->iCompression);

  // Close the file (this also unlocks it)
  f.close();
  return result;
}
#endif


QString PiiImageFileWriter::outputDirectory() const { return _d()->strOutputDirectory; }
void PiiImageFileWriter::setOutputDirectory(const QString& dirName) { _d()->strOutputDirectory = dirName; }
QString PiiImageFileWriter::namePrefix() const { return _d()->strNamePrefix; }
void PiiImageFileWriter::setNamePrefix(const QString& prefix) { _d()->strNamePrefix = prefix; }
QString PiiImageFileWriter::extension() const { return _d()->strExtension; }
void PiiImageFileWriter::setExtension(const QString& extension) { _d()->strExtension = extension; }
int PiiImageFileWriter::nextIndex() const { return _d()->iNextIndex; }
void PiiImageFileWriter::setNextIndex(int index) { _d()->iNextIndex = index; }
void PiiImageFileWriter::setStripPath(bool stripPath) { _d()->bStripPath = stripPath; }
bool PiiImageFileWriter::stripPath() const { return _d()->bStripPath; }
void PiiImageFileWriter::setLockFiles(bool lockFiles) { _d()->bLockFiles = lockFiles; }
bool PiiImageFileWriter::lockFiles() const { return _d()->bLockFiles; }
void PiiImageFileWriter::setWriteEnabled(bool writeEnabled) { _d()->bWriteEnabled = writeEnabled; }
bool PiiImageFileWriter::writeEnabled() const { return _d()->bWriteEnabled; }
void PiiImageFileWriter::setAutoCreateDirectory(bool autoCreateDirectory) { _d()->bAutoCreateDirectory = autoCreateDirectory; }
bool PiiImageFileWriter::autoCreateDirectory() const { return _d()->bAutoCreateDirectory; }
void PiiImageFileWriter::setPixelSize(const QSizeF& pixelSize) { _d()->pixelSize = pixelSize; }
QSizeF PiiImageFileWriter::pixelSize() const { return _d()->pixelSize; }
void PiiImageFileWriter::setCompression(int compression) { _d()->iCompression = compression; }
int PiiImageFileWriter::compression() const { return _d()->iCompression; }
void PiiImageFileWriter::setMetaFields(const QStringList& metaFields)
{
  PII_D;
  d->lstMetaFields = metaFields;
  d->lstStaticMeta.clear();
  setNumberedInputs(metaFields.size(), d->iStaticInputCount, "meta");
}
QStringList PiiImageFileWriter::metaFields() const { return _d()->lstMetaFields; }

void PiiImageFileWriter::setStoreAlpha(bool storeAlpha) { _d()->bStoreAlpha = storeAlpha; }
bool PiiImageFileWriter::storeAlpha() const { return _d()->bStoreAlpha; }
