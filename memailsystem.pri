QT += core
CONFIG += c++11

INCLUDEPATH += $$PWD

HEADERS += $$PWD/memailsystem.h \
    $$PWD/memailconfig.h

SOURCES += $$PWD/memailsystem.cpp \
    $$PWD/memailconfig.cpp

DEFINES += MEMAILSYSTEM_LIB