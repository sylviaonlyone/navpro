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

#ifndef _PIIINPUTSOCKET_H
#define _PIIINPUTSOCKET_H

#include "PiiSocket.h"
#include "PiiAbstractInputSocket.h"
#include "PiiInputController.h"

class PiiOutputSocket;

/** @file */

/**
 * A macro that throws a PiiExecutionException. The error message
 * states that an object of an unknown type was received in @p input.
 *
 * @param input a pointer to the input that received an unrecognized
 * object.
 */
#define PII_THROW_UNKNOWN_TYPE(input) \
  PII_THROW(PiiExecutionException, PiiYdin::unknownTypeErrorMessage(input))

/**
 * A macro that throws a PiiExecutionException. The error message
 * states that the @p matrix received in @p input has a wrong size,
 * and it should actually have the specified number of @p rows and @p
 * columns.
 *
 * @param INPUT a pointer to the input that reveived the matrix.
 *
 * @param MATRIX a matrix (or a reference)
 *
 * @param ROWS the correct number of rows, e.g. 1 or "N"
 *
 * @param COLUMNS the correct number of columns, e.g. 4 or "M"
 */
#define PII_THROW_WRONG_SIZE(INPUT, MATRIX, ROWS, COLUMNS) \
  PII_THROW(PiiExecutionException, \
            PiiYdin::wrongSizeErrorMessage(INPUT, \
                                           QString("%1 x %2").arg((MATRIX).rows()).arg((MATRIX).columns()), \
                                           QString("%1 x %2").arg(ROWS).arg(COLUMNS)))

/**
 * Default implementation of an input socket. This class maintains an
 * input queue whose capacity can be controlled with the
 * #queueCapacity property. A producer operation can send objects to
 * the input queue and continue working until the input queue is full. 
 * Once this happens, the sending thread must wait until the receiver
 * has processed one or more objects.
 *
 * At the start of each processing round, the parent operation takes
 * one object from the queue to processing. New objects may then
 * appear at any time until the queue is full again.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiInputSocket : public PiiSocket, public PiiAbstractInputSocket
{
  Q_OBJECT

  /**
   * The capacity of the input queue. The default value is 3, which
   * makes it possible to queue two objects in addition to the one
   * currently being processed. The minimum value for queue capacity
   * is 2. Note that queue capacity can safely be changed only if the
   * parent operation is stopped. Changing the capacity will destroy
   * all objects currently in the queue.
   */
  Q_PROPERTY(int queueCapacity READ queueCapacity WRITE setQueueCapacity);

