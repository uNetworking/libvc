TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.cpp \
    src/commandbuffer.cpp \
    src/device.cpp \
    src/buffer.cpp \
    src/arguments.cpp \
    src/program.cpp \
    src/devicepool.cpp
HEADERS += include/vc.h \
    include/buffer.h \
    include/commandbuffer.h \
    include/devicepool.h \
    include/device.h \
    include/constants.h \
    include/program.h \
    include/arguments.h

INCLUDEPATH += include
LIBS += -L$$_PRO_FILE_PWD_/lib -l:libvulkan.so.1
QMAKE_CXXFLAGS += -Wno-missing-field-initializers
