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

#ifndef _PIIOPERATIONCOMPOUND_H
#define _PIIOPERATIONCOMPOUND_H

#include "PiiOperation.h"
#include "PiiProxySocket.h"

#include <QMap>

/// @file

/**
 * Declares a virtual piiMetaObject() function and implements a
 * serialization function that serializes the child operations and the
 * properties of the class.
 */
#define PII_COMPOUND_SERIALIZATION_FUNCTION \
  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION; \
  PII_PROPERTY_SERIALIZATION_FUNCTION(PiiOperationCompound)

/**
 * PiiOperationCompound is a class that controls a set of operations. 
 * It can contain any number of other operations, which may also be
 * compounds themselves.
 *
 * @par Execution
 *
 * PiiOperationCompound does actually nothing when it is started,
 * paused, or interrupted. It merely delegates the commands to its
 * direct ancestors. The state of a compound changes when the state of
 * all its children has changed. For example, when compound is
 * started, it changes state to @p Starting immediately after any of
 * its children changes state to @p Starting or @p Running. The state
 * of the compound will turn to @p Running when all child operations
 * have changed their state to @p Running.
 *
 * If the execution of any of a compound's child operations is
 * terminated before all of its siblings have changed their state to
 * @p Running, the compound itself may never change its state to @p
 * Running. This can happen, for example, if a data source quickly
 * runs out of data. In such a case the state turns first to @p
 * Starting and then to @p Stopping and finally to @p Stopped,
 * omitting @p Running, although the operations themself were
 * successfully started.
 *
 * It is important to note that the start(), pause() and interrupt()
 * functions may not immediately change the state of
 * PiiOperationCompound to @p Started, @p Paused, or @p Stopped but to
 * the corresponding intermediate states @p Starting, @p Pausing and
 * @p Interrupted. The state will change to the final state only after
 * all child operations have changed their state. The thread that
 * creates a PiiOperationCompound must run Qt's event handler every
 * now and then. Otherwise, state change signals from child operations
 * won't get delivered, and the compound itself will never change its
 * state. Typically, one does not need to do anything to accomplish
 * this as operations are created in the main thread, which calls
 * QApplication::exec() at some point. If a PiiOperationCompound is
 * created in another thread, one must either run a thread-specific
 * event loop (QThread::exec()) or call
 * QCoreApplication::processEvents() directly.
 *
 *
 * @par Naming
 *
 * The inner operations of the compound can be
 * accessed with a naming scheme in which sub-operations are denoted
 * by their @p objectName, and name parts are separated by dots.
 *
 * Assume the compound contains a PiiImageViewer sub-operation whose
 * @p objectName is "imageviewer". The @p image input of this object
 * can be read as follows:
 *
 * @code
 * // PiiOperationCompound *compound;
 * PiiInputSocket* input = compound->input("imageviewer.image");
 * @endcode
 *
 * The parsing recurses to sub-operations. Thus, if one knows the
 * internals of compound operations, any input can be found (e.g. 
 * "subop.subop.image"). @p parent denotes the parent operation.
 * This makes it possible to connect to a socket that is within
 * another compound.
 *
 * In the example below, @p Op0 contains @p Op1. In the context of
 * the root operation that contains both @p Op0 and @p Op2 (not
 * drawn), the sockets of the operations are denoted by
 * "Op0.Op1.in1", "Op0.Op1.out", and "Op2.in2". In the context of @p
 * Op0, @p in2 is "parent.Op2.in2".
 *
 * @image html operationcompound.png
 *
 * @par Inputs and Outputs
 *
 * Inputs and outputs for an operation compound can be created in two
 * alternative ways: aliasing and proxying. When a socket is aliased,
 * it just appears in the interface of the compound with an alias
 * name. Connections to or from the socket are made directly to the
 * socket itself. This method works for all output sockets and for
 * inputs that are connected to just one input within the compound. 
 * Since the connection through an alias is actually made directly to
 * the lower-level socket, there is no overhead in passing objects.
 *
 * Let us assume we have a pointer to @p Op0 in the figure above. We
 * can create an alias, i.e. @e expose any socket within the compound
 * as follows:
 *
 * @code
 * //PiiOperation* op0, *op1;
 * op0->exposeInput("Op1.in1", "input", PiiOperationCompound::AliasConnection);
 * // Analogously:
 * op0->exposeInput(op1.input("in1"), "input", PiiOperationCompound::AliasConnection);
 * // Map the output
 * op0->exposeOutput("Op1.out", "output", PiiOperationCompound::AliasConnection);
 * @endcode
 *
 * The result is that @p Op0 appears to have an input and an output
 * that actually are redirected to/from @p Op1.
 *
 * Proxying is useful if a "virtual" input needs to be redirected to
 * many operations. For this, PiiOperationCompound uses the
 * PiiSocketProxy class. Consider the illustration below. The compound
 * operation @p Masker creates a binary mask for an input image, and
 * multiplies the input by the mask. For this, the @p image input must
 * be connected to the operation that creates the mask and to the
 * operation that multiplies the mask and the input. What we need to
 * do is to create a proxy input that is connected to two inputs
 * within the compound. From the outside, the input appears as one:
 *
 * @image html operationcompoundproxy.png
 *
 * @code
 * // We assume that the objectName of PiiThresholdingOperation is
 * // "threshold" and that of PiiArithmeticOperation is "multiplier".
 * masker->exposeInputs(QStringList() << "threshold.image" << "multiplier.input0",
 *                      "image");
 *
 * // Alias the output as "image"
 * masker->exposeOutput("multiplier.output", "image", PiiOperationCompound::AliasConnection);
 * @endcode
 *
 * @par Serialization
 *
 * Operation compounds usually need to work differently when
 * constructed directly by user than when deserialized. Since the
 * child operations are automatically deserialized, it is not needed
 * to create them in the constructor. Therefore, two constructors are
 * usually provided, and the serialization system is told to use a
 * special "Void" function when deserializing.
 *
 * @code
 * class MyOperation : public PiiOperationCompound
 * {
 *   // Called by the serialization library. Doesn't create child operations.
 *   MyOperation(PiiSerialization::Void);
 *
 *   // This ensures that properties and child operations are serialized
 *   PII_COMPOUND_SERIALIZATION_FUNCTION
 * public:
 *   // Called by everybody else. Creates child operations.
 *   MyOperation();
 * };
 * @endcode
 *
 * When registering an operation compound to a plug-in, use the @ref
 * PII_REGISTER_COMPOUND macro instead of @ref PII_REGISTER_OPERATION.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiOperationCompound : public PiiOperation
{
  Q_OBJECT

  Q_ENUMS(ConnectionType);
  
  friend struct PiiSerialization::Accessor;
  PII_SEPARATE_SAVE_LOAD_MEMBERS
  PII_DECLARE_SAVE_LOAD_MEMBERS
  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION;

public:
  /**
   * Connection types.
   *
   * @lip ProxyConnection - exposed socket is routed through a proxy
   * that remains valid even if the exposed socket is deleted. A proxy
   * can be connected to multiple inputs at once.
   *
   * @lip AliasConnection - the exposed socket is shown in the public
   * interface as such. Aliases work with no processing overhead, but
   * an alias input can be connected to just one internal input.
   */
  enum ConnectionType { ProxyConnection, AliasConnection };

  /// Constructs a new operation compound.
  PiiOperationCompound();
  /// Destroys the compound.
  ~PiiOperationCompound();
  
  /**
   * Checks all child operations.
   *
   * @note This function must always be called before start().
   * Otherwise, start() will do nothing.
   */
  void check(bool reset);

  /**
   * Starts all child operations. The compound immediately changes its
   * state to @p Starting and turns @p Running once all children have
   * turned @p Running. The call has no effect if the compound is not
   * @p Stopped or @p Paused.
   */
  void start();

  /**
   * Pauses all child operations. The compound immediately changes its
   * state to @p Pausing and turns @p Paused once all children have
   * turned @p Paused. The call has no effect if the compound is not
   * @p Running.
   */
  Q_INVOKABLE void pause();

  /**
   * Interrupts all child operations. The compound immediately changes
   * its state to @p Stopping and turns @p Stopped once all children
   * have turned @p Stopped. The call has no effect if the compound is
   * @p Stopped.
   *
   * @note Calling interrupt terminates all child operations as soon
   * as possible, independent of their current processing state. If
   * your configuration has many parallel processing pipelines, they
   * may exit at different phases. For example, if the configuration
   * has two parallel PiiImageFileWriter operations, the image
   * currently being written may be finished by only one of them.
   *
   * @see stop()
   */
  Q_INVOKABLE void interrupt();

  /**
   * Stops all child operations. The compound immediately changes its
   * state to @p Stopping and turns @p Stopped once all children have
   * turned @p Stopped. The call has no effect if the compound is not
   * @p Running.
   */
  Q_INVOKABLE void stop();

  /**
   * Waits for all child operations to terminate their execution.
   *
   * @param time the maximum time to wait (in milliseconds). @p
   * ULONG_MAX causes the function not to time out.
   *
   * @return @p true if all child operations exited before the call
   * timed out, @p false otherwise.
   *
   * @note All children may not have signalled their state change to
   * @p Stopped yet even if this method returns @p true. They have,
   * however, finished execution. It is @b unsafe to delete the
   * compound immediately. Use @ref QObject::deleteLater()
   * "deleteLater()" or wait until the state has changed.
   */
  bool wait(unsigned long time = ULONG_MAX);

  /**
   * Waits for the compound to change state to @a state.
   *
   * @param state the target state
   *
   * @param time the maximum time to wait (in milliseconds). @p
   * ULONG_MAX causes the method not to time out.
   *
   * @return @p true if the state was reached before the call timed
   * out, @p false otherwise.
   *
   * @code
   * PiiEngine engine;
   * engine.interrupt();
   * engine.wait(PiiOperation::Stopped);
   * @endcode
   *
   * It is in general unsafe not to specify a timeout value. In some
   * cases it may happen that the compound never turns into the state
   * you expect. For example, if an error occurs in one of a
   * compound's sub-operations while start-up, the compound turns into
   * @p Stopped state. Another unexpected situation may arise if a
   * producer operation quickly runs out of data and spontaneously
   * stops while other operations are still @p Pausing or @p Starting. 
   * In this case the compound may will turn into @p Running state and
   * immediately to @p Stopping. Waiting for @p Stopped after stop()
   * or interrupt() and waiting for @p Paused after pause() is safe
   * with well-behaved operations, but a better convention is to
   * always specify a reasonable timeout and inspect the return value.
   *
   * @code
   * PiiEngine engine;
   * engine.pause();
   * // Wait for 2 seconds
   * if (!engine.wait(PiiOperation::Paused, 2000))
   *   {
   *     engine.interrupt();
   *     // Handle error here
   *   }
   * @endcode
   *
   * @note This function executes the event loop of the active thread
   * in order to deliver the state change signals of the operations
   * and to keep the application responsive. As a result, any of your
   * application events, including timer events, may get processed
   * during a the wait() call. This may result in unexpected behavior
   * especially if you alter the state of the compound in the slots
   * that will be called.
   */
  Q_INVOKABLE bool wait(State state, unsigned long time = ULONG_MAX);

  /**
   * Adds an operation to the compound's internal operation list.
   *
   * The new operation is placed into the QObject's child list of the
   * PiiOperationCompound object. Therefore, the memory taken up by
   * the operations is automatically released upon the destruction of
   * the compound object.
   *
   * If the compound is not stopped, this function has no effect.
   *
   * PiiOperationCompound does its best to ensure that its children do
   * not have duplicate object names. If you don't set the object
   * names manually, addOperation() creates a unique name by
   * concatenating the object's class name and an arbitrary number. If
   * the operation already has a name, it will not be modified.
   *
   * @param op the operation
   */
  Q_INVOKABLE virtual void addOperation(PiiOperation* op);

  /**
   * Removes an operation from the compound's internal operation list. 
   * The operation no longer belongs to the children of the compound
   * object.
   *
   * If the compound is neither stopped nor paused, this function has
   * no effect.
   *
   * @param op the operation to remove. All incoming and outgoing
   * connections will be disconnected.
   */
  Q_INVOKABLE virtual void removeOperation(PiiOperation* op);

  /**
   * Returns all direct child operations of this compound as a list.
   */
  Q_INVOKABLE QList<PiiOperation*> childOperations() const;

  /**
   * Returns the number of child operations. Equal to but faster than
   * childOperations.size().
   */
  Q_INVOKABLE int childCount() const;
  
  /**
   * Replaces @a oldOp with @a newOp. If the compound is neither
   * stopped nor paused, this function has no effect. If @a oldOp has
   * connected inputs and/or outputs, they will be disconnected and
   * reconnected to @a newOp to the corresponding sockets.
   *
   * @param oldOp the operation to remove
   *
   * @param newOp the operation to add in stead. If @a newOp is zero,
   * the function behaves like removeOperation(@p oldOp).
   *
   * @return @p true if the operation was successsfully
   * replaced/removed, @p false otherwise.
   */
  Q_INVOKABLE bool replaceOperation(PiiOperation *oldOp, PiiOperation* newOp);
  
  /**
   * Deletes all child operations and exposed inputs.
   */
  Q_INVOKABLE void clear();
  
  /**
   * Returns the named input socket. The name may be either an alias
   * or it can be specified by explicitly mentioning the
   * sub-operations involved. For example,
   * <tt>input("sub.socket")</tt> returns the input called "socket" in
   * a child operation called "sub".
   */
  PiiAbstractInputSocket* input(const QString& name) const;

  /**
   * Returns the named output socket. See input() for details.
   */
  PiiAbstractOutputSocket* output(const QString& name) const;

  /**
   * Returns exposed input sockets.
   */
  QList<PiiAbstractInputSocket*> inputs() const;

  /**
   * Returns exposed output sockets.
   */
  QList<PiiAbstractOutputSocket*> outputs() const;

  /**
   * Returns the value associated with @a name in @a socket. If the
   * "name" property is requested, this function first checks if the
   * socket is exposed in the interface of this operation, and returns
   * the alias name. If the socket is not exposed, it is recursively
   * searched in all of this operations child operations. If it is
   * found, a dotted name (in the stype childop.childop.socketname)
   * will be returned.
   *
   * If any other property is requsted, only exposed output sockets
   * will be searched. This is because inputs may be connected to many
   * input sockets, and the property values would not be unique. If a
   * match is found in the output sockets, the query will be passed to
   * the parent of its @ref PiiAbstractOutputSocket::rootOutput()
   * "root output".
   *
   * If @a socket is not found or there is no property called @a name
   * associated with it, an invalid property will be returned.
   */
  QVariant socketProperty(PiiAbstractSocket* socket, const char* name) const;

  /**
   * Exposes an input socket to this compound's interface. This
   * function adds an alias or a proxy to a socket attached to an
   * operation within the compound. After exposing, the socket is
   * available with the given name on the public interface of this
   * compound.
   *
   * By default, this function always creates a proxy socket, which
   * imposes some overhead to passing objects. To create direct
   * connections to the hidden socket, set @a connectionType to @p
   * AliasConnection.
   *
   * Note that output sockets seldom need to be proxied because any
   * output can directly be connected to many inputs. The need for a
   * proxy output arises if you want to connect an output both
   * internally and externally, or you want to preserve the output
   * even when internal operations are deleted.
   *
   * @param socket the socket to be exposed. Must be owned by an
   * operation that is a child of this compound. If @a socket is an
   * input, all previously exposed connections to it will be removed
   * first. Outputs can be aliased with multiple names.
   *
   * @param alias the alias name for the socket. The #input() and
   * #output() functions use this alias name to find the named
   * socket.
   *
   * @param connectionType the connection type. If set to @p
   * ProxyConnection (the default), multiple internal inputs can be
   * exposed with the same name. If @a connectionType is @p
   * AliasConnection, any existing proxy or alias will be replaced. If
   * @a connectionType is @p ProxyConnection, existing internal
   * connections will be preserved independent of the ConnectionType
   * of the existing connection.
   *
   * This function does its best in preserving existing connections. 
   * If an existing alias/proxy is replaced, its external connections
   * will be reconnected to the new exposed socket, be it either an
   * alias or a proxy. If an existing proxy is replaced with an alias,
   * only one internal connection will be retained.
   */
  void exposeInput(PiiAbstractInputSocket* socket,
                   const QString& alias,
                   ConnectionType connectionType = ProxyConnection);

  /**
   * Exposes on output socket to this compound's interface. See
   * #exposeInput().
   */
  void exposeOutput(PiiAbstractOutputSocket* socket,
                    const QString& alias,
                    ConnectionType connectionType = ProxyConnection);
  /**
   * A convenience function that allows one to expose an input socket
   * with the dot syntax (explained @ref PiiOperationCompound
   * "above").
   */
  void exposeInput(const QString& fullName,
                   const QString& alias,
                   ConnectionType connectionType = ProxyConnection);

  /**
   * A convenience function that exposes each named socket in @p
   * fullNames as @a alias.
   */
  void exposeInputs(const QStringList& fullNames, const QString& alias);
  
  /**
   * A convenience function that allows one to expose an output socket
   * with the dot syntax (explained @ref PiiOperationCompound
   * "above").
   */
  void exposeOutput(const QString& fullName,
                    const QString& alias,
                    ConnectionType connectionType = ProxyConnection);

  /**
   * Removes @a socket from the public interface. If the socket is
   * directly aliased, the alias will be removed. If the socket is
   * exposed through a proxy, the proxy will remain in effect. If the
   * socket is an output, all of its aliases will be removed.
   *
   * @param socket the socket to be removed from the public interface
   */
  void unexposeInput(PiiAbstractInputSocket* socket);
  
  /**
   * Removes @a socket from the public interface. See #unexposeInput().
   */
   void unexposeOutput(PiiAbstractOutputSocket* socket);

  /**
   * Removes an aliased input socket from the public interface. This
   * will break connections to all sockets aliased with @a alias.
   */
  void unexposeInput(const QString& alias);
  /**
   * Removes an aliased output socket from the public interface.
   */
  void unexposeOutput(const QString& alias);

  /**
   * Removes all exposed input and output sockets from the public
   * interface.
   */
  Q_INVOKABLE void unexposeAll();
  
  /**
   * Creates an unconnected proxy input. This function creates a new
   * PiiSocketProxy and reflects its input socket as @a alias. If @a
   * alias already exists, the function does nothing.
   */
  void createInputProxy(const QString& alias);

  /**
   * Creates an unconnected proxy output. This function creates a new
   * PiiSocketProxy and reflects its output socket as @a alias.  If @a
   * alias already exists, the function does nothing.
   */
  void createOutputProxy(const QString& alias);

  /**
   * Returns the proxy whose input is reflected as @a alias. If the
   * input does not exist or it is not a proxy input, 0 will be
   * returned.
   */
  PiiProxySocket* inputProxy(const QString& alias) const;

  /**
   * Returns the proxy whose output is reflected as @a alias. If the
   * output does not exist or it is not a proxy output, 0 will be
   * returned.
   */
  PiiProxySocket* outputProxy(const QString& alias) const;
  
  /**
   * Returns the number of output sockets. Equivalent to but faster
   * than outputs().size().
   */
  int outputCount() const;

  /**
   * Returns the number of input sockets. Equivalent to but faster
   * than inputs().size().
   */
  int inputCount() const;

  State state() const;

  /**
   * A convenience function that creates an instance of the named
   * class and adds it as a child to this compound. If the operation
   * cannot be created, 0 will be returned.
   *
   * @param className the name of the operation to create.
   *
   * @param objectName set the objectName property of the newly
   * created operation. This parameter is provided for convenience as
   * it often saves a separate setObjectName() call.
   *
   * @return a pointer to the operation or 0 if the class could not be
   * instantiated.
   */
  Q_INVOKABLE PiiOperation* createOperation(const QString& className, const QString& objectName = "");

  /**
   * Sets a property in this compound. This function supports the "dot
   * syntax" for setting properties. If the compound has a child
   * operation called @p child, then the properties of the child can
   * be set with <tt>setProperty("child.property", value)</tt>.
   */
  virtual bool setProperty(const char* name, const QVariant& value);

  /**
   * Returns a property using the "dot syntax".
   */
  virtual QVariant property(const char * name);
  
  /**
   * Creates a clone of this compound. This function recursively
   * clones all of its child operations.
   *
   * @return a deep copy of the compound.
   */
  Q_INVOKABLE PiiOperationCompound* clone() const;

