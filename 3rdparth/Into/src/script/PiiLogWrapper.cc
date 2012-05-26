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

#include <PiiLog.h>

#include "PiiScript.h"

namespace PiiLogWrapper
{
  PII_STATIC_TR_FUNC(PiiLog)

  static QScriptValue piiLog(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_ARGUMENT_COUNT(piiLog, 3);
    ::piiLog(qPrintable(context->argument(0).toString()),
             (QtMsgType)qBound(0, context->argument(1).toInt32(), 3),
             "%s", context->argument(2).toString().toLocal8Bit().constData());
    return engine->undefinedValue();
  }

  static QScriptValue findLogModule(QScriptContext* context)
  {
    if (context == 0)
      return QScriptValue(QScriptValue::UndefinedValue);
    QScriptValue value = context->thisObject().property("PII_LOG_MODULE");
    if (!value.isUndefined())
      return value;
    return findLogModule(context->parentContext());
  }

  static QScriptValue piiLog(QScriptContext* context, QScriptEngine* engine, QtMsgType level)
  {
    QScriptValue module = findLogModule(context);
    QString strModule = module.isUndefined() ? "Into" : module.toString();
    piiLog(qPrintable(strModule),
           level,
           "%s", context->argument(0).toString().toLocal8Bit().constData());
    return engine->undefinedValue();
  }
  
#define PII_SCRIPT_DEBUG_FUNCTION(TYPE) \
  static QScriptValue pii ## TYPE(QScriptContext* context, QScriptEngine* engine) \
  {                                                                     \
    PII_CHECK_ARGUMENT_COUNT(pii ## TYPE, 1);                           \
    return piiLog(context, engine, Qt ## TYPE ## Msg);                  \
  }

  PII_SCRIPT_DEBUG_FUNCTION(Debug);
  PII_SCRIPT_DEBUG_FUNCTION(Warning);
  PII_SCRIPT_DEBUG_FUNCTION(Critical);
  PII_SCRIPT_DEBUG_FUNCTION(Fatal);
}


void initPiiLog(QScriptEngine* engine)
{
#define PII_LOG_WRAPPER_FUNC(NAME) \
  engine->globalObject().setProperty(#NAME, engine->newFunction(PiiLogWrapper::NAME));
  
  PII_LOG_WRAPPER_FUNC(piiLog);
  PII_LOG_WRAPPER_FUNC(piiDebug);
  PII_LOG_WRAPPER_FUNC(piiWarning);
  PII_LOG_WRAPPER_FUNC(piiCritical);
  PII_LOG_WRAPPER_FUNC(piiFatal);  
}
