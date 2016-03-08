TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp
HEADERS += vc.h

LIBS += -L$$_PRO_FILE_PWD_ -l:libvulkan.so.1
