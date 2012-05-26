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

#ifndef _PIIABSTRACTINPUTSOCKET_H
#define _PIIABSTRACTINPUTSOCKET_H

#include "PiiAbstractSocket.h"

class PiiInputController;
class PiiAbstractOutputSocket;

/**
 * An abstract superclass for input sockets. Input sockets work as
 * receivers in inter-operation connections. An input can be connected
 * to only one output socket.
 *
 * This class represents a connection point but provides no means of
 * actually handling the received objects. Received objects are
 * handled by an input controller (#controller()).
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiAbstractInputSocket : public virtual PiiAbstractSocket
{
public:
  virtual ~PiiAbstractInputSocket();

  /**
   * Connects this socket to the given output socket. If this socket
   * is already connected, the old connection will be deleted. This is
   * a utility function that calls
   * PiiAbstractOutputSocket::connectInput().
   */
  void connectOutput(PiiAbstractOutputSocket* output);

  /**
   * Disconnects this socket from its output socket. If this socket is
   * not connected, nothing happens. This is a utility function that
   * calls PiiAbstractOutputSocket::disconnectInput().
   */
  void disconnectOutput();
  
  /**
   * Returns the input controller. Input controller is responsible for
   * handling all incoming objects to this socket. Typically, a single
   * controller is shared by all inputs within an operation. This
   * allows the controller to synchronize inputs with respect to each
   * other.
   */
  virtual PiiInputController* controller() const = 0;

  /**
   * Returns the output this input socket is connected to. If the
   * socket is not connected, 0 will be returned.
   *
   * @return the output that sends objects to this input, or 0 if the
   * socket is not connected
   */
  PiiAbstractOutputSocket* connectedOutput() const;

protected:
  /// @internal
  class PII_YDIN_EXPORT Data
  {
  public:
    Data();
    virtual ~Data();

    /**
     * Set the connection status of this socket to @p connected. The
     * default implementation just returns @p connected.
     *
     * @param connected a flag that indicates whether the sequence of
     * sockets leading to this socket is connected to an operation or
     * not.
     *
     * @return @p true if the socket is connected, @p false otherwise. 
     * If the socket is a proxy, it may be unconnected even after
     * setInputConnected(true) if none of its outgoing connections leads
     * to a connected input.
     */
    virtual bool setInputConnected(bool connected);
    
    PiiAbstractOutputSocket* pConnectedOutput;
  } *d;

  /// @internal
  PiiAbstractInputSocket(Data* d);
  
private:
  // Must be able to set pConnectedOutput and call setInputConnected.
  friend class PiiAbstractOutputSocket;
};

Q_DECLARE_METATYPE(PiiAbstractInputSocket*);

#endif //_PIIABSTRACTINPUTSOCKET_H
