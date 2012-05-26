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

#ifndef _PIISCRIPT_H
#define _PIISCRIPT_H

#include <QScriptEngine>
#include <QCoreApplication>

/// Creates conversion functions to/from pointers of type CLASS
#define PII_QOBJECT_CONVERSION_FUNCTIONS(CLASS)                         \
  static QScriptValue convert ## CLASS ## ToScriptValue(QScriptEngine* engine, CLASS* const & ptr) \
  {                                                                     \
    return engine->newQObject(ptr, QScriptEngine::AutoOwnership, PiiScript::defaultWrapOptions); \
  }                                                                     \
  static void convertScriptValueTo ## CLASS(const QScriptValue& obj, CLASS*& ptr) \
  {                                                                     \
    ptr = qobject_cast<CLASS*>(obj.toQObject());                         \
  }

/**
 * Creates conversion functions to/from a sequence type LIST_TYPE and
 * uses NAME as the human-readable name. An example:
 * typedef QList<PiiSocket*> PiiSocketList;
 * Q_DECLARE_METATYPE(PiiSocketList);
 * PII_SEQUENCE_CONVERSION_FUNCTIONS(PiiSocketList)
 */
#define PII_SEQUENCE_CONVERSION_FUNCTIONS(TYPE_NAME)                    \
  static QScriptValue convert ## TYPE_NAME ## ToScriptValue(QScriptEngine* engine, TYPE_NAME const & list) \
  {                                                                     \
    return qScriptValueFromSequence(engine, list);                      \
  }                                                                     \
  static void convert ## ScriptValueTo ## TYPE_NAME(const QScriptValue& obj, TYPE_NAME& list) \
  {                                                                     \
    qScriptValueToSequence(obj, list);                                  \
  }

/**
 * Registers CLASS as an automatically converted type to the script
 * engine. The conversion functions must be defined with
 * PII_QOBJECT_CONVERSION_FUNCTIONS or
 * PII_SEQUENCE_CONVERSION_FUNCTIONS.
 */
#define PII_REGISTER_SCRIPT_TYPE_CONVERSION(NS, CLASS)                  \
  qScriptRegisterMetaType(engine, NS::convert ## CLASS  ## ToScriptValue, NS::convertScriptValueTo ## CLASS)

// Creates a constructor for CLASS.
#define PII_QOBJECT_CONSTRUCTOR(CLASS)                                  \
  static QScriptValue create ## CLASS(QScriptContext* context,          \
                                      QScriptEngine* engine)            \
  {                                                                     \
    engine->newQObject(context->thisObject(),                           \
                       new CLASS,                                       \
                       QScriptEngine::AutoOwnership,                    \
                       PiiScript::defaultWrapOptions);                  \
    return engine->undefinedValue();                                    \
  }

