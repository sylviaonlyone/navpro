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

#ifndef _PIIVIDEOWRITER_H
#define _PIIVIDEOWRITER_H

#include <PiiMatrix.h>
#include <PiiColor.h>
#include <PiiVideoException.h>

//HACK Normally ffmpeg is compiled with C compiler not C++. 
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


/**
 * An interface for writing video files with avcodec.
 *
 * @ingroup PiiVideoPlugin 
 */
class PII_VIDEO_EXPORT PiiVideoWriter
{
public:
  
  PiiVideoWriter(const QString& fileName, int width = 100, int height = 100, int frameRate = 25);
  ~PiiVideoWriter();

  /**
   * Initialize the writer. This function must be called before frames are
   * being written into video. The implementation initializes
   * libavcodec. 
   *
   * @exception PiiVideoException& if the initialization could
   * not be performed.
   */
  void initialize();

  bool saveNextGrayFrame(const PiiMatrix<unsigned char> &matrix );
  bool saveNextColorFrame(const PiiMatrix<PiiColor<unsigned char> > &matrix );
  
  void setFileName(const QString& fileName );
  QString fileName() const;

  void setWidth( int width );
  int width() const;
  
  void setHeight( int height );
  int height() const;
  
  void setSize(int width, int height);

  void setFrameRate( int frameRate );
  int frameRate() const;

  
protected: 
  bool allocateMediaContext();
  bool initializeCodec();
  
  bool convertGrayToYUV(const PiiMatrix<unsigned char> &matrix);
  bool convertColorToYUV(const PiiMatrix<PiiColor<unsigned char> > &matrix);
  
  
private:
  void open_video(AVFormatContext *oc, AVStream *st);
  void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height);
  bool write_video_frame(AVFormatContext *oc, AVStream *st);
  void close_video(AVStream *st);

  /**
   * Add a video output stream.
   */
  AVStream* add_video_stream(AVFormatContext *oc, CodecID codec_id);
  AVFrame* alloc_picture(PixelFormat pix_fmt, int width, int height);
  
  /// @internal
  class Data
  {
  public:
    Data(const QString& fileName, int width, int height, int frameRate);
    
    QString         strFileName;
    AVOutputFormat  *pFmt;
    AVFormatContext *pOc;
    int             iWidth, iHeight, iFrameRate;
    AVFrame         *pPicture;
    AVStream        *pVideost; //!!!!!!!!!!!!
    double          dVideopts;//!!!!!!!!!!!!
    uint8_t         *pVideooutbuf; //!!!!!!!!!!!!!
    int             iFramecount, iVideooutbufsize; //!!!!!!!
  } *d;

};

#endif //_PIIVIDEOWRITER_H
