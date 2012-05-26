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

#ifndef _PIICAMERADRIVER_H
#define _PIICAMERADRIVER_H

#include <QObject>
#include <QSize>
#include "PiiCameraDriverException.h"
#include "PiiCamera.h"
#include <PiiPlugin.h>

/**
 * A common interface for cameras that read images into a frame
 * buffer. Images are stored as @e frames containing at least one @e
 * scan @e line. Valid frame sizes are determined by the type of
 * camera and the driver.
 *
 * Frames are captured by initiating a capture sequence by
 * #startCapture(). Whenever a new frame is ready, the driver invokes
 * the @ref PiiCameraDriver::Listener::frameCaptured()
 * "frameCaptured()" function of a registered listener.
 *
 * PiiCameraDriver is designed to suit the needs detailed in the
 * paragraphs below (and hopefully others).
 *
 * @par On-line video via DMA
 *
 * This type of capture is used by IEEE 1394 (FireWire) cameras and
 * many frame grabbers. The interface card works as a bus master and
 * transfers image data to memory without interrupting the CPU. The
 * term "on-line" refers to a situation in which image frames are
 * requested one by one, and the driver uses a single frame buffer to
 * capture the frames. The driver provides a way to take snapshots of
 * the video whenever needed. It also works in "push" mode in which
 * new frames are written into the frame buffer all the time. The
 * receiving program is signalled after each completed frame.
 *
 * @par High-speed cameras
 *
 * The speed of a camera is said to be high if image data cannot be
 * processed or stored into a hard drive at the original frame rate. 
 * In such situations, the driver allocates a larger frame buffer in
 * which it can store many consequent frames. The buffer can also work
 * in a circular fashion: once it is full, the next frame will be
 * stored at the beginning again.
 *
 * @par Line-scan cameras
 *
 * Line-scan capture is a combination of high-speed and ordinary DMA
 * capture. The driver allocates a circularly accessed frame buffer
 * into which scan-lines are read. Each frame is just one pixel high. 
 * Although the data is usually analyzed in real time, line-scan
 * drivers can also be used in high-speed mode to capture just a
 * predefined number of frames.
 *
 * @par Non-DMA devices
 *
 * Some devices, such as web cameras and some analog frame grabbers,
 * are not capable of DMA transfers. It is up to the camera driver
 * implementation to allocate space for storage and to keep it
 * sufficiently large not to drop frames.
 *
 * @par Accessing frame buffers
 *
 * By default, the memory for image storage is managed by the driver. 
 * With DMA devices, this is often (but not necessarily) the only
 * option. The frame buffer can be accessed either directly or by
 * copying its contents. Direct access is faster, but care must be
 * taken to ensure proper mutual exclusion. To directly access the
 * frame buffer memory, use the #frameBuffer() function.
 *
 * @ingroup PiiCameraPlugin
 */
class PII_CAMERA_EXPORT PiiCameraDriver : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QSize frameSize READ frameSize WRITE setFrameSize);
  Q_PROPERTY(int imageFormat READ imageFormat WRITE setImageFormat);
  Q_PROPERTY(QSize resolution READ resolution);
  Q_PROPERTY(int bitsPerPixel READ bitsPerPixel);
  Q_PROPERTY(int cameraType READ cameraType);
  
