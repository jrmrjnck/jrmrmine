######################################################################
# Automatically generated by qmake (3.0) Mon Jan 20 23:08:54 2014
######################################################################

TEMPLATE = app
TARGET = miner-cpu
INCLUDEPATH += .
QT += network
CONFIG += c++11

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -funroll-loops

# Input
HEADERS += Sha256.h JsonRpc.h
SOURCES += main.cpp Sha256.cpp JsonRpc.cpp
