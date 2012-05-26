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

#include "PiiImageFileReader.h"
#include <PiiYdinTypes.h>
#include <PiiUtil.h>
#include <QDir>
#include <QFileInfo>
#include <QtGui>

#ifndef Q_OS_WIN
#  include <sys/file.h>
#  include <QFile>
#endif

using namespace PiiYdin;
using namespace Pii;

PiiImageFileReader::Data::Data() :
  iRepeatCount(1), bFirst(false), bLockFiles(false),
  bTriggered(false), bNameConnected(false),
  randMode(NoRandomization),
  bSendKeys(false)
{
}

PiiImageFileReader::PiiImageFileReader(const QString& pattern) :
  PiiImageReaderOperation(new Data)
{
  PII_D;
  if (!pattern.isEmpty())
    setFileNamePattern(pattern);

  addSocket(d->pNameInput = new PiiInputSocket("filename"));
  d->pNameInput->setOptional(true);
  addSocket(d->pNameOutput = new PiiOutputSocket("filename"));

  addSocket(d->pKeyOutput = new PiiOutputSocket("key"));
  addSocket(d->pValueOutput = new PiiOutputSocket("value"));

  d->iStaticOutputCount = outputCount();

  setProtectionLevel("metaFields", WriteWhenStoppedOrPaused);
}

void PiiImageFileReader::check(bool reset)
{
  PII_D;
  PiiImageReaderOperation::check(reset);
  if (reset)
    {
      d->iCurrentIndex = 0;
      d->bFirst = true;
    }

  d->bNameConnected = d->pNameInput->isConnected();
  if (d->lstFileNames.size() == 0 && !d->bNameConnected)
    PII_THROW(PiiExecutionException,
              "Image source cannot start because file names have not been set "
              "or there are no files that match the given file name pattern.");

  d->bSendKeys = false;
  // Check key, value, and dynamic meta outputs
  for (int i=d->iStaticOutputCount-2; i<outputCount(); ++i)
    if (outputAt(i)->isConnected())
      {
        d->bSendKeys = true;
        break;
      }

  d->bTriggered = d->pTriggerInput->isConnected() || d->bNameConnected;
}

void PiiImageFileReader::process()
{
  PII_D;
  if (!d->bNameConnected &&
      d->randMode == RandomizeOnEachIteration &&
      d->iCurrentIndex % d->lstFileNames.size() == 0)
    Pii::shuffle(d->vecIndices);

  QString fileName;
  //We only track the counts if neither trigger input isn't connected
  if (!d->bTriggered)
    {
      int loopIndex = d->iCurrentIndex / d->lstFileNames.size();
      if ((d->iMaxImages > 0 && d->iCurrentIndex >= d->iMaxImages) ||
          (d->iRepeatCount > 0 && loopIndex >= d->iRepeatCount))
        operationStopped(); //stop here
      fileName = d->lstFileNames[d->vecIndices[d->iCurrentIndex % d->lstFileNames.size()]];
    }
  else if (d->bNameConnected) // name input is connected -> we don't care about trigger
    {
      fileName = PiiYdin::convertToQString(d->pNameInput);
    }
  else // only trigger is connected
    {
      // If a trigger object is received and it is an integer, it is
      // added to the current image index.
      PiiVariant obj = d->pTriggerInput->firstObject();

      int step = obj.type() == PiiVariant::IntType ? obj.valueAs<int>() : 1;
      if (d->bFirst)
        {
          d->bFirst = false;
          if (step > 0) step--;
        }
      d->iCurrentIndex += step;
      while (d->iCurrentIndex < 0)
        d->iCurrentIndex += d->lstFileNames.size();
      fileName = d->lstFileNames[d->vecIndices[d->iCurrentIndex % d->lstFileNames.size()]];
    }

  //qDebug("PiiImageFileReader: Emitting image %d/%d", d->iCurrentIndex+1, d->lstFileNames.size());

  QImage img;
#ifdef Q_OS_WIN // no locking on windows
  if (!img.load(fileName))
    PII_THROW(PiiExecutionException, tr("Cannot read image \"%1\".").arg(fileName));
#else
  // Must manually open the file to obtain its handle
  // See PiiImageFileReader.h for a detailed description.
  QFile f(fileName);
  if (!f.open(QIODevice::ReadOnly))
    {
      f.close();
      PII_THROW(PiiExecutionException, tr("Cannot open %1.").arg(fileName));
    }
  if (d->bLockFiles && flock(f.handle(), LOCK_SH) == -1)
    {
      f.close();
      PII_THROW(PiiExecutionException, tr("Cannot lock %1.").arg(fileName));
    }
  if (!img.load(&f, QFileInfo(fileName).suffix().toAscii().constData()))
    {
      f.close();
      PII_THROW(PiiExecutionException, tr("Cannot decode %1.").arg(fileName));
    }
  f.close();
#endif

  if (d->bSendKeys)
    sendKeys(img);

  if (d->imageType == GrayScale)
    emitGrayImage(img);
  else if (d->imageType == Color)
    emitColorImage(img);
  else
    emitImage(img);

  d->pNameOutput->emitObject(fileName);

  // Auto-advance if no trigger
  if (!d->bTriggered)
    d->iCurrentIndex++;
}

