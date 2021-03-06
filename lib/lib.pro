#-------------------------------------------------
#
# Project created by QtCreator 2015-01-17T11:44:08
#
#-------------------------------------------------

include(../defaults.pri)

QT       += network script
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
    timeoutgenerator.cpp \
    frigoclock.cpp

HEADERS += \
    frigomessage.h \
    frigopacket.h \
    common.h \
    expiringset.h \
    frigotunnel.h \
    frigoconnection.h \
    timeoutgenerator.h \
    frigoclock.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

!android {
    QT += serialport
}
