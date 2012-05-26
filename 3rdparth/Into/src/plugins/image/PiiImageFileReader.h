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

#ifndef _PIIIMAGEFILEREADER_H
#define _PIIIMAGEFILEREADER_H

#include <PiiQImage.h>
#include <PiiColor.h>
#include <QStringList>
#include <QVector>
#include "PiiImageReaderOperation.h"

/**
 * An operation for reading images from files. The operation provides
 * an UI component for selecting the image files.
 *
 * @image html imagefilereader.png
 *
 * @inputs
 *
 * @in trigger - an optional trigger input. A new image is emitted
 * whenever any object is received in this input.
 *
 * @in filename - an optional filename input (QString). If this input
 * is connected, file names are read from it instead of the #fileNames
 * property.
 *
 * @outputs
 *
 * @out image - the image output. Emits either four-channel color (the
 * default, PiiMatrix<PiiColor4<unsigned char> >) or grayscale images
 * in 8-bit (PiiMatrix<unsigned char>) channel format.
 *
 * @out filename - the file name of the current image (QString)
 *
 * @out key - the name of a meta-data field in the image (QString). 
 * There may be 0-N meta-data fields that will be emitted per image. 
 * This output emits them all with a flow level one higher than that
 * of @a image.
 *
 * @out value - the value of the field corresponding to @p key
 * (QString).
 *
 * @out metaX - X ranges from 0 to the number of #metaFields - 1. 
 * Emits the value of the meta field X, where X is an index to
 * #metaFields. If there is no such meta field, uses the default
 * value.
 *
 * @ingroup PiiImagePlugin
 */
class PII_IMAGE_EXPORT PiiImageFileReader : public PiiImageReaderOperation
{
  Q_OBJECT

  /**
   * A list of file names to open. The names may be either absolute or
   * relative. Setting this value clears #fileNamePattern.
   *
   * @code
   * pReader->setProperty("fileNames", QStringList() << "image1.bmp" << "image2.png" << "/tmp/image3.ppm");
   * @endcode
   */
  Q_PROPERTY(QStringList fileNames READ fileNames WRITE setFileNames);

  /**
   * A file name pattern that matches all the image files you want to
   * be opened. An example: "images/img_*.jpg". Setting this value
   * overwrites #fileNames.
   */
  Q_PROPERTY(QString fileNamePattern READ fileNamePattern WRITE setFileNamePattern STORED false);

  /**
   * The number of times the source emits the images. This is useful
   * if you want to process the same images again and again. 1 means
   * once, < 1 means eternally. This property has effect only if the
   * trigger input is not connected.
   *
   * @see PiiImageReaderOperation::maxImages
   */
  Q_PROPERTY(int repeatCount READ repeatCount WRITE setRepeatCount);

  /**
   * The total number of images the source will emit. This may be
   * different from @ref PiiImageReaderOperation::maxImages
   * "maxImages", if #repeatCount is set. -1 means eternally.
   */
  Q_PROPERTY(int totalImageCount READ totalImageCount);

  /**
   * Emission order randomization mode.
   */
  Q_PROPERTY(RandomizationMode randomizationMode READ randomizationMode WRITE setRandomizationMode);
  Q_ENUMS(RandomizationMode);

  /**
   * Lock files for reading. See PiiImageFireWriter::lockFiles for a
   * detailed description. This flag has no effect on Windows. The
   * default value is @p false.
   */
  Q_PROPERTY(bool lockFiles READ lockFiles WRITE setLockFiles);

  /**
   * A list of meta-data fields to be extracted from the image. Each
   * element in this list is a QVariantMap with two elements:
   *
   * @lip name - the name of the meta field to extract from the image
   * (QString)
   *
   * @lip defaultValue - default value for the field (int, double, or
   * QString). The meta field will be automatically converted to the
   * type of the default value.
   *
   * @code
   * // Extracts the "Software" meta field from input images
   * QVariantMap meta;
   * meta["name"] = "Software";
   * meta["defaultValue"] = QString();
   * pReader->setProperty("metaFields", QVariantList() << meta);
   * @endcode
   *
   * The meta field at index X in the list will be sent to the @p
   * metaX output.
   */
  Q_PROPERTY(QVariantList metaFields READ metaFields WRITE setMetaFields);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Emission order randomization mode.
   *
   * @lip NoRandomization - the order of images is determined by the
   * order of file names.
   *
   * @lip RandomizeOnce - the order of images is shuffled once in the
   * beginning.
   *
   * @lip RandomizeOnEachIteration - the order is shuffled at the
   * start of each repeat, if @ref repeatCount is larger than one.
   */
  enum RandomizationMode { NoRandomization, RandomizeOnce, RandomizeOnEachIteration };

  /**
   * Create an image source that reads the files indicated by the
   * given file name wildcard pattern (glob).
   */
  PiiImageFileReader(const QString& pattern = "");

  /**
   * Read an image from the file denoted by @p fileName. The image is
   * automatically converted to gray scale. (Y = (R+G+B)/3)
   *
   * @return a newly allocated PiiQImage. Must be deleted by the
   * caller. Returns 0 if reading does not succeed.
   */
  static PiiGrayQImage* readGrayImage(const QString& fileName);
  /**
   * Read an image from the file denoted by @p fileName. The image is
   * automatically converted to 32-bit RGBA. For most image types, the
   * alpha channel is zero.
   *
   * @return a newly allocated PiiQImage. Must be deleted by the
   * caller. Returns 0 if reading does not succeed.
   */
  static PiiColorQImage* readColorImage(const QString& fileName);

  void check(bool reset);
protected:
  void process();

  QStringList fileNames() const;
  void setFileNames(const QStringList& fileNames);
  
  QString fileNamePattern() const;
  void setFileNamePattern(const QString& pattern);

  int repeatCount() const;
  void setRepeatCount(int cnt);

  void setLockFiles(bool lockFiles);
  bool lockFiles() const;

  int totalImageCount() const;

  RandomizationMode randomizationMode() const;
  void setRandomizationMode(RandomizationMode mode);
  
  void setMetaFields(const QVariantList& metaFields);
  QVariantList metaFields() const;

private:
  void createIndices();
  void sendKeys(const QImage& img);
  
  /// @internal
  class Data : public PiiImageReaderOperation::Data
  {
  public:
    Data();
    QStringList lstFileNames;
    QString strPattern;
    int iRepeatCount;
    bool bFirst;
    bool bLockFiles;
    bool bTriggered, bNameConnected;
    int iStaticOutputCount;
    RandomizationMode randMode;
    QVector<int> vecIndices;
    
    PiiInputSocket* pNameInput;
    PiiOutputSocket *pNameOutput, *pKeyOutput, *pValueOutput;
    QList<QPair<QString,PiiVariant> > lstMetaFields;
    bool bSendKeys;
  };
  PII_D_FUNC;
};

#endif //_PIIIMAGEFILEREADER_H
