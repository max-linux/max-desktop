TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += . ../../

CONFIG += warn_on
CONFIG += debug

QT += xml network


# Input
HEADERS += testclient.h
FORMS += testclient.ui
SOURCES += main.cpp testclient.cpp

unix{
    CONFIG(debug, debug|release){
        LIBS += ../../libqxmlrpc_debug.so
    }    else{
        LIBS += ../../libqxmlrpc.so
    }
}

win32: LIBS += ../../xmlrpc/qxmlrpc.lib

