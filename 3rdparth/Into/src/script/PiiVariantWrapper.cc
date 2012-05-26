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

#include "PiiVariantWrapper.h"

#include "PiiScript.h"

#include <PiiVariant.h>
#include <PiiYdinTypes.h>

namespace PiiVariantWrapper
{
  PII_STATIC_TR_FUNC(PiiVariant)

  static QScriptValue createPiiVariant(QScriptContext *context, QScriptEngine *engine)
  {
    PII_CHECK_ARGUMENT_COUNT_RANGE(PiiVariant, 0, 1);
    PiiVariant variant;
    if (context->argumentCount() == 1)
      {
        QScriptValue arg = context->argument(0);
        if (arg.isNumber())
          {
            // int
            if (arg.toNumber() - arg.toInt32() == 0.0)
              variant = PiiVariant(arg.toInt32());
            // double
            else
              variant = PiiVariant(arg.toNumber());
          }
        // QString
        else if (arg.isString())
          variant = PiiVariant(arg.toString());
      }
    engine->newQObject(context->thisObject(),
                       new PiiVariantScriptObject(variant),
                       QScriptEngine::AutoOwnership,
                       PiiScript::defaultWrapOptions);
    return engine->undefinedValue();
  }
  static QScriptValue convertPiiVariantToScriptValue(QScriptEngine* engine, const PiiVariant& var)
  {
    return engine->newQObject(new PiiVariantScriptObject(var),
                              QScriptEngine::AutoOwnership,
                              PiiScript::defaultWrapOptions);
  }
  static void convertScriptValueToPiiVariant(const QScriptValue& scriptObj, PiiVariant& var)
  {
    PiiVariantScriptObject* pWrapper = qobject_cast<PiiVariantScriptObject*>(scriptObj.toQObject());
    if (pWrapper != 0)
      var = pWrapper->variant;
  }
}

void initPiiVariant(QScriptEngine* engine)
{
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiVariantWrapper, PiiVariant);
  engine->globalObject().setProperty("PiiVariant", engine->newFunction(PiiVariantWrapper::createPiiVariant));
}
