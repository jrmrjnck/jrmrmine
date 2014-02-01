TEMPLATE = app
TARGET = jrmrmine
INCLUDEPATH += .
CONFIG -= qt
CONFIG += c++11

LIBS += -lcurl -ljsoncpp

QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -funroll-loops 

# Input
HEADERS += Sha256.h JsonRpc.h
SOURCES += main.cpp Sha256.cpp JsonRpc.cpp
