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

#ifndef _PIIEXECUTIONEXCEPTION_H
#define _PIIEXECUTIONEXCEPTION_H

#include <PiiException.h>
#include <QList>
#include <QPair>
#include "PiiYdin.h"

class PiiOperation;

/**
 * Thrown when an operation cannot be started, or an error is detected
 * during execution.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiExecutionException : public PiiException
{
public:
  /**
   * Codes for different exception types.
   *
   * @lip Error - the exception was caused by an error. The operation
   * must be terminated.
   *
   * @lip Finished - the operation that threw the exception finished
   * execution due to end of data (or similar reason) upon reception
   * of a stop tag.
   *
   * @lip Interrupted - the operation was abruptly interrupted by an
   * external signal.
   *
   * @lip Paused - the operation was paused upon reception of a pause
   * tag.
   */
  enum Code { Error, Finished, Interrupted, Paused };
  
  /**
   * Constructs a new @p Error with the given message and location.
   */
  PiiExecutionException(const QString& message = "", const QString& location = "");

  /**
   * Constructs a new PiiExecutionException with the given code.
   */
  PiiExecutionException(Code code);

  /**
   * Constructs a new PiiExecutionException with the given code, error
   * message and location.
   */
  PiiExecutionException(Code code, const QString& message, const QString& location = "");

  /**
   * Copies another exception.
   */
  PiiExecutionException(const PiiExecutionException& other);

  ~PiiExecutionException();
  
  /**
   * Get the cause of the exception.
   */
  Code code() const;

  /**
   * Set the error code.
   */
  void setCode(Code code);

  /**
   * Returns a textual representation of the given @a code.
   */
  static const char* errorName(Code code);

protected:
  /// @cond null
  class Data : public PiiException::Data
  {
  public:
    Data(const QString& message, const QString& location, Code code);
    Data(const QString& message, const QString& location);
    Data(const Data& other);
    Code code;
  };
  PII_D_FUNC;

  PiiExecutionException(Data* d);
  /// @endcond
};

/**
 * Thrown by PiiOperationCompound when errors occur during check(). 
 * This exception combines the possibly many error messages of all
 * child operations.
 *
 * @ingroup PiiYdin
 */
class PII_YDIN_EXPORT PiiCompoundExecutionException : public PiiExecutionException
{
public:
  typedef QList<QPair<PiiOperation*,PiiExecutionException> > ExceptionList;

  PiiCompoundExecutionException(const QString& message = "", const QString& location = "");

  PiiCompoundExecutionException(const PiiCompoundExecutionException& other);
  
  void addException(PiiOperation* operation, const PiiExecutionException& exception);

  ExceptionList exceptions() const;

private:
  /// @internal
  class Data : public PiiExecutionException::Data
  {
  public:
    Data(const QString& message, const QString& location);
    Data(const Data& other);
    ExceptionList lstExceptions;
  };
  PII_D_FUNC;
};

#endif //_PIIEXECUTIONEXCEPTION_H
