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

#ifndef _PIINETWORKGLOBAL_H
#define _PIINETWORKGLOBAL_H

#include <PiiGlobal.h>

#ifdef PII_BUILD_CORE
#  define PII_BUILD_NETWORK
#  define PII_BUILDING_NETWORK 1
#  define PII_NETWORK_EXPORT PII_DECL_EXPORT
#else
#  define PII_BUILDING_NETWORK 0
#  define PII_NETWORK_EXPORT PII_DECL_IMPORT
#endif

#define PII_TEXT_ARCHIVE_MIMETYPE "application/x-into-txt"
#define PII_BINARY_ARCHIVE_MIMETYPE "application/x-into-bin"
#define PII_CONTENT_NAME_HEADER "Content-Description"

/**
 * @defgroup Network Networking
 *
 * The networking module contains classes for building client/server
 * applications.
 *
 * The most important features in this module is the remote object
 * framework implemented by PiiRemoteObjectServer and
 * PiiRemoteObjectClient. It provides a simple and easy-to-use
 * HTTP-based alternative to QtDBus. The framework doesn't try to be
 * universal and support any programming language (although JavaScript
 * works). That said, the HTTP-based protocol is easy to implement and
 * provides a natural interface to web applications, without the need
 * for external daemons/services. Furthermore, the "native" Qt
 * implementation has the advantage that no stub compilers or adaptors
 * are needed.
 *
 * The networking module also provides PiiNetworkServer, a generic
 * multi-threaded server for network applications. It is used to
 * implement @ref PiiHttpServer "a multi-threaded web server". HTTP
 * clients and servers can be implemented easily with the aid of
 * PiiHttpDevice. PiiMultipartDecoder makes it easy to parse
 * multi-part MIME messages such as form submissions.
 */

/**
 * Definitions and functions used commonly by networking classes.
 *
 * @ingroup PiiNetwork
 */
namespace PiiNetwork
{
  enum StopMode { InterruptClients, WaitClients };

  /**
   * Returns "application/x-www-form-urlencoded".
   */
  PII_NETWORK_EXPORT const char* formContentType();
}

#endif //_PIINETWORKGLOBAL_H
