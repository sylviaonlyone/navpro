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

#ifndef _PIIFIREWIREOPERATION_H
#define _PIIFIREWIREOPERATION_H

#include <PiiMatrix.h>
#include <PiiDefaultOperation.h>
#include "PiiFireWireInterface.h"
#include <PiiColor.h>
#include <QString>
#include <QList>
#include <PiiUtil.h>

/**
 * An operation for reading frames from IEEE1394 cameras. @b NOTE. 
 * This operation is availabe only if a compatible hardware driver is
 * available and supported by your compiler. Currently, this means
 * only MSVC.
 *
 * @inputs
 *
 * @in trigger - an optional trigger input. A new image is emitted
 * whenever any object is received in this input.
 *
 * @outputs
 *
 * @out image - the image output. Emits either four-channel color
 * or grayscale (the default) images in 8-bit (unsigned char) channel
 * format.
 *
 * @ingroup PiiCameraPlugin
 */
class PiiFireWireOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Set an index of the current camera.
   */
  Q_PROPERTY(int index READ camera WRITE setCamera);

  /**
   * Set an image format.
   **/
  Q_PROPERTY(int format READ format WRITE setFormat);

  /**
   * Set a video mode.
   **/
  Q_PROPERTY(int mode READ mode WRITE setMode);

  /**
   * Set a frame rate.
   **/
  Q_PROPERTY(int frameRate READ frameRate WRITE setFrameRate);

  /**
   * Set the partial scan width.
   */
  Q_PROPERTY(int width READ width WRITE setWidth);

  /**
   * Set the partial scan height.
   */
  Q_PROPERTY(int height READ height WRITE setHeight);

  /**
   * Set the partial scan left.
   */
  Q_PROPERTY(int left READ left WRITE setLeft);

  /**
   * Set the partial scan top.
   */
  Q_PROPERTY(int top READ top WRITE setTop);

  /**
   * Get number of cameras.
   **/
  Q_PROPERTY(int nodes READ cameraCount);

  /**
   * Get a vendor of the current camera.
   **/
  Q_PROPERTY(QString vendor READ vendor);

  /**
   * Get a mode of the current camera.
   **/
  Q_PROPERTY(QString model READ model);

  /**
   * Get supported video formats.
   **/
  Q_PROPERTY(QVariantList formatList READ supportedFormats);

  /**
   * Get supported video modes.
   **/
  Q_PROPERTY(QVariantList modeList READ supportedModes);

  /**
   * Get supported frame rates.
   **/
  Q_PROPERTY(QVariantList frameRateList READ supportedFrameRates);

  /**
   * The maximum number of frames the source will emit. This is the
   * upper limit for the number of frames the source will emit. The
   * default value is -1, which means that source will emit frames
   * non-stop.
   */
  Q_PROPERTY(int maxFrames READ maxFrames WRITE setMaxFrames);

  /**
   * The type of the frames. 
   */
  Q_PROPERTY(FrameType frameType READ frameType WRITE setFrameType);
  Q_ENUMS(FrameType);

  /**
   * The index of the next frame to be emitted.
   */
  Q_PROPERTY(int currentFrameIndex READ currentFrameIndex);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Possible frame types.
   *
   * @li @p GrayScale - the output images will be of an 8-bit gray
   * scale type (unsigned char)
   *
   * @li @p Color - the output images will be of a 24-bit RGB color
   * type (PiiColor<unsigned char>).
   */
  enum FrameType { GrayScale, Color };

  enum VideoFormatType { Gray8, Gray16, RGB, YUV, Format7, Other };
  /**
   * Create a fireWireOperation source that reads the current camera.
   */
  PiiFireWireOperation();
  ~PiiFireWireOperation();
  
  int camera() const;
  void setCamera( int index );

  int format() const;
  void setFormat( int format );

  int mode() const;
  void setMode( int mode );
  
  int frameRate() const;
  void setFrameRate( int frameRate );

  int width() const;
  void setWidth( int width );

  int height() const;
  void setHeight( int height );

  int left() const;
  void setLeft( int left );

  int top() const;
  void setTop( int top );
  
  QString vendor() const;
  QString model() const;
  int cameraCount() const;

  QVariantList supportedFormats() const;
  QVariantList supportedModes() const;
  QVariantList supportedFrameRates() const;
  
  int maxFrames() const;
  void setMaxFrames(int cnt);

  FrameType frameType() const;
  void setFrameType(FrameType type);

  int currentFrameIndex() const;

  void check(bool reset);

protected:
  void process();

private slots:
  void stopCapture(int state);

private:
  void initVideoFormatType();
  void setSize();
  void setPosition();
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    
    int                  iMaxFrames, iCurrentIndex, iWidth, iHeight, iLeft, iTop;
    FrameType            frameType;
    VideoFormatType      videoFormatType;
    PiiFireWireInterface *pFwi;
    PiiInputSocket       *pTrigger;
    PiiOutputSocket      *pOutputSocket;
  };
  PII_D_FUNC;
};


#endif //_PIIFIREWIREOPERATION_H
