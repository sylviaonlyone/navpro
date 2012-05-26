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

#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QScriptClass>
#include <QApplication>
#include <QMainWindow>
#include <QFile>
#include <QTimer>
#include <QDebug>

#include <QDeclarativeView>
#include <QDeclarativeEngine>

#include <iostream>

#include <PiiSmartPtr.h>

#include "VisibilityChecker.h"

using std::cout;
using std::cerr;

static QScriptValue importExtension(QScriptContext* context, QScriptEngine* engine)
{
  // QML HACK
  QScriptClass* pOldClass = engine->globalObject().scriptClass();
  if (pOldClass != 0)
    engine->globalObject().setScriptClass(0);

  if (context->argumentCount() != 1)
    return context->throwError("importExtension() takes exactly one argument.");
  if (!context->argument(0).isString())
    return context->throwError(QScriptContext::TypeError, "importExtension(): argument must be a string.");
  QScriptValue result = engine->importExtension(context->argument(0).toString());

  if (pOldClass != 0)
    engine->globalObject().setScriptClass(pOldClass);
  return result;
}

static QScriptValue importedExtensions(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() != 0)
    return context->throwError("importedExtensions() takes no arguments.");
  return qScriptValueFromSequence(engine, engine->importedExtensions());
}

static QScriptValue exitScript(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() != 1)
    return context->throwError("exit() takes exactly one argument.");
  if (!context->argument(0).isNumber())
    return context->throwError(QScriptContext::TypeError, "exit(): argument must be a number.");
  engine->abortEvaluation(context->argument(0));
  return engine->undefinedValue();
}

static void configureEngine(QScriptEngine* engine, int argc, char* argv[], const QStringList& extensions)
{
  // QML HACK
  QScriptClass* pOldClass = engine->globalObject().scriptClass();
  if (pOldClass != 0)
    engine->globalObject().setScriptClass(0);

  // Augment the global object with command line arguments
  QScriptValue lstArgs = engine->newArray();
  for (int i=0; i<argc; ++i)
    lstArgs.setProperty(quint32(i), argv[i]);
  engine->globalObject().setProperty("args", lstArgs);

  // Add a couple of new functions.
  engine->globalObject().setProperty("importExtension", engine->newFunction(importExtension));
  engine->globalObject().setProperty("importedExtensions", engine->newFunction(importedExtensions));

  for (int i=0; i<extensions.size(); ++i)
    {
      QScriptValue result = engine->importExtension(extensions[i]);
      if (!result.isUndefined())
        {
          cerr << qPrintable(result.toString()) << "\n";
          exit(1);
        }
    }
  
  if (pOldClass != 0)
    engine->globalObject().setScriptClass(pOldClass);
}

static QScriptEngine* findScriptEngine(QDeclarativeEngine* engine)
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

static void usage(int status = 1)
{
  cerr <<
    "Usage: jsrunner [options] [script_filename] [arguments]\n"
    "\n"
    "Options\n"
    "    -h, --help\n"
    "        Display this information.\n"
    "    -d, --debug\n"
    "        Attach a debugger to the script.\n"
    "    -i, --import-extension name\n"
    "        Import a script extension. Multiple -i options are allowed.\n"
    "    -l, --list-extensions\n"
    "        List available extensions.\n"
    "   --qml\n"
    "        Run the script as qml. By default, JavaScript will be assumed\n"
    "        unless script_filename ends with .qml.\n";
  exit(status);
}

int main(int argc, char* argv[])
{
  bool bDebug = false, bQml = false;
  int iParamIndex = 1;
  QStringList lstExtensions;

#define IF_ARG(A,B) if (!strcmp(argv[iParamIndex], A) || !strcmp(argv[iParamIndex], B))
#define NEXT_ARG if (++iParamIndex >= argc) usage()

  for (; iParamIndex < argc; ++iParamIndex)
    {
      IF_ARG("-h", "--help")
        usage(0);
      else IF_ARG("-d", "--debug")
        bDebug = true;
      else IF_ARG("-i", "--import-extension")
        {
          NEXT_ARG;
          lstExtensions << argv[iParamIndex];
        }
      else IF_ARG("-l", "--list-extensions")
        {
          QCoreApplication app(argc, argv);
          QScriptEngine engine;
          QStringList lstExtensions = engine.availableExtensions();
          for (int i=0; i<lstExtensions.size(); ++i)
            cout << qPrintable(lstExtensions[i]) << "\n";
          return 0;
        }
      else if(!strcmp(argv[iParamIndex], "--qml"))
        bQml = true;
      else
        break;
    }

  if (iParamIndex == argc)
    usage();

  QString strFileName(argv[iParamIndex]);
  if (strFileName.endsWith(".qml"))
    bQml = true;

  QString strScript;

  int iStatus = 0;
 
  if (bQml)
    {
      QApplication app(argc, argv);

      QDeclarativeView view;
      QScriptEngine* pEngine = findScriptEngine(view.engine());
      if (pEngine == 0)
        {
          cerr << "EPIC FAIL!\n";
          return 1;
        }

      configureEngine(pEngine, argc - iParamIndex, argv + iParamIndex, lstExtensions);
      PiiSmartPtr<QScriptEngineDebugger> pDebugger;
      if (bDebug)
        {
          pDebugger = new QScriptEngineDebugger;
          pDebugger->attachTo(pEngine);
        }
      
      view.setSource(QUrl::fromLocalFile(strFileName));
      view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
      view.show();
      iStatus = app.exec();
    }
  else
    {
      QFile file;
      if (strFileName == "-")
        {
          if (!file.open(stdin, QIODevice::ReadOnly))
            {
              cerr << "Cannot open standard input for reading.\n";
              return 1;
            }
        }
      else
        {
          file.setFileName(strFileName);
          if (!file.open(QIODevice::ReadOnly))
            {
              cerr << "Cannot open " << argv[iParamIndex] << " for reading.\n";
              return 1;
            }
        }
      int iFirstLine = 1;
      if (file.peek(2) == "#!") // get rid of shebang
        {
          file.readLine();
          iFirstLine = 2;
        }
      strScript = file.readAll();
      file.close();

      PiiSmartPtr<QCoreApplication> pApp(bDebug  ?
                                         new QApplication(argc, argv) :
                                         new QCoreApplication(argc, argv));
      QScriptEngine engine;
      
      configureEngine(&engine, argc - iParamIndex, argv + iParamIndex, lstExtensions);

      engine.globalObject().setProperty("exit", engine.newFunction(exitScript));
  
      if (bDebug)
        {
          QScriptEngineDebugger debugger;
          debugger.attachTo(&engine);
          engine.evaluate(strScript, argv[iParamIndex], iFirstLine);
          QTimer timer;
          timer.setInterval(300);
          VisibilityChecker checker(&debugger);
          QObject::connect(&timer, SIGNAL(timeout()), &checker, SLOT(check()));
          timer.start();
          iStatus = pApp->exec();
        }
      else
        {
          QScriptValue result = engine.evaluate(strScript, argv[iParamIndex], iFirstLine);
          if (engine.hasUncaughtException())
            {
              cerr << argv[iParamIndex] << ":" << engine.uncaughtExceptionLineNumber() << ": " <<
                qPrintable(result.toString()) << "\n";
              iStatus = 1;
            }
          if (result.isNumber())
            iStatus = result.toInt32();
        }
    }
  
  return iStatus;
}
