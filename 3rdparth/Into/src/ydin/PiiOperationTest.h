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

#ifndef _PIIOPERATIONTEST_H
#define _PIIOPERATIONTEST_H

#include <QObject>
#include <PiiYdin.h>
#include <PiiVariant.h>
#include <QMap>
#include "PiiOperation.h"
#include "PiiYdinTypes.h"

class PiiProbeInput;

/**
 * A class that makes it easier to write unit tests for operations. 
 * %PiiOperationTest attaches @ref PiiProbeInput "probes" to the
 * outputs of an operation to be tested. A derived class can then
 * selectively connect inputs of the operation and send arbitrary data
 * to the input sockets.
 *
 * A typical unit test for an operation is as follows:
 *
 * @code
 * // In TestMyOperation.h
 * #include <PiiOperationTest.h>
 *
 * class TestMyOperation : public PiiOperationTest
 * {
 *   Q_OBJECT
 *
 * private slots:
 *   void initTestCase();
 *   void process();
 * };
 *
 * // In main.cc
 * #include "TestMyOperation.h"
 * #include <QtTest>
 * 
 * void TestMyOperation::initTestCase()
 * {
 *   QVERIFY(createOperation("myplugin", "TestMyOperation"));
 * }
 *
 * void TestMyOperation::process()
 * {
 *   // No inputs connected -> should fail
 *   QVERIFY(start(ExpectFail));
 *   // Set properties
 *   QVERIFY(operation()->setProperty("property", "value"));
 *   // Connect inputs
 *   connectAllInputs();
 *   // Try to start again
 *   QVERIFY(start());
 *   // Send object(s)
 *   QVERIFY(sendObject("input", 1));
 *   // Verify outputs
 *   QCOMPARE(outputValue("output", 0), 1);
 * }
 * 
 * QTEST_MAIN(TestMyOperation)
 * @endcode
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiOperationTest : public QObject
{
  Q_OBJECT

public:
  ~PiiOperationTest();
  
protected slots:
  /**
   * Clears all collected output values by calling
   * clearAllOutputValues(). This function will be called by Qt's test
   * system after each test function.
   */
  virtual void cleanup();

protected:
  /**
   * Failure handling types in #start().
   */
  enum FailMode { ExpectSuccess, ExpectFail };

  PiiOperationTest();
  
  /**
   * Creates the operation to be tested.
   *
   * @param plugin the name of the plug-in that contains the
   * operation
   *
   * @param operation the name of the operation to test
   *
   * @return @p true if the plug-in was successfully loaded and the
   * operation was successfully created, @p false otherwise
   */
  bool createOperation(const char* plugin, const char* operation);

  /**
   * Sets the operatito be tested. This function can be used if the
   * operation is not in a plug-in.
   */
  void setOperation(PiiOperation* operation);

  /**
   * Returns a pointer to the operation being tested.
   */
  PiiOperation* operation() const;

  /**
   * Starts the tester. If the @p mode flag is @p ExpectSuccess, this
   * function ensures that the tester was successfully started. If @p
   * mode is @p ExpectFail, it ensures that the startup fails. @p
   * ExpectFail is useful when one needs to check that the operation
   * correctly identifies unconnected inputs, missing configuration
   * etc. One needs to stop the tester (stop()) once the
   * test is finished.
   *
   * @return @p true if the @p mode == ExpectSuccess and the operation
   * was successfully started, or if @p mode == ExpectFail and the
   * start-up fails, and @p false otherwise.
   */
  bool start(FailMode mode = ExpectSuccess);
  
  /**
   * Makes the named input connected. This function connects the named
   * input into an output socket that is not attached to any
   * operation.
   */
  bool connectInput(const QString& name);

  /**
   * Makes all inputs of the internal operation connected.
   */
  void connectAllInputs();

  /**
   * Disconnects the named input.
   */
  void disconnectInput(const QString& name);

  /**
   * Disconnects all input sockets in the internal operation.
   */
  void disconnectAllInputs();

  /**
   * Sends an object to the named input.
   *
   * @param name the name of the input socket to which the object is
   * to be sent
   *
   * @param value the object to be sent
   *
   * @return @p true if the object was successfully sent, @p false
   * otherwise
   */
  bool sendObject(const QString& name, const PiiVariant& value);

  /**
   * Sends an object to the named input. The function automatically
   * converts the object to a PiiVariant.
   *
   * @param name the name of the input socket to which the object is
   * to be sent
   *
   * @param value the value of the object
   *
   * @return @p true if the object was successfully sent, @p false
   * otherwise
   */
  template <class T> bool sendObject(const QString& name, T value)
  {
    return sendObject(name, PiiVariant(value));
  }

  /**
   * @overload
   *
   * Automatically converts const char* to QString.
   */
  bool sendObject(const QString& name, const char* value);
  
  /**
   * Stops the execution of the operation. This function calls @ref
   * PiiOperation::interrupt() on the internal operation. Returns @p
   * true if the operation finished within a second, @p false
   * otherwise.
   */
  bool stop();
  
  /**
   * Returns the names of outputs that have received output.
   */
  QStringList outputsWithData() const;

  /**
   * Returns the last received value in the named output socket.
   */
  PiiVariant outputValue(const QString& name) const;

  /**
   * Checks that the named output has an object and that its type
   * matches @p T. If both of these hold, return the value of the
   * object. Otherwise return @p defaultValue.
   */
  template <class T> T outputValue(const QString& name, const T& defaultValue);
  
  /**
   * A utility function that checks if any output has an object.
   */
  bool hasOutputValue() const;

  /**
   * Returns @p true if the named output contains a stored output
   * value and @p false otherwise.
   */
  bool hasOutputValue(const QString& name) const;

  /**
   * Clears the output value stored into the named output.
   */
  void clearOutputValue(const QString& name);

  /**
   * Clears all stored output values.
   */
  void clearAllOutputValues();
  
signals:
  /**
   * Emitted whenever an object is emitted through any of the output
   * sockets.
   *
   * @param name the name of the output socket that emitted the object
   *
   * @param obj the object that was captured
   *
   * @see PiiProbeInput
   */
  void objectReceived(const QString& name, const PiiVariant& obj);

private slots:
  void emitObject(const PiiVariant& obj);
  void deleteOutput(QObject* input);
  void deleteProbe(QObject* output);

private:
  typedef QMap<QString,PiiProbeInput*> ProbeMapType;

  void connectInput(PiiAbstractInputSocket* input);
  PiiProbeInput* createProbe(PiiAbstractOutputSocket* output, const QString& name);
  void createProbes();

  class Data
  {
  public:
    Data();
    PiiOperation* pOperation;
    ProbeMapType mapProbes;
  } *d;
};

template <class T> T PiiOperationTest::outputValue(const QString& name, const T& defaultValue)
{
  PiiVariant pObj = outputValue(name);
  if (pObj.type() != Pii::typeId<T>())
    return defaultValue;
  return pObj.valueAs<T>();
}

#endif //_PIIOPERATIONTEST_H
