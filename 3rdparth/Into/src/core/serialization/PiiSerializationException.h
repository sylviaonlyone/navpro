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

#ifndef _PIISERIALIZATIONEXCEPTION_H
#define _PIISERIALIZATIONEXCEPTION_H

#include <PiiException.h>
#include "PiiSerializationGlobal.h"

/**
 * @file
 *
 * Macros and declarations for handling serialization errors.
 *
 * @ingroup Serialization
 */

/**
 * Throw a PiiSerializationException with the given error code. This
 * macro is useful mostly in archive implementations.
 *
 * @code
 * PII_SERIALIZATION_ERROR(StreamError);
 * @endcode
 */
#define PII_SERIALIZATION_ERROR(code) throw PiiSerializationException(PiiSerializationException::code, QString(__FILE__ ":%1").arg(__LINE__))

/**
 * Throw a PiiSerializationException with the given error code and
 * extra information. This macro is useful mostly in archive
 * implementations.
 *
 * @code
 * PII_SERIALIZATION_ERROR_INFO(SerializerNotFound, className);
 * @endcode
 */
#define PII_SERIALIZATION_ERROR_INFO(code, info) throw PiiSerializationException(PiiSerializationException::code, info, QString(__FILE__ ":%1").arg(__LINE__))

/**
 * Throw a PiiSerializationException with a custom error message.
 */
#define PII_SERIALIZATION_CUSTOM_ERROR(message) throw PiiSerializationException(message, QString(__FILE__ ":%1").arg(__LINE__))

/**
 * Thrown when an error occurs in (de)serializing data.
 *
 * @ingroup Serialization
 */
class PII_SERIALIZATION_EXPORT PiiSerializationException : public PiiException
{
public:
  /**
   * Error codes for serialization failures.
   *
   * @lip Unknown An unknow error.
   *
   * @lip InvalidDataFormat The data read from an archive is
   * corrupted. This error happens only when the archive notices it is
   * reading something it did not except.
   *
   * @lip UnregisteredClass An input archive does not find a factory
   * for a class name read from an archive.
   *
   * @lip SerializerNotFound An output archive cannot find a
   * serializer for the object to be serialized.
   *
   * @lip ClassVersionMismatch The class version number read from an
   * archive is greater than the current class version number.
   *
   * @lip StreamError The underlying input stream cannot be accessed.
   *
   * @lip StreamNotOpen The underlying input stream is not open.
   *
   * @lip UnrecognizedArchiveFormat The magic key in the beginning of
   * an archive does not match the key of the reading archive.
   *
   * @lip ArchiveVersionMismatch The input archive version is greater
   * than the current version of the archive implementation.
   */
  enum Code
    {
      Unknown = 0,
      InvalidDataFormat,
      UnregisteredClass,
      SerializerNotFound,
      ClassVersionMismatch,
      StreamError,
      StreamNotOpen,
      UnrecognizedArchiveFormat,
      ArchiveVersionMismatch
    };
  
  /**
   * Construct a new PiiSerializationException with the given error
   * code. The exception message will be automatically generated with
   * messageForCode().
   */
  PiiSerializationException(Code code, const QString& location = "");

  /**
   * Construct an @p Unknown exception with the given error message.
   */
  PiiSerializationException(const QString& message, const QString& location = "");

  /**
   * Construct a new PiiSerializationException with the given error
   * code. The exception message will be automatically generated with
   * messageForCode(). The @p info parameter gives extra
   * information such as a class name with @p UnregisteredClass.
   */
  PiiSerializationException(Code code, const QString& info, const QString& location);

  /// Creates a copy of @a other.
  PiiSerializationException(const PiiSerializationException& other);
  
  /**
   * Convert a numerical error code into human-readable text.
   */
  static const char* messageForCode(Code code);

  /**
   * Get the error code.
   */
  Code code() const;

  /**
   * Get additional information. This can be used for any purpose.
   * Currently, @p SerializerNotFound and @p UnregisteredClass store
   * the class name as additional information.
   */
  QString info() const;

private:
  /// @internal
  class Data : public PiiException::Data
  {
  public:
    Data(Code c, const QString& location);
    Data(const QString& message, const QString& location);
    Data(Code c, const QString& info, const QString& location);
    Code code;
    QString strInfo;
  };
  PII_D_FUNC;
};

#endif //_PIISERIALIZATIONEXCEPTION_H
