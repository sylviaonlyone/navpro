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

#include "PiiVideoWriter.h"
#include <PiiMatrix.h>
#include <PiiColor.h>

PiiVideoWriter::Data::Data(const QString& fileName, int width, int height, int frameRate) :
  strFileName(fileName), pFmt(0), pOc(0), iWidth(width), iHeight(height), iFrameRate(frameRate), pPicture(0),
  pVideost(0), dVideopts(0), pVideooutbuf(0), iFramecount(0), iVideooutbufsize(0)
{
}

PiiVideoWriter::PiiVideoWriter(const QString& fileName, int width, int height, int frameRate) :
  d(new Data(fileName, width, height, frameRate))
{
  
}

PiiVideoWriter::~PiiVideoWriter()
{
  // close codec 
  if (d->pVideost != 0)
    close_video(d->pVideost);
   

  // write the trailer, if any
  if(d->pOc != 0)
    av_write_trailer(d->pOc);

  // free the streams
  if(d->pOc != 0)
    for(int i = 0; i < int(d->pOc->nb_streams); ++i)
      av_freep(&d->pOc->streams[i]);

  // close the output file 
  if (!d->pFmt != 0 && !(d->pFmt->flags & AVFMT_NOFILE))
    url_fclose(d->pOc->pb);

  // free the stream
  av_free(d->pOc);
}

void PiiVideoWriter::initialize()
{
  // close codec 
  if (d->pVideost)
    close_video(d->pVideost);
  
  // free the streams

  if(d->pOc != 0)
    for(int i = 0; i < int(d->pOc->nb_streams); ++i)
      av_freep(&d->pOc->streams[i]);

  //close the output file 

  if (d->pFmt != 0 && d->pOc != 0 && !(d->pFmt->flags & AVFMT_NOFILE))
    url_fclose(d->pOc->pb);

  //free the stream

  if(d->pOc != 0)
    av_free(d->pOc);

  
  // Must be called before using avcodec lib 
  avcodec_init();

  // Register all codecs.
  avcodec_register_all();
  
  
  // initialize libavcodec 
  av_register_all();

  // auto detect the output format from the name. default is
  // mpeg. 
  d->pFmt = guess_format(0, d->strFileName.toStdString().c_str(), 0);
  if (d->pFmt == 0)
    {
      qWarning("Could not deduce output format from file extension: using MPEG.\n");
      d->pFmt = guess_format("mpeg", 0, 0);
    }

  if (d->pFmt == 0)
    PII_THROW(PiiVideoException, "Could not find suitable output fromat");

  
  // allocate the output media context 
  if (!allocateMediaContext())
    PII_THROW(PiiVideoException, "Could not allocate media contex");
  
  // add the video stream using the default format codec
  // and initialize the codec 
  if (!initializeCodec())
    PII_THROW(PiiVideoException, "Could not initialize the codec");

  // now that all the parameters are set, we can open video codec
  // and allocate the necessary encode buffer

  open_video(d->pOc, d->pVideost);
      
  // open the output file, if needed
  if (!(d->pFmt->flags & AVFMT_NOFILE))
    if (url_fopen(&d->pOc->pb, d->strFileName.toStdString().c_str(), URL_WRONLY) < 0)
      {
        QString message = QString("Could not open output file %1.").arg(d->strFileName);
        PII_THROW(PiiVideoException, message.toAscii().constData());
      }

    
  // write the stream header, if any 
  
  av_write_header(d->pOc);
    
}

bool PiiVideoWriter::allocateMediaContext()
{
  d->pOc = av_alloc_format_context();
  if (!d->pOc)
    {
      qWarning("Memory error\n");
      return false;
    }
  
  d->pOc->oformat = d->pFmt;
  int len = sizeof(d->pOc->filename)-1;
  if (d->strFileName.size() < len)
    len = d->strFileName.size();
  memcpy(d->pOc->filename, d->strFileName.toStdString().c_str(), len);
  d->pOc->filename[len] = 0;

  return true;
}

bool PiiVideoWriter::initializeCodec()
{
  d->pVideost = 0;
  if (d->pFmt->video_codec != CODEC_ID_NONE)
    d->pVideost = add_video_stream(d->pOc, d->pFmt->video_codec);

  if (!d->pVideost)
    return false;
  
  //set the output parameters (must be done even if no
  //parameters) 
  if (av_set_parameters(d->pOc, 0) < 0)
    {
      qDebug("Invalid output format parameters\n");
      return false;
    }
  
  dump_format(d->pOc, 0, d->strFileName.toStdString().c_str(), 1);

  return true;
}

