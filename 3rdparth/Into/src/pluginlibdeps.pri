# qmake instructions for creating linker parameters for plug-in
# dependencies.
#
# Usage example:
# PLUGINDEPS = Base Image Texture
# include($$INTODIR/pluginlibdeps.pri)

isEmpty(PLUGINSPATH) {
  PLUGINSPATH = plugins
}

# Creates library dependencies
defineReplace(pluginLibDep) {
  plugins = $$ARGS
  libs =
  for(plugin, plugins) {
    dirname = $$lower($$plugin)
    libs += -L$$INTODIR/$$PLUGINSPATH/$$dirname/$$MODE
    libs += -lpii$$lower($$basename(plugin))
  }
  return($$libs)
}

# Creates linker command line parameters that force MSVC to link the
# plug-ins. We need to use the mangled C++ names here.
defineReplace(msvcForceDeps) {
  plugins = $$ARGS
  incs =
  for(plugin, plugins) {
    p = Pii$$basename(plugin)Plugin
    incs += /INCLUDE:??0$$p@@QAE@XZ
  }
  return($$incs)
}

# Get rid of a run-time dependency.
defineTest(removeLib) {
  win32: QMAKE_LFLAGS_RELEASE -= $$msvcForceDeps($$ARGS)
  win32: QMAKE_LFLAGS_DEBUG -= $$msvcForceDeps($$ARGS)
  for(plugin, $$ARGS) {
    libs -= -lpii$$lower($$basename(plugin))
  }
  return(true)
}

!isEmpty(PLUGINDEPS) {
  LIBS += $$pluginLibDep($$PLUGINDEPS)
  #win32-msvc*:QMAKE_LFLAGS_RELEASE += $$msvcForceDeps($$PLUGINDEPS)
  #win32-msvc*:QMAKE_LFLAGS_DEBUG += $$msvcForceDeps($$PLUGINDEPS)
}
