#
# QGroundControl Library
#

QGC_DIR = $$PWD/../qgc_gui

#CONFIG += $$fromfile($$QGC_DIR/qgc_gui.pro, CONFIG)

BASETRG = $$TARGET
BASETPL = $$TEMPLATE
BASESRC = $$SOURCES
BASEHDR = $$HEADERS
BASEFRM = $$FORMS
BASERES = $$RESOURCES
include($$QGC_DIR/qgc_gui.pro)
TARGET = $$BASETRG
TEMPLATE = $$BASETPL
SOURCES = $$BASESRC
HEADERS = $$BASEHDR
FORMS = $$BASEFRM
RESOURCES = $$BASERES

LIST = $$INCLUDEPATH
for(ITEM, LIST) : INCLUDEPATH += $$QGC_DIR/$$ITEM

DebugBuild {
    QGC_LIB = $$OUT_PWD/../qgc_gui/debug
} else {
    QGC_LIB = $$OUT_PWD/../qgc_gui/release
}

LIBS += -lQGroundControl \
    -L$$QGC_LIB \
    -Wl,-rpath,$$QGC_LIB \

DEPENDPATH += $$QGC_LIB \
