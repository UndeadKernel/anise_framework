QT       += core
QT       -= gui

TARGET = anise.bin
CONFIG += console
CONFIG -= app_bundle
QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app

LIBS += -ldl -rdynamic

#INCLUDEPATH += ../src_data

#FORMS     += forms/mainwindow.ui
#RESOURCES += resources/images.qrc

CONFIG(debug,debug|release) {
  # Debug...
  DESTDIR = ../bin/debug
  OBJECTS_DIR = build/debug
  UI_DIR = build/debug/forms
  UI_HEADERS_DIR = forms
  UI_SOURCES_DIR = forms
  MOC_DIR = build/debug/moc
  RCC_DIR = build/debug/rcc
} else {
  # Release...
  DESTDIR = ../bin/release
  OBJECTS_DIR = build/release
  UI_DIR = build/release/forms
  UI_HEADERS_DIR = forms
  UI_SOURCES_DIR = forms
  MOC_DIR = build/release/moc
  RCC_DIR = build/release/rcc
  #DEFINES += QT_NO_DEBUG_OUTPUT
  #QMAKE_CXXFLAGS_RELEASE += -O2
  # Include the context of the messages in release and debug
  DEFINES += QT_MESSAGELOGCONTEXT
}

# Install the script that sets the correct library path.
anise.path = $$DESTDIR
anise.files = resources/anise.sh

INSTALLS += anise

QMAKE_CLEAN += $$DESTDIR/*$$TARGET*

SOURCES += main.cpp \
    framework.cpp \
    node/node.cpp \
    node/nodeconfig.cpp \
    node/gate.cpp \
    node/nodefactory.cpp \
    dynamicfactory.cpp \
    data/datafactory.cpp \
    data/data.cpp \
    node/nodemesh.cpp \
    ../src_common/qt-json/json.cpp \
    data/errordata.cpp \
    node/nodegatetask.cpp \
    node/nodestarttask.cpp \
    data/messagedata.cpp \
    messagehandler.cpp \
    progressinfo.cpp \
    loginfo.cpp\
    settings.cpp


HEADERS += \
    framework.h \
    node/node.h \
    node/nodeconfig.h \
    node/gate.h \
    node/nodefactory.h \
    dynamicfactory.h \
    data/datafactory.h \
    data/data.h \
    node/nodemesh.h \
    ../src_common/qt-json/json.h \
    data/errordata.h \
    node/nodegatetask.h \
    node/nodestarttask.h \
    data/messagedata.h \
    messagehandler.h \
    progressinfo.h \
    settings.h \
    loginfo.h
