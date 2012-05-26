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

#ifndef _PIIVIDEOREADER_H
#define _PIIVIDEOREADER_H

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <QString>
#include <PiiMatrix.h>
#include <PiiColor.h>
#include <PiiVideoException.h>

/**
 * An interface for reading videos with avcodec.
 *
 * @code
 * // Typical use with gray-scale videos
 *
 * PiiVideoReader reader("video.avi");
 *
 * try
 *   {
 *    reader.initialize();
 *    PiiMatrix<unsigned char> img(reader.getFrame<unsigned char>());
 *    // Do something with the frame
 *   }
 * catch (PiiVideoException& ex)
 *   {
 *     // Handle exception. 
 *   }
 *
 * // When video contains color 
 *
 * PiiVideoReader reader("mycolorvideo.mpeg");
 *
 * try
 *   {
 *     reader.initialize();
 *     PiiMatrix<PiiColor4<> > img(reader.getFrame<PiiColor4<> >();
 *   }
 * catch (PiiVideoException& ex)
 *   {
 *      // Handle exception.   
 *   }
 * @endcode
 *
 * @ingroup PiiVideoPlugin
 */
class PII_VIDEO_EXPORT PiiVideoReader
{
public:
  /**
   * Create a new instance of %PiiVideoReader that reads video from
   * the given file.
   */
  PiiVideoReader(const QString& filename = "");

  /**
   * Get rid of the reader.
   */
  ~PiiVideoReader();
  
  /**
   * Initialize the reader. If initialization was not succesful,
   * initializate function will throw exception. 
   *
   * @exception PiiVideoException& if initialization fails.
   */
  void initialize() throw(PiiVideoException&);

  /**
   * Encode one frame of the input stream. The template argument @p T
   * determines the output type. Use <tt>unsigned char</tt> to get an
   * 8-bit (gray-scale) frame and PiiColor4<unsigned char> to get a
   * 32-bit RGB frame.
   *
   * @param skipFrames skip this many frames before encoding a frame.
   * -1 seeks the video stream back one frame and essentially
   * re-decodes the previous frame.
   *
   * @return the next video frame in the stream or an empty matrix if
   * an error occurs.
   */
  template <class T> PiiMatrix<T> getFrame(int skipFrames = 0);

  /**
   * Set the file name. This function has no effect after
   * initialize().
   */
  void setFileName(const QString& filename);
  /**
   * Get the file name.
   */
  QString fileName() const;

  /**
   * Seek at begin of the stream.
   */
  void seekToBegin();

  /**
   * Seek at end of the stream.
   */
  void seekToEnd();
  
private:
  /**
   * Reads one frame from video stream.
   *
   * @return true if reading was succesful false if end-of-file or in
   * case of a reading error.
   */ 
  bool getFrame(AVFrame* frame, int skipFrames);

  static QString tr(const char* text) { return QCoreApplication::translate("PiiVideoReader", text); }

  /// @internal
  class Data
  {
  public:
    Data(const QString& fileName);
    ~Data();
    
    // Stores information about video format.
    AVFormatContext* pFormatCtx;  
    // Index to current video stream.
    int iVideoStream;
    // Video codec information
    AVCodecContext* pCodecCtx;
    // Video frame read from the stream. Reused on each iteration.
    AVFrame* pFrame;
    // The time consumed by each frame, in stream units.
    int64_t iFrameTime;
    // Duration of the stream, in stream units.
    int64_t iStreamDuration;
    // Last grabbed frame, in stream units
    int64_t iLastFramePts;
    // The next target frame, in stream units.
    int64_t iTargetPts;
    // The flag which tell if iTargetPts has changed outside of the
    // getFrame()-function (for example seekToBegin() or seekToEnd())
    bool bTargetChanged;
    
    QString strFileName;
  } *d;
};

template <> PiiMatrix<unsigned char> PiiVideoReader::getFrame(int skipFrames);
template <> PiiMatrix<PiiColor4<> > PiiVideoReader::getFrame(int skipFrames);

#endif //_PIIVIDEOREADER_H
