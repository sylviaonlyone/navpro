# Usage:
#
# INTODIR = path/to/into
# EXTENSIONS = opencv libsvm
# include($$INTODIR/extensions.pri)

defineReplace(intoDir) {
  # There is no isAbsolute(path) function...
  exists(/$$INTODIR/into.pro) {
    return($$INTODIR)
  } else {
    return($$OUT_PWD/$$INTODIR)
  }
}

defineTest(enabled) {
  contains(DISABLE,$$1):return(false)
  path = $$intoDir()/3rdparty/$$1
  exists($$path/include):return(true)
  exists($$path/*.h):return(true)
  exists($$path/config.pri) {
    enabled = $$fromfile($$path/config.pri, EXT_ENABLED)
    !isEmpty(enabled):return(true)
  }
  return(false)
}

for(ext, EXTENSIONS) {
  enabled($$ext) {
    path = $$intoDir()/3rdparty/$$ext
    relpath = 3rdparty/$$ext
    exists($$path/include):INCLUDEPATH += $$INTODIR/$$relpath/include
    exists($$path/*.h):INCLUDEPATH += $$INTODIR/$$relpath
    exists($$path/lib):LIBS += -L$$INTODIR/$$relpath/lib
    exists($$path/config.pri):include($$PWD/$$relpath/config.pri)
    exists($$OUT_PWD/$$ext/config.pri) {
      include($$OUT_PWD/$$ext/config.pri)
    } else {
      exists($$OUT_PWD/$$ext/*.h): HEADERS += $$ext/*.h
      exists($$OUT_PWD/$$ext/*.cc): SOURCES += $$ext/*.cc
    }
  } else {
    warning($$ext disabled)
    DEFINES += PII_NO_$$upper($$ext)
  }
}
