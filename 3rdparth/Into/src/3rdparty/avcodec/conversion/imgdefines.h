/* This file is part of ffmpeg-hack.
 * Copyright (C) 2008 Intopii
 * All rights reserved.
 *
 * Created: Sun Nov 16 2008 by Heikki Laitala
 *
 * $Revision:$
 */

#ifndef _IMGDEFINES_H
#define _IMGDEFINES_H


#ifndef MAX_NEG_CROP
#define MAX_NEG_CROP 1024
#endif

//HACK
extern uint32_t HACK_ff_squareTbl[512];

//HACK HACK 
#ifndef bswap_16
static inline uint16_t bswap_16(uint16_t x)
{
    x= (x>>8) | (x<<8);
    return x;
}
#endif

#endif //_IMGDEFINES_H
