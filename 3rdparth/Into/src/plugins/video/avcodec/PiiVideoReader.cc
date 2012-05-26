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

#include "PiiVideoReader.h"
#include <PiiFraction.h>
#include "avcodec_hacks.h"
#include <imgconvert.h>

PiiVideoReader::Data::Data(const QString& fileName) :
  pFormatCtx(0),
  iVideoStream(-1),
  pCodecCtx(0),
  pFrame(0),
  iFrameTime(0),
  iStreamDuration(0),
  iLastFramePts(0),
  iTargetPts(0),
  bTargetChanged(false),
  strFileName(fileName)
{
}

PiiVideoReader::Data::~Data()
{
  // Free the decoded frame
  if (pFrame != 0)
    av_free(pFrame);

  // Close the codec
  if (pCodecCtx != 0)
    avcodec_close(pCodecCtx);

  // Close the video file
  if (pFormatCtx != 0)
    av_close_input_file(pFormatCtx);
}

PiiVideoReader::PiiVideoReader(const QString& filename) :
  d(new Data(filename))
{
}

PiiVideoReader::~PiiVideoReader()
{
  delete d;
}

void PiiVideoReader::setFileName(const QString& filename)
{
  d->strFileName = filename;
}

QString PiiVideoReader::fileName() const
{
  return d->strFileName;
}

