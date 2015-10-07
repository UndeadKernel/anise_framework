QT += core
QT -= gui

TARGET = csvdumpnode
TEMPLATE = lib
CONFIG += plugin
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../../src_framework \
               ../../src_data

CONFIG(debug,debug|release) {
  # Debug...
  DESTDIR = ../../bin/debug/nodes
  OBJECTS_DIR = build/debug
  MOC_DIR = build/debug/moc
  RCC_DIR = build/debug/rcc
} else {
  # Release...
  DESTDIR = ../../bin/release/nodes
  OBJECTS_DIR = build/release
  MOC_DIR = build/release/moc
  RCC_DIR = build/release/rcc
  #DEFINES += QT_NO_DEBUG_OUTPUT
  DEFINES += QT_MESSAGELOGCONTEXT
}

QMAKE_CLEAN += $$DESTDIR/*$$TARGET*

HEADERS += \
    csvdumpnode.h \
    interface.h

SOURCES += \
    csvdumpnode.cpp \
    interface.cpp