protected:
  /// @internal
  class PII_YDIN_EXPORT Data;
  PII_UNSAFE_D_FUNC;

  PiiOperationCompound(Data* data);

  /**
   * An action function that starts a child operation.
   */
  struct Start { static void perform(PiiOperation* op) { op->start(); } };
  /**
   * An action function that pauses a child operation.
   */
  struct Pause { static void perform(PiiOperation* op) { op->pause(); } };
  /**
   * An action function that stops a child operation.
   */
  struct Stop { static void perform(PiiOperation* op) { op->stop(); } };
  /**
   * An action function that interrupts a child operation.
   */
  struct Interrupt { static void perform(PiiOperation* op) { op->interrupt(); } };
  
  /**
   * Send a command to all child operations. Use the action structures
   * to specify the action to be taken. This function loops through
   * the list of child operation and applies the action to each.
   *
   * @code
   * commandChildren<Execute>();
   * @endcode
   *
   * @see PiiOperationCompound::Start
   * @see PiiOperationCompound::Pause
   * @see PiiOperationCompound::Stop
   * @see PiiOperationCompound::Interrupt
   */
  template <class Action> void commandChildren()
  {
    QList<PiiOperation*> lstOperations = childOperations();
    for (int i=0; i<lstOperations.size(); ++i)
      Action::perform(lstOperations[i]);
  }

  /**
   * Sets the state of the operation. If the state changes, the
   * stateChanged() signal will be emitted.
   */
  void setState(State state);

  /**
   * Called by #setState() just before the operation changes to a new
   * state. The function will be called independent of the cause of
   * the state change (internal or external). Derived classes may
   * implement this function to perform whatever is needed when a
   * state changes. The default implementation does nothing.
   */
  virtual void aboutToChangeState(State newState);

