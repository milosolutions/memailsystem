QT += core network
CONFIG += c++11

MCDB = ..
!include($$MCDB/mconfig/mconfig.pri): error("MEmailSystem requires MConfig module")

INCLUDEPATH += $$PWD
HEADERS += $$PWD/memailsystem.h $$PWD/emailconfig.h
SOURCES += $$PWD/memailsystem.cpp

DEFINES += MEMAILSYSTEM_LIB
