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

#ifndef _PIIENGINE_H
#define _PIIENGINE_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QMutex>
#include <PiiVersionNumber.h>
#include "PiiLoadException.h"
#include "PiiOperationCompound.h"

class QLibrary;

/**
 * An execution engine. The task of %PiiEngine is to handle the
 * loading and unloading of plug-in modules. It inherits from
 * PiiOperationCompound and can thus be used as an executor for a set
 * of interconnected operations. %PiiEngine provides a convenience
 * function #execute() to check the configuration and to start
 * execution.
 *
 * A typical, simple usage scenario for the engine is as follows:
 *
 * @code
 * //1. create a PiiEngine instance
 * PiiEngine engine;
 *
 * //2. load the necessary plug-ins
 * engine.loadPlugin("piimage");
 *
 * //3. create operations
 * PiiOperation* reader = engine.createOperation("PiiImageFileReader");
 * PiiOperation* writer = engine.createOperation("PiiImageFileWriter");
 *
 * //4. configure them
 * reader->setProperty("fileNamePattern", "*.bmp");
 * writer->setProperty("outputDirectory", ".");
 * writer->setProperty("extension", "jpg");
 *
 * //5. connect them
 * reader->connectOutput("image", writer, "image");
 *
 * //6. monitor for run-time errors
 * connect(engine, SIGNAL(errorOccured(PiiOperation*,const QString&)), myMonitor, SLOT(handleError(PiiOperation*,const QString&)));
 *
 * //7. start the engine
 * try
 *   {
 *     engine.execute();
 *   }
 * catch (PiiExecutionException& ex)
 *   {
 *     //handle possible start-up errors here
 *   }
 * @endcode
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiEngine : public PiiOperationCompound
{
  Q_OBJECT

  Q_ENUMS(FileFormat);

  friend struct PiiSerialization::Accessor;
  PII_SEPARATE_SAVE_LOAD_MEMBERS
  PII_DECLARE_SAVE_LOAD_MEMBERS
  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION;

public:
  /**
   * File formats.
   *
   * @lip TextFormat - data is saved as UTF-8 text. See
   * PiiTextOutputArchive and PiiTextInputArchive.
   *
   * @lip BinaryFormat - data is saved in a raw binary format. See
   * PiiBinaryOutputArchive and PiiBinaryInputArchive.
   */
  enum FileFormat { TextFormat, BinaryFormat };

  class Plugin;
  
  /// Constructs a new PiiEngine.
  PiiEngine();

  /// Destroys the engine.
  ~PiiEngine();
  
  /**
   * Loads a plug-in into the engine. The name of the plug-in is the
   * name of the plug-in library file without a file name extension. 
   * For example, to load the flow control plug-in
   * ({libpiiflowcontrol.so, piiflowcontrol.dll}), do this:
   *
   * @code
   * PiiEngine::loadPlugin("piiflowcontrol");
   * @endcode
   *
   * This will load the plug-in in the default plug-in location. In
   * Unix, the plug-in library file @p libpiiflowcontrol.so will be
   * searched in @p LD_LIBRARY_PATH. In Windows, @p piiflowcontrol.dll
   * will be searched in @p PATH. If the plug-in is located in another
   * directory, either relative or absolute path names can be used. 
   * Use slash as the path separator (backslash will also work on
   * Windows). Note that in this case you need to use the full file
   * name (preferably without the extension, though).
   *
   * @code
   * PiiEngine::loadPlugin("relative/path/to/libmyplugin");
   * PiiEngine::loadPlugin("/absolute/path/to/libmyotherplugin");
   * @endcode
   *
   * Plug-ins are always in process-wide use. It is not possible to
   * load a plug-in to a single PiiEngine instance. Each plug-in is
   * identified by its base name (see QFileInfo::baseName()). Thus,
   * avoid using similar names even in separate directories.
   *
   * Successive calls to loadPlugin() with the same plug-in name are
   * OK. To really unload the plug-in one needs to issue the same
   * number of #unloadPlugin() calls.
   *
   * This function is thread-safe.
   *
   * @return Basic information about the loaded plug-in.
   *
   * @exception PiiLoadException& if the plug-in cannot be loaded
   */
  static Plugin loadPlugin(const QString& name);

  /**
   * A convenience function that loads many plugins at once.
   *
   * @code
   * PiiEngine::loadPlugins(QStringList() << "piiimage" << "piibase");
   * @endcode
   *
   * @exception PiiLoadException& if any of the plug-ins cannot be
   * loaded
   *
   * @see ensurePlugins
   */
  static void loadPlugins(const QStringList& plugins);

  /**
   * Ensures that @a plugin is loaded. This function tries to load the
   * plug-in is it is not yet loaded. Unlike #loadPlugin(), this
   * function doesn't increase the reference count of plug-ins that
   * are already loaded.
   *
   * @exception PiiLoadException& if the plug-in cannot be loaded
   */
  static void ensurePlugin(const QString& plugin);

  /**
   * Ensures that all plug-ins listed in @a plugins are loaded. This
   * function tries to load all plug-ins that are not yet loaded.
   * Unlike #loadPlugins(), this function doesn't increase the
   * reference count of plug-ins that are already loaded.
   *
   * @exception PiiLoadException& if any of the plug-ins cannot be
   * loaded
   */
  static void ensurePlugins(const QStringList& plugins);

  /**
   * Remove the named plugin. Either the full path or the base name
   * will do as @a name.
   *
   * @param force if @p false, the plug-in will not be removed from
   * the address space of the process until all #loadPlugin() calls
   * have been abrogated. If @p true, a single call removes the
   * plug-in irrespective of the number of references.
   *
   * @return the number of references left
   *
   * @b WARNING! Unloading plug-ins needs special attention. Make
   * extremely sure that no instances of classes created by the plugin
   * are in memory! Otherwise, all bets are off. If you have created
   * an operation with PiiOperationCompound::createOperation(), detach
   * and delete the operation from the engine before trying to unload
   * the plugin.
   *
   * This function is thread-safe.
   */
  static int unloadPlugin(const QString& name, bool force = false);

  /**
   * Returns @p true if the the plugin called @p name is loaded and @p
   * false otherwise.
   */
  static bool isLoaded(const QString& name);

  /**
   * Returns loaded plug-ins.
   */
  static QList<Plugin> plugins();

  /**
   * Returns the library names of loaded plug-ins.
   */
  static QStringList pluginLibraryNames();
  
  /**
   * Returns the resource names of loaded plug-ins.
   */
  static QStringList pluginResourceNames();

  /**
   * Checks and executes all child operations. This function first
   * calls @ref PiiOperation::check() for all child operations. If
   * none of them throws an exception, @ref PiiOperation::start() will
   * be called. This is a convenience function that saves one from
   * manually performing the sanity check. If the engine is neither @p
   * Stopped nor @p Paused, this function does nothing.
   */
  void execute();

  /**
   * Creates a deep copy of the engine.
   */
  Q_INVOKABLE PiiEngine* clone() const;

  /**
   * Saves the engine to @a fileName. The @a format argument specifies
   * the file format. The @a config map is used to add configuration
   * information to the file. The following keys are recognized:
   *
   * @lip plugins - the names of plug-ins that need to be loaded to be
   * able to run the engine. If the @a config map doesn't contain the
   * @p plugins key, #pluginLibraryNames() will be used.
   *
   * @lip application - the name of the application that created the
   * engine. If this value is not given, "Into" will be used.
   *
   * @lip version - the version of the application that created the
   * engine. If @p application is not given, the current Into version
   * will be used.
   *
   * One may store any application-specific configuration values to
   * the map.
   *
   * @exception PiiException& if @a fileName cannot be opened for
   * writing
   *
   * @exception PiiSerializationException& if the serialization of the
   * engine fails for any reason.
   *
   * @code
   * try
   *   {
   *     PiiEngine::loadPlugin("piibase");
   *     PiiEngine engine;
   *     engine.addOperation("PiiObjectCounter", "counter");
   *     engine.save("counter_engine.cft");
   *   }
   * catch (PiiException& ex)
   *   {
   *     // handle errors here
   *   }
   * @endcode
   */
  void save(const QString& fileName,
            const QVariantMap& config = QVariantMap(),
            FileFormat format = TextFormat) const;

  /**
   * Loads a stored engine from @a fileName. The stored configuration
   * values will be written to @a config. Archive file format will be
   * automatically recognized.
   *
   * @exception PiiException& if @a fileName cannot be opened for
   * reading
   *
   * @exception PiiLoadException& if any of the required plug-ins
   * cannot be loaded.
   *
   * @exception PiiSerializationException& if the archive type cannot
   * be recognized or an error occurs when reading the engine
   * instance.
   *
   * @code
   * PiiVariantMap mapConfig;
   * PiiEngine* pEngine = PiiEngine::load("counter_engine.cft", &mapConfig);
   * QCOMPARE(mapConfig["application"].toString(), QString("Into"));
   * @endcode
   */
  static PiiEngine* load(const QString& fileName,
                         QVariantMap* config = 0);

