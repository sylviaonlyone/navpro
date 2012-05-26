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

/* Hacks to make avcodec work with MSVC.
 *
 * The problem: Avcodec uses optimized code (SSE instructions) to
 * perform encoding and decoding. Some instructions assume 16-byte
 * alignment in memory addresses. MSVC aligns stack at 4-byte
 * boundaries (two zero bits in esp register) when calling functions.
 * Therefore, data in the stack may be invalid for SSE, which causes a
 * segfault. For this reason and because of the fact that MSVC does
 * not support the C99 standard, MinGW must be used to compile
 * Avcodec. The problem is that when calling avcodec's functions from
 * MSVC the stack may be off by a few bytes. If the stack is off in
 * the beginning, it will be off at the end.
 *
 * The fix: create wrapper functions that have no compiler-generated
 * prolog or epilog code and adjust the stack to the closest 16-byte
 * boundary before invoking the wrapped function.
 */

#include "avcodec_hacks.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <imgconvert.h>


#define NAKED __declspec(naked)
#define PROLOG _asm { push ebp } _asm { mov ebp,esp } _asm { and esp,~0x0f }
#define EPILOG _asm { mov esp,ebp } _asm { pop ebp } _asm { ret }


NAKED int avcodec_decode_video_msvc_hack(AVCodecContext* c,
                                         AVFrame *frame,
                                         int *frameFinished,
                                         uint8_t *buf,
                                         int bufSize)
{
  PROLOG
  avcodec_decode_video(c, frame,
                       frameFinished,
                       buf, bufSize);
  EPILOG
}

NAKED int av_read_frame_msvc_hack(AVFormatContext *s,
                                  AVPacket *pkt)
{
  PROLOG
  av_read_frame(s, pkt);
  EPILOG
}

NAKED int imgconvert_msvc_hack(AVPicture *dst,
                               int dst_pix_fmt,
                               const AVPicture *src,
                               int src_pix_fmt,
                               int src_width,
                               int src_height)
{
  PROLOG
  imgconvert(dst, dst_pix_fmt, src, src_pix_fmt, src_width, src_height);
  EPILOG
}

#endif
