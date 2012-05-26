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

#ifndef _PIICAMERAOPERATION_H
#define _PIICAMERAOPERATION_H

#include <PiiImageReaderOperation.h>
#include <PiiCameraDriver.h>
#include <PiiWaitCondition.h>

/**
 * PiiCameraOperation description
 *
 * @ingroup PiiCameraPlugin
 */
class PII_CAMERA_EXPORT PiiCameraOperation : public PiiImageReaderOperation, public PiiCameraDriver::Listener
{
  Q_OBJECT

  /**
   * The name of the camera driver to be used. If the driver is not
   * available, the operation will fail to start.
   */
  Q_PROPERTY(QString driverName READ driverName WRITE setDriverName STORED false);

  /**
   * A list of available camera IDs. Note that not all camera drivers
   * provide a camera list. In such cases, the #cameraId property must
   * be set according to the camera driver. Some drivers provide a
   * list of accessible cameras, but still support other cameras as
   * well.
   */
  Q_PROPERTY(QStringList cameraList READ cameraList);

  /**
   * The ID of the selected camera. A list of selectable cameras can
   * be read from #cameraList. The ID of a camera can be any string. 
   * For example, IP camera drivers use a URL to identify the camera.
   */
  Q_PROPERTY(QString cameraId READ cameraId WRITE setCameraId);

  /**
   * If this property is @p true, the operation will make a deep copy
   * of each captured frame. Otherwise, it is up to the driver how the
   * memory is allocated. This mode is usually faster. However,
   * drivers that use a circular frame buffer, will silently overwrite
   * image data if the frame buffer is not big enough. The default
   * value is @p false.
   */
  Q_PROPERTY(bool copyImage READ copyImage WRITE setCopyImage);

  /**
   * If this property is @p true, the operation doesn't try convert
   * the grabbed image automatically. It only send the raw image
   * forward. The default value is @p false.
   */
  Q_PROPERTY(bool useRawImage READ useRawImage WRITE setUseRawImage);
  
  friend struct PiiSerialization::Accessor;
  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION;
  template <class Archive> void serialize(Archive& archive, const unsigned int)
  {
    //PII_D;
    PII_SERIALIZE_BASE(archive, PiiOperation);
    PiiSerialization::serializeProperties(archive, *this);

    //archive & PII_NVP("driver", d->pCameraDriver);
  }
public:
  PiiCameraOperation();
  ~PiiCameraOperation();
  
  QVariant property(const char* name);
  bool setProperty(const char* name, const QVariant& value);

  void check(bool reset);
  void start();
  void interrupt();
  void pause();
  void stop();
  
  // Listener functions
  void frameCaptured(int frameIndex, void *frameBuffer, qint64 elapsedTime);
  void framesMissed(int startIndex, int endIndex);
  void captureFinished();
  void captureError(const QString& message);
  void setUseRawImage(bool useRawImage);
  
  bool useRawImage() const;
protected:
  void process();

  void setDriverName(const QString& driverName);
  QString driverName() const;
  
  QObject* driver() const;
  QStringList cameraList() const;
  
  void setCameraId(const QString& cameraId);
  QString cameraId() const;

  void setCopyImage(bool copy);
  bool copyImage() const;

  /**
   * Processes an image before delivery. The default implementation
   * returns @a image. Subclasses may add custom functionality by
   * overriding this function.
   */
  virtual PiiVariant processImage(const PiiVariant& image, int frameIndex, qint64 elapsedTime);

public slots:
  /**
   * Loads the camera configuration from @a file. There might be the
   * configuration values which needs to start the processing again.
   * If no driver has been loaded yet, this function does nothing.
   * Returns @p true on success, @p false otherwise.
   */
  bool loadCameraConfig(const QString& file);

  /**
   * Saves the camera configuration into @a file. If no driver has
   * been loaded yet, this function does nothing. Returns @p true on
   * success, @p false otherwise.
   */
  bool saveCameraConfig(const QString& file);

private:
  template <class T> void convert(void *frameBuffer, Pii::PtrOwnership ownership, int frameIndex, qint64 elapsedTime);
  template <class T> void emitImage(const PiiMatrix<T>& image, Pii::PtrOwnership ownership, int frameIndex, qint64 elapsedTime);

protected:
  /// @internal
  class PII_CAMERA_EXPORT Data : public PiiImageReaderOperation::Data
  {
  public:
    Data();
    PiiCameraDriver *pCameraDriver;
    QString strCameraId;
    bool bTriggered;
    PiiWaitCondition waitCondition;
    int iImageWidth;
    int iImageHeight;
    PiiCamera::ImageFormat imageFormat;
    int iBitsPerPixel;
    bool bCopyImage;
    bool bUseRawImage;
  };
  PII_D_FUNC;

  PiiCameraOperation(Data* data);
};


#endif //_PIICAMERAOPERATION_H
