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

#ifndef _PIIOPERATION_H
#define _PIIOPERATION_H

#include <PiiSerializationUtil.h>
#include <PiiMatrixSerialization.h>
#include "PiiInputSocket.h"
#include "PiiOutputSocket.h"
#include "PiiExecutionException.h"
#include "PiiYdin.h"

#include <QMutex>

/// @file

/**
 * Declares a virtual piiMetaObject() function and implements a
 * serialization function that serializes the properties of the class.
 */
#define PII_OPERATION_SERIALIZATION_FUNCTION \
  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION; \
  PII_PROPERTY_SERIALIZATION_FUNCTION(PiiOperation)

/**
 * A superclass for operations that can be run by Ydin. Operations can
 * be roughly divided into produces, consumers, and transformations. 
 * An image source, such as a camera, is a producer (an input), image
 * processing operations are transforms, and a connection to a process
 * control system is a consumer (an output). There are however
 * operations that can work in many roles simultaneously.
 *
 * Each operation can be seen as a process that receives some input
 * data and produces some results after processing. An operation may
 * also have no inputs or no outputs, in which case it is considered a
 * producer or a consumer, respectively.
 *
 * Operations can work either synchronously or asynchronously. For
 * example, an image source can emit images in its own internal
 * frequency, irrespective of any consumers of the image data
 * (asynchronous operation). The receivers process the data if they
 * aren't too busy doing other things. In most cases it is however
 * necessary to process all images captured. In this case the image
 * source halts until all processing has been done before it sends the
 * next image.
 *
 * Each operation can have any number (0-N) of inputs and outputs
 * called sockets (see PiiSocket). Each socket has a data type or
 * multiple data types associated with it. That is, each socket
 * handles certain types of objects. Input and output sockets with
 * matching types can be connected to each other, and an output can be
 * connected to multiple inputs.
 *
 * Sockets are automatically destroyed with their parent operation. 
 * Furthermore, all connections between deleted sockets are
 * automatically deleted. Thus, one doesn't need to care about
 * deleting anything but the operation itself.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiOperation : public QObject
{
  Q_OBJECT

  Q_ENUMS(State);

  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION;
  PII_DEFAULT_SERIALIZATION_FUNCTION(QObject);

public:

  ~PiiOperation();
  
  /**
   * The state of an operation can assume six different values:
   *
   * @lip Stopped - the operation is not running.
   *
   * @lip Starting - the operation has received a #start() signal, but
   * it is not running yet.
   *
   * @lip Running - the operation is running.
   *
   * @lip Pausing - the operation has received a #pause() signal, but
   * it hasn't finished execution yet.
   *
   * @lip Paused - the operation has finished execution due to a
   * #pause() command.
   *
   * @lip Stopping - the operation has received a #stop() signal, but
   * it hasn't stopped yet.
   *
   * @lip Interrupted - the operation has received an #interrupt()
   * signal, but it hasn't stopped yet.
   */
  enum State
    {
      Stopped,
      Starting,
      Running,
      Pausing,
      Paused,
      Stopping,
      Interrupted
    };

  /**
   * Protection levels for setting properties.
   *
   * @lip WriteAlways - setting the value of a property is always
   * allowed. This is the default value for all properties.
   *
   * @lip WriteWhenStoppedOrPaused - setting the value of a property
   * is allowed only if the state of the operation is either @p
   * Stopped or @p Paused.
   *
   * @lip WriteWhenStopped - setting the value of a property is
   * allowed only if the state of the operation is @p Stopped.
   *
   * @lip WriteNotAllowed - setting the value of a property is not
   * allowed at all. This value makes it possible for subclasses to
   * make the properties of a superclass read-only even if they had a
   * setter function.
   */
  enum ProtectionLevel
    {
      WriteAlways,
      WriteWhenStoppedOrPaused,
      WriteWhenStopped,
      WriteNotAllowed
    };

  /**
   * Returns a string presentation of the given state.
   */
  static const char* stateName(State state);
  
  /**
   * Checks that the necessary preconditions for processing are met. 
   * This function is called by before processing is started. It is
   * followed by a #start() call, provided that all operations in an
   * engine have been successfully initialized. The function should
   * check, for example, that all required inputs are connected.
   *
   * This function is called by PiiEngine once for all operations
   * before any of them is started. This ensures there will be no
   * input into any operation before each of them have been checked.
   *
   * @param reset if @p true, reset the operation to its initial
   * state. This flag is true whenever the operation was interrupted
   * and started again. If the operation was just paused, the reset
   * flag is set to @p false, and the operation should continue where
   * ever it was left.
   *
   * @exception PiiExecutionException& if the operation cannot be run.
   */
  virtual void check(bool reset) = 0;

  /**
   * Starts the operation and executes it until interrupted or paused. 
   * The operation should prepare itself for work and change state to
   * @p Starting or @p Running.
   *
   * This function will be called not only on start-up but also before
   * restarting after a #pause() call.
   *
   * Note that PiiOperationCompound (and PiiEngine) commands child
   * operations in an arbitrary order. Therefore, objects may appear
   * in the inputs of a operation before %start() is invoked. Any
   * resetting action should take place in #check().
   */
  virtual void start() = 0;

  /**
   * Pauses the operation. The operation should change its state to @p
   * Pausing. Pausing had direct effect on producer operations only. 
   * If a producer is in @p Pausing state, it'll finish its current
   * processing round, turn to @p Paused state and inform all
   * connected operations that it is now safe to pause. This will
   * finally turn all operations in a pipeline to @p Paused state. 
   * Before the next #start() call, #check() will be called with the
   * @p reset parameter set to @p false.
   *
   * Note that pause() has no effect if the operation is not in @p
   * Running state.
   */
  virtual void pause() = 0;

  /**
   * Stops the operation. The operation should change its state to @p
   * Stopping. Stopping and pausing work similarly in most cases. 
   * Usually, the only difference is in the final state of the
   * operation. Overriding this function makes it possible for an
   * operation to prepare for stopping. The difference between @p
   * stop() and #interrupt() is in that the former performs a "clean"
   * stop, which won't leave any object currently being processed in
   * the pipeline.
   *
   * Note that stop() has no effect if the operation is not in @p
   * Running state.
   */
  virtual void stop() = 0;
  
  /**
   * Interrupts the execution. Calling this function should stop the
   * execution of this operation as soon as possible, even in the
   * middle of a processing round. After an interrupt, the next
   * #start() call will be preceded by a #check() call with the @p
   * reset parameter set to @p true.
   */
  virtual void interrupt() = 0;

  /**
   * Returns the current state of the operation.
   */
  Q_INVOKABLE virtual State state() const = 0;

  /**
   * Waits for this operation to stop running. The calling function
   * blocks until the operation stops running or @p time milliseconds
   * has elapsed. By default, the call will never time out. Note that
   * the state of the operation may not have changed yet when this
   * function returns.
   *
   * @return @p true if the operation exited within @p time
   * milliseconds, @p false if the call timed out.
   */
  virtual bool wait(unsigned long time = ULONG_MAX) = 0;

  /**
   * Returns the number of input sockets. The default implementation
   * returns inputs().size().
   */
  Q_INVOKABLE virtual int inputCount() const;
  
  /**
   * Returns a pointer to the input associated with @a name.
   *
   * @return a pointer to the input socket or 0 if no socket matches
   * @a name
   */
  Q_INVOKABLE virtual PiiAbstractInputSocket* input(const QString& name) const = 0;

  /**
   * Returns the number of output sockets. The default implementation
   * returns outputs().size().
   */
  Q_INVOKABLE virtual int outputCount() const;

  /**
   * Returns a pointer to the output associated with @a name.
   *
   * @return a pointer to the output socket or 0 if no socket matches
   * @a name
   */
  Q_INVOKABLE virtual PiiAbstractOutputSocket* output(const QString& name) const = 0;

  /**
   * Returns a list of all input sockets connected to this operation. 
   * The order in which the sockets are returned should be
   * "intuitive", but no strict restrictions are imposed.
   */
  Q_INVOKABLE virtual QList<PiiAbstractInputSocket*> inputs() const = 0;

  /**
   * Returns the names of all inputs.
   */
  Q_INVOKABLE QStringList inputNames() const;

  /**
   * Returns a list of all output sockets connected to this operation. 
   * Analogous to inputs().
   */
  Q_INVOKABLE virtual QList<PiiAbstractOutputSocket*> outputs() const = 0;

  /**
   * Returns the names of all outputs.
   */
  Q_INVOKABLE QStringList outputNames() const;

  /**
   * Returns the name of a socket in the context of this operation. 
   * The name of a socket may change based on nesting level. The same
   * socket may be accessed with multiple names, if it is exported to
   * the interface of an @ref PiiOperationCompound "operation compound".
   *
   * Operations are free to implement any scheme for naming their
   * sockets. For example, PiiBasicOperation uses the @p objectName
   * property of the socket, and PiiOperationCompound keeps an
   * internal map of socket aliases. Since there is no default naming
   * scheme, the default implementation returns an empty string.
   *
   * This is a convenience function that calls socketProperty(@a
   * socket, "name").
   *
   * @param socket the socket whose name is to be found
   *
   * @return the name of the socket or an empty string if the socket
   * is not owned by this operation.
   */
  Q_INVOKABLE QString socketName(PiiAbstractSocket* socket) const;

  /**
   * Returns meta information associated with @a socket. This function
   * can be used to query named properties of input and output
   * sockets. Operations are required to provide at least the @p name
   * property. Other properties can be used depending on application. 
   * Below is a short list of commonly used properies:
   *
   * @lip name - the name of the socket in the context of this
   * operation. See socketName().
   *
   * @lip min - the minimum possible scalar value a socket can
   * send/receive
   *
   * @lip max - the maximum possible scalar value a socket can
   * send/receive
   *
   * @lip resolution - the resolution of the value. Integers have a
   * resolution of 1. If the value can take values only in known
   * steps, the @p resolution property specifies the step size.
   *
   * @lip displayName - a user-displayable name of the socket. May be
   * translated.
   *
   * @param socket the socket whose properties are queried
   *
   * @param the name of the property to query
   *
   * @return the value of the property, or an invalid QVariant if the
   * socket is not owned by this operation or the named property does
   * not exist.
   *
   * @see PiiYdin::isNameProperty()
   */
  virtual QVariant socketProperty(PiiAbstractSocket* socket, const char* name) const;

  /**
   * @overload
   */
  Q_INVOKABLE QVariant socketProperty(PiiAbstractSocket* socket, const QString& name) const;
  
  /**
   * A convenience function for connecting a named output socket to a
   * named input socket in another operation. This is (almost)
   * analogous to:
   *
   * @code
   * output("output")->connectInput(other->input(input));
   * @endcode
   *
   * The difference is in that this function returns @p false in case
   * of a failure.
   *
   * @param output the name of the output socket
   *
   * @param other the operation that contains the input we need to
   * connect to
   *
   * @param input the name of the input socket in @p other
   *
   * @return @p true if both sockets were found, @p false otherwise
   */
  Q_INVOKABLE bool connectOutput(const QString& output, PiiOperation* other, const QString& input);

  /**
   * Virtual version of QObject::setProperty(). Making a non-virtual
   * function virtual in a subclass is @e baad. But we need to be able
   * to override this function to support dotted property names and
   * mutual exclusion.
   *
   * @see PiiOperationCompound::setProperty()
   * @see PiiDefaultOperation::setProperty()
   */
  virtual bool setProperty(const char* name, const QVariant& value);

  /**
   * Virtual version of QObject::property(). Making a non-virtual
   * function virtual in a subclass is @e baad. But we need to be able
   * to override this function.
   */
  virtual QVariant property(const char* name);

  /**
   * A convenience function that automatically creates a QVariant out
   * of a PiiVariant.
   *
   * @code
   * PiiOperation* op = ...;
   * op->setProperty("property", Pii::createVariant(PiiMatrix<int>(4,4)));
   * @endcode
   */
  bool setProperty(const char* name, const PiiVariant& value);

  /**
   * Creates a clone of this operation. The default implementation
   * uses the serialization library to find a factory object for the
   * class. Once an instance is created it iterates over all
   * properties and copies their values to the new operation instance.
   *
   * @return a deep copy of the operation, or 0 if no factory was
   * found.
   */
  Q_INVOKABLE virtual PiiOperation* clone() const;

  /**
   * Disconnects all inputs.
   */
  Q_INVOKABLE void disconnectAllInputs();

  /**
   * Disconnects all outputs.
   */
  Q_INVOKABLE void disconnectAllOutputs();
  
  /**
   * Returns the protection level of @a property.
   */
  ProtectionLevel protectionLevel(const char* property) const;
  
