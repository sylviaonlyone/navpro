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

#include "PiiOperationWrapper.h"
#include "PiiOperationCompoundWrapper.h"

#include "PiiScript.h"

#include <PiiOperationCompound.h>


namespace PiiOperationCompoundWrapper
{
  PII_STATIC_TR_FUNC(PiiOperationCompound)

  PII_QOBJECT_CONVERSION_FUNCTIONS(PiiOperationCompound)
  PII_QOBJECT_CONSTRUCTOR(PiiOperationCompound)
    
  PII_SEQUENCE_CONVERSION_FUNCTIONS(PiiOperationList)

  static QScriptValue addOperations(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_THIS_TYPE(PiiOperationCompound, addOperations);
    PII_CHECK_AT_LEAST_ONE_ARGUMENT(addOperations);

    for (int i=0; i<context->argumentCount(); ++i)
      {
        PiiOperation* pOperation;
        PII_CHECK_ARGUMENT_TYPE(addOperations, i, pOperation, PiiOperation);
        pThis->addOperation(pOperation);
      }
    return engine->undefinedValue();
  }


  struct InputExposer
  {
    typedef PiiAbstractInputSocket SocketType;
    static PiiSocket::Type type() { return PiiSocket::Input; }
    static const char* errorMsg() { return QT_TR_NOOP("%1(): argument 1 must be either a valid input socket name or an instance of PiiInputSocket."); }
    static SocketType* toSocket(PiiOperationCompound* compound, QScriptValue value)
    {
      if (value.isString())
        return compound->input(value.toString());
      PiiSocket* pSocket = qscriptvalue_cast<PiiSocket*>(value);
      if (pSocket == 0 || pSocket->type() not_member_of (PiiSocket::Input, PiiSocket::Proxy))
        return 0;
      return pSocket->asInput();
    }
    static void expose(PiiOperationCompound* compound,
                       SocketType* socket,
                       const QString& alias,
                       PiiOperationCompound::ConnectionType connectionType)
    {
      compound->exposeInput(socket, alias, connectionType);
    }
    static void unexpose(PiiOperationCompound* compound, SocketType* socket)
    {
      compound->unexposeInput(socket);
    }
  };

  struct OutputExposer
  {
    typedef PiiAbstractOutputSocket SocketType;
    static PiiSocket::Type type() { return PiiSocket::Output; }
    static const char* errorMsg() { return QT_TR_NOOP("%1(): argument 1 must be either a valid output socket name or an instance of PiiOutputSocket."); }
    static SocketType* toSocket(PiiOperationCompound* compound, QScriptValue value)
    {
      if (value.isString())
        return compound->output(value.toString());
      PiiSocket* pSocket = qscriptvalue_cast<PiiSocket*>(value);
      if (pSocket == 0 || pSocket->type() not_member_of (PiiSocket::Output, PiiSocket::Proxy))
        return 0;
      return pSocket->asOutput();
    }
    static void expose(PiiOperationCompound* compound,
                       SocketType* socket,
                       const QString& alias,
                       PiiOperationCompound::ConnectionType connectionType)
    {
      compound->exposeOutput(socket, alias, connectionType);
    }
    static void unexpose(PiiOperationCompound* compound, SocketType* socket)
    {
      compound->unexposeOutput(socket);
    }
  };

  template <class Exposer> static QScriptValue expose(QScriptContext* context,
                                                      QScriptEngine* engine,
                                                      const char* function)
  {
    PiiOperationCompound* pThis = qscriptvalue_cast<PiiOperationCompound*>(context->thisObject());
    if (pThis == 0)
      return context->throwError(tr(PiiScript::pInstanceOfXRequired).arg(function).arg("PiiOperationCompound"));
    if (context->argumentCount() < 2 || context->argumentCount() > 3)
      return context->throwError(tr(PiiScript::pTakesNArguments).arg(function).arg("2-3"));
    PiiOperationCompound::ConnectionType connectionType = PiiOperationCompound::ProxyConnection;
    if (context->argumentCount() == 3)
      {
        if (!context->argument(2).isNumber())
          return context->throwError(tr(PiiScript::pArgumentNMustBeX).arg(function).arg(3).arg("number"));
        connectionType = (PiiOperationCompound::ConnectionType)context->argument(2).toInt32();
      }

    // Convert the first argument to a socket pointer.
    typename Exposer::SocketType* pSocket = Exposer::toSocket(pThis, context->argument(0));
    if (pSocket == 0)
      return context->throwError(tr(Exposer::errorMsg()).arg(function));
    // Expose the socket
    Exposer::expose(pThis, pSocket, context->argument(1).toString(), connectionType);
    return engine->undefinedValue();
  }

  static QScriptValue exposeInput(QScriptContext* context, QScriptEngine* engine)
  {
    return expose<InputExposer>(context, engine, "exposeInput");
  }

  static QScriptValue exposeOutput(QScriptContext* context, QScriptEngine* engine)
  {
    return expose<OutputExposer>(context, engine, "exposeOutput");
  }

  template <class Exposer> static QScriptValue unexpose(QScriptContext* context,
                                                        QScriptEngine* engine,
                                                        const char* function)
  {
    PiiOperationCompound* pThis = qscriptvalue_cast<PiiOperationCompound*>(context->thisObject());
    if (pThis == 0)
      return context->throwError(tr(PiiScript::pInstanceOfXRequired).arg(function).arg("PiiOperationCompound"));
    if (context->argumentCount() != 1)
      return context->throwError(tr(PiiScript::pTakesOneArgument).arg(function));

    // Convert the first argument to a socket pointer.
    typename Exposer::SocketType* pSocket = Exposer::toSocket(pThis, context->argument(0));
    if (pSocket == 0)
      return context->throwError(tr(Exposer::errorMsg()).arg(function));
    // Unexpose the socket
    Exposer::unexpose(pThis, pSocket);
    return engine->undefinedValue();
  }

  static QScriptValue unexposeInput(QScriptContext* context, QScriptEngine* engine)
  {
    return unexpose<InputExposer>(context, engine, "unexposeInput");
  }

  static QScriptValue unexposeOutput(QScriptContext* context, QScriptEngine* engine)
  {
    return unexpose<OutputExposer>(context, engine, "unexposeOutput");
  }
}

void initPiiOperationCompound(QScriptEngine* engine)
{
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiOperationCompoundWrapper, PiiOperationCompound);
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiOperationCompoundWrapper, PiiOperationList);

  QScriptValue prototype = engine->newObject();
  PiiScript::setSuperclass(prototype, "PiiOperation");
  prototype.setProperty("addOperations", engine->newFunction(PiiOperationCompoundWrapper::addOperations));
  prototype.setProperty("exposeInput", engine->newFunction(PiiOperationCompoundWrapper::exposeInput));
  prototype.setProperty("exposeOutput", engine->newFunction(PiiOperationCompoundWrapper::exposeOutput));
  prototype.setProperty("unexposeInput", engine->newFunction(PiiOperationCompoundWrapper::unexposeInput));
  prototype.setProperty("unexposeOutput", engine->newFunction(PiiOperationCompoundWrapper::unexposeOutput));
  
  QScriptValue constructor = engine->newFunction(PiiOperationCompoundWrapper::createPiiOperationCompound, prototype);
  PiiScript::configure(constructor, engine->newQMetaObject(&PiiOperationCompound::staticMetaObject));

  engine->setDefaultPrototype(qMetaTypeId<PiiOperationCompound*>(), prototype);
  engine->globalObject().setProperty("PiiOperationCompound", constructor);
}