void PiiVideoReader::initialize() throw(PiiVideoException&)
{
  // Free frame. 
  if (d->pFrame != 0)
    av_free(d->pFrame);

  // Close the codec
  if (d->pCodecCtx != 0)
    avcodec_close(d->pCodecCtx);
  
  // Close the video file
  if (d->pFormatCtx != 0)
    av_close_input_file(d->pFormatCtx);
  
  // Must be called before using avcodec lib 
  avcodec_init();

  // Register all codecs.
  avcodec_register_all();
  
  av_register_all();

  // The last three arguments are used to specify the file format,
  // buffer size, and format options, but by setting this to 0,
  // libavformat will try auto-detect these.
  
  if (av_open_input_file(&d->pFormatCtx, d->strFileName.toLocal8Bit().constData(), 0, 0, 0)!=0)
    PII_THROW(PiiVideoException, tr("Couldn't open file"));
  
  // This fills the streams field of the AVFormatContext with valid
  // information.
  
  if (av_find_stream_info(d->pFormatCtx)<0)
    PII_THROW(PiiVideoException, tr("Couldn't find stream information"));

  // Dump information about file onto standard error
  //dump_format(d->pFormatCtx, 0, d->strFileName.toLocal8Bit().constData(), false);

  // Find the first video stream
  for (int i=0; i<int(d->pFormatCtx->nb_streams); ++i)
    if (d->pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
      {
        d->iVideoStream = i;
        break;
      }
  if (d->iVideoStream == -1)
    PII_THROW(PiiVideoException, tr("Cannot find a video stream."));

  // Get a pointer to the codec context for the video stream
  d->pCodecCtx = d->pFormatCtx->streams[d->iVideoStream]->codec;

  // OK, so now we've got a pointer to the so-called codec context for our video
  // stream, but we still have to find the actual codec and open it.
  
  // Find the decoder for the video stream
  AVCodec* pCodec = avcodec_find_decoder(d->pCodecCtx->codec_id);

  if (pCodec==0)
    PII_THROW(PiiVideoException, tr("Codec not found."));
  
  // Inform the codec that we can handle truncated bitstreams -- i.e.,
  // bitstreams where frame boundaries can fall in the middle of packets
  if (pCodec->capabilities & CODEC_CAP_TRUNCATED)
    d->pCodecCtx->flags |= CODEC_FLAG_TRUNCATED;

  // Open codec
  if (avcodec_open(d->pCodecCtx, pCodec) < 0)
    PII_THROW(PiiVideoException, tr("Couldn't open codec."));

  AVStream *pStream = d->pFormatCtx->streams[d->iVideoStream];
  
  // time_base stores the fps of the stream as a fraction
  PiiFraction<qint64>
    frameTime(d->pCodecCtx->time_base.num,
              d->pCodecCtx->time_base.den),
    streamTimeBase(pStream->time_base.den,
                   pStream->time_base.num);
  PiiFraction<qint64> temp = frameTime * streamTimeBase;
  /*qDebug(" d->pCodecCtx->time_base.num = %i", (int)d->pCodecCtx->time_base.num);
    qDebug(" d->pCodecCtx->time_base.den = %i", (int)d->pCodecCtx->time_base.den);
    qDebug(" pStream->time_base.num = %i", (int)pStream->time_base.num);
    qDebug(" pStream->time_base.den = %i", (int)pStream->time_base.den);
    qDebug(" frameTime.numerator = %i", (int)frameTime.numerator);
    qDebug(" frameTime.denominator = %i", (int)frameTime.denominator);
    qDebug(" streamTimeBase.numerator = %i", (int)streamTimeBase.numerator);
    qDebug(" streamTimeBase.denominator = %i", (int)streamTimeBase.denominator);
    qDebug(" temp.numerator = %i", (int)temp.numerator);
    qDebug(" temp.denominator = %i", (int)temp.denominator);
  */

  // Calculate and initialize variables
  d->iFrameTime = int64_t(double(frameTime * streamTimeBase) + 0.5);
  d->iStreamDuration = d->pFormatCtx->duration / pStream->time_base.num;
  d->iLastFramePts = 0;
  d->iTargetPts = 0;
  d->bTargetChanged = false;
  
  // Allocate a video frame
  d->pFrame = avcodec_alloc_frame();
  
  /*qDebug("PiiVideoReader::initialize()\n"
         "  Frame size: %d x %d\n"
         "  Data:       %p %p %p %p\n"
         "  Linesizes:  %d %d %d %d",
         d->pCodecCtx->width, d->pCodecCtx->height,
         d->pFrame->data[0], d->pFrame->data[1], d->pFrame->data[2], d->pFrame->data[3],
         d->pFrame->linesize[0], d->pFrame->linesize[1],
         d->pFrame->linesize[2], d->pFrame->linesize[3]);
  */
}

bool PiiVideoReader::getFrame(AVFrame *frame, int frameStep = 1)
{
  AVPacket packet;
  packet.data = 0;

  bool bSeeked = false;

  /**
   * If the target of the next frame has changed OR frameStep != 1, we
   * must to seek the stream.
   */
  if (d->bTargetChanged || frameStep != 1)
    {
      // If the target of the next frame has not changed, we will
      // calculate a new target depends on frameStep.
      if (!d->bTargetChanged)
        d->iTargetPts += frameStep * d->iFrameTime;
      
      d->bTargetChanged = false;
      bSeeked = true;


      // Seek the video stream to the next target
      if (av_seek_frame(d->pFormatCtx, d->iVideoStream, d->iTargetPts, frameStep < 0 ? AVSEEK_FLAG_BACKWARD : 0) < 0)
        return false;

      d->pCodecCtx->skip_frame = AVDISCARD_BIDIR;
    }
  else
    {
      d->pCodecCtx->skip_frame = AVDISCARD_DEFAULT;
      d->iTargetPts = d->iLastFramePts + d->iFrameTime;
    }
  
  while (AV_READ_FRAME(d->pFormatCtx, &packet) >= 0)
    {
      // Is this a packet from the video stream?
      if (packet.stream_index == d->iVideoStream)
        {
          int iFrameFinished = 0;
          
          // Decode video frame
          if (AVCODEC_DECODE_VIDEO(d->pCodecCtx, frame,
                                   &iFrameFinished,
                                   packet.data, packet.size) < 0)
            {
              av_free_packet(&packet);
              return false;
            }
          if (iFrameFinished)
            {
              // Full video frame received. Store the presentation
              // time stamp of the packet as the last decoded frame
              // time (global stream pos).
              d->iLastFramePts = packet.pts;
              
              // If we weren't seeking, return now. Otherwise continue
              // until we hit the correct position.
              if (!bSeeked || d->iLastFramePts >= d->iTargetPts)
                {
                  av_free_packet(&packet);
                  return true;
                }
            }
        }
      av_free_packet(&packet);
    }
  
  return false;
}

template <> PiiMatrix<unsigned char> PiiVideoReader::getFrame(int frameStep)
{
  bool bSuccess = getFrame(d->pFrame, frameStep);
  /*qDebug("Frame size: %d x %d\n"
         "  Data:       %p %p %p %p\n"
         "  Linesizes:  %d %d %d %d\n"
         "  Padding:    %d",
         d->pCodecCtx->width, d->pCodecCtx->height,
         d->pFrame->data[0], d->pFrame->data[1], d->pFrame->data[2], d->pFrame->data[3],
         d->pFrame->linesize[0], d->pFrame->linesize[1], d->pFrame->linesize[2], d->pFrame->linesize[3],
         d->pFrame->linesize[0] - d->pCodecCtx->width);
  */
  if (bSuccess)
    {
      PiiMatrix<unsigned char> matResult(d->pCodecCtx->height,
                                         d->pCodecCtx->width,
                                         static_cast<unsigned char*>(d->pFrame->data[0]),
                                         d->pFrame->linesize[0]);
      return matResult;
    }
 
  return PiiMatrix<unsigned char>();
}

template <> PiiMatrix<PiiColor4<> > PiiVideoReader::getFrame(int frameStep)
{
  bool bSuccess = getFrame(d->pFrame, frameStep);
  /*qDebug("Frame size: %d x %d\n"
         "Data:       %p %p %p %p\n"
         "Linesizes:  %d %d %d %d",
         d->pCodecCtx->width, d->pCodecCtx->height,
         d->pFrame->data[0], d->pFrame->data[1], d->pFrame->data[2], d->pFrame->data[3],
         d->pFrame->linesize[0], d->pFrame->linesize[1],
         d->pFrame->linesize[2], d->pFrame->linesize[3]);
  */
  
  if (bSuccess)
    {
      // Allocate an AVFrame structure for conversion result
      AVFrame *pResultFrame = avcodec_alloc_frame();
      if (pResultFrame == 0)
        return PiiMatrix<PiiColor4<> >();

      // Malloc ensures we can safely leave the buffer to PiiMatrix.
      void* bfr = ::malloc(avpicture_get_size(PIX_FMT_RGB32, d->pCodecCtx->width, d->pCodecCtx->height));
      
      if (bfr == 0)
        {
          av_free(pResultFrame);
          return PiiMatrix<PiiColor4<> >();
        }

      // Assign appropriate parts of buffer to image planes in pResultFrame
      avpicture_fill((AVPicture *)pResultFrame, (uint8_t*)bfr, PIX_FMT_RGB32,
                     d->pCodecCtx->width, d->pCodecCtx->height);

      // Convert color space (this stores the result into bfr)
      int iResult = IMGCONVERT((AVPicture *)pResultFrame, PIX_FMT_RGB32, (AVPicture*)d->pFrame,
                               d->pCodecCtx->pix_fmt, d->pCodecCtx->width, d->pCodecCtx->height);

      // Get rid of the conversion result frame. This does not free
      // the data buffer itself.
      av_free(pResultFrame);

      if (iResult < 0)
        {
          free(bfr);
          return PiiMatrix<PiiColor4<> >();
        }

      // Let PiiMatrix take the ownership of the buffer.
      return PiiMatrix<PiiColor4<> >(d->pCodecCtx->height, d->pCodecCtx->width, bfr, Pii::ReleaseOwnership, 0);
    }
  return PiiMatrix<PiiColor4<> >();
}

void PiiVideoReader::seekToBegin()
{
  // Initialize the next target to the start of the stream and switch
  // bTargetChanged flag on.
  d->iTargetPts = 0;
  d->bTargetChanged = true;
}

void PiiVideoReader::seekToEnd()
{
  // If we don't know a stream duration, we must find it to search the
  // latest frame of the stream.
  if (d->iStreamDuration <= 0)
    {
      d->iStreamDuration = 0;
      while(av_seek_frame(d->pFormatCtx, d->iVideoStream, d->iStreamDuration, 0) >= 0)
        {
          d->iStreamDuration += d->iFrameTime;
        }
    }

  // Initialize the next target to the end of the stream and switch
  // bTargetChanged flag on
  d->iTargetPts = d->iStreamDuration - d->iFrameTime;
  d->bTargetChanged = true;
}
  

