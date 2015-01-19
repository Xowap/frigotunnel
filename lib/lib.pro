#-------------------------------------------------
#
# Project created by QtCreator 2015-01-17T11:44:08
#
#-------------------------------------------------

include(../defaults.pri)

QT       += network

QT       -= gui

TARGET = frigotunnel
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    frigomessage.cpp \
    frigopacket.cpp \
    expiringset.cpp \
    frigotunnel.cpp \
    frigoconnection.cpp \
    timeoutgenerator.cpp

HEADERS += \
    frigomessage.h \
    frigopacket.h \
    common.h \
    expiringset.h \
    frigotunnel.h \
    frigoconnection.h \
    timeoutgenerator.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
