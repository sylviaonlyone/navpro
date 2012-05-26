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

#ifndef _PIIGENERICSOCKETDESCRIPTOR_H
#define _PIIGENERICSOCKETDESCRIPTOR_H

#include "PiiNetworkGlobal.h"
#include <cstring>

/**
 * A generic socket descriptor that can be used with many native
 * socket descriptor types. It supports at least network sockets
 * (UDP/TCP) and local sockets (unix domain sockets/pipes). More
 * socket descriptors can be added by using the @p pCustomDescriptor
 * member.
 *
 * @ingroup PiiNetwork
 */
struct PII_NETWORK_EXPORT PiiGenericSocketDescriptor
{
  /**
   * Create an invalid socket descriptor.
   */
  PiiGenericSocketDescriptor() : pCustomDescriptor(0) {}
  
  /**
   * Create a new %SocketDescriptor with the given native
   * socketDescriptor. This constructor is suitable for use with
   * network sockets.
   *
   * @see PiiTcpServer
   */
  PiiGenericSocketDescriptor(int socketDescriptor) : iSocketDescriptor(socketDescriptor) {}
  /**
   * Create a new %SocketDescriptor with the given native
   * socketDescriptor. This constructor is suitable for use with
   * local sockets.
   *
   * @see PiiLocalServer
   */
  PiiGenericSocketDescriptor(quintptr socketDescriptor) : pSocketDescriptor(socketDescriptor) {}
  /**
   * Create a new %SocketDescriptor with the given native
   * socketDescriptor. This constructor is suitable for use with any
   * native socket descriptor.
   */
  PiiGenericSocketDescriptor(void* socketDescriptor) : pCustomDescriptor(socketDescriptor) {}
    
  union
  {
    /// A network socket descriptor.
    int iSocketDescriptor;
    /// A local socket descriptor.
    quintptr pSocketDescriptor;
    /// A custom socket descriptor.
    void* pCustomDescriptor;
  };

  inline bool operator== (PiiGenericSocketDescriptor other) { return memcmp(this, &other, sizeof(PiiGenericSocketDescriptor)) == 0; }
};


#endif //_PIIGENERICSOCKETDESCRIPTOR_H