public:
  class Listener;

  PiiCameraDriver();
  ~PiiCameraDriver();

  enum TriggerMode
    {
      SoftwareTrigger,
      HardwareTrigger
    };
  
  /**
   * Returns a list of the cameras that can be automatically detected
   * by the driver. Note that this is not necessarily a complete list
   * of accessible cameras. The IDs on the list can be used as the @a
   * cameraId parameter to the #initialize().
   */
  virtual QStringList cameraList() const = 0;

  /**
   * Initializes the driver. This function is called before frames are
   * being captured. The driver implementation may now initialize DMA
   * buffers, connect to a camera etc.
   *
   * The driver may be able to handle many cameras. The active unit is
   * selected by a generic camera id. With frame grabbers, the camera
   * id is typically just a zero-based numerical unit index. Other
   * possibilities include, for example, network addresses. The
   * #cameraList() function returns a list of automatically found
   * camera IDs.
   *   
   * You can configure camera driver via properties, but
   * sometimes you must call initialize(cameraId) again before your
   * configuration value is updated.
   *
   * @exception PiiCameraDriverException& if the initialization could
   * not be performed.
   */
  virtual void initialize(const QString& cameraId) = 0;

  /**
   * Closes an initialized driver. After @p close(), #initialize()
   * must be called again before the driver is functional.
   *
   * @return @p true on success, @p false otherwise
   */
  virtual bool close() = 0;

  /**
   * Starts capturing frames.
   *
   * @param frames the maximum number of frames to capture. If this
   * value is less than one, frames will be captured until explicitly
   * interrupted.
   *
   * @return @p true if the capture was successfully started, @p false
   * otherwise
   */
  virtual bool startCapture(int frames = 0) = 0;

  /**
   * Stops capturing frames.
   *
   * @return @p true if the capture was successfully stopped, @p false
   * otherwise.
   */
  virtual bool stopCapture() = 0;

  /**
   * Returns @p true if the driver is open and @p false otherwise.
   */
  virtual bool isOpen() const = 0;

  /**
   * Returns @p true if frames are currently being captured and @p
   * false otherwise.
   */
  virtual bool isCapturing() const = 0;

  /**
   * Tells to the driver that the next image must be triggered now.
   */
  virtual bool triggerImage() = 0;

  /**
   * Sets the trigger mode.
   */
  virtual bool setTriggerMode(TriggerMode mode) = 0;

  /**
   * Returns the trigger mode.
   */
  virtual TriggerMode triggerMode() const = 0;

  /**
   * Returns frame size.
   */
  virtual QSize frameSize() const = 0;

  /**
   * Returns resolution
   */
  virtual QSize resolution() const = 0;

  /**
   * Returns bits per pixel.
   */
  virtual int bitsPerPixel() const = 0;

  /**
   * Returns image format.
   */
  virtual int imageFormat() const = 0;

  /**
   * Returns camera type. Default implementation returns
   * PiiCamera::AreaScan.
   */
  virtual int cameraType() const;

  /**
   * Set image format. Default implementation does nothing.
   */
  virtual bool setImageFormat(int format) = 0;
  
  /**
   * Sets frame size. Default implementation does nothing.
   */
  virtual bool setFrameSize(const QSize& frameSize) = 0;
  
  /**
   * Returns a pointer to the beginning of a frame buffer.
   *
   * @param frameIndex the index of the frame buffer
   *
   * @return the frame buffer's memory address
   */
  virtual void* frameBuffer(int frameIndex = 0) const = 0;

  /**
   * Sets the listener that handles received image frames.
   */
  virtual void setListener(Listener* listener);

  /**
   * Returns the current listener or 0 if there is no listener.
   */
  virtual Listener* listener() const;

  /**
   * Returns true if the driver must be initialized again when setting
   * the property named as 'name'. The default implementation returns
   * false.
   */
  virtual bool requiresInitialization(const char* name) const;

  /**
   * Returns the reference to the map where is all critical
   * properties.
   */
  QVariantMap& propertyMap();
  
  virtual QVariant property(const char* name);
  virtual bool setProperty(const char* name, const QVariant& value);

private:
  class Data
  {
  public:
    Data();
    Listener *pListener;
    QVariantMap mapProperties;
  } *d;
};

/**
 * A listener that receives image capture events. Drivers never return
 * frames from capture functions. Instead, they use this callback
 * interface to signal that a new frame is available. All functions in
 * this interface have an empty default implementation.
 */
class PiiCameraDriver::Listener
{
public:
  virtual ~Listener();
  
  /**
   * Called whenever when a frame has been captured. The receiver can
   * perform its image processing tasks here while the driver
   * continues capturing frames.
   *
   * @param frameIndex the index of the last captured frame. The frame
   * index is a linear counter that can be used to fetch the frame
   * data from the driver using PiiCameraDriver::frameBuffer().
   *
   * @param frameBuffer a pointer to the captured buffer. If @p
   * frameBuffer is non-zero, a new frame was allocated inside the
   * driver and must be deallocated with @p free() by the caller.
   *
   * @param elapsedTime a time in micro seconds measured from the
   * previous frame. Zero (the default value) means that the driver
   * can't measure the time.
   */
  virtual void frameCaptured(int frameIndex, void *frameBuffer = 0, qint64 elapsedTime = 0);

  /**
   * Called when the driver notices that the receiver cannot process
   * all frames. Depending on the size of the frame buffer the driver
   * may allow the processing lag behind for a while. When it decides
   * the processing will never catch up capture, it'll inform the
   * listener. The next frame sent to #frameCaptured() will be after
   * @p endIndex.
   *
   * @param startIndex the first missed frame
   *
   * @param endIndex the last missed frame
   *
   * @note Missed frames are not necessarily accessible in the driver. 
   * Therefore, it is not allowed to call
   * PiiCameraDriver::frameBuffer() for missed frames. Doing so may
   * crash your application, cause a deadlock or anything else. To
   * avoid trouble, it is best not to access the driver from the
   * implementation of this function. You have been warned.
   */
  virtual void framesMissed(int startIndex, int endIndex);

  /**
   * Called just after the driver has stopped capturing frames.
   */
  virtual void captureFinished();

  /**
   * Called when an error occurs while capturing images.
   *
   * @param message a description of the error
   */
  virtual void captureError(const QString& message);
};

PII_SERIALIZATION_NAME(PiiCameraDriver);

#endif //_PIICAMERADRIVER_H