private slots:
  void childStateChanged(int state);
  void childDestroyed(QObject* op);
  void handleError(PiiOperation* sender, const QString& msg);
  void removeInput(QObject* socket);
  void removeOutput(QObject* socket);
  
private:
  class ExposedSocket
  {
  public:
    ExposedSocket(const QString& name, QObject* parent);
    ~ExposedSocket();

    QString name() const { return _strName; }

    bool isProxy() const { return _type == PiiSocket::Proxy; }
    PiiAbstractSocket* socket() const { return _pSocket; }
    PiiProxySocket* proxy() const { return isProxy() ? dynamic_cast<PiiProxySocket*>(_pSocket) : 0; }
    PiiAbstractInputSocket* input() const { return dynamic_cast<PiiAbstractInputSocket*>(_pSocket); }
    PiiAbstractOutputSocket* output() const { return dynamic_cast<PiiAbstractOutputSocket*>(_pSocket); }
    QObject* qObject() const { return _pQObject; }
    void reset() { if (isProxy()) proxy()->reset(); }
 
    QList<PiiAbstractInputSocket*> outgoingConnections() const;
   
    void expose(PiiAbstractOutputSocket* socket,
                ConnectionType connectionType);
    void expose(PiiAbstractInputSocket* socket,
                ConnectionType connectionType);

  private:
    QString _strName;
    PiiSocket::Type _type;
    PiiAbstractSocket* _pSocket;
    // The socket pointer needs to be stored as a QObject because
    // socket pointers being deleted already lack rtti
    // when the destroyed() signal arrives. (dynamic_cast not possible)
    QObject* _pQObject;
    QObject* _pParent;
  };

  class SocketList : public QList<ExposedSocket*>
  {
  public:
    SocketList() {}
    SocketList(const SocketList& other) : QList<ExposedSocket*>(other) {}
    SocketList& operator= (const SocketList& other) { return static_cast<SocketList&>(QList<ExposedSocket*>::operator= (other)); }

    ExposedSocket* operator[] (const QString& name) const;
    ExposedSocket* operator[] (PiiAbstractOutputSocket* socket) const;
    ExposedSocket* operator[] (int index) const { return QList<ExposedSocket*>::operator[](index); }
    bool contains(const QString& name) const;
    ExposedSocket* take(const QString& name);
  };

  template <class Socket> static void remove(QObject* socket, SocketList& sockets);
  
  // Recursive socket look-up
  struct InputFinder;
  struct OutputFinder;

  // Finders for properties
  struct SetPropertyFinder;
  struct GetPropertyFinder;
  
  // Generic find function for sockets and UI components
  template <class Finder> typename Finder::Type find(Finder f, const QString& path) const;

  // State changing utilities
  bool checkSteadyStateChange(State newState, State intermediateState, State steadyState);
  bool checkChildStates(State state);

  // One-level child look-up
  PiiOperation* findChildOperation(const QString& childName) const;
  
  // Serialization stuff
  typedef QPair<PiiOperation*, QString> EndPointType;
  typedef QList<EndPointType> EndPointListType;
  PiiOperation* findCommonParent(QObject* obj1, QObject* obj2, int& parentIndex) const;
  EndPointType locateSocket(PiiAbstractSocket* socket, const PiiOperationCompound *context = 0) const;
  EndPointListType buildEndPointList(PiiAbstractOutputSocket* socket, const PiiOperationCompound *context = 0) const;

  // Look up a socket ignoring aliases
  QString internalName(PiiAbstractSocket* socket) const;

  // Interrupt emissions in input proxies
  void interruptProxies();
  // Reset all proxy inputs
  void resetProxies();

  QString fullName(QObject* operation);
};

