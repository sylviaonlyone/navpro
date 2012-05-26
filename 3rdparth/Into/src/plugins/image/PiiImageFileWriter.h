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

#ifndef _PIIIMAGEFILEWRITER_H
#define _PIIIMAGEFILEWRITER_H

#include <PiiDefaultOperation.h>
#include <PiiQImage.h>
#include <QFileInfo>
#include "PiiImageGlobal.h"

/**
 * An operation that writes images into files in standard image
 * formats. For each input image, a new file will be created. The full
 * name of the new file follows the pattern
 * [#outputDirectory]/[#namePrefix][#nextIndex].[#extension]. For
 * example, if outputDirectory = "images", namePrefix = "img", index =
 * 22, and extension = jpg, the output file will be
 * "images/img000022.jpg". The index is incremented by one on each
 * processing round. If an image with the same name already exists, it
 * will be overwritten.
 *
 * @inputs
 *
 * @in image - an image with 8, 24, or 32 bits per pixel. 
 * Floating-point images are converted to gray-scale by multiplying by
 * 255 and quantizing into 256 levels.
 *
 * @in filename - the name of the output file. If this input is
 * connected, automatic file name generation will be turned off. 
 * The output directory and image prefix are still in effect. For
 * example, if "images/naama.bmp" is read, the output file name will
 * be [#outputDirectory]/images/[#namePrefix]naama.bmp. If the
 * #stripPath flag is @p true, the file name will be
 * [#outputDirectory]/[#namePrefix]naama.bmp. If the file name is
 * equipped with an absolute path, the output file will be exactly the
 * same, expect for an optional name prefix.
 *
 * @in key - the name of a custom meta-data field to be stored in the
 * image (QString). Note that not all image formats support custom
 * meta-data. The key input must work at a flow level one higher than
 * that of @p image.
 *
 * @in value - the value corresponding to the key (QString).
 *
 * @out metaX - X ranges from 0 to the number of #metaFields - 1. 
 * Receives the value of the meta field X, where X is an index to
 * #metaFields. Accepted types are @p int, @p double, and @p QString.
 *
 * @outputs
 *
 * @out filename - the full name of the current image, including path. 
 * The path is relative unless the @p filename input or the
 * #outputDirectory property contains an absolute path.
 *
 * @ingroup PiiImagePlugin
 */
class PII_IMAGE_EXPORT PiiImageFileWriter : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The directory the writer will store the images in. Relative and
   * absolute paths are OK. An example: "images".
   */
  Q_PROPERTY(QString outputDirectory READ outputDirectory WRITE setOutputDirectory);

  /**
   * A string the file names will be prefixed with. The default prefix
   * is "img".
   */
  Q_PROPERTY(QString namePrefix READ namePrefix WRITE setNamePrefix);

  /**
   * The file name extension for the files. Despite the name, the
   * extension also determines the format of the file. Valid
   * extensions may vary depending on your setup, but in general at
   * least "png", "jpg", "bmp", and "xpm" are safe. The default
   * extension is bmp. Do not include a dot in the extension. If the
   * @p filename input is connected, the name read from the input
   * contains an extension, it will override this property.
   */
  Q_PROPERTY(QString extension READ extension WRITE setExtension);
  
  /**
   * The index of the next image. Indices are padded with zeros so
   * that their length always equals six. Thus, at most one million
   * images may be written into a single directory without overwriting
   * anything. The default start index is zero.
   */
  Q_PROPERTY(int nextIndex READ nextIndex WRITE setNextIndex);

  /**
   * Strip path names from input file names. This flag is useful if
   * you use the name supplied by PiiImageFileReader and want to store
   * images in a different directory. The default value is @p false.
   */
  Q_PROPERTY(bool stripPath READ stripPath WRITE setStripPath);
  
  /**
   * Use file locking. Setting this flag to true causes
   * %PiiImageFileWriter to acquire an exclusive (write) lock for the
   * output file before writing to it. This is useful if another
   * process is reading the files concurrently. The default value is
   * @p false.
   *
   * @par Notes
   *
   * @li The flock() function is used for locking. Thus, the other
   * process accessing the files must also use flock(). fcntl() won't
   * work.
   *
   * @li Locking is only available where flock() is available (Unix,
   * not Windows).
   *
   * @li flock() doesn't work on NFS or Samba shares.
   *
   * @li flock() locks are process-wide. You cannot use them for
   * mutual exclusion within the same process. That is, you cannot
   * write to the same file with two PiiImageFileWriters in a single
   * configuration.
   *
   * To safely read the images written by %PiiImageFileWriter, do the
   * following (the C way):
   *
   * @code
   * int handle = open("image.jpg", O_RDONLY);
   * // Acquire a shared (read) lock for the file.
   * // This call blocks until the file is available.
   * flock(handle, LOCK_SH); // returns -1 if locking fails
   * char bfr[10];
   * // Read data
   * read(handle, bfr, 10);
   * // Closes the handle and releases the lock
   * close(handle);
   * @endcode
   *
   * The %Qt way:
   *
   * @code
   * QFile f("image.jpg");
   * f.open(QIODevice::ReadOnly);
   * flock(f.handle(), LOCK_SH);
   * f.read(...);
   * f.close();
   * @endcode
   */
  Q_PROPERTY(bool lockFiles READ lockFiles WRITE setLockFiles);

  /**
   * Enable/disable writing. This property can be used to temporarily
   * disable writing without removing the operation itself. Default is
   * @p true.
   */
  Q_PROPERTY(bool writeEnabled READ writeEnabled WRITE setWriteEnabled);

  /**
   * A flag that controls automatic creation of directories. If the
   * output directory does not exist, the operation will create it for
   * you automatically, if this flag is @p true. Otherwise the image
   * just won't be written. Default is @p false.
   */
  Q_PROPERTY(bool autoCreateDirectory READ autoCreateDirectory WRITE setAutoCreateDirectory);
  
  /**
   * The physical size of a pixel. The default value is (1.0, 1.0).
   */
  Q_PROPERTY(QSizeF pixelSize READ pixelSize WRITE setPixelSize);

  /**
   * This is an image format specific property that sets the
   * compression of an image. For image formats that do not support
   * setting the compression, this value is ignored. The compression
   * must be in the range 0 to 100 or -1. Specify 0 to obtain small
   * compressed files, 100 for large uncompressed files, and -1 (the
   * default) to use the default settings.
   */
  Q_PROPERTY(int compression READ compression WRITE setCompression);

  /**
   * A list of meta-data fields to be written to the image. Element X
   * element in this list specifies the name of a meta field
   * corresponding to the value read from the @p metaX input. If the
   * same key is present in @p metaFields and the @p key input, the
   * value read from the @p metaX input will be used.
   */
  Q_PROPERTY(QStringList metaFields READ metaFields WRITE setMetaFields);

  /**
   * Enables/disables storing of image alpha channel with image
   * formats that support alpha. The default value is @p false.
   */
  Q_PROPERTY(bool storeAlpha READ storeAlpha WRITE setStoreAlpha);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiImageFileWriter();
  
  /**
   * Write a matrix as an image to a file.
   *
   * @param image the image to be written. The data type will be
   * automatically converted to a type supported by QImage.
   *
   * @param fileName the name of the file. The file name extension
   * determines the type of the file.
   *
   * @param lock if @p true, flock() is used to acquire an exclusive
   * lock on the file before writing. This features is not available
   * on Windows. See @ref PiiImageFileWriter "above" for details.
   *
   * @return @p true if the image was successfully written, @p false
   * otherwise
   */
  template <class T> bool writeImage(const PiiMatrix<T>& matrix, const QString& fileName, bool lock = false);

  void check(bool reset);
  