AVStream* PiiVideoWriter::add_video_stream(AVFormatContext *oc, CodecID codec_id)
{
  AVCodecContext *c = 0;
  AVStream *st = 0;
  
  st = av_new_stream(oc, 0);
  if (!st) {
    qDebug("Could not alloc stream\n");
    return NULL;
  }
  
  c = st->codec;
  c->codec_id = codec_id;
  c->codec_type = CODEC_TYPE_VIDEO;
  /* put sample parameters */
  c->bit_rate = 4000000;
  /* resolution must be a multiple of two */
  c->width = d->iWidth; 
  c->height = d->iHeight;
  c->pix_fmt = PIX_FMT_YUV420P;
  /* PENDING: does this set fps? */
  AVRational rat;
  rat.num = 1;
  rat.den = d->iFrameRate;
  //c->time_base = (AVRational){1,d->iFrameRate};
  c->time_base = rat;
    
  //c->rate_emu = d->iFrameRate;
  //c->frame_rate_base = 1;
  c->gop_size = 12; /* emit one intra frame every twelve frames at most */

  if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
    /* just for testing, we also add B frames */
    c->max_b_frames = 2;
  }

  if (c->codec_id == CODEC_ID_MPEG1VIDEO){
    /* needed to avoid using macroblocks in which some coeffs overflow 
       this doesnt happen with normal video, it just happens here as the 
       motion of the chroma plane doesnt match the luma plane */
    c->mb_decision=2;
  }
  // some formats want stream headers to be seperate
  if(!strcmp(oc->oformat->name, "mp4") || !strcmp(oc->oformat->name, "mov") || !strcmp(oc->oformat->name, "3gp"))
    c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    
  return st;
}

AVFrame* PiiVideoWriter::alloc_picture(PixelFormat pix_fmt, int width, int height)
{
  AVFrame *picture = 0;
  uint8_t *picture_buf = 0;
   
    
  picture = avcodec_alloc_frame();

  if (!picture)
    return 0;


  int size = avpicture_get_size((PixelFormat)pix_fmt, width, height);
  picture_buf = (uint8_t*)malloc(size);

  if (!picture_buf)
    {
      av_free(picture);
      return 0;
    }
    
  avpicture_fill((AVPicture *)picture, picture_buf, 
                 pix_fmt, width, height);

  return picture;
}

void PiiVideoWriter::open_video(AVFormatContext *oc, AVStream *st)
{
  AVCodec *codec = 0;
  AVCodecContext *c = 0;
    
  c = st->codec;

  // find the video encoder 
  codec = avcodec_find_encoder(c->codec_id);

  if (codec == 0)
    PII_THROW(PiiVideoException, "Could not find suitable codec");
    
  // open the codec 
  if (avcodec_open(c, codec) < 0)
    PII_THROW(PiiVideoException,"Could not open codec");

  if(d->pVideooutbuf != 0)
    PII_THROW(PiiVideoException, "Video outbuf was not empty, memory leak detected");

  d->pVideooutbuf = 0;

  if (!(oc->oformat->flags & AVFMT_RAWPICTURE)) {
    /* allocate output buffer */
    /* XXX: API change will be done */
    d->iVideooutbufsize = 200000;
    d->pVideooutbuf = (uint8_t*)malloc(d->iVideooutbufsize);
  }

  // allocate the encoded raw picture 

  d->pPicture = alloc_picture(c->pix_fmt, d->iWidth, d->iHeight);

  if (!d->pPicture)
    PII_THROW(PiiVideoException, "pPicture struct was not correctly allocated");
       
}

/* prepare a dummy image */
void PiiVideoWriter::fill_yuv_image(AVFrame *pict, int frame_index, int width, int height)
{
  int x, y, i;

  i = frame_index;

  /* Y */
  for(y=0;y<height;y++) {
    for(x=0;x<width;x++) {
      pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;
    }
  }
    
  /* Cb and Cr */
  for(y=0;y<height/2;y++) {
    for(x=0;x<width/2;x++) {
      pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
      pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
    }
  }
}

bool PiiVideoWriter::write_video_frame(AVFormatContext *oc, AVStream *st)
{
  int out_size, ret;
  AVCodecContext *c;
  AVFrame *picture_ptr;

  c = st->codec;
  picture_ptr = d->pPicture;
  
  
  if (oc->oformat->flags & AVFMT_RAWPICTURE)
    {
      /* raw video case. The API will change slightly in the near
         futur for that */
      AVPacket pkt;
      av_init_packet(&pkt);
      
      pkt.flags |= PKT_FLAG_KEY;
      pkt.stream_index= st->index;
      pkt.data= (uint8_t *)picture_ptr;
      pkt.size= sizeof(AVPicture);
      
      ret = av_write_frame(oc, &pkt);
    }
  else
    {
      /* encode the image */
      out_size = avcodec_encode_video(c, d->pVideooutbuf, d->iVideooutbufsize, picture_ptr);
      /* if zero size, it means the image was buffered */
      if (out_size != 0)
        {
          AVPacket pkt;
          av_init_packet(&pkt);
          
          pkt.pts= c->coded_frame->pts;
          if(c->coded_frame->key_frame)
            pkt.flags |= PKT_FLAG_KEY;
          pkt.stream_index= st->index;
          pkt.data= d->pVideooutbuf;
          pkt.size= out_size;
          
          /* write the compressed frame in the media file */
          ret = av_write_frame(oc, &pkt);
        }
      else
        {
          ret = 0;
        }
    }
  if (ret != 0)
    {
      qWarning("Error while writing video frame\n");
      return false;
    }
  
  return true;
}

