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

#ifndef _PIICLASSIFICATIONEXCEPTION_H
#define _PIICLASSIFICATIONEXCEPTION_H

#include <PiiException.h>
#include <PiiClassificationGlobal.h>

/**
 * PiiClassificationException is thrown when errors occur in
 * classification.
 *
 * @ingroup PiiClassificationPlugin
 */
class PII_CLASSIFICATION_EXPORT PiiClassificationException : public PiiException
{
public:
  /**
   * Codes for different exception types.
   *
   * @lip Unknown - unknown error
   *
   * @lip LearningInterrupted - learning was interrupted by user.
   */
  enum Code
    {
      Unknown,
      LearningInterrupted
    };
  
  /**
   * Construct a new PiiClassificationException with the given error
   * code. The exception message will be automatically generated with
   * messageForCode().
   */
  PiiClassificationException(Code code, const QString& location = "");

  /**
   * Construct a @p Unknown exception with the given error message.
   */
  PiiClassificationException(const QString& message, const QString& location = "");
  
  /**
   * Get the cause of the exception.
   */
  Code code() const;

  /**
   * Convert a numerical error code into human-readable text.
   */
  static const char* messageForCode(Code code);
  
private:
  class Data : public PiiException::Data
  {
  public:
    Data(Code code, const QString& location);
    Data(const QString& message, const QString& location);
    Code code;
  };
  PII_D_FUNC;
};


#endif //_PIICLASSIFICATIONEXCEPTION_H
