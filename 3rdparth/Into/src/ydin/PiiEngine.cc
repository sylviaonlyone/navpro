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

#include <PiiSerializationUtil.h>
#include "PiiEngine.h"
#include <PiiSerializableExport.h>
#include <PiiUtil.h>
#include "PiiPlugin.h"
#include <PiiGenericTextOutputArchive.h>
#include <PiiGenericBinaryOutputArchive.h>
#include <PiiGenericTextInputArchive.h>
#include <PiiGenericBinaryInputArchive.h>

#include <QLibrary>

PII_DEFINE_VIRTUAL_METAOBJECT_FUNCTION(PiiEngine)
PII_SERIALIZABLE_EXPORT(PiiEngine);

static int iEngineMetaType = qRegisterMetaType<PiiEngine*>("PiiEngine*");

PiiEngine::PluginMap PiiEngine::_pluginMap;
QMutex PiiEngine::_pluginLock;

class PiiEngine::Plugin::Data
{
public:
  Data();
  Data(QLibrary* lib, const QString& name, const PiiVersionNumber& version, int iRefCount = 1);
  Data(const Data& other);
  
  QLibrary* pLibrary;
  QString strResourceName, strLibraryName;
  PiiVersionNumber version;
  int iRefCount;
} *d;


PiiEngine::PiiEngine()
{
#ifdef __MINGW32__
  /* HACK
   * Mingw exception handling is not thread safe. The first exception
   * thrown makes one-time initialization that occassionally crashes
   * the program, if many exceptions are thrown simultaneously from
   * different threads.
   */
  try { throw PiiException(); } catch (...) {}
#endif
}

PiiEngine::~PiiEngine()
{ }

void PiiEngine::execute()
{
  PII_D;
  QMutexLocker lock(&d->stateMutex);
  State s = state();
  if (s == Stopped || s == Paused)
    {
      // Reset children if we were stopped
      check(s == Stopped);
      setState(Starting);
      commandChildren<Start>();
    }
}

void PiiEngine::loadPlugins(const QStringList& plugins)
{
  for (int i=0; i<plugins.size(); ++i)
    loadPlugin(plugins[i]);
}

PiiEngine::Plugin PiiEngine::loadPlugin(const QString& name)
{
  QMutexLocker lock(&_pluginLock);

  if (_pluginMap.contains(name))
    {
      // The plug-in is already loaded. Just increase reference count.
      ++_pluginMap[name].d->iRefCount;
      return _pluginMap[name];
    }
  
  // Exception safety
  struct Unloader
  {
    Unloader(QLibrary* lib) : pLib(lib) {}
    ~Unloader()
    {
      if (pLib)
        {
          pLib->unload();
          delete pLib;
        }
    }
    QLibrary* release()
    {
      QLibrary* pTmp = pLib;
      pLib = 0;
      return pTmp;
    }

    QLibrary* pLib;
  };
  
  // Load library
  Unloader unloader(new QLibrary(name));
  
  if (!unloader.pLib->load())
    PII_THROW(PiiLoadException, tr("Cannot load the shared library \"%1\".\n"
                                   "Error message: %2").arg(name).arg(unloader.pLib->errorString()));
  
  // Resolve plug-in information functions
  pii_plugin_function pNameFunc = (pii_plugin_function)(unloader.pLib->resolve(PII_PLUGIN_NAME_FUNCTION_STR));
  if (pNameFunc == 0)
    PII_THROW(PiiLoadException, tr("The shared library \"%1\" does not contain a valid plug-in. Missing plug-in name function.").arg(name));

  pii_plugin_function pVersionFunc = (pii_plugin_function)(unloader.pLib->resolve(PII_PLUGIN_VERSION_FUNCTION_STR));
  if (pNameFunc == 0)
    PII_THROW(PiiLoadException, tr("The shared library \"%1\" does not contain a valid plug-in. Missing plug-in version function.").arg(name));

  PiiVersionNumber pluginVersion((*pVersionFunc)());
  PiiVersionNumber intoVersion(INTO_VERSION_STR);

  if (intoVersion < pluginVersion)
    PII_THROW(PiiLoadException, tr("The plug-in \"%1\" is compiled against a newer version of Into (%2, current is %3).")
              .arg(name)
              .arg(pluginVersion.toString())
              .arg(INTO_VERSION_STR));

  if (pluginVersion.part(0) < intoVersion.part(0))
    PII_THROW(PiiLoadException, tr("The plug-in is not binary compatible with your version of Into."));

  Plugin plugin(unloader.release(), (*pNameFunc)(), pluginVersion);
  _pluginMap.insert(name, plugin);

  return plugin;
}

int PiiEngine::unloadPlugin(const QString& name, bool force)
{
  QMutexLocker lock(&_pluginLock);

  // Cannot unload a non-loaded plug-in.
  // Still some references left...
  if (!_pluginMap.contains(name))
    return 0;
  if (!force &&
      --_pluginMap[name].d->iRefCount > 0)
    return _pluginMap[name].d->iRefCount;

  // Remove the plug-in from our map.
  Plugin plugin = _pluginMap.take(name);
  plugin.d->pLibrary->unload();
  delete plugin.d->pLibrary;
  return 0;
}

bool PiiEngine::isLoaded(const QString& name)
{
  QMutexLocker lock(&_pluginLock);
  return _pluginMap.contains(QFileInfo(name).baseName());
}

