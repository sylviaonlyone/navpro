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

#include "PiiSocketWrapper.h"

#include "PiiScript.h"

#include <PiiOperation.h>
#include <PiiInputSocket.h>
#include <PiiOutputSocket.h>
#include <PiiProxySocket.h>
#include <PiiProbeInput.h>

#define PII_SOCKET_CONVERSION_FUNCTIONS_IMPL(CLASS, DIRECTION)          \
  static QScriptValue convert ## CLASS ## ToScriptValue(QScriptEngine* engine, CLASS* const & socket) \
  {                                                                     \
    return engine->newQObject(socket != 0 ? socket->socket() : 0,       \
                              QScriptEngine::AutoOwnership,             \
                              PiiScript::defaultWrapOptions);           \
  }                                                                     \
  static void convertScriptValueTo ## CLASS(const QScriptValue& obj, CLASS*& socket) \
  {                                                                     \
    PiiSocket* pSocket = qobject_cast<PiiSocket*>(obj.toQObject());     \
    if (pSocket != 0)                                                   \
      socket = pSocket->as ## DIRECTION();                              \
    else                                                                \
      socket = 0;                                                       \
  }

// Creates conversion functions to/from socket pointers of type CLASS
#define PII_SOCKET_CONVERSION_FUNCTIONS(DIRECTION) PII_SOCKET_CONVERSION_FUNCTIONS_IMPL(PiiAbstract ## DIRECTION ## Socket, DIRECTION)

namespace PiiSocketWrapper
{
  PII_STATIC_TR_FUNC(PiiSocket);

  static QScriptValue convertPiiAbstractSocketToScriptValue(QScriptEngine* engine, PiiAbstractSocket* const & socket)
  {
    return engine->newQObject(socket != 0 ? socket->socket() : 0,
                              QScriptEngine::AutoOwnership,
                              PiiScript::defaultWrapOptions);
  }

  static void convertScriptValueToPiiAbstractSocket(const QScriptValue& obj, PiiAbstractSocket*& socket)
  {
    PiiSocket* pSocket = qobject_cast<PiiSocket*>(obj.toQObject());
    if (pSocket != 0)
      {
        socket = pSocket->asInput();
        if (socket == 0)
          socket = pSocket->asOutput();
      }
    else
      socket = 0;
  }

  PII_SOCKET_CONVERSION_FUNCTIONS(Input)
  PII_SOCKET_CONVERSION_FUNCTIONS(Output)
  PII_QOBJECT_CONVERSION_FUNCTIONS(PiiProxySocket)
  PII_QOBJECT_CONVERSION_FUNCTIONS(PiiProbeInput)

  PII_SEQUENCE_CONVERSION_FUNCTIONS(PiiInputSocketList)
  PII_SEQUENCE_CONVERSION_FUNCTIONS(PiiOutputSocketList)

  // Checks that "this" is an output socket and the first argument an
  // input socket. Places the socket pointers to the output arguments.
  static QScriptValue convertSockets(QScriptContext* context,
                                     PiiAbstractOutputSocket*& output,
                                     PiiAbstractInputSocket*& input,
                                     const char* function)
  {
    PII_CHECK_THIS_TYPE(PiiSocket, convertSockets);
    output = pThis->asOutput();
    if (output == 0)
      return context->throwError(tr(PiiScript::pInstanceOfXRequired).arg(function).arg("PiiOutputSocket"));

    PiiSocket* pSocket = qscriptvalue_cast<PiiSocket*>(context->argument(0));
    if (pSocket == 0 || pSocket->type() not_member_of (PiiSocket::Input, PiiSocket::Proxy))
      return context->throwError(tr(PiiScript::pArgumentNMustBeX).arg(function).arg(0).arg("PiiInputSocket"));
    input = pSocket->asInput();
    return QScriptValue();
  }
  
  static QScriptValue connectInput(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_ONE_ARGUMENT(connectInput);
    PiiAbstractOutputSocket* pOutput = 0;
    PiiAbstractInputSocket* pInput = 0;
    QScriptValue result = convertSockets(context, pOutput, pInput, "connectInput");
    if (result.isError())
      return result;
    pOutput->connectInput(pInput);
    return engine->undefinedValue();
  }

  static QScriptValue disconnectInput(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_ONE_ARGUMENT(disconnectInput);
    PiiAbstractOutputSocket* pOutput = 0;
    PiiAbstractInputSocket* pInput = 0;
    QScriptValue result = convertSockets(context, pOutput, pInput, "disconnectInput");
    if (result.isError())
      return result;
    pOutput->disconnectInput(pInput);
    return engine->undefinedValue();
  }

  static QScriptValue connectedInputs(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_THIS_TYPE(PiiSocket, type);
    PII_CHECK_NO_ARGUMENTS(connectedInputs);
    PiiAbstractOutputSocket* pOutput = pThis->asOutput();
    if (pOutput == 0)
      return context->throwError(tr(PiiScript::pInstanceOfXRequired).arg("connectedInputs").arg("PiiOutputSocket"));
    return qScriptValueFromSequence(engine, pOutput->connectedInputs());
  }

  static QScriptValue connectedOutput(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_THIS_TYPE(PiiSocket, type);
    PII_CHECK_NO_ARGUMENTS(connectedOutput);
    PiiAbstractInputSocket* pInput = pThis->asInput();
    if (pInput == 0)
      return context->throwError(tr(PiiScript::pInstanceOfXRequired).arg("connectedOutput").arg("PiiInputSocket"));
    return qScriptValueFromValue(engine, pInput->connectedOutput());
  }
}

void initPiiSocket(QScriptEngine* engine)
{
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiSocketWrapper, PiiAbstractSocket);
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiSocketWrapper, PiiAbstractInputSocket);
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiSocketWrapper, PiiAbstractOutputSocket);
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiSocketWrapper, PiiProxySocket);
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiSocketWrapper, PiiProbeInput);

  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiSocketWrapper, PiiInputSocketList);
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiSocketWrapper, PiiOutputSocketList);
  
  QScriptValue socketObj = engine->newQMetaObject(&PiiSocket::staticMetaObject);
  engine->globalObject().setProperty("PiiSocket", socketObj);

  QScriptValue outputProto = engine->newObject();
  outputProto.setProperty("connectInput", engine->newFunction(PiiSocketWrapper::connectInput));
  outputProto.setProperty("disconnectInput", engine->newFunction(PiiSocketWrapper::disconnectInput));
  outputProto.setProperty("connectedInputs", engine->newFunction(PiiSocketWrapper::connectedInputs));
  engine->setDefaultPrototype(qMetaTypeId<PiiOutputSocket*>(), outputProto);

  QScriptValue inputProto = engine->newObject();
  inputProto.setProperty("connectedOutput", engine->newFunction(PiiSocketWrapper::connectedOutput));
  engine->setDefaultPrototype(qMetaTypeId<PiiInputSocket*>(), inputProto);

  engine->setDefaultPrototype(qMetaTypeId<PiiProbeInput*>(), inputProto);
}
