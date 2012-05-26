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

#include "PiiScript.h"
#include "PiiSocketWrapper.h"

#include <PiiOperation.h>
#include <QScriptValueIterator>

namespace PiiOperationWrapper
{
  PII_STATIC_TR_FUNC(PiiOperation)

  PII_QOBJECT_CONVERSION_FUNCTIONS(PiiOperation);

  static void connect(PiiAbstractOutputSocket* output, PiiOperation* operation, QScriptValue input)
  {
    PiiAbstractInputSocket* pInput = 0;
    if (input.isString())
      pInput = operation->input(input.toString());
    else if (input.isQObject())
      {
        PiiSocket* pSocket = qobject_cast<PiiSocket*>(input.toQObject());
        if (pSocket != 0)
          pInput = pSocket->asInput();
      }

    if (pInput == 0)
      piiWarning("connect(): input \"%s\" does not exist.", qPrintable(input.toString()));
    
    if (output != 0 && pInput != 0)
      output->connectInput(pInput);
  }
  
  static QScriptValue connect(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_ONE_ARGUMENT(connect);
    PII_CHECK_THIS_TYPE(PiiOperation, connect);

    QScriptValue connections = context->argument(0);
    QScriptValueIterator it(connections);
    while (it.hasNext())
      {
        it.next();
        //qDebug("%s : %s", qPrintable(it.name()), qPrintable(it.value().toString()));
        PiiAbstractOutputSocket* pOutput = pThis->output(it.name());
        if (pOutput == 0)
          piiWarning("connect(): output \"%s\" does not exist.", qPrintable(it.name())); 

        QScriptValue currentValue = it.value();
        if (currentValue.isArray())
          {
            int iInputCnt = currentValue.property("length").toInt32();
            for (int i=0; i<iInputCnt; ++i)
              connect(pOutput, pThis, currentValue.property(i));
          }
        else
          connect(pOutput, pThis, currentValue);
      }
    return engine->undefinedValue();
  }
}

void initPiiOperation(QScriptEngine* engine)
{
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiOperationWrapper, PiiOperation);

  // No constructor function for PiiOperation
  QScriptValue operationObject = engine->newQMetaObject(&PiiOperation::staticMetaObject);
  QScriptValue prototype = operationObject.property("prototype");
  prototype.setProperty("connect", engine->newFunction(PiiOperationWrapper::connect));

  engine->setDefaultPrototype(qMetaTypeId<PiiOperation*>(), prototype);

  engine->globalObject().setProperty("PiiOperation", operationObject);
}

