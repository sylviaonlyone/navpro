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

#ifndef _PIIOUTPUTSOCKET_H
#define _PIIOUTPUTSOCKET_H

#include "PiiSocket.h"
#include "PiiAbstractOutputSocket.h"
#include "PiiSocketState.h"
#include "PiiExecutionException.h"
#include "PiiVariant.h"
#include "PiiMatrix.h"

#include <PiiWaitCondition.h>

class PiiAbstractInputSocket;
class PiiInputSocket;
class PiiInputController;

namespace PiiYdin
{ 
  /// @internal
  template <class Matrix, class T>
  inline PiiVariant createVariant(const PiiConceptualMatrix<Matrix>*,
                                  const T& mat)
  {
    // This should actually go into PiiYdinTypes.h, but that would
    // make inclusion order a huge trouble. Stupid C++.
    return PiiVariant(Pii::matrix(mat));
  }

  /// @internal
  template <class T> inline PiiVariant createVariant(const void*, const T& value)
  {
    return PiiVariant(value);
  }

  /**
   * Returns a variant that contains @p value. By default, this
   * function just returns @p PiiVariant(value). Using this function
   * as an indirection is however useful since it allows
   * specialized versions for different data types. For example, all
   * conceptual matrices are converted into concrete PiiMatrix
   * instances before creating a variant out of them.
   *
   * @code
   * PiiMatrix<int> a, b;
   * PiiVariant v1(a+b); //Won't work, because a+b is not a concrete matrix
   * PiiVariant v2(PiiYdin::createVariant(a+b)); // works
   * @endcode
   */
  template <class T> inline PiiVariant createVariant(const T& value)
  {
    // Hack resolution order for superclasses. Stupid C++.
    return createVariant(&value, value);
  }
}

