PLUGIN = ClassificationGui
INTODIR = ../../..
include(../../piiplugin.pri)

FORMS += ui/*.ui
RESOURCES += ui/general.qrc

INCLUDEPATH += . $$INTODIR/gui
LIBS += -L$$INTODIR/gui/$$MODE -lpiigui$$LIBVER
