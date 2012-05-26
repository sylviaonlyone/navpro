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

#ifndef _PIISEQUENCEFILELISTENER_H
#define _PIISEQUENCEFILELISTENER_H

#include "PiiCameraDriver.h"
#include "PiiSequenceFile.h"

/**
 * A listener for PiiCameraDriver that writes received frames into a
 * sequence file.
 *
 * @ingroup PiiCameraPlugin
 */
class PII_CAMERA_EXPORT PiiSequenceFileListener : public PiiSequenceFile, public PiiCameraDriver::Listener
{
  Q_OBJECT

public:
  /**
   * Create a sequence file. The newly instantiated class will be
   * installed as the listener to @p driver.
   */
  PiiSequenceFileListener(const QString& fileName, PiiCameraDriver* driver);

  /**
   * Open a sequence file. For this to succeed, the camera
   * driver must have been initialized. The function will initialize
   * the image header based on information obtained from the driver.
   */
  bool open(QIODevice::OpenMode mode);
  
  /**
   * Store a frame into the sequence file.
   */
  void frameCaptured(int frameIndex);

private:
  /// @internal
  class Data : public PiiSequenceFile::Data
  {
  public:
    Data(PiiCameraDriver *driver);
    
    PiiCameraDriver* pDriver;
  };
  PII_D_FUNC;
};

#endif //_PIISEQUENCEFILELISTENER_H