signals:
  /**
   * Signals an error. The @a message should be a user-friendly
   * explanation of the cause of the error. @a sender is the original
   * source of the message.
   */
  void errorOccured(PiiOperation* sender, const QString& message);

  /**
   * Indicates that the state of this operation has changed. If you
   * connect to this signal from outside, make sure you either run an
   * event loop in the receiving thread or create a direct connection. 
   * This is needed because the signal will most likely be emitted
   * from another thread. If you create a queued connection and don't
   * run an event loop in the receiving thread, the signal will be
   * lost. If you create a direct connection, you must explicitly
   * implement a mutual exclusion mechanism in the receiving slot.
   *
   * @param state the new state of the operation. The type of this
   * value is actually PiiOperation::State, but @p int is used to
   * avoid registering a new meta type.
   */
  void stateChanged(int state);
  
protected:
  /// @internal
  typedef QList<QPair<const char*, ProtectionLevel> > ProtectionList;
  /// @internal
  class PII_YDIN_EXPORT Data
  {
  public:
    Data();
    virtual ~Data();

    ProtectionList lstProtectionLevels;
    QMutex stateMutex;
  } *d;
  
  /**
   * Constructs a new PiiOperation.
   */
  PiiOperation();

  /// @internal
  PiiOperation(Data* d);

  /**
   * Sets the protection level of @a property to @a level. This
   * function is a generic way of controlling write access to
   * properties. By default, all properties writable independent of
   * the state of the operation. Some properties do however affect the
   * internal structure of an operation in a manner that cannot be
   * handled at run time. For example, the number of sockets cannot be
   * changed on the fly without careful handling of the internal
   * synchronization mechanism.
   *
   * @code
   * MyOperation::MyOperation() : PiiDefaultOperation(Simple)
   * {
   *   // Disallow changing of the processing mode
   *   setWritePermission("processingMode", WriteNever);
   * }
   * @endcode
   *
   * @note Protection is only effective if properties are set through
   * setProperty(). Calling property setters directly bypasses the
   * protection mechanism.
   */
  void setProtectionLevel(const char* property, ProtectionLevel level);

  /**
   * Returns a pointer to the mutex that prevents concurrent access to
   * the state of this operation.
   *
   * @code
   * void MyOperation::stop()
   * {
   *   synchronized (stateLock())
   *   {
   *     if (state() == Running)
   *       setState(Stopping);
   *   }
   * }
   * @endcode
   */
  QMutex* stateLock();

