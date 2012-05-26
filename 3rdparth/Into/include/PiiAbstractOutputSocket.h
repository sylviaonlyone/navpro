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

#ifndef _PIIABSTRACTOUTPUTSOCKET_H
#define _PIIABSTRACTOUTPUTSOCKET_H

#include "PiiAbstractSocket.h"
#include <QVector>
#include <QList>
#include <QPair>

class PiiAbstractInputSocket;
class PiiInputController;

/**
 * An abstract superclass for output sockets. Operations transfer
 * objects to other operations through an output socket. The
 * connection between an output socket and an input socket is
 * one-directional: the output socket pushes data forwards, but the
 * input cannot "pull" new objects. The input can, however, tell the
 * output when it is ready to receive new data via the #inputReady()
 * signal.
 *
 * This class is a representation of a connection point that is able
 * to receive "I'm ready" signals from connected input sockets
 * (#inputReady()). The actual mechanism of passing data is defined
 * in subclasses (PiiOutputSocket, PiiProxySocket).
 *
 * When an input socket is deleted, its connection to an output socket
 * is automatically destroyed and vice-versa.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiAbstractOutputSocket : public virtual PiiAbstractSocket
{
public:
  ~PiiAbstractOutputSocket();
  
  /**
   * Connects this socket to the specified input socket. The new input
   * will be appended to the list of connected inputs.
   *
   * @param input the input socket to connect
   */
  void connectInput(PiiAbstractInputSocket* input);

  /**
   * Updates any cached information related to @p input. This function
   * must be called by a connected input socket whenever its
   * configuration (such as the controller) changes.
   */
  void updateInput(PiiAbstractInputSocket* input);
  
  /**
   * Disconnects this socket from an input. After disconnect, objects
   * emitted through this socket are no longer delivered to this input
   * socket.
   *
   * @param input the input to disconnect from, or 0 if all
   * connections are to be broken.
   */
  void disconnectInput(PiiAbstractInputSocket* input = 0);

  /**
   * Reconnects all inputs currently connected to this output to @p
   * output, and reconnects this output to @p input.
   */
  void reconnect(PiiAbstractOutputSocket* output, PiiAbstractInputSocket* input);

  /**
   * Returns all inputs this socket is connected to.
   */
  QList<PiiAbstractInputSocket*> connectedInputs() const;

  /**
   * Indicates this output socket that @p input is ready to receive
   * new objects. Used in communications between socket pairs.
   */
  virtual void inputReady(PiiAbstractInputSocket* input) = 0;

  /**
   * Finds (backwards) the most distant output connected to this
   * socket through proxies. If the output is connected to an
   * operation, this function returns @p this. Otherwise it goes
   * recursively back through all proxies until it finds an output
   * that is connected to an operation. If there is no such output,
   * returns 0.
   */
  PiiAbstractOutputSocket* rootOutput() const;
  
protected:
  /// @cond null
  typedef QVector<QPair<PiiAbstractInputSocket*,PiiInputController*> > InputListParent;

  class InputList : public InputListParent
  {
  public:
    InputList() {}
    InputList(const InputList& other) : InputListParent(other) {}

    InputList& operator=(const InputList& other) { InputListParent::operator=(other); return *this; }
    
    int indexOf(PiiAbstractInputSocket* input) const;
    PiiAbstractInputSocket* takeInputAt(int index);
    PiiAbstractInputSocket* inputAt(int index) const;
    PiiInputController* controllerAt(int index) const;
    void append(PiiAbstractInputSocket* input);
    void updateController(PiiAbstractInputSocket* input);
  };

  class PII_YDIN_EXPORT Data
  {
  public:
    Data(PiiAbstractOutputSocket* owner);
    virtual ~Data();

    /**
     * Called by the non-virtual function in the public interface.
     */
    virtual PiiAbstractOutputSocket* rootOutput() const = 0;
    
    /**
     * Recursively update connection statuses of input sockets
     * connected to this output.
     *
     * @param connected a flag that indicates whether the sequence of
     * sockets leading to this socket is connected to an operation or
     * not.
     *
     * @return @p true if the socket is connected, @p false otherwise. 
     * If the socket is a proxy, it may be unconnected even after
     * setInputConnected(true) if none of its outgoing connections
     * leads to a connected input.
     */
    virtual bool setOutputConnected(bool connected);

    // All connected input sockets.
    InputList lstInputs;
    PiiAbstractOutputSocket* q;
  } *d;

  friend class Data;

  PiiAbstractOutputSocket(Data* data);
  /// @endcond
  
  /**
   * Called by #connectInput() when an input has been connected.
   * The default implementation does nothing.
   */
  virtual void inputConnected(PiiAbstractInputSocket* input);
  /**
   * Called by #updateInput() when an input has been updated.
   * The default implementation does nothing.
   */
  virtual void inputUpdated(PiiAbstractInputSocket* input);
  /**
   * Called by #disconnectInput() when an input has been disconnected.
   * The default implementation does nothing.
   */
  virtual void inputDisconnected(PiiAbstractInputSocket* input);
  
private:
  void disconnectInputAt(int index);
};

Q_DECLARE_METATYPE(PiiAbstractOutputSocket*);

#endif //_PIIABSTRACTOUTPUTSOCKET_H
