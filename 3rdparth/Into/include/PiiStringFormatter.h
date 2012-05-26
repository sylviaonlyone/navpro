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

#ifndef _PIISTRINGFORMATTER_H
#define _PIISTRINGFORMATTER_H

#include <PiiDefaultOperation.h>

/**
 * An operation that composes formatted strings out of data read from
 * a configurable number of inputs.
 *
 * @inputs
 *
 * @in inputX - any type convertable to a QString. All primitive types
 * and QString are accepted.
 *
 * @outputs
 *
 * @out output - the output
 *
 * @ingroup PiiBasePlugin
 */
class PiiStringFormatter : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Format string. Use "%0", "%1", etc. to insert the data read from
   * @p input0, @p input1, etc. The default value is "%0". Setting
   * this property automatically changes the number of input sockets
   * to match the number of distinct argument places in the format
   * string. For example "%0 %3 %0" as the format string adds two
   * inputs to the operation. If the number of argument places is
   * zero, one input will still be retained.
   *
   * @code
   * PiiOperation* op = engine.createOperation("PiiStringFormatter");
   * // This will add two inputs to the operation.
   * op->setProperty("format", "%0 (%1)");
   *
   * otherOp->connectOutput("int output", op, "input0");
   * otherOp2->connectOutput("bool output", op, "input1");
   *
   * // output now emits something like
   * // 0 (false)
   * // 1 (true)
   * // 2 (false)
   * @endcode
   */
  Q_PROPERTY(QString format READ format WRITE setFormat);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiStringFormatter();

  void setFormat(const QString& format);
  QString format() const;
  
protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    QString strFormat;
  };
  PII_D_FUNC;
};


#endif //_PIISTRINGFORMATTER_H
