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

#include "PiiOperationCompoundWrapper.h"
#include "PiiEngineWrapper.h"

#include <QScriptContextInfo>

#include <PiiEngine.h>
#include "PiiScript.h"

#include <QDebug>
#include <PiiYdinResources.h>

namespace PiiEngineWrapper
{
  PII_STATIC_TR_FUNC(PiiEngine)
    
  PII_QOBJECT_CONVERSION_FUNCTIONS(PiiEngine);
  PII_QOBJECT_CONSTRUCTOR(PiiEngine);
  
  static QScriptValue convertPiiPluginToScriptValue(QScriptEngine* engine, const PiiEngine::Plugin& plugin)
  {
    QScriptValue result = engine->newObject();
    result.setProperty("resourceName", plugin.resourceName());
    result.setProperty("libraryName", plugin.libraryName());
    result.setProperty("version", plugin.version().toString());
    return result;
  }

  static QScriptValue createPiiOperation(QScriptContext* context, QScriptEngine* engine)
  {
    QScriptValue constructor = context->callee();
    QString strName = constructor.property("className").toString();
    PiiOperation* pOperation = PiiYdin::createResource<PiiOperation>(qPrintable(strName));
    if (pOperation != 0)
      {
        engine->newQObject(context->thisObject(), pOperation, QScriptEngine::AutoOwnership, PiiScript::defaultWrapOptions);
        int iArgs = context->argumentCount();
        int i = 0;
        // If the first argument is a string, it is regarded as objectName.
        if (iArgs > 0 && context->argument(0).isString())
          {
            pOperation->setObjectName(context->argument(0).toString());
            ++i;
          }
        for (; i<iArgs; ++i)
          PiiScript::configure(context->thisObject(), context->argument(i));

        // When the constructor is first invoked, use the operation
        // instance to get meta information and add it to the
        // constructor object.
        if (!constructor.property("__meta__").isValid())
          {
            PiiScript::configure(constructor, engine->newQMetaObject(pOperation->metaObject()));
            constructor.setProperty("__meta__", true);
          }
      }
    return engine->undefinedValue();
  }

  static QScriptValue createOperationConstructor(QScriptEngine* engine, const QString& name, const char* superClass)
  {
    QScriptValue constructor = engine->newFunction(createPiiOperation);
    PiiScript::setSuperclass(constructor.property("prototype"), superClass);
    constructor.setProperty("className", name);
    return constructor;
  }

  static void registerOperations(QScriptEngine* engine, const char* superClass, const QString& parentResource = "")
  {
    PiiResourceDatabase* pDb = PiiYdin::resourceDatabase();
    // Find all operations ...
    QList<QString> lstOperations = pDb->select(Pii::subject, Pii::attribute("pii:class") == superClass);
    
    // ... in the specified plug-in (if any)
    if (!parentResource.isEmpty())
      lstOperations = lstOperations && pDb->select(Pii::subject, Pii::attribute("pii:parent") == parentResource);

    QScriptValue globalObject = engine->globalObject();
    // Create a constructor function for each
    for (int i=0; i<lstOperations.size(); ++i)
      {
        int iTemplateIndex = lstOperations[i].indexOf('<');
        if (iTemplateIndex == -1)
          {
            // No template parameters
            if (!globalObject.property(lstOperations[i]).isObject())
              globalObject.setProperty(lstOperations[i],
                                       createOperationConstructor(engine, lstOperations[i], superClass));
          }
        else
          {
            // Split Operation<T> to Operation and T
            QString strName = lstOperations[i].left(iTemplateIndex);
            QString strTemplateParam = lstOperations[i].mid(iTemplateIndex+1,
                                                            lstOperations[i].size() - iTemplateIndex - 2);
            QScriptValue tmpObj = globalObject.property(strName);
            if (!tmpObj.isObject())
              {
                tmpObj = engine->newObject();
                globalObject.setProperty(strName, tmpObj);
              }
            // Now, Operation<T> can be created with "new Operation.T()"
            tmpObj.setProperty(strTemplateParam,
                               createOperationConstructor(engine, lstOperations[i], superClass));
          }
      }
  }
  
  static void registerOperations(QScriptEngine* engine, const QString& parentResource = "")
  {
    // HACK QML has a read-only global object
    QScriptValue globalObject = engine->globalObject();
    QScriptClass* pOldClass = globalObject.scriptClass();
    if (pOldClass != 0)
      globalObject.setScriptClass(0);

    registerOperations(engine, "PiiOperation", parentResource);
    registerOperations(engine, "PiiOperationCompound", parentResource);
    
    if (pOldClass != 0)
      globalObject.setScriptClass(pOldClass);
  }
  
  static QScriptValue loadPlugins(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_AT_LEAST_ONE_ARGUMENT(loadPlugins);
    for (int i=0; i<context->argumentCount(); ++i)
      if (!context->argument(i).isString())
        return context->throwError(QScriptContext::TypeError,
                                   tr("loadPlugins(): argument %1 is not a string.").arg(i+1));
    try
      {
        QScriptValue lstPlugins = engine->newArray();
        for (int i=0; i<context->argumentCount(); ++i)
          {
            PiiEngine::Plugin plugin = PiiEngine::loadPlugin(context->argument(i).toString());
            registerOperations(engine, plugin.resourceName());
            lstPlugins.setProperty(i, convertPiiPluginToScriptValue(engine, plugin));
          }
        return lstPlugins;
      }
    catch (PiiLoadException& ex)
      {
        return context->throwError(QScriptContext::UnknownError, ex.message());
      }
  }