public:
  /**
   * Constructs a new input socket with the given name. This
   * constructor sets @p name as the @p objectName property of this
   * class.
   */
  PiiInputSocket(const QString& name);

  /// Destroys the socket.
  ~PiiInputSocket();

  /**
   * Returns @p Input.
   */
  Type type() const;
  
  /**
   * Sets the group id. Input sockets are organized to groups
   * identified by numeric ids. Sockets with the same group id are
   * assumed to work in parallel. The default synchronization
   * mechanism in Ydin ensures that all synchronous inputs with the
   * same group id contain a readable object when an operation is
   * processed. This makes it possible to read in objects with
   * different flow rates. The default group id is zero, which causes
   * all synchronous inputs to work in parallel.
   */
  void setGroupId(int groupId);

  /**
   * Returns the group id of this socket.
   */
  Q_INVOKABLE int groupId() const;

  /**
   * Checks if the input is connected. Returns @p true if the input is
   * connected to a PiiOutputSocket either directly or through a
   * PiiProxySocket, @p false otherwise.
   */
  Q_INVOKABLE bool isConnected() const;

  /**
   * Checks if this input socket can be left unconnected in processing. 
   * If an optional input socket is left unconnected, the receiving
   * operation ignores it at in processing.
   *
   * @return @p true if the socket can be left unconnected, @p false
   * otherwise
   */
  Q_INVOKABLE bool isOptional() const;

  /**
   * Sets the @p optional flag.
   */
  void setOptional(bool optional);

  /**
   * Releases the last slot in the object queue by shifting incoming
   * objects (if any) one step forwards. Once the queue is
   * successfully shifted, the socket signals inputReady().
   */
  void release();

  /**
   * Jumps the queue. This function moves the object at @p oldIndex to
   * a new place, identified by @p newIndex, shifting intermediate
   * objects back.
   */
  void jump(int oldIndex, int newIndex);

  /**
   * Searches the queue for an object whose type ID matches @p type.
   *
   * @param type the type ID to look for
   *
   * @param startIndex start search at this index
   *
   * @return the index of the first object with the given type ID, or
   * -1 if no such object is found.
   */
  int indexOf(unsigned int type, int startIndex = 0) const;

  /**
   * Resets the socket. This clears the object queue.
   */
  void reset();
  
  /**
   * Puts @p obj into the incoming queue.
   */
  void receive(const PiiVariant& obj);

  /**
   * Checks if the input queue in this socket still has room for a new
   * object. This function is a shorthand for queueCapacity() >
   * queueLength().
   */
  bool canReceive() const;

  /**
   * Sets the input queue capacity.
   */
  void setQueueCapacity(int queueCapacity);
  /**
   * Returns the capacity of the input queue.
   */
  int queueCapacity() const;

  /**
   * Returns the number of objects currently in the input queue.
   */
  int queueLength() const;
  
  /**
   * Returns the object at @p index in the input queue. If there is no
   * such object, an invalid variant will be returned.
   */
  PiiVariant objectAt(int index) const;
  /**
   * Returns a reference to the object at @p index in the input queue.
   */
  PiiVariant& objectAt(int index);
  /**
   * Returns the type ID of the object at @p index in the input queue.
   */
  unsigned int typeAt(int index) const;

  /**
   * Sets an object in the queue. There is usually no use for this
   * function as objects are received through output sockets, but the
   * function is provided for completeness.
   *
   * @param index the index of the object to set
   *
   * @param object the new object
   */
  void setObject(int index, const PiiVariant& object);

  /**
   * Returns the first object in the input queue. If the queue is
   * empty an invalid variant will be returned.
   *
   * @code
   * PiiVariant obj(pInput->firstObject());
   * if (obj.isValid())
   *   cout << "We just received an object!" << endl;
   * @endcode
   */
  PiiVariant firstObject() const;

  /**
   * Returns a reference to the first object in the input queue.
   */
  PiiVariant& firstObject();

  /**
   * Checks if there queue is empty. Returns @p true if there are no
   * objects in the queue, @p false otherwise.
   */
  bool isEmpty() const;

  /**
   * Sets the input controller.
   */
  void setController(PiiInputController* controller);

  /**
   * Returns the input controller.
   */
  PiiInputController* controller() const;

  PiiInputSocket* socket();
  PiiAbstractInputSocket* asInput();
  PiiAbstractOutputSocket* asOutput();
  
protected:
  /// @internal
  class Data : public PiiAbstractInputSocket::Data
  {
  public:
    Data();
    
    bool setInputConnected(bool connected);
  
    int iGroupId;
    bool bConnected;
    bool bOptional;
    PiiInputController* pController;
    PiiVariant* pQueue;
    int iQueueCapacity, iQueueStart, iQueueLength;
  };
  PII_UNSAFE_D_FUNC;

  /// @internal
  PiiInputSocket(Data* data, const QString& name);
  
private:
  inline int queueIndex(int index) const { return (_d()->iQueueStart+index) % _d()->iQueueCapacity; }
};

Q_DECLARE_METATYPE(PiiInputSocket*);

namespace PiiYdin
{
  /// @internal
  PII_YDIN_EXPORT QString unknownTypeErrorMessage(PiiInputSocket* input);

  /// @internal
  PII_YDIN_EXPORT QString wrongSizeErrorMessage(PiiInputSocket* input, const QString& received, const QString& assumed);
}

#endif //_PIIINPUTSOCKET_H