/**
 * An output socket.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiOutputSocket : public PiiSocket, public PiiAbstractOutputSocket
{
  Q_OBJECT
  
public:
  /**
   * Construct a new output socket with the given name. This
   * constructor sets @p name as the @p objectName property of the
   * class.
   */
  PiiOutputSocket(const QString& name);

  /**
   * Destroy the socket. Automatically disconnects all input sockets.
   */
  ~PiiOutputSocket();

  /**
   * Returns @p Output.
   */
  Type type() const;
  
  /**
   * Set the group id of this socket. The group id of an output socket
   * is used to find synchronization pairs. The default flow control
   * implementation sends synchronization information from an input
   * socket to all output sockets matching the group id of the input.
   */
  void setGroupId(int groupId);

  /**
   * Get the group id of this socket.
   */
  Q_INVOKABLE int groupId() const;

  /**
   * Set this socket synchronized with respect to @p input. This is a
   * convenience function that sets the socket's group id to that of
   * @p input, which indicates that this output emits objects in
   * relation to the specified input. For example, the output may emit
   * an object each time a new object is received in @p input. Or it
   * may emit two objects for each input, or one for each three
   * inputs, or two for every other input and three for the others. In
   * principle, any variation is allowed. The main point is that the
   * output is caused by objects read from @p input. By default, all
   * inputs and outputs are in sync group 0. There is usually no need
   * to change this.
   */
  void synchronizeTo(PiiInputSocket* input);

  /**
   * Inform the synchronization system that the rate of the following
   * emissions differs from that of the synchronized input socket. 
   * This function must be called whenever you start sending more
   * objects than you receive (or no objects at all). To emit two
   * objects for each input, do something like this in the process()
   * function of your operation:
   *
   * @code
   * PiiVariant obj = readObject();
   * output->startMany();
   * output->emitObject(1);
   * output->emitObject(2);
   * output->endMany();
   * @endcode
   */
  void startMany();

  /**
   * End the flow rate change.
   *
   * @see startMany()
   */
  void endMany();

  /**
   * Inform the synchronization system that the output from this
   * socket will be delayed. There is seldom need to use this function
   * as a synchronized output usually emits an object immediately upon
   * processing an input object. In some cases, however, the output
   * will be available only after some additional data has been
   * received. Examples of such operations include PiiImagePieceJoiner
   * and PiiHistogramCollector.
   *
   * The following example assumes @p MyOperation has two inputs: one
   * for a large image and another for values calculated from image
   * pieces just like PiiImagePieceJoiner. @p MyOperation calculates a
   * value for each large image, but it needs both the large image and
   * the values from sub-images for it.
   *
   * @code
   * void MyOperation::process()
   * {
   *   if (activeInputGroup() == _pImageInput->groupId())
   *     {
   *       _pResultOutput->startDelay();
   *       storeImage(_pImageInput->firstObject());
   *     }
   *   else
   *     storeValue(_pValueInput->firstObject());
   * }
   *
   * void MyOperation::syncEvent(SyncEvent* event)
   * {
   *   if (event.type() == SyncEvent::EndInput)
   *     {
   *       _pResultOutput->emitObject(calculateValue());
   *       _pResultOutput->endDelay();
   *     }
   * }
   * @endcode
   *
   * It is possible to delay emissions more than one processing round,
   * but this function must be called once for each delayed object. 
   * Once the objects are finally sent, the same number of endDelay()
   * calls must be performed.
   *
   * One must ensure that the size of the input queue at a receiver is
   * larger enough. In the example above, if @p _pResultOutput is
   * connected to an operation that needs both the result and the
   * large image, the size of the input queue for the input that
   * receives the image must be at least three (one for the object
   * being processed, one for the buffered object, and one for the
   * next object. The default queue size is three. Thus, delaying one
   * object is always fine.
   *
   * @see endDelay()
   */
  void startDelay();

  /**
   * Informs the synchronization system that a delayed object has now
   * been sent.
   *
   * @see startDelay()
   */
  void endDelay();

  /**
   * Restore socket state after pause. This function will modify @p
   * state according to the current state of this socket and pass the
   * information to all connected input sockets.
   *
   * @param state the state of a synchronized input socket group.
   */
  void resume(PiiSocketState inputState);

  /**
   * Send an object through this output to all connected inputs. This
   * function blocks until all connected synchronous inputs are able to
   * receive the object.
   *
   * @exception PiiExecutionException& if the emission was interrupted
   * by an external signal.
   */
  void emitObject(const PiiVariant& obj);

  /**
   * Create a PiiVariant out of the given @p value and pass it. This
   * is a convenience function that frees you from manually creating
   * the variant. The function calls PiiYdin::createVariant(T). The
   * following code emits a PiiVariant whose type() function returns
   * PiiVariant::IntType.
   *
   * @code
   * output->emitObject(5);
   * @endcode
   */
  template <class T> inline void emitObject(const T& value)
  {
    emitObject(PiiYdin::createVariant(value));
  }


  /**
   * Check if the output is connected. Returns @p true if the output
   * is connected to at least one input that is not a proxy whose
   * output is not connected and @p false otherwise.
   *
   * It is not advised to call this function at run time, because it
   * may be necessary to go through a long sequence of proxy sockets
   * to find the final connection status. One should instead cache the
   * connected status when an operation is checked for execution.
   *
   * @see PiiSocketProxy
   */
  Q_INVOKABLE bool isConnected() const;

  /**
   * Interrupt any ongoing object emission. This function is used when
   * the operation must be cancelled as soon as possible without
   * waiting that receivers are ready.
   */
  void interrupt();

  /**
   * Clear pending emissions.
   */
  void reset();

  void inputReady(PiiAbstractInputSocket* input);

  PiiOutputSocket* socket();
  PiiAbstractInputSocket* asInput();
  PiiAbstractOutputSocket* asOutput();

protected:
  /// @internal
  class Data : public PiiAbstractOutputSocket::Data
  {
  public:
    Data(PiiOutputSocket* owner);
    
    bool setOutputConnected(bool connected);
    PiiAbstractOutputSocket* rootOutput() const;
  
    int iGroupId;
    bool bConnected;
    // A wait condition that is used when some inputs aren't ready to
    // receive new objects.
    PiiWaitCondition outputPending;
    PiiAbstractInputSocket* pFirstInput;
    PiiInputController* pFirstController;
    bool bInterrupted;
    bool *pbInputCompleted;
    PiiSocketState state;
  };
  PII_UNSAFE_D_FUNC;

  /// @internal
  PiiOutputSocket(Data* data, const QString& name);

  void inputConnected(PiiAbstractInputSocket* input);
  void inputDisconnected(PiiAbstractInputSocket* input);
  void inputUpdated(PiiAbstractInputSocket* input);
  
private:
  // Create pbInputCompleted.
  void createFlagArray();
};

Q_DECLARE_METATYPE(PiiOutputSocket*);

#endif //_PIIOUTPUTSOCKET_H
