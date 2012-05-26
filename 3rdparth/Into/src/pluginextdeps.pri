# This file resolves all 3rd party extensions modules required by
# plug-ins the current project depends on.
#
# Usage example:
# PLUGINDEPS = Base Image Texture
# include($$INTODIR/pluginextdeps.pri)

isEmpty(PLUGINSPATH) {
  PLUGINSPATH = plugins
}

defineReplace(pluginExtDep) {
  newExtensions =
  for(plugin, ARGS) {
    # If "plugin" is the name of a dependency file, use it.
    # Otherwise build a default file name using the plugin name and PLUGINSPATH.
    exists($$plugin) {
      depFile = $$plugin
    } else {
      depFile = $$PLUGINSPATH/$$lower($$plugin)/dependencies.pri
    }
    #message(Inspecting $$depFile)
    exists($$depFile) {
      exts = $$fromfile($$depFile, EXTENSIONS)
      !isEmpty(exts) {
        for(ext, exts) {
          pluginextpath = $$PLUGINSPATH/$$lower($$plugin)/$$ext
          exists($$pluginextpath): INCLUDEPATH += $$INTODIR/$$pluginextpath
          # Plugin-specific extension config
          exists($$pluginextpath/config.pri): include($$pluginextpath/config.pri)
          # If the extensions isn't already handled, add it to the list
          !isEmpty(ext):!contains(EXTENSIONS, $$ext) {
            #message(Adding ext to list)
            newExtensions += $$ext
          }
        }
      }
    }
  }
  export(INCLUDEPATH)
  return($$unique(newExtensions))
}

!isEmpty(PLUGINDEPS): EXTENSIONS += $$pluginExtDep($$PLUGINDEPS)

include(extensions.pri)