/// Creates a constructor function for CLASS to the script engine.
#define PII_CREATE_SCRIPT_CONSTRUCTOR(CLASS) \
  engine->globalObject().setProperty(#CLASS, \
                                     engine->newQMetaObject(&CLASS::staticMetaObject, \
                                                            engine->newFunction(create ## CLASS)))

/// Checks that the "this" object in current context is an instance of
/// CLASS.
#define PII_CHECK_THIS_TYPE(CLASS, FUNCTION)                            \
  CLASS* pThis = qscriptvalue_cast<CLASS*>(context->thisObject());      \
  if (pThis == 0)                                                       \
    return context->throwError(tr(PiiScript::pInstanceOfXRequired).arg(#FUNCTION).arg(#CLASS))

/// Ensures that the function was called without arguments.
#define PII_CHECK_NO_ARGUMENTS(FUNCTION)                                \
  if (context->argumentCount() != 0)                                    \
    return context->throwError(tr(PiiScript::pTakesNoArguments).arg(#FUNCTION))

/// Ensures that there is exactly one argument in the calling context.
#define PII_CHECK_ONE_ARGUMENT(FUNCTION)                                \
  if (context->argumentCount() != 1)                                    \
    return context->throwError(tr(PiiScript::pTakesOneArgument).arg(#FUNCTION))

/// Ensures that there is at least one argument in the calling context.
#define PII_CHECK_AT_LEAST_ONE_ARGUMENT(FUNCTION)                       \
  if (context->argumentCount() == 0)                                    \
    return context->throwError(tr(PiiScript::pTakesAtLeastOneArgument).arg(#FUNCTION))

/// Ensures that there is exactly one argument in the calling context.
#define PII_CHECK_ARGUMENT_COUNT(FUNCTION, ARGS)                        \
  if (context->argumentCount() != ARGS)                                 \
    return context->throwError(tr(PiiScript::pTakesNArguments).arg(#FUNCTION).arg(ARGS))

/// Ensures that the argument count is in [MIN,MAX].
#define PII_CHECK_ARGUMENT_COUNT_RANGE(FUNCTION, MIN, MAX)              \
  if (context->argumentCount() < MIN || context->argumentCount() > MAX) \
    return context->throwError(tr(PiiScript::pTakesNArguments).arg(#FUNCTION).arg(#MIN "-" #MAX))

/**
 * Returns an error object that says that the argument INDEX+1 of
 * FUNCTION() must be CLASS_NAME.
 */
#define PII_INVALID_ARGUMENT_TYPE(FUNCTION, INDEX, CLASS_NAME)          \
  return context->throwError(tr(PiiScript::pArgumentNMustBeX).arg(#FUNCTION).arg(INDEX+1).arg(CLASS_NAME))

/**
 * Tries to cast the INDEXth argument of current context to CLASS.
 * Stores the return value to VARIABLE, and returns an error object on
 * failure.
 */
#define PII_CHECK_ARGUMENT_TYPE(FUNCTION, INDEX, VARIABLE, CLASS)       \
  VARIABLE = qscriptvalue_cast<CLASS*>(context->argument(INDEX));       \
  if (VARIABLE == 0)                                                    \
    PII_INVALID_ARGUMENT_TYPE(FUNCTION, INDEX, #CLASS)


/// Defines a static tr() function that uses CLASS as the translation context.
#define PII_STATIC_TR_FUNC(CLASS) \
  static inline QString tr(const char* text)                \
  {                                                         \
    return QCoreApplication::translate(#CLASS, text);       \
  }


namespace PiiScript
{
  extern const char
    *pInstanceOfXRequired,
    *pTakesNoArguments,
    *pTakesOneArgument,
    *pTakesAtLeastOneArgument,
    *pTakesNArguments,
    *pArgumentNMustBeX;

  extern const QScriptEngine::QObjectWrapOptions defaultWrapOptions;

  /**
   * Sets the @a __proto__ property of the given @a prototype object
   * to be the prototype of a global object called @a superClass. This
   * function is used in building class hierarchies. The preferred
   * pattern of creating a new script class is as follows:
   *
   * @code
   * // Create a prototype object
   * QScriptValue prototype = engine->newObject();
   * // MyOperation drives from piiOperation
   * PiiScript::setSuperclass(prototype, "PiiOperation");
   * // Create a constructor function (createMyOperation is a native function)
   * QScriptValue constructor = engine->newFunction(createMyOperation, prototype);
   * // Use the prototype also for classes created in C++.
   * engine->setDefaultPrototype(qMetaTypeId<MyOperation*>(), prototype);
   * // Place the constructor into the global scope
   * engine->globalObject().setProperty("MyOperation", constructor);
   * @endcode   
   */
  void setSuperclass(QScriptValue prototype, const char* superclass);

  /**
   * Copies all properies of @a source to @a target.
   */
  void configure(QScriptValue target, QScriptValue source);

  /**
   * Stores the key-value pairs in @a map as properties to @a object.
   * Returns @a object.
   */
  QScriptValue configure(QScriptValue object, const QVariantMap& map);

  /**
   * Stores the properties of @a object to a variant map and returns
   * the map.
   */
  QVariantMap objectToMap(QScriptValue object);

  /**
   * Stores the key-value pairs in @a map as properties in a script
   * object and returns the object. The new object will be attached to
   * @a engine.
   */
  QScriptValue mapToObject(QScriptEngine* engine, const QVariantMap& map);
}

#endif //_PIISCRIPT_H