void PiiImageFileReader::sendKeys(const QImage& img)
{
  PII_D;
  QStringList lstKeys = img.textKeys();
  d->pKeyOutput->startMany();
  d->pValueOutput->startMany();
  for (int i=0; i<lstKeys.size(); ++i)
    {
      QString strValue(img.text(lstKeys[i]));
      d->pKeyOutput->emitObject(lstKeys[i]);
      d->pValueOutput->emitObject(strValue);
    }
  d->pKeyOutput->endMany();
  d->pValueOutput->endMany();
  for (int i=0; i<d->lstMetaFields.size(); ++i)
    {
      PiiVariant varMeta(d->lstMetaFields[i].second);
      // If this image has the specified text field, take it as the value.
      if (lstKeys.contains(d->lstMetaFields[i].first))
        {
          QString strValue = img.text(d->lstMetaFields[i].first);
          switch (d->lstMetaFields[i].second.type())
            {
            case PiiVariant::IntType:
              varMeta = PiiVariant(strValue.toInt());
              break;
            case PiiVariant::DoubleType:
              varMeta = PiiVariant(strValue.toDouble());
              break;
            default:
              varMeta = PiiVariant(strValue);
              break;
            }
        }
      outputAt(d->iStaticOutputCount + i)->emitObject(varMeta);
    }
}

void PiiImageFileReader::createIndices()
{
  PII_D;
  if (d->vecIndices.size() != d->lstFileNames.size())
    {
      d->vecIndices.clear();
      d->vecIndices.reserve(d->lstFileNames.size());
    }
  for (int i=0; i<d->lstFileNames.size(); i++)
    d->vecIndices << i;
  if (d->randMode != NoRandomization)
    Pii::shuffle(d->vecIndices);
}

void PiiImageFileReader::setFileNamePattern(const QString& pattern)
{
  PII_D;
  QDir directory = QFileInfo(pattern).dir();
  QString glob = QFileInfo(pattern).fileName();

  d->lstFileNames.clear();
  // Take the names of matching file names relative to "directory"
  QStringList names = directory.entryList(QStringList() << glob, //take all matching entries
					  QDir::Files | QDir::Readable | QDir::CaseSensitive, //only readable files
					  QDir::Name); //sort by name
  // Add path to each name
  foreach (QString name, names)
    d->lstFileNames << directory.path() + "/" + name;
  d->strPattern = pattern;
  createIndices();
  d->iCurrentIndex = 0;
}

int PiiImageFileReader::totalImageCount() const
{
  const PII_D;
  int maxCount = -1;
  if (d->iRepeatCount > 0)
    maxCount = d->iRepeatCount * d->lstFileNames.size();
  if (d->iMaxImages > 0)
    maxCount = (d->iMaxImages < maxCount || maxCount == -1) ? d->iMaxImages : maxCount;
  return maxCount;
}

PiiGrayQImage* PiiImageFileReader::readGrayImage(const QString& fileName)
{
  QImage img;
  if (!img.load(fileName))
    return 0;

  Pii::convertToGray(img);
  return PiiGrayQImage::create(img);
}

PiiColorQImage* PiiImageFileReader::readColorImage(const QString& fileName)
{
  QImage img;
  if (!img.load(fileName))
    return 0;

  Pii::convertToRgba(img);
  return PiiColorQImage::create(img);
}

QStringList PiiImageFileReader::fileNames() const { return _d()->lstFileNames; }
void PiiImageFileReader::setFileNames(const QStringList& fileNames)
{
  PII_D;
  d->lstFileNames = fileNames;
  d->strPattern = "";
  createIndices();
  d->iCurrentIndex = 0;
}

QString PiiImageFileReader::fileNamePattern() const { return _d()->strPattern; }
int PiiImageFileReader::repeatCount() const { return _d()->iRepeatCount; }
void PiiImageFileReader::setRepeatCount(int cnt) { _d()->iRepeatCount = cnt; }
void PiiImageFileReader::setLockFiles(bool lockFiles) { _d()->bLockFiles = lockFiles; }
bool PiiImageFileReader::lockFiles() const { return _d()->bLockFiles; }
void PiiImageFileReader::setRandomizationMode(RandomizationMode mode)
{
  _d()->randMode = mode;
  createIndices();
}
PiiImageFileReader::RandomizationMode PiiImageFileReader::randomizationMode() const { return _d()->randMode; }

void PiiImageFileReader::setMetaFields(const QVariantList& metaFields)
{
  PII_D;
  d->lstMetaFields.clear();
  for (int i=0; i<metaFields.size(); ++i)
    {
      QVariantMap meta = metaFields[i].toMap();
      QString strName = meta["name"].toString();
      QVariant varDefault = meta["defaultValue"];
      if (strName.isEmpty() || !varDefault.isValid())
        {
          piiWarning(tr("Invalid meta field specification in metaFields[%1].").arg(i));
          continue;
        }
      PiiVariant defaultVal;
      switch (varDefault.type())
        {
        case QVariant::Int:
          defaultVal = PiiVariant(varDefault.toInt());
          break;
        case QVariant::Double:
          defaultVal = PiiVariant(varDefault.toDouble());
          break;
        default:
          defaultVal = PiiVariant(varDefault.toString());
          break;
        }
      d->lstMetaFields << qMakePair(strName, defaultVal);
    }
  setNumberedOutputs(d->lstMetaFields.size(), d->iStaticOutputCount, "meta");
}

QVariantList PiiImageFileReader::metaFields() const
{
  const PII_D;
  QVariantList lstResult;
  for (int i=0; i<d->lstMetaFields.size(); ++i)
    {
      QVariantMap meta;
      meta["name"] = d->lstMetaFields[i].first;
      switch (d->lstMetaFields[i].second.type())
        {
        case PiiVariant::IntType:
          meta["defaultValue"] = d->lstMetaFields[i].second.valueAs<int>();
          break;
        case PiiVariant::DoubleType:
          meta["defaultValue"] = d->lstMetaFields[i].second.valueAs<double>();
          break;
        default:
          meta["defaultValue"] = d->lstMetaFields[i].second.valueAs<QString>();
          break;
        }
      lstResult << meta;
    }
  return lstResult;
}