protected:
  void syncEvent(SyncEvent* event);
  void process();

  QString outputDirectory() const;
  void setOutputDirectory(const QString& dirName);

  QString namePrefix() const;
  void setNamePrefix(const QString& prefix);

  QString extension() const;
  void setExtension(const QString& extension);

  int nextIndex() const;
  void setNextIndex(int index);

  void setStripPath(bool stripPath);
  bool stripPath() const;

  void setLockFiles(bool lockFiles);
  bool lockFiles() const;

  void setWriteEnabled(bool writeEnabled);
  bool writeEnabled() const;
  
  void setAutoCreateDirectory(bool autoCreateDirectory);
  bool autoCreateDirectory() const;

  void setPixelSize(const QSizeF& pixelSize);
  QSizeF pixelSize() const;

  void setCompression(int compression);
  int compression() const;

  void setMetaFields(const QStringList& metaFields);
  QStringList metaFields() const;

  void setStoreAlpha(bool storeAlpha);
  bool storeAlpha() const;

private:
  void clearKeyValues();
  void processImage();
  void writeKeyValues(QImage* image);
  bool writeImage(QImage* image, const QString& fileName, bool lock);
  template <class T> void writeGrayImage(const PiiVariant& obj, const QString& fileName);
  template <class T> void writeColorImage(const PiiVariant& obj, const QString& fileName);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    QString strOutputDirectory, strNamePrefix, strExtension;
    int iNextIndex;
    bool bStripPath;
    bool bLockFiles;
    bool bWriteEnabled;
    bool bAutoCreateDirectory;
    QSizeF pixelSize;
    int iCompression;
    QStringList lstKeys, lstValues;
    bool bKeyValuesConnected, bNameInputConnected;
    
    PiiInputSocket* pImageInput;
    PiiInputSocket* pNameInput;
    PiiInputSocket* pKeyInput;
    PiiInputSocket* pValueInput;
    PiiOutputSocket* pNameOutput;
    
    PiiVariant imageObject, nameObject;
    QStringList lstMetaFields;
    int iStaticInputCount;
    QList<PiiVariant> lstStaticMeta;
    bool bStoreAlpha;
  };
  PII_D_FUNC;
};

template <class T> bool PiiImageFileWriter::writeImage(const PiiMatrix<T>& matrix, const QString& fileName, bool lock)
{
  return writeImage(Pii::createQImage(matrix), fileName, lock);
}

#endif //_PIIIMAGEFILEWRITER_H
