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

#ifndef _PIIPROXYSOCKET_H
#define _PIIPROXYSOCKET_H

#include "PiiYdin.h"
#include "PiiSocket.h"
#include "PiiAbstractInputSocket.h"
#include "PiiAbstractOutputSocket.h"
#include "PiiInputController.h"

/**
 * An input-output socket. This socket implements both
 * PiiAbstractOutputSocket and PiiAbstractInputSocket, and works as a
 * PiiInputController for itself. It merely passes all incoming
 * objects to the output.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiProxySocket :
  public PiiSocket,
  public PiiAbstractOutputSocket,
  public PiiAbstractInputSocket,
  public PiiInputController
{
  Q_OBJECT

public:
  PiiProxySocket();
  ~PiiProxySocket();

  /**
   * Returns @p Proxy.
   */
  Type type() const;
  
  PiiInputController* controller() const;
  PiiAbstractOutputSocket* rootOutput() const;
  bool setInputConnected(bool connected);
  bool tryToReceive(PiiAbstractInputSocket* sender, const PiiVariant& object) throw ();
  void inputReady(PiiAbstractInputSocket*);

  void reset();

  PiiProxySocket* socket();
  PiiAbstractInputSocket* asInput();
  PiiAbstractOutputSocket* asOutput();

protected:
  void inputConnected(PiiAbstractInputSocket* input);
  void inputDisconnected(PiiAbstractInputSocket* input);
  
private:
  class Data :
    public PiiAbstractOutputSocket::Data,
    public PiiAbstractInputSocket::Data
  {
  public:
    Data(PiiProxySocket* owner);

    PiiAbstractOutputSocket* rootOutput() const;
    bool setInputConnected(bool connected);
    
    bool *pbInputCompleted;

    PII_Q_FUNC(PiiProxySocket);
  };
  
  inline Data* _d() { return static_cast<Data*>(PiiAbstractOutputSocket::d); }
  inline const Data* _d() const { return static_cast<const Data*>(PiiAbstractOutputSocket::d); }
};

Q_DECLARE_METATYPE(PiiProxySocket*);

#endif //_PIIPROXYSOCKET_H