QList<PiiEngine::Plugin> PiiEngine::plugins()
{
  QMutexLocker lock(&_pluginLock);
  QList<Plugin> lstPlugins;

  for (PluginMap::const_iterator i = _pluginMap.constBegin(); i != _pluginMap.constEnd(); ++i)
    lstPlugins << i.value();

  return lstPlugins;
}

QStringList PiiEngine::pluginLibraryNames()
{
  QMutexLocker lock(&_pluginLock);
  return _pluginMap.keys();
}

QStringList PiiEngine::pluginResourceNames()
{
  QMutexLocker lock(&_pluginLock);
  QStringList lstResult;
  for (PluginMap::const_iterator i = _pluginMap.constBegin(); i != _pluginMap.constEnd(); ++i)
    lstResult << i.value().resourceName();
  return lstResult;
}

PiiEngine* PiiEngine::clone() const
{
  PiiEngine *pResult = static_cast<PiiEngine*>(PiiOperationCompound::clone());

  // Set properties if not subclassed
  if (pResult != 0 && PiiEngine::metaObject() == metaObject())
    Pii::setPropertyList(*pResult, Pii::propertyList(*this));

  return pResult;
}

void PiiEngine::save(const QString& fileName,
                     const QVariantMap& config,
                     FileFormat format) const
{
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
    PII_THROW(PiiException, tr("Cannot open %1 for writing.").arg(fileName));

  QVariantMap mapConfig(config);

  if (!mapConfig.contains("application"))
    {
      mapConfig["application"] = "Into";
      if (!mapConfig.contains("version"))
        mapConfig["version"] = INTO_VERSION_STR;
    }
  if (!mapConfig.contains("plugins"))
    mapConfig["plugins"] = pluginLibraryNames();

  if (format == TextFormat)
    {
      PiiGenericTextOutputArchive oa(&file);
      oa << PII_NVP("config", mapConfig);
      oa << PII_NVP("engine", this);
    }
  else
    {
      PiiGenericBinaryOutputArchive oa(&file);
      oa << PII_NVP("config", mapConfig);
      oa << PII_NVP("engine", this);
    }
}

void PiiEngine::ensurePlugin(const QString& plugin)
{
  if (!isLoaded(plugin))
    PiiEngine::loadPlugin(plugin);
}

void PiiEngine::ensurePlugins(const QStringList& plugins)
{
  foreach (QString strPlugin, plugins)
    if (!isLoaded(strPlugin))
      PiiEngine::loadPlugin(strPlugin);
}

PiiEngine* PiiEngine::load(const QString& fileName,
                           QVariantMap* config)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
    PII_THROW(PiiException, tr("Cannot open %1 for reading.").arg(fileName));
  PiiEngine* pEngine = 0;
  QVariantMap mapConfig;
  if (file.peek(PII_TEXT_ARCHIVE_ID_LEN) == PII_TEXT_ARCHIVE_ID)
    {
      PiiGenericTextInputArchive ia(&file);
      ia >> PII_NVP("config", mapConfig);
      ensurePlugins(mapConfig["plugins"].toStringList());
      ia >> PII_NVP("engine", pEngine);
    }
  else if (file.peek(PII_BINARY_ARCHIVE_ID_LEN) == PII_BINARY_ARCHIVE_ID)
    {
      PiiGenericBinaryInputArchive ia(&file);
      ia >> PII_NVP("config", mapConfig);
      ensurePlugins(mapConfig["plugins"].toStringList());
      ia >> PII_NVP("engine", pEngine);
    }
  else
    PII_SERIALIZATION_ERROR(UnrecognizedArchiveFormat);

  if (config != 0)
    *config = mapConfig;
  
  return pEngine;
}


PiiEngine::Plugin::Data::Data() : pLibrary(0), iRefCount(0)
{
}

PiiEngine::Plugin::Data::Data(QLibrary* lib, const QString& name, const PiiVersionNumber& v, int refCount) :
  pLibrary(lib), strResourceName(name), strLibraryName(lib->objectName()), version(v), iRefCount(refCount)
{
}

PiiEngine::Plugin::Data::Data(const Data& other) :
  pLibrary(other.pLibrary),
  strResourceName(other.strResourceName),
  strLibraryName(other.strLibraryName),
  version(other.version),
  iRefCount(other.iRefCount)
{
}

PiiEngine::Plugin::Plugin() : d(new Data)
{
}

PiiEngine::Plugin::Plugin(QLibrary* lib, const QString& name, const PiiVersionNumber& version) :
  d(new Data(lib, name, version))
{
}

PiiEngine::Plugin::Plugin(const Plugin& other) :
  d(new Data(*other.d))
{
}

PiiEngine::Plugin& PiiEngine::Plugin::operator= (const PiiEngine::Plugin& other)
{
  if (d != other.d)
    {
      d->pLibrary = other.d->pLibrary;
      d->strResourceName = other.d->strResourceName;
      d->strLibraryName = other.d->strLibraryName;
      d->version = other.d->version;
      d->iRefCount = other.d->iRefCount;
    }
  return *this;
}

PiiEngine::Plugin::~Plugin()
{
  delete d;
}

QString PiiEngine::Plugin::resourceName() const
{
  return d->strResourceName;
}

QString PiiEngine::Plugin::libraryName() const
{
  return d->strLibraryName;
}

PiiVersionNumber PiiEngine::Plugin::version() const
{
  return d->version;
}
