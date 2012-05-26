PLUGIN = Camera
include(../piiplugin.pri)

QT              += network

# FireWire driver for Linux
unix {
  !contains(DISABLE,firewire) {
    HEADERS       += firewire/*.h
    SOURCES       += firewire/*.cc
    SOURCES       += firewire/unix/*.cc
    INCLUDEPATH   += firewire
  } else {
    DEFINES       += PII_NO_FIREWIRE
  }
}

# FireWire driver for Windows
win32-msvc.net {
  !contains(DISABLE,firewire):exists(../../3rdparty/cmu1394/include) {
    INCLUDEPATH += ../histogram ../../3rdParty/cmu1394/include
    LIBS        += -L../../3rdParty/cmu1394/lib -l1394Camera
    HEADERS     += firewire/*.h
    SOURCES     += firewire/*.cc
    SOURCES     += firewire/windows/*.cc
    INCLUDEPATH += firewire
  } else {
    DEFINES     += PII_NO_FIREWIRE
  }
} else:win32 {
  DEFINES       += PII_NO_FIREWIRE
}

contains(DEFINES,PII_NO_FIREWIRE) {
  warning (No compatible FireWire driver found. FireWire cameras disabled.)
}

HEADERS       += protocols/*.h genicam/*.h
SOURCES       += protocols/*.cc genicam/*.cc  
