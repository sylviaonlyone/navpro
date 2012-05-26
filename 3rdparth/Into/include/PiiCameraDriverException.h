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

#ifndef _PIICAMERADRIVEREXCEPTION_H
#define _PIICAMERADRIVEREXCEPTION_H

#include <PiiException.h>
#include "PiiCameraGlobal.h"

/**
 * PiiCameraException is thrown when errors occur in camera operations.
 */
class PII_CAMERA_EXPORT PiiCameraDriverException : public PiiException
{
public:
  /**
   * Construct a new PiiCameraDriverException.
   */
  PiiCameraDriverException(const QString& message = "", const QString& location = "") : PiiException(message, location) {}
};


#endif //_PIICAMERADRIVEREXCEPTION_H