private:
  int indexOf(const char* property) const;
  PII_DISABLE_COPY(PiiOperation);
};

Q_DECLARE_METATYPE(PiiOperation*);
typedef QList<PiiAbstractInputSocket*> PiiInputSocketList;
Q_DECLARE_METATYPE(QList<PiiAbstractInputSocket*>);
typedef QList<PiiAbstractOutputSocket*> PiiOutputSocketList;
Q_DECLARE_METATYPE(QList<PiiAbstractOutputSocket*>);

#define PII_SERIALIZABLE_CLASS PiiOperation
#define PII_SERIALIZABLE_IS_ABSTRACT
#define PII_SERIALIZABLE_SHARED
#define PII_VIRTUAL_METAOBJECT
#define PII_BUILDING_LIBRARY PII_BUILDING_YDIN

#include <PiiSerializableRegistration.h>


#include <PiiQVariantWrapper.h>
#define PII_SERIALIZABLE_CLASS PiiQVariantWrapper::Template<PiiOperation*>
#define PII_SERIALIZABLE_CLASS_NAME "PiiQVariantWrapper<PiiOperation*>"
#define PII_SERIALIZABLE_SHARED
#define PII_BUILDING_LIBRARY PII_BUILDING_YDIN

#include <PiiSerializableRegistration.h>

#endif //_PIIOPERATION_H
