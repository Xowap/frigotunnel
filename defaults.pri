INCLUDEPATH += $$PWD/lib
SRC_DIR = $$PWD
CONFIG += c++11

!android {
    QT += serialport
}