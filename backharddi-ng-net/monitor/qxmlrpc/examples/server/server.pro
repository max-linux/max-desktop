TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += . ../../

win32: CONFIG += console

CONFIG += warn_on
CONFIG += debug


QT += xml network

# Input
HEADERS += testserver.h
SOURCES += main.cpp testserver.cpp

win32: LIBS += ../../xmlrpc/qxmlrpc.lib


unix{
    CONFIG(debug, debug|release){
        LIBS += ../../libqxmlrpc_debug.a
    }    else{
        LIBS += ../../libqxmlrpc.a
    }
}


