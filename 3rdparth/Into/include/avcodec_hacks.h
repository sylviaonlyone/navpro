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

#ifdef _MSC_VER

#define AVCODEC_DECODE_VIDEO avcodec_decode_video_msvc_hack
#define AV_READ_FRAME av_read_frame_msvc_hack
#define IMGCONVERT imgconvert_msvc_hack

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int avcodec_decode_video_msvc_hack(AVCodecContext* c,
                                   AVFrame *frame,
                                   int *frameFinished,
                                   uint8_t *buf,
                                   int bufSize);

int av_read_frame_msvc_hack(AVFormatContext *s,
                            AVPacket *pkt);


int imgconvert_msvc_hack(AVPicture *dst,
                         int dst_pix_fmt,
                         const AVPicture *src,
                         int src_pix_fmt,
                         int src_width,
                         int src_height);

#else

#define AVCODEC_DECODE_VIDEO avcodec_decode_video
#define AV_READ_FRAME av_read_frame
#define IMGCONVERT imgconvert

#endif
