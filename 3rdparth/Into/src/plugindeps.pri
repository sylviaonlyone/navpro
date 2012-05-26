# Recursively finds dependencies between plug-ins. This file is
# automatically included from piiplugin.pri, but can also be used
# directly in projects that depend on plug-ins.

!isEmpty(APPLICATIONPATH) {
  PLUGIN = $$APPLICATIONPATH/dependencies.pri
} else:isEmpty(PLUGIN) {
  PLUGIN = $$OUT_PWD/dependencies.pri
}

isEmpty(PLUGINSPATH) {
  PLUGINSPATH = plugins
}

# qmake programmers apparently don't know what a stack is for.
# Recursive calls crash the program. This function works iteratively,
# one dependency level at a time. (breadth-first search)

defineReplace(resolveDependencies) {
  newDeps =
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
      deps = $$fromfile($$depFile, DEPENDENCIES)
      !isEmpty(deps) {
        for(dep, deps) {
          #message($$plugin depends on $$dep)
          # See if the dependency is conditional
          parts = $$split(dep,":")
          # There are conditions
          !count(parts,1) {
            extensions = $$member(parts, 0, -2)
            dep = $$last(parts)
            for(ext, extensions) {
              !enabled($$ext) {
                dep =
                break()
              }
            }
          }
          # If the dependency isn't already handled, add it to the list
          !isEmpty(dep):!contains(PLUGINDEPS, $$dep) {
            #message(Adding $$dep to list)
            newDeps += $$dep
          }
        }
      }
    }
  }
  return($$unique(newDeps))
}

PLUGINDEPS = $$resolveDependencies($$PLUGIN)
!isEmpty(PLUGINDEPS) {
  newDeps = $$PLUGINDEPS
  # There's no while loop in qmake. Hope 10 levels is enough.
  levels = 0 1 2 3 4 5 6 7 8 9
  for(level, levels) {
    newDeps = $$resolveDependencies($$newDeps)
    isEmpty(newDeps) {
      break()
    }
    PLUGINDEPS += $$newDeps
  }
}
#message(Final deps: $$PLUGINDEPS)
# Remove myself from dependencies (dependency loop)
PLUGINDEPS -= $$PLUGIN
include($$PWD/pluginincdeps.pri)
include($$PWD/pluginlibdeps.pri)
PLUGINDEPS += $$PLUGIN
include($$PWD/pluginextdeps.pri)
