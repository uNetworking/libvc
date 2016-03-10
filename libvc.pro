TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.cpp
HEADERS += include/vc.h \
    include/buffer.h \
    include/error.h \
    include/pipeline.h \
    include/commandbuffer.h

INCLUDEPATH += include
LIBS += -L$$_PRO_FILE_PWD_/lib -l:libvulkan.so.1