private:
  typedef QHash<QString,Plugin> PluginMap;

  static PluginMap _pluginMap;
  static QMutex _pluginLock;
};

/**
 * A class that stores information about a loaded plug-in. Each
 * plug-in has two names: the name of the shared library the plug-in
 * was loaded from, and the name of the plug-in in Ydin's @ref
 * PiiYdin::resourceDatabase() "resource database".
 */
class PII_YDIN_EXPORT PiiEngine::Plugin
{
public:
  Plugin();
  Plugin(const Plugin& other);
  ~Plugin();

  Plugin& operator= (const Plugin& other);

  /**
   * Get the resource name of the plug-in. Note that this is not the
   * name of the shared library but the resource ID of the plug-in in
   * Ydin's resource database. (See PiiYdin::resourceDatabase())
   */
  QString resourceName() const;

  /**
   * Get the library name of the plug-in. This function returns the
   * name of the plug-in as passed to the PiiEngine::loadPlugin()
   * function.
   */
  QString libraryName() const;

  /**
   * Get the version of Into the plug-in was originally compiled and
   * linked against.
   */
  PiiVersionNumber version() const;

private:
  friend class PiiEngine;
  
  Plugin(QLibrary* library, const QString& name, const PiiVersionNumber& version);

  class Data;
  Data* d;
};

Q_DECLARE_METATYPE(PiiEngine*);

#include "PiiEngine-templates.h"

#define PII_SERIALIZABLE_CLASS PiiEngine
#define PII_SERIALIZABLE_CLASS_VERSION 2
#define PII_SERIALIZABLE_SHARED
#define PII_BUILDING_LIBRARY PII_BUILDING_YDIN

#include <PiiSerializableRegistration.h>

#endif //_PIIENGINE_H