  static QScriptValue loadPlugin(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_ONE_ARGUMENT(loadPlugin);
    QScriptValue plugins = loadPlugins(context, engine);
    if (plugins.isError())
      return plugins;
    return plugins.property(0);
  }

  static QScriptValue execute(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_THIS_TYPE(PiiEngine, execute);
    try
      {
        pThis->execute();
      }
    catch (PiiExecutionException& ex)
      {
        return context->throwError(ex.message());
      }
    return engine->undefinedValue();
  }

  static QScriptValue save(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_THIS_TYPE(PiiEngine, save);
    PII_CHECK_ARGUMENT_COUNT_RANGE(save, 1, 3);

    QVariantMap mapConfig;
    PiiEngine::FileFormat format = PiiEngine::TextFormat;
    if (context->argumentCount() == 3)
      format = (PiiEngine::FileFormat)context->argument(2).toInt32();
    if (context->argumentCount() > 1)
      mapConfig = PiiScript::objectToMap(context->argument(1));
    try
      {
        pThis->save(context->argument(0).toString(),
                    mapConfig,
                    format);
      }
    catch (PiiSerializationException& ex)
      {
        // PENDING At least GCC seems to determine the type of ex
        // incorrectly. This block is never reached.
        QString strMessage(ex.message());
        if (!ex.info().isEmpty())
          strMessage += "(" + ex.info() + ")";
        return context->throwError(strMessage);
      }
    catch (PiiException& ex)
      {
        return context->throwError(ex.message());
      }
    return engine->undefinedValue();
  }

  static QScriptValue load(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_ARGUMENT_COUNT_RANGE(load, 1, 2);

    QVariantMap mapConfig;
    try
      {
        PiiEngine* pEngine = PiiEngine::load(context->argument(0).toString(),
                                             &mapConfig);
        if (context->argumentCount() == 2)
          PiiScript::configure(context->argument(1), mapConfig);
        return engine->newQObject(pEngine,
                                  QScriptEngine::AutoOwnership,
                                  PiiScript::defaultWrapOptions);
      }
    catch (PiiSerializationException& ex)
      {
        QString strMessage(ex.message());
        if (!ex.info().isEmpty())
          strMessage += "(" + ex.info() + ")";
        return context->throwError(strMessage);
      }
    catch (PiiException& ex)
      {
        return context->throwError(ex.message());
      }
  }
  
  static QScriptValue plugins(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_NO_ARGUMENTS(plugins);
    QList<PiiEngine::Plugin> lstPlugins = PiiEngine::plugins();
    QScriptValue lstResult = engine->newArray();
    for (int i=0; i<lstPlugins.size(); ++i)
      lstResult.setProperty(i, convertPiiPluginToScriptValue(engine, lstPlugins[i]));
    return lstResult;
  }

  static QScriptValue pluginLibraryNames(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_NO_ARGUMENTS(pluginLibraryNames);
    return qScriptValueFromSequence(engine, PiiEngine::pluginLibraryNames());
  }

  static QScriptValue pluginResourceNames(QScriptContext* context, QScriptEngine* engine)
  {
    PII_CHECK_NO_ARGUMENTS(pluginResourceNames);
    return qScriptValueFromSequence(engine, PiiEngine::pluginResourceNames());
  }
};

void initPiiEngine(QScriptEngine* engine)
{
  PII_REGISTER_SCRIPT_TYPE_CONVERSION(PiiEngineWrapper, PiiEngine);
  
  QScriptValue prototype = engine->newObject();
  PiiScript::setSuperclass(prototype, "PiiOperationCompound");
  prototype.setProperty("execute", engine->newFunction(PiiEngineWrapper::execute));
  prototype.setProperty("save", engine->newFunction(PiiEngineWrapper::save));
  
  QScriptValue constructor = engine->newFunction(PiiEngineWrapper::createPiiEngine, prototype);
  PiiScript::configure(constructor, engine->newQMetaObject(&PiiEngine::staticMetaObject));
  constructor.setProperty("loadPlugins", engine->newFunction(PiiEngineWrapper::loadPlugins));
  constructor.setProperty("loadPlugin", engine->newFunction(PiiEngineWrapper::loadPlugin));
  constructor.setProperty("load", engine->newFunction(PiiEngineWrapper::load));
  constructor.setProperty("plugins", engine->newFunction(PiiEngineWrapper::plugins));
  constructor.setProperty("pluginLibraryNames", engine->newFunction(PiiEngineWrapper::pluginLibraryNames));
  constructor.setProperty("pluginResourceNames", engine->newFunction(PiiEngineWrapper::pluginResourceNames));

  engine->setDefaultPrototype(qMetaTypeId<PiiEngine*>(), prototype);
  engine->globalObject().setProperty("PiiEngine", constructor);

  PiiEngineWrapper::registerOperations(engine);
}