void PiiVideoWriter::close_video(AVStream *st)
{
  avcodec_close(st->codec);
  av_free(d->pPicture->data[0]);
  av_free(d->pPicture);
  av_free(d->pVideooutbuf);
}

bool PiiVideoWriter::saveNextGrayFrame(const PiiMatrix<unsigned char> &matrix)
{
  convertGrayToYUV(matrix);
  if ( !write_video_frame(d->pOc, d->pVideost) )
    return false;

  
  return true;
}

bool PiiVideoWriter::saveNextColorFrame(const PiiMatrix<PiiColor<unsigned char> > &matrix)
{
  convertColorToYUV(matrix);

  if ( !write_video_frame(d->pOc, d->pVideost) )
    return false;

  return true;
}

bool PiiVideoWriter::convertGrayToYUV(const PiiMatrix<unsigned char> &matrix)
{
  for ( int r=0; r<d->iHeight; r++ )
    {
      const unsigned char* ptr = matrix.row(r);
      for ( int c=0; c<d->iWidth; c++)
        d->pPicture->data[0][r*d->pPicture->linesize[0] + c] = (uint8_t)(0.859*ptr[c]);
    }
  
  /* fill U and V with 128 */
  int w = d->iWidth / 2; 
  int h = d->iHeight / 2;
  
  uint8_t *d1 = d->pPicture->data[1];
  uint8_t *d2 = d->pPicture->data[2];
  
  for(int y = 0; y<h; y++)
    {
      memset(d1, 128, w);
      memset(d2, 128, w);
      d1 += w;
      d2 += w;
    }

  return true;
}

bool PiiVideoWriter::convertColorToYUV(const PiiMatrix<PiiColor<unsigned char> > &matrix )
{
  PiiColor<unsigned char> clr;
  for ( int r=0; r<d->iHeight; r++ )
    {
      for ( int c=0; c<d->iWidth; c++)
        {
           clr = matrix(r,c);
           int bb = (int)clr.channels[0];
           int gg = (int)clr.channels[1];
           int rr = (int)clr.channels[2];
           d->pPicture->data[0][r*d->pPicture->linesize[0] + c] = (uint8_t)(0.257*rr + 0.504*gg + 0.098*bb + 16);
        }
    }
  
  /* fill U and V */
  int w = d->iWidth / 2; 
  int h = d->iHeight / 2;
  
  for(int r = 0; r<h; r++)
    {
      for ( int c = 0; c<w; c++ )
        {
          clr = matrix(2*r,2*c);
          int bb = (int)clr.channels[0];
          int gg = (int)clr.channels[1];
          int rr = (int)clr.channels[2];
          d->pPicture->data[1][r*d->pPicture->linesize[1] + c] = (uint8_t)(-0.148*rr - 0.291*gg + 0.439*bb + 128);
          d->pPicture->data[2][r*d->pPicture->linesize[2] + c] = (uint8_t)(0.439*rr - 0.368*gg - 0.071*bb + 128);
          
        }
    }

  return true;

  /*

  Basic equation:
  ****************************************************
  Y  =  Kr *  R'       + (1 - Kr - Kb) * G' + Kb * B'
  Cb = 0.5 * (B' - Y') / (1 - Kb)
  Cr = 0.5 * (R' - Y') / (1 - Kr)
  ****************************************************

  ITU-R BT.601
  ************************************************
  Kb = 0.114
  Kr = 0.299

  Y  =  0.299    * R + 0.587    * G + 0.114    * B 
  Cb = -0.168736 * R - 0.331264 * G + 0.5      * B
  Cr =  0.5      * R - 0.418688 * G - 0.081312 * B
  ************************************************

  
  ITU-R BT.709, the constants are:
  ************************************************
  Kb = 0.0722
  Kr = 0.2126
  ************************************************

  
  SMPTE 240M standard
  ************************************************
  Kb = 0.087 and Kr = 0.212.
  ************************************************


  From somewhere, it works
  ****************************************************
  Y  =  0.257 * R + 0.504 * G + 0.098 * B + 16 
  Cb = -0.148 * R - 0.291 * G + 0.439 * B + 128
  Cr =  0.439 * R - 0.368 * G - 0.071 * B + 128 
  ****************************************************
  
  */
  
}

void PiiVideoWriter::setFileName(const QString& fileName) { d->strFileName = fileName; }
QString PiiVideoWriter::fileName() const { return d->strFileName; }
void PiiVideoWriter::setWidth(int width) { d->iWidth = width; }
int PiiVideoWriter::width() const { return d->iWidth; }
void PiiVideoWriter::setHeight(int height) { d->iHeight = height; }
int PiiVideoWriter::height() const { return d->iHeight; }
void PiiVideoWriter::setSize(int width, int height) { d->iWidth = width; d->iHeight = height; }
void PiiVideoWriter::setFrameRate(int frameRate) { d->iFrameRate = frameRate; }
int PiiVideoWriter::frameRate() const { return d->iFrameRate; }
