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

#ifndef _PIIEXCEPTION_H
#define _PIIEXCEPTION_H

#include <QString>
#include <QCoreApplication>
#include "PiiGlobal.h"

/** @file
 *
 * @ingroup Core
 */

/**
 * Construct an instance of @p EXCEPTION with the given @p MESSAGE.
 * This macro automatically fills in file and line number information
 * in debug builds. Error location will be omitted in release builds.
 */
#ifndef _DEBUG
// Release builds save memory by not storing error location.
#  define PII_MAKE_EXCEPTION(EXCEPTION, MESSAGE) EXCEPTION(MESSAGE)
#else
#  define PII_MAKE_EXCEPTION(EXCEPTION, MESSAGE) EXCEPTION(MESSAGE, QString(__FILE__ ":%1").arg(__LINE__))
#endif

/**
 * A macro for throwing an exception with error location information. 
 * With this macro, the file name and line number of the current code
 * line are automatically stored as the error location. If you don't
 * need the location information, just throw the exception as in throw
 * PiiException("Everything just went kablooie.");. An example:
 *
 * @code
 * PII_THROW(PiiException, tr("On synti‰ k‰‰nt‰‰ suomea englanniksi."));
 * @endcode
 *
 * @param EXCEPTION the class name of the exception to be thrown, e.g. 
 * PiiException.
 *
 * @param MESSAGE the error message
 */
#define PII_THROW(EXCEPTION, MESSAGE) throw PII_MAKE_EXCEPTION(EXCEPTION, MESSAGE)

/**
 * PiiException is the base class of all exceptions.  Usually, one
 * does not throw an PiiException directly but creates a sub-class
 * whose type identifies the exception more precisely.
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiException
{
public:
  /**
   * Constructs an empty exception.
   */
  PiiException();
  
  /**
   * Constructs a new exception with the given @a message.
   *
   * @param message the error message. The message should be a
   * user-readable explation of the error, and it is typically
   * translatable.
   */
  PiiException(const QString& message);
  /**
   * Constructs a new exception with the given @a message and error
   * location.
   *
   * @param message the error message. The message should be a
   * user-readable explation of the error, and it is typically
   * translatable.
   *
   * @param location the location of the code this error occured at. 
   * The standard, official, God-given format is "%file:line", e.g. 
   * "%PiiException.h:30". The reason is that such a string works as a
   * hyperlink to source code when debugging applications with
   * (X)Emacs (which is the standard editor).
   */
  PiiException(const QString& message, const QString& location);

  /**
   * Copy another exception.
   */
  PiiException(const PiiException& other);

  PiiException& operator= (const PiiException& other);

  virtual ~PiiException();

  /**
   * Get the message stored in this exception.
   */
  QString message() const;

  /**
   * Set the message stored in this exception.
   *
   * @param message the new exception message
   */
  void setMessage(const QString& message);

  /**
   * Returns the error location, for example
   * "PiiException.h:106". Note that if you use the @ref PII_THROW
   * macro, location will not be included in release builds.
   */
  QString location() const;

  /**
   * Returns @a prefix + #location() + @a suffix, if location is
   * non-empty. Otherwise returns an empty string.
   */
  QString location(const QString& prefix, const QString& suffix) const;

  /**
   * Set the error location.
   */
  void setLocation(const QString& location);

protected:
  /// @internal
  class PII_CORE_EXPORT Data
  {
  public:
    Data();
    Data(const QString& message, const QString& location);
    virtual ~Data();
    QString strMessage;
    QString strLocation;
  } *d;

  /// @internal
  PiiException(Data* data);
};

/**
 * An exception for situations in which invalid arguments were used in
 * a function or constructor call.
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiInvalidArgumentException : public PiiException
{
public:
  /**
   * Construct a new PiiInvalidArgumentException.
   */
  PiiInvalidArgumentException(const QString& message = "", const QString& location = "");

  /**
   * Copy another exception.
   */
  PiiInvalidArgumentException(const PiiInvalidArgumentException& other);

  PiiInvalidArgumentException& operator= (const PiiInvalidArgumentException& other);
};

#endif //_PIIEXCEPTION_H
