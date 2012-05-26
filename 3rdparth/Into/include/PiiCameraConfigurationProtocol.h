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

#ifndef _PIICAMERACONFIGURATIONPROTOCOL_H
#define _PIICAMERACONFIGURATIONPROTOCOL_H

#include <PiiDefaultConfigurable.h>
#include <QObject>
#include <QIODevice>

class PiiMetaConfigurationValue;

/**
 * TODO: REVIEW THIS COMMENT
 * This is a base class for handling the serial communication to/from
 * cameras. It inherits both from @p QObject and @p
 * PiiDefaultConfigurable. The class contains general purpose
 * interface for setting the commands for the camera. So the
 * programmer don't have to send serial ASCII commands for setting up
 * the parameters for the camera, but just call the functions of this
 * interface. The classed inherited from this class must implement the
 * camera specific protocol.
 */
class PiiCameraConfigurationProtocol : public QObject, public PiiDefaultConfigurable
{
  Q_OBJECT

public:

  PiiCameraConfigurationProtocol();

  /**
   * Sets the serial device for the protocol. The device must be set,
   * before the configuration values can be read or written.
   */
  void setDevice(QIODevice *device);

  QIODevice* device() const;

private:
  QIODevice* _pDevice;
};

#endif //_PIICAMERACONFIGURATIONPROTOCOL_H
