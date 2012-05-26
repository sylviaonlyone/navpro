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

#include "PiiDeclarativeExtensionPlugin.h"
#include <QDeclarativeEngine>
#include <QScriptEngine>

Q_EXPORT_PLUGIN2(piideclarative, PiiDeclarativeExtensionPlugin);

void PiiDeclarativeExtensionPlugin::initializeEngine(QDeclarativeEngine* engine, const char* uri)
{
  Q_UNUSED(uri);

  QScriptEngine* pEngine = findScriptEngine(engine);
  QScriptClass* pOldClass = pEngine->globalObject().scriptClass();
  pEngine->globalObject().setScriptClass(0);

  QScriptValue result = pEngine->importExtension("Into");
  if (pEngine->hasUncaughtException())
    qWarning("%s", qPrintable(result.toString()));
  
  pEngine->globalObject().setScriptClass(pOldClass);
}

void PiiDeclarativeExtensionPlugin::registerTypes(const char* uri)
{
  Q_UNUSED(uri);
}

QScriptEngine* PiiDeclarativeExtensionPlugin::findScriptEngine(QDeclarativeEngine* engine)
{
  struct FakeScriptEngineData
  {
    // we know this address (QScriptEngine::rootContext())
    void *rootContext;
    bool isDebugging;

    bool outputWarningsToStdErr;

    void *contextClass;
    void *sharedContext;
    void *sharedScope;
    void *objectClass;
    void *valueTypeClass;
    void *typeNameClass;
    void *listClass;
    void *globalClass;
    void *cleanup;
    void *erroredBindings;
    int inProgressCreations;
    QScriptEngine scriptEngine;
  };

  // skip vtable and assume each address in the structure is aligned
  // to sizeof(void*)
  void** pPrivateData = reinterpret_cast<void***>(engine)[1];
  void* pRootContext = engine->rootContext();
  for (int i=8; i<32; ++i)
    if (pPrivateData[i] == pRootContext)
      {
        FakeScriptEngineData* pData = reinterpret_cast<FakeScriptEngineData*>(pPrivateData + i);
        return &pData->scriptEngine;
      }
  return 0;
}
