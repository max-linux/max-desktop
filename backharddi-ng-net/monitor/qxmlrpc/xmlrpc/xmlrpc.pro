
TEMPLATE = lib
unix: TARGET = ../qxmlrpc-ng
win32: TARGET = ../qxmlrpc-ng

CONFIG += warn_on
CONFIG += staticlib
CONFIG += release
CONFIG += precompile_header \
dll
DEPENDPATH += .
QT += xml
QT += network 

HEADERS += stable.h
PRECOMPILED_HEADER = stable.h
# Input
HEADERS += \
    client.h \
    server.h \
    server_private.h \
    serverintrospection.h \
    request.h \
    response.h \
    variant.h \

SOURCES += \
    client.cpp \
    server.cpp \
    serverintrospection.cpp \
    request.cpp \
    response.cpp \
    variant.cpp \

!debug_and_release|build_pass{
    CONFIG(debug, debug|release){
        TARGET = $$member(TARGET, 0)_debug
    }
}

win32{
    # no -O2 allowed
    QMAKE_CFLAGS_RELEASE = -MD
    QMAKE_CXXFLAGS_RELEASE = -MD
}

# universal binaries for release
# build from command line
macx-g++{
    !debug_and_release|build_pass{
        CONFIG(release, debug|release){
            CONFIG += x86 ppc
            CONFIG -= precompile_header
            QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
        }
    }
}

INCLUDEPATH += . \
..
