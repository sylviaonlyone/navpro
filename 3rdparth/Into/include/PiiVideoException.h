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

#ifndef _PIIVIDEOEXCEPTION_H
#define _PIIVIDEOEXCEPTION_H

#include <PiiException.h>
#include <PiiVideoGlobal.h>

/**
 * PiiVideoException is thrown when errors occur in video operations.
 */
class PII_VIDEO_EXPORT PiiVideoException : public PiiException
{
public:
  /**
   * Construct a new PiiVideoException.
   */
  PiiVideoException(const QString& message = "", const QString& location = "") : PiiException(message, location) {}
};


#endif //_PIIVIDEOEXCEPTION_H
