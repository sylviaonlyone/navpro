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

#include "PiiScriptExtensionPlugin.h"

#include <QScriptEngine>

#include "PiiScript.h"

#include "PiiSocketWrapper.h"
#include "PiiOperationWrapper.h"
#include "PiiOperationCompoundWrapper.h"
#include "PiiEngineWrapper.h"
#include "PiiVariantWrapper.h"
#include "PiiLogWrapper.h"
#include "PiiQtTypeWrapper.h"

#include <PiiYdinTypes.h>

Q_EXPORT_PLUGIN2(piiscript, PiiScriptExtensionPlugin);

namespace PiiScript
{
  PII_STATIC_TR_FUNC(PiiScript)
    
  static QScriptValue configure(QScriptContext* context, QScriptEngine* /*engine*/)
  {
    PII_CHECK_AT_LEAST_ONE_ARGUMENT(configure);
    for (int i=0; i<context->argumentCount(); ++i)
      configure(context->thisObject(), context->argument(i));
    return context->thisObject();
  }
}

void PiiScriptExtensionPlugin::initialize(const QString& /*key*/, QScriptEngine* engine)
{
  engine->globalObject().setProperty("PiiYdin", engine->newQMetaObject(&PiiYdin::staticMetaObject));
  engine->globalObject().property("Object").property("prototype").setProperty("configure",
                                                                              engine->newFunction(PiiScript::configure));

  initPiiSocket(engine);
  initPiiOperation(engine);
  initPiiOperationCompound(engine);
  initPiiEngine(engine);
  initPiiVariant(engine);
  initPiiLog(engine);
  initQtTypes(engine);
}

QStringList	PiiScriptExtensionPlugin::keys() const
{
  return QStringList() << "Into";
}
