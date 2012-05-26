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

#error "Documentation.h is not intended for inclusion. Its sole purpose is to provide documentation for a package."

/**
 * @defgroup Script JavaScript API
 *
 * The scripting API in Into builds on QtScript, an implementation of
 * the ECMA-262 standard (JavaScript). To get started, one should be
 * familiar with JavaScript and the QtScript API. The purpose of this
 * documentation is to merely describe the scripting API Into provides
 * for application programmers.
 *
 * @par Enabling Into JavaScript API
 *
 * Into JavaScript API is implemented as a script extension plug-in
 * (QScriptExtensionPlugin). To use Into-specific features the plug-in
 * must be imported to a QScriptEngine. The script extension plug-in
 * must be found in Qt plug-in search path. The simplest way to ensure
 * this is to place the plug-in library (piiscript.dll or
 * libpiiscript.so) in a folder called @p script under the folder
 * where the application binary is. If your application is at
 * path/myapp (path\myapp), the plug-in should be located at
 * path/myapp/script/libpiiscript.so
 * (path\myapp\script\piiscript.dll). Another alternative is to use
 * QCoreApplication::addLibraryPath().
 *
 * Once the plug-in library is in place, accessing the Into JavaScript
 * API from a C++ program is easy:
 *
 * @code
 * QScriptEngine engine;
 * engine.importExtension("Into");
 * // strScript is the JavaScript code to be evaluated
 * engine.evaluate(strScript);
 * @endcode
 *
 * Note that JavaScript programs cannot be executed natively. That is,
 * there must always be a C++ program that first constructs a
 * QScriptEngine, loads the Into script extension, and calls
 * engine.evaluate() with the script to be evaluated as its parameter. 
 * Into however comes with a command-line tool, @p jsrunner, for
 * running scripts stored in .js files. Its usage in Into command
 * prompt is straightforward:
 *
 * @code
 * jsrunner -i Into scriptfile.js
 * @endcode
 *
 * The -i option loads Into extensions before executing the script. 
 * Type <tt>jsrunner -h</tt> for a complete list of command-line
 * options.
 *
 *
 * @par Common Behavior of Into JavaScript Objects
 *
 * All signals, slots, and properties of QObject -based classes are
 * available in JavaScript applications. This means, that you can
 *
 * @li access any property declared as a SCRIPTABLE Q_PROPERTY of any
 * QObject -based class as a JavaScript object property.
 *
 * @li access the enumerations of a class as read-only properties of
 * the constructor function.
 *
 * @li call all slot functions from script code
 *
 * @li connect signals defined in C++ code to functions defined in
 * script code or to slots defined in C++ classes.
 *
 * @code
 * // Slot function
 * var showState = function(state)
 * {
 *   // Access enum value as a property of the constructor function
 *   if (state == PiiOperation.Stopped)
 *     piiDebug("The operation stopped.");
 * }
 *
 * // Create an instance of PiiObjectCounter
 * var operation = new PiiObjectCounter();
 * // Access a property
 * operation.objectName = 'counter';
 * // Connect its stateChanged signal to a script function
 * operation.stateChanged.connect(showState);
 * @endcode
 *
 * @note Enumerations of dynamically loaded operations are available
 * only after at least one instance of the operation has been created.
 * The values can always be given as strings however.
 *
 * @code
 * var threshold = new PiiThresholdingOperation();
 * // MeanStdThreshold cannot be given as a constructor parameter
 * // because it only exists after the instance has been created.
 * threshold.thresholdType = PiiThresholdingOperation.MeanStdThreshold;
 *
 * // Strings can always be used.
 * var threshold = new PiiThresholdingOperation({ thresholdType: 'MeanStdThreshold' });
 * @endcode
 *
 * The classes and functions (except signals and slots) available in
 * Into JavaScript API are listed below. Unless stated otherwise, the
 * interface to the functions is similar to the C++ interface. All
 * functions that are @p static class members in C++ code appear as
 * properties of the constructor function in JavaScript code, but not
 * as members of the object instance.
 *
 * @code
 * var engine = new PiiEngine();
 * engine.loadPlugin('piibase');     // won't work
 * PiiEngine.loadPlugin('piibase');  // works, loadPlugin is static
 * piiDebug(engine.BinaryFormat);    // won't work, enums are members of the constructor only
 * piiDebug(PiiEngine.BinaryFormat); // works
 * @endcode
 *
 * @par PiiOperation
 *
 * Being an abstract class, PiiOperation has no constructor
 * function. PiiOperation is an object in the global scope. The
 * members of the PiiOperation::State enumeration are stored as its
 * read-only properties.
 *
 * @li PiiOperation::clone()
 * @li connect()<br>This function has no C++ correspondent. It takes a
 * single JavaScript object as an argument. See examples below.
 * @li PiiOperation::connectOutput()
 * @li PiiOperation::disconnectAllInputs()
 * @li PiiOperation::disconnectAllOutputs()
 * @li PiiOperation::input()
 * @li PiiOperation::inputCount()
 * @li PiiOperation::inputNames()
 * @li PiiOperation::inputs()
 * @li PiiOperation::output()
 * @li PiiOperation::outputCount()
 * @li PiiOperation::outputNames()
 * @li PiiOperation::outputs()
 * @li PiiOperation::socketName()
 * @li PiiOperation::socketProperty()
 * @li PiiOperation::state()
 *
 * The connect() function takes a JavaScript object as an argument
 * Each associated property in the object is a name of an output, and
 * the value of the property is the name of the output to which the
 * input will be connected, or an array of such inputs. For example,
 * <tt>engine.connect({'reader.image': 'writer.image'})</tt> finds the
 * @p image output of a sub-operation whose @p objectName is @p
 * reader, and connects it to the @p image input of a sub-operation
 * whose @p objectName is @p writer. If the value of a property in the
 * connection object is an array, the output will be connected to all
 * specified by the elements of the array. Each input can be either a
 * string (name of an input) or an instance of PiiSocket.
 *
 * @code
 * engine.connect
 * ({
 *    // Connect named output to named input
 *    'trigger.trigger': 'reader.trigger',
 *    // Connect named output to many inputs
 *    'reader.image': [ 'threshold.image', 'writer.image' ],
 *    // Connect named output to an instance of PiiSocket
 *    'threshold.image': engine.morph.input('image')
 * });
 * @endcode
 *
 *
 * @par PiiOperationCompound
 *
 * PiiOperationCompound has a global constructor function. The members
 * of the PiiOperationCompound::ConnectionType enumeration are stored
 * as its read-only properties. Other functions available to scripts
 * are:
 *
 * @li PiiOperationCompound::addOperation()
 * @li addOperations()<br>This function has no C++ correspondent. It
 * works analogously to addOperation() but can take an unlimited
 * number of operations as parameters.
 * @li PiiOperationCompound::childCount()
 * @li PiiOperationCompound::childOperations()
 * @li PiiOperationCompound::clear()
 * @li PiiOperationCompound::createOperation()
 * @li PiiOperationCompound::exposeInput()<br>The first parameter to
 * this function can be either an input socket or the name of an input
 * socket.
 * @li PiiOperationCompound::exposeOutput()<br>The first parameter to
 * this function can be either an output socket or the name of an
 * output socket.
 * @li PiiOperationCompound::interrupt()
 * @li PiiOperationCompound::pause()
 * @li PiiOperationCompound::removeOperation()
 * @li PiiOperationCompound::replaceOperation()
 * @li PiiOperationCompound::stop()
 * @li PiiOperationCompound::unexposeAll()
 * @li PiiOperationCompound::unexposeInput()<br>The first parameter to
 * this function can be either an input socket or the name of an input
 * socket.
 * @li PiiOperationCompound::unexposeOutput()<br>The first parameter
 * to this function can be either an output socket or the name of an
 * output socket.
 * @li PiiOperationCompound::wait()<br>This function takes one or two
 * parameters. The first one is the state to be waited for, and the
 * second (optional) one the maximum number of milliseconds to wait.
 *
 * @par PiiEngine
 *
 * PiiEngine has a global constructor function. The members of the
 * PiiEngine::FileFormat enumeration are stored as its read-only
 * properties. Other functions available to scripts are:
 * 
 * @li PiiEngine::execute()
 * @li PiiEngine::load()
 * @li PiiEngine::loadPlugin()
 * @li PiiEngine::loadPlugins()<br>Takes an unlimited number of
 * parameters instead of a list/array. Returns an array of plugin
 * objects (see %PiiEngine::Plugin below).
 * @li PiiEngine::pluginLibraryNames()
 * @li PiiEngine::pluginResourceNames()
 * @li PiiEngine::plugins()
 *
 * @par PiiSocket
 *
 * None of the socket classes have constructor functions because
 * sockets need not to be explicitly created in JavaScript
 * code. PiiSocket is an object in the global scope, and the members
 * of the PiiSocket::Type enumeration are stored as read-only
 * properties. All socket instances also provide the following
 * functions:
 *
 * @li PiiSocket::type()
 *
 * @par PiiInputSocket
 *
 * @li PiiInputSocket::groupId()
 * @li PiiInputSocket::isConnected()
 * @li PiiInputSocket::isOptional()
 *
 * @par PiiOutputSocket
 *
 * @li PiiOutputSocket::groupId()
 * @li PiiOutputSocket::isConnected()
 * @li PiiOutputSocket::connectInput()
 * @li PiiOutputSocket::disconnectInput()
 *
 * @par PiiEngine::Plugin
 *
 * PiiEngine::Plugin has no JavaScript constructor. Whenever plugin
 * instances are returned from functions, they will be converted to
 * JavaScript objects that reflect the following properties of
 * PiiEngine::Plugin:
 *
 * @li PiiEngine::Plugin::resourceName()
 * @li PiiEngine::Plugin::libraryName()
 * @li PiiEngine::Plugin::version()
 *
 * @par PiiYdin
 *
 * PiiYdin is an object in the global scope. It contains the member of
 * the following enumerations as read-only properties:
 *
 * @li @ref PiiYdin::MatrixTypeId
 * @li @ref PiiYdin::ColorTypeId
 * @li @ref PiiYdin::ComplexTypeId
 * @li @ref PiiYdin::QtTypeId
 *
 * @par Qt Types Used as Properties
 *
 * QSize, QSizeF, QRect, and QRectF have a global constructor
 * function, and their properties are mapped to script objects.
 *
 * @code
 * var size = new QSize(100, 100);
 * size.width = 50;
 * size.height = 150;
 *
 * var rect = new QRectF(1.0, 0.0, 1.0, 2.5);
 * rect.x = 3.7;
 * rect.y = 2.8;
 * rect.width = rect.height * 2;
 * @endcode
 *
 * @par Global Scope
 *
 * @li piiDebug(), piiWarning(), piiCritical(), piiFatal()<br>Unlike
 * the printf-like C++ versions, these functions only take one
 * argument that will be written to the log as a character string.
 *
 * @li piiLog()<br>This function takes three arguments: the name of
 * the logging module, log level (0-3), and the message to write to
 * the log.
 *
 * @par Other Extensions to ECMAScript
 *
 * @li Object.prorotype.configure<br>This function takes any number of
 * JavaScript objects as parameters. It copies the properties of each
 * to the @p this object. For example, <tt>object.configure({ one: 1, two:
 * 2})</tt> is equivalent to <tt>object.one = 1; object.two = 2;</tt>.
 *
 *
 * @par Operations Loaded from Plug-ins
 *
 * Unlike C++, JavaScript allows one to use the @p new operator on
 * types that are not visible at compile time. When Into loads a
 * plug-in, all registered operations will be made available as global
 * constructor functions. One can still use
 * PiiOperationCompound::addOperation(), and this technique has the
 * advantage of having the same syntax in both JavaScript and C++.
 *
 * The constructor function created for each registered operation
 * takes a variable number of parameters. If the first parameter is a
 * string, it will be used as the objectName of the new operation. If
 * the parameters are JavaScript objects, their properties are copied
 * to the newly created operation instance. Thus, instead of doing
 * this:
 *
 * @code
 * var reader = engine.addOperation('PiiImageFileReader', 'reader');
 * reader.fileNamePattern = '*.jpg*;
 * reader.imageType = 'GrayScale';
 * @endcode
 *
 * One can also do this:
 *
 * @code
 * var reader = new PiiImageFileReader
 *   ({
 *     objectName: 'reader',
 *     fileNamePattern: '*.jpg',
 *     imageType: 'GrayScale'
 *   });
 *
 * // Template parameters need to be specified like this with the constructor.
 * var som = new PiiSomOperation.double("som", { size: new QSize(20, 20) });
 * // Enum values in namespaces are given as strings
 * som.initMode = 'PiiClassification::SomSampleInit';
 *
 * engine.addOperations(reader, som);
 * @endcode
 *
 *
 * @par Exceptions
 *
 * Whenever a C++ function throws an exception, the exception will be
 * converted to an @p Error object, which is then thrown to the caller
 * in the script code. Consider the following C++ code:
 *
 * @code
 * try
 *   {
 *     PiiEngine engine;
 *     //...
 *     engine.execute();
 *   }
 * catch (PiiExecutionException& ex)
 *   {
 *     //...
 *   }
 * @endcode
 *
 * The corresponding code in JavaScript would look like this:
 *
 * @code
 * try
 *   {
 *     var engine = new PiiEngine();
 *     //...
 *     engine.execute();
 *   }
 * catch (ex)
 *   {
 *     //...
 *   }
 * @endcode
 *
 *
 * @par Data Type Conversions
 *
 * Whenever a function takes as an argument or returns a QList, the
 * C++ object will be converted to a JavaScript array and vice versa. 
 * For example, the PiiEngine::pluginLibraryNames() function, whose
 * C++ version returns a QStringList, will return an @p Array of @p
 * String objects in JavaScript. Some functions, such as
 * PiiEngine::loadPlugins() will not take an @p Array as a parameter. 
 * Since it is possible to pass any number of arguments in JavaScript,
 * the arguments don't need to be bundled into a single data
 * structure.
 *
 * QVariantMap, used as a parameter to PiiEngine::save() and
 * PiiEngine::load(), is directly mapped to JavaScript object. The
 * keys of the map will be converted to property names and the
 * corresponding values to property values.
 *
 * In the C++ API, socket classes are separated to two hierarchies:
 * PiiAbstractSocket provides a QObject-free superclass that can be
 * used with multiple inheritance. All concrete subclasses derive from
 * QObject via PiiSocket. In JavaScript code, sockets are just
 * represented as PiiInputSocket, PiiOutputSocket, or PiiProxySocket
 * objects. They all "derive from" PiiSocket.
 *
 * @par A Simple Example
 *
 * @include threshold.js
 */
