#-------------------------------------------------
#
# Project created by QtCreator 2015-01-17T20:03:52
#
#-------------------------------------------------

include(../defaults.pri)

QT       += core testlib network
QT       -= gui

TARGET = tests

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    parsepackettest.cpp \
    expiringsettest.cpp \
    frigotunneltest.cpp \
    frigoconnectiontest.cpp \
    timeoutgeneratortest.cpp \
    frigoclocktest.cpp

LIBS += -L../lib -lfrigotunnel

HEADERS += \
    parsepackettest.h \
    expiringsettest.h \
    frigotunneltest.h \
    frigoconnectiontest.h \
    timeoutgeneratortest.h \
    frigoclocktest.h

RESOURCES += \
    resources.qrc
