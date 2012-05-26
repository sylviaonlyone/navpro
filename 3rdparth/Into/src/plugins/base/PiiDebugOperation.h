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

#ifndef _PIIDEBUGOPERATION_H
#define _PIIDEBUGOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiOneInputFlowController.h>

/**
 * An operation that prints all objects passing through it to the
 * debug console. It is useful in debugging connections.
 *
 * @inputs
 *
 * @in input - any object
 *
 * @input
 *
 * @out output - the object read from @p input
 *
 * @ingroup PiiBasePlugin
 */
class PiiDebugOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Where to write the debug output? The default is @p StdOut.
   */
  Q_PROPERTY(OutputStream outputStream READ outputStream WRITE setOutputStream);
  Q_ENUMS(OutputStream);

  /**
   * Output format. The format may contain one or more variables. 
   * Recognized variables are:
   *
   * @li $count - the number of objects received since reset
   * @li $type - the type ID of the object in hexadecimal
   * @li $objectName - the objectName of the operation
   * @li $value - the value of the object. The operation recognizes
   * primitive types, strings and matrices.
   * @li $symbol - a one-character symbol for the object. Normal,
   * processable objects are denoted with a dot (.), and
   * synchronization primitives with the following symbols: < = start,
   * > = end, S = stop, P = pause, R = resume.
   *
   * The default format is "$objectName: 0x$type received ($count since
   * reset)\n".
   */
  Q_PROPERTY(QString format READ format WRITE setFormat);

  /**
   * Enables/disables printing of control objects. The default is @p
   * false.
   */
  Q_PROPERTY(bool showControlObjects READ showControlObjects WRITE setShowControlObjects);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Output streams.
   *
   * @lip StdOut - standard output
   * @lip StdErr - standard error
   * @lip Debug - piiDebug() output. Note that in this mode newlines
   * will be automatically appended and you may need to change #format
   * accordingly.
   */
  enum OutputStream { StdOut, StdErr, Debug };
  
  PiiDebugOperation();

  void check(bool reset);

protected:
  void process();

  PiiFlowController* createFlowController();

  void setOutputStream(OutputStream outputStream);
  OutputStream outputStream() const;
  void setFormat(const QString& format);
  QString format() const;
  void setShowControlObjects(bool showControlObjects);
  bool showControlObjects() const;
  
private:
  class Controller : public PiiOneInputFlowController
  {
  public:
    Controller(PiiDebugOperation* parent) :
      PiiOneInputFlowController(parent->inputAt(0), parent->outputSockets()),
      _pParent(parent)
    {}

    FlowState prepareProcess()
    {
      PII_D;
      if (PiiYdin::isControlType(d->pInput->typeAt(1)))
        _pParent->printControlObject(d->pInput->objectAt(1));
      return PiiOneInputFlowController::prepareProcess();
    }
  private:
    PiiDebugOperation* _pParent;
  };

  friend class Controller;

  template <class T> QString printMatrix(const PiiVariant& obj);
  void printControlObject(const PiiVariant& obj);
  void print();

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    int iCnt;
    QString strFormat;
    OutputStream outputStream;
    QMap<QString,QString> mapVariables;
    bool bShowControlObjects;
  };
  PII_D_FUNC;
};

#endif //_PIIDEBUGOPERATION_H
