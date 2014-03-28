TEMPLATE = app
TARGET = jrmrmine
INCLUDEPATH += .
CONFIG -= qt
CONFIG += c++11
CONFIG += debug

LIBS += -lcurl -ljsoncpp -lboost_regex

QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -funroll-loops 

# Input
HEADERS += Sha256.h JsonRpc.h Settings.h
SOURCES += main.cpp Sha256.cpp JsonRpc.cpp Settings.cpp
