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

#ifndef _PIIRESOURCECONNECTOR_H
#define _PIIRESOURCECONNECTOR_H

#include "PiiYdin.h"
#include <PiiSerializationTraits.h>

/**
 * An interface for classes that connect resource instances together. 
 * A "resource" can be an operation, a widget, or a distance measure,
 * to name a few examples. To establish a connection between two
 * resources one needs to first instantiate them and the connector. 
 * Then, pointers to the two resources are passed to the connector to
 * bind the instances together.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiResourceConnector
{
public:
  virtual ~PiiResourceConnector();

  /**
   * Connect two resources. There are different flavors of
   * "connection" including, for example, Qt's signal-slot connections
   * and socket connections between PiiOperations. Each connector is
   * specific to a pair of resources. Thus, it knows how to cast the
   * provided @p void pointers to the correct type.
   *
   * @return @p true if the resources were successfully connected, @p
   * false otherwise.
   */
  virtual bool connectResources(void* resource1, void* resource2, const QString& role) const = 0;
  /**
   * Disconnect resources. If the resources are not connected, this
   * function does nothing.
   *
   * @return @p true if the resources were successfully disconnected,
   * @p false otherwise.
   */
  virtual bool disconnectResources(void* resource1, void* resource2, const QString& role) const = 0;
};
PII_SERIALIZATION_NAME(PiiResourceConnector);
PII_SERIALIZATION_ABSTRACT(PiiResourceConnector);

#endif //_PIIRESOURCECONNECTOR_H
