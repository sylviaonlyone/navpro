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

#ifndef _PIISEQUENCEFILE_H
#define _PIISEQUENCEFILE_H

#include <QFile>
#include "PiiCameraDriver.h"
#include "PiiCamera.h"

/**
 * Stores and restores captured frames to/from a sequence file. The
 * sequence file contains raw image data, preceded by a short header
 * defined in PiiSequenceFile::Header.
 *
 * @ingroup PiiCameraPlugin
 */
class PII_CAMERA_EXPORT PiiSequenceFile : public QFile
{
public:
  /**
   * Sequence file header. Note that all integers are in little-endian
   * format (LSB first). When the structure is written into a file,
   * the first byte will be 0x27.
   */
  struct Header
  {
    Header() :
      magic(0x31415927),
      width(0), height(0),
      bitsPerPixel(8),
      imageFormat(PiiCamera::MonoFormat),
      frameCount(-1)
    {}
    
    /**
     * Magic number: 0x31415927.
     */
    unsigned int magic;
    /**
     * Frame width.
     */
    unsigned int width;
    /**
     * Frame height.
     */
    unsigned int height;
    /**
     * Number of bits per pixel. Only one byte is used for storing the
     * value. The other three are reserved for future use.
     */
    unsigned int bitsPerPixel;
    /**
     * Pixel format.
     */
    PiiCamera::ImageFormat imageFormat;
    /**
     * The number of frames in the sequence. -1 means unknown.
     */
    int frameCount;
  };

  /**
   * Write a frame into the sequence file. @p frameBuffer points to
   * the beginning of a raw frame buffer.
   * <tt>width*height*bytesPerPixel</tt> bytes will be written into
   * the file.
   */
  bool writeFrame(const void* frameBuffer);

  /**
   * Read a frame from the sequence file. Image data will be stored to
   * the memory buffer pointed by @p frameBuffer, which must be
   * allocated to hold at least <tt>width*height*bytesPerPixel</tt>
   * bytes.
   *
   * @return true if reading was successful, false otherwise
   */
  bool readFrame(void* frameBuffer);

  /**
   * Open a sequence file. If the file is opened for writing, the
   * header will be written to it. For this to succeed, the camera
   * driver must have been initialized. If the file is opened for
   * reading, the header will be read. The header is accessible with
   * the #header() function.
   */
  bool open(QIODevice::OpenMode mode);

  /**
   * Seek to the given frame index.
   * 
   * @param frameIndex seek to the beginning of this frame.
   * 
   * @return true if seek was successful, false otherwise.
   */
  bool seekFrame(int frameIndex);

  /**
   * Close the sequence file. This will rewrite the header to the
   * beginning of the file. Therefore, any changes you make to the
   * header will be stored.
   */
  void close(); 

  /**
   * Get image format information. If the sequence is opened for
   * reading, the returned value contains the header read from the
   * file. If the sequence is opened for writing, the header is
   * constructed from the current image settings.
   */
  Header header() const;

  /**
   * Set the size of stored frames. The value will be stored to the
   * file header and must be correct.
   */
  void setFrameSize(QSize frameSize);

  /**
   * Get the size of stored frames. The same information can be
   * retrieved via #header().
   */
  QSize frameSize() const;

  /**
   * Set the number of bits per pixel.
   */
  void setBitsPerPixel(unsigned int bitsPerPixel);
  /**
   * Get the number of bits per pixel.
   */
  unsigned int bitsPerPixel() const;

  /**
   * Set image format.
   */
  void setImageFormat(PiiCamera::ImageFormat format);
  /**
   * Get image format.
   */
  PiiCamera::ImageFormat imageFormat() const;

  /**
   * Set the number of frames in the sequence. The file automatically
   * changes this value, and there should be no need to alter it.
   */
  void setFrameCount(int frameCount);

  /**
   * Get the number of frames in the sequence. -1 means unknown.
   */
  int frameCount() const;

  /**
   * Get the number of bytes each frame occupies. The buffers used
   * with #readFrame() and #writeFrame() must have a capacity of at
   * least this many bytes.
   */
  int bytesPerFrame() const;
  
private:
  void calculateFrameSize();

protected:
  /// @internal
  class Data
  {
  public:
    Data();
    virtual ~Data();

    Header header;
    int iBytesPerFrame;
  } *d;

  /**
   * Create a %PiiSequenceFile that writes/reads frames to/from the
   * given file.
   */
  PiiSequenceFile(Data *data, const QString& fileName);
  PiiSequenceFile(const QString& fileName);
  virtual ~PiiSequenceFile();
};

#endif //_PIISEQUENCEFILE_H
