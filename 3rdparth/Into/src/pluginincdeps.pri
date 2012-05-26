# qmake instructions for creating compiler parameters for plug-in
# dependencies. This is required for compile-time dependencies.
#
# Usage example:
# PLUGINDEPS = Base Image Texture application/SmartClassifier
# include($$INTODIR/pluginincdeps.pri)

isEmpty(PLUGINSPATH) {
  PLUGINSPATH = plugins
}

defineReplace(pluginIncDep) {
  plugins = $$ARGS
  incpath =
  for(plugin, plugins) {
    incpath += $$INTODIR/$$PLUGINSPATH/$$lower($$plugin)
  }
  return($$incpath)
}

!isEmpty ( PLUGINDEPS ) {
  INCLUDEPATH += $$pluginIncDep($$PLUGINDEPS)
}