/// @internal
class PiiOperationCompound::Data : public PiiOperation::Data
{
public:
  Data();
  virtual ~Data();

  /**
   * The current state.
   */
  State state;
  
  /**
   * List of child operations.
   */
  QList<PiiOperation*> lstOperations;

private:
  friend class PiiOperationCompound;
  
  /**
   * Exposed input sockets.
   */
  SocketList lstInputs;
  /**
   * Exposed output sockets.
   */
  SocketList lstOutputs;
    
  /**
   * States of child operations. In each pair, "first" is the state of
   * the operation and "second" a flag that tells if the operation has
   * been in Running state.
   */
  QVector<QPair<State,bool> > vecChildStates;

  bool bChecked, bWaiting;
};

Q_DECLARE_METATYPE(PiiOperationCompound*);
typedef QList<PiiOperation*> PiiOperationList;
Q_DECLARE_METATYPE(PiiOperationList);

#include "PiiOperationCompound-templates.h"

#define PII_SERIALIZABLE_CLASS PiiOperationCompound
#define PII_SERIALIZABLE_CLASS_VERSION 1
#define PII_SERIALIZABLE_SHARED
#define PII_BUILDING_LIBRARY PII_BUILDING_YDIN

#include <PiiSerializableRegistration.h>

#endif //_PIIOPERATIONCOMPOUND_H
